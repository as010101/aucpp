#include "XSectionQBox.H"
#include "tupleGenerator.H"

XSectionQBox::XSectionQBox()
{
}

XSectionQBox::~XSectionQBox()
{
}

void XSectionQBox::setState(AggregateState *agg)
{
  AggregateFunction *af = agg->af;
  const char *group_by_atts = agg->group_by;
  output_arg output = agg->output;
  Timestamp unique_timeout = agg->unique_time;
  until_arg unless = agg->until;

  // initialize most of the variables
  _maxtime = 0;
  _output = output;
  _timeout = unless;
  _unique_timeout = unique_timeout.tv_sec;
  _af = af;
  _new_tuple_size = 0;
  _curr_pos = 0; 

  // count the number of attributes in the group_by_atts, 
  // the attributes are delimited by ','
  int y = 0;
  int count = 0;
  _num_atts = 0;
  while (group_by_atts[y] != '\0') 
    { 
      if (group_by_atts[y] == ',')
	count++;
      y++;
    }
  _num_atts = count + 1;   // +1 for the attribute after the last ','

  _fields = parseAtts(group_by_atts, _num_atts);
  _group_by_size = 0;
  for (int i = 0; i < _num_atts; i++)
    _group_by_size += _fields[i]->getReturnedSize();

  _new_tuple_size = _group_by_size;
  _group_by_values = new char[_group_by_size];

  // add the size of sid and output value
  //  _sid_size = sizeof(int);  // sid is int right now 
  _new_tuple_size += getSidSize() + _af->getReturnedSize(); // size of output

  // add the size of timestamp --- which is an int
  int ts_size = getTsSize();
  _new_tuple_size += getTsSize();

  // the key for hashing will be a concatenation of group_by_values
  _sid_size = getSidSize();
  _key_size = _group_by_size;
  _hidden_size = ts_size + _sid_size; 
  _tuple_size = _tuple_descr->getSize() + _hidden_size;
  if (agg->output == WHENEVER)
    setWheneverPredicate(agg->whenever_pred);
  if (agg->until == SATISFIES)
    setSatisfiesPredicate(agg->satisfies_pred);
  if (agg->until == TIMEOUT)
    setTimeoutS(agg->unless_timeout);
  _hash = agg->reg_hash;
  _trash_hash = agg->trash;
  _group_hash = agg->group_hash;
  _hash->setKeySize(_key_size);
  _trash_hash->setKeySize(_key_size);
}

Box_Out_T XSectionQBox::doBox() 
{  
  //  time_t curr_time;
  int index;
  int temp_size;
  return_val.output_tuples = 0;
  _num_of_output_tuples = 0;
  _output_tuple = _outStream[0];

  for (int ii = 0; ii < _train_size[0]; ii++)
    {
      _curr_tuple = new char[_tuple_size];
      memcpy(_curr_tuple, _inStream[0] + (ii * _tuple_size), _tuple_size);
      // extract the ts from the tuple in _curr_tuple
      Timestamp tuple_ts = getTs(_curr_tuple); 

      // get the current time from the system
      //      time(&curr_time);
      //      _curr_ts = curr_time;
      gettimeofday(&_curr_ts, 0);
      _curr_seconds = _curr_ts.tv_sec;

      index = 0;
      temp_size = 0;
      for (int i = 0; i < _num_atts; i++) 
	{
	  temp_size = _fields[i]->getReturnedSize();
	  memcpy(_group_by_values + index, _fields[i]->evaluate(_curr_tuple), temp_size);
	  index += temp_size;
	}
      
      // check to see if any of the states have timed out
      timeOutStates(tuple_ts);
      
      // try to get the states for the current group_by_values
      State *s = _hash->getState(_group_by_values);
      
      if (!s)   // if the state for the current atts does not exist 
	{
	  // the appropriate state may be in trash in which case the tuple 
	  // has to be discarded
	  if (checkInTrash(_group_by_values))
	    continue;
	  else      // if not found in the trash
	    {
	      s = new State("\0\0\0", getSidSize(), _group_by_values, _group_by_size, (_af->makeNew()), _curr_ts, _curr_ts);
	      activateState(s);
	      _hash->addState(s, _group_by_values, 1);
	      incrState(s);
	      checkForClose(s);
	    }
	}
      else     // if the state for the current atts exists
	{
	  incrState(s);
	  checkForClose(s);
	}
      delete[] _curr_tuple;
    }
  return_val.kept_input_count = 0;
  return_val.output_tuples = _num_of_output_tuples;
  return_val.output_tuple_size = _new_tuple_size;

  // ADDED BY CJC TO ADDRESS VALGRIND'S COMPLAINT ABOUT AN UNINITIALIZED 
  // VARIABLE BEING USED IN AN IF(...) EXPRESSION IN WorkerThread.C.
  // IF THIS FIXUP IS AN ERROR, PLEASE CORRECT IT. -CJC, 3 MARCH 2003.
  return_val.kept_input_count_array = NULL;

  return return_val;
}

void XSectionQBox::timeOutStates(Timestamp t)
{
  //  if (t > _maxtime)
  if (t.tv_sec > _maxtime)
    _maxtime = t.tv_sec;
  
  State *s;
  _iter = _hash->begin();
  
  // go through all the states in the _hash too see if any state has timed out
  // check the UNTIL timeout
  //int size = _hash->getSize();
  int counter = 0;
  while (_iter != _hash->end()) //(counter < size) 
    {     
      counter++;
      s = (*_iter).second;     // (*_iter).second is a pointer to the state
      if (!s)
	break;    

      if ((s->getStatus() == ACTIVE) && (_timeout == TIMEOUT))
	
	// right now, we compare with oldest time...might want to change that.
//	if ((_curr_ts - s->getTo()) > _until_timeout_s)
	if ((_curr_seconds -  (s->getTo()).tv_sec) > _until_timeout_s)
	  {
	    _iter++;
	    close(s);
	    continue;
	  }
      _iter++;
    }
}

void XSectionQBox::close(State *s) 
{
  char *atts = s->getAttributeVals();
  
  if (_output == LAST)
    emitTuple(s);
  s->setStatus(DONE);

  // create a TrashState that will store att_values, and the current ts
  TrashState *temp = new TrashState(atts, _group_by_size, _curr_ts);

  _hash->removeState(atts);           // remove from hash table
  _trash_hash->addTrashState(temp);   // add to trash
  //destroy(s);   // clear up memory taken by the state
  delete s;
}

void XSectionQBox::activateState(State *s)
{
  s->getAF()->init();
  s->setStatus(ACTIVE);
}

void XSectionQBox::incrState(State *s)
{

  s->getAF()->incr(_curr_tuple);

  if (_curr_ts > s->getTn())
    s->setTn(_curr_ts);

  if (_curr_ts < s->getTo())
    s->setTo(_curr_ts);

  if (_output == ALL)
    emitTuple(s);
  else if (_output == WHENEVER)
    if(_whenever_pred->evaluate(_curr_tuple))
      emitTuple(s);
}
  
void XSectionQBox::checkForClose(State *s)
{
  if (_timeout == SATISFIES)
    if(_satisfies_pred->evaluate(_curr_tuple))
      close(s);
}

bool XSectionQBox::checkInTrash(char *value)
{
  bool flag = false;
  TrashState *s;
  _trash_iter = _trash_hash->begin();

  // go through all the states in the trash and check UNIQUE FOR timeout
  // also see if value is in trash -> which means a new state should not 
  // be created : denoted by boolean flag
  int counter = 0;
  int size = _trash_hash->getSize();
  while (_trash_iter != _trash_hash->end()) //(counter < size)
    {
      cout << counter << "    " << size  << "    ";
      counter++;
      
      s = (*_trash_iter).second;
      //      cout << "if s is null print 1    --"  << !s << endl;
      
      if (!s) {
	//cout << "key: " << (_trash_iter == _trash_hash->end()) << endl;
	break;
      }
      
      //      if ((_curr_ts - s->getTimer()) > _unique_timeout)     // UNIQUE FOR timeout
      if ((_curr_seconds - (s->getTimer()).tv_sec) > _unique_timeout)
	{
	  //cout << "removing: " << s->getKey()->key_size << " or rather"; s->printAll();
	  _trash_iter++;
	  _trash_hash->removeTrashState(s->getValues());
	  delete s;
	  //_trash_iter++;
	  continue;
	}
      if (memcmp(value, s->getValues(), _key_size) == 0)
	flag = true;       // state for current tuple's group_by_value was
                           // found in the trash -> do not create the state
      _trash_iter++;
    }
  return flag;
}

void XSectionQBox::emitTuple(State *s)
{
  char *group_by = s->getAttributeVals();
  GroupByState *temp = _group_hash->getGroupByState(group_by);
  int new_sid;
  if (!temp)
    {
      //can use getSize because we don't currently delete from the GroupByHash
      //might want to change this later because it seems counterintuitive that
      //this hash remembers everythiing while trashhash does not.
      new_sid = _group_hash->getSize();
      temp = new GroupByState(group_by, _group_by_size, new_sid);
      _group_hash->addGroupByState(temp);
    }
  else
    new_sid = temp->getNewSid();

 // copy the timestamp to output_tuple
  Timestamp ts = s->getTo();
  memcpy(_output_tuple, &ts, getTsSize());
  
  // copy the sid to output_tuple
  _output_tuple += getTsSize();
  memcpy(_output_tuple, &new_sid, getSidSize());
  _output_tuple += getSidSize();

  // copy the attribute values to output_tuple
  memcpy(_output_tuple, group_by, _group_by_size);
  _output_tuple += _group_by_size;
  
  // copy the output result to output_tuple
  char *result = s->getAF()->final();
  memcpy(_output_tuple, result, _af->getReturnedSize());
  _output_tuple += _af->getReturnedSize();
  
  _num_of_output_tuples++;

  // *****************************************************
  // temporarily print to see the output_tuple
  //const char *desc =  "tiii";   //"tiicccci";
  //printTuple(_output_tuple - _new_tuple_size, desc);
  printf ("\t  will be placed in the output stream -- Yahoo!\n\n");
  // *****************************************************
  
}


void XSectionQBox::setWheneverPredicate(Predicate *p)
{
  _whenever_pred = p;
}

void XSectionQBox::setSatisfiesPredicate(Predicate *p)
{
  _satisfies_pred = p;
}

void XSectionQBox::setTimeoutS(Timestamp s)
{
  _until_timeout_s = s.tv_sec;
}

