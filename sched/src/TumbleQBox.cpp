#include "TumbleQBox.H"

TumbleQBox::TumbleQBox()
{
}

TumbleQBox::~TumbleQBox()
{
  /*
  _iter = _hash->begin();
  int size = _hash->getSize();
  int counter = 0;
  while (counter < size) 
    {
      counter++;
      State* temp = (*_iter).second;      
      _iter++;
      if (temp) {
	delete temp;
	temp = NULL;
      }
    }
  delete _hash;
  delete _last;
  _trash_iter = _trash_hash->begin();
  size = _trash_hash->getSize();
  counter = 0;
  while (counter < size) 
    {
      counter++;
      TrashState* temp = (*_trash_iter).second;      
      _trash_iter++;
      if (temp) {
	delete temp;
	temp = NULL;
      }
    }
  delete _trash_hash;
  delete _af;
  if (_whenever_pred)
    delete _whenever_pred;
  if (_satisfies_pred)
    delete _satisfies_pred;
  for (int i = 0; i < _num_atts; i++)
    delete _fields[i];
  if (_sid)
    delete[] _sid;
  if (_group_by_values)
    delete[] _group_by_values;
  //??delete _iter;
  //??delete _trash_iter;
  */
}

void TumbleQBox::setState(AggregateState *agg)
{
  AggregateFunction *af = agg->af;
  const char *group_by_atts = agg->group_by;
  output_arg output = agg->output;
  Timestamp slack_timeout = agg->slack_time;
  Timestamp unique_timeout = agg->unique_time;
  until_arg unless = agg->until;
  // initialize most of the variables
  _maxtime = 0;
  //  _mintime = 0;
  _output = output;
  _timeout = unless;
  _slack_timeout = slack_timeout.tv_sec;
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
  _new_tuple_size += getSidSize() + _af->getReturnedSize(); //  size of output

  // add the size of timestamp --- which is an int
  int ts_size = getTsSize();
  _new_tuple_size += ts_size;

  // an extern variable that will be used to see if the right state is 
  // being referred to in the hash tables
  // the key for hashing will be a concatenation of sid & group_by_values
  _sid_size = getSidSize();
  _key_size = _sid_size + _group_by_size;
  _hidden_size = ts_size + _sid_size; 
  _tuple_size = _tuple_descr->getSize() + _hidden_size;
  if (agg->output == WHENEVER)
    setWheneverPredicate(agg->whenever_pred);
  if (agg->until == SATISFIES)
    setSatisfiesPredicate(agg->satisfies_pred);
  if (agg->until == TIMEOUT)
    setTimeoutS(agg->unless_timeout);
  _hash = agg->reg_hash;
  _last = agg->last;
  _trash_hash = agg->trash;
  _hash->setKeySize(_key_size);
  _trash_hash->setKeySize(_key_size);
  _last->setKeySize(_sid_size);
}

Box_Out_T TumbleQBox::doBox() 
{ 
  //  time_t curr_time;
  int index;
  int temp_size;
  char *hash_string;
  bool closed_state;
  _num_of_output_tuples = 0;
  _output_tuple = _outStream[0];

  for (int ii = 0; ii < _train_size[0]; ii++)
    {
      //sleep(1);
      _curr_tuple = new char[_tuple_size];
      memcpy(_curr_tuple, _inStream[0] + (ii * _tuple_size), _tuple_size);
      // extract the ts from the tuple in _curr_tuple
      Timestamp tuple_ts = getTs(_curr_tuple);  
      
      // extract the sid from the tuple in _curr_tuple
      _sid = getSid(_curr_tuple); 
      
      cout << "[TumbleQBox] input" ;
      if (_tuple_size == 25)
	  printTuple(_curr_tuple, "ticfiii");
      else if (_tuple_size == 16)
	  printTuple(_curr_tuple, "tiii");

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
      
      // hash_string will store sid+group_by_values for hashing
      hash_string = new char[_sid_size + _group_by_size];   // + 1];
      memcpy(hash_string, _sid, _sid_size);
      memcpy(hash_string + _sid_size, _group_by_values, _group_by_size);
      //  hash_string[getSidSize() + _group_by_size] = '\0';
  
      // try to get the states for the current sid+group_by_values
      State *s = _hash->getState(hash_string);

      if (!s)   // if the state for the current sid and atts does not exist 
	{
	  // the appropriate state may be in trash in which case the tuple 
	  // has to be discarded
	  if (checkInTrash(hash_string))
	    continue;
	  else      // if not found in the trash
	    {
	      s = new State(_sid, _sid_size, _group_by_values, _group_by_size, (_af->makeNew()), _curr_ts, _curr_ts);
	      activateState(s);
	      _hash->addState(s, hash_string, 1);
	      incrState(s);
	      closed_state = checkForClose(s);
	      
	      // update the _last pointer for the sid of the current tuple
	      State *temp = _last->getState(_sid);
	      if (temp != NULL)
		{
		  temp->setTimer(_curr_ts);
		  _last->changeState(s);
		}
	      else if (!closed_state){
		_last->addState(s, _sid, 0);
	      }
	    }
	}
      else     // if the state for sid and atts exists
	{
	  incrState(s);
	  closed_state = checkForClose(s);
	  
	  // update the _last pointer for the sid of the current tuple
	  State *temp = _last->getState(_sid);
	  if ((temp != NULL) && (temp != s))
	    {
	      temp->setTimer(_curr_ts);
	      _last->changeState(s);
	    }
	  else if (!closed_state) {
	    _last->addState(s, _sid, 0);
	  }
	}

      cout << "[TumbleQBox] State Hash Table: " << endl;
      _hash->printAll();
      cout << "[TumbleQBox]  Last Hash Table: " << endl;
      _last->printAll();
      cout << endl << " [TumbleQBox]   Trash Hash Table: " << endl;
      _trash_hash->printAll();
      delete[] _curr_tuple;
      delete[] hash_string;
      delete[] _sid;
    }
  return_val.kept_input_count = 0;
	return_val.kept_input_count_array = new int[_numInputArcs];
	return_val.kept_input_count_array[0] = 0;

  return_val.output_tuples = _num_of_output_tuples;
  return_val.output_tuple_size = _new_tuple_size;
  

  cout << "[TumbleQBox] State Hash Table: " << endl;
  _hash->printAll();
  cout << "[TumbleQBox]  Last Hash Table: " << endl;
  _last->printAll();
  cout << endl << " [TumbleQBox]   Trash Hash Table: " << endl;
  _trash_hash->printAll();
  cout << endl;
  //  return_val.currStates = _hash;
  cout << "[TumbleQBox]num_return_values = " << return_val.output_tuples << endl;
  return return_val;
}

void TumbleQBox::timeOutStates(Timestamp t)
{
  //  if (t < _mintime)
  //  _mintime = t;
  //  if (t > _maxtime)
  if (t.tv_sec > _maxtime)
    _maxtime = t.tv_sec;
  
  State *s;
  _iter = _hash->begin();

  // go through all the states in the _hash too see if any state has timed out
  // check both UNLESS and SLACK timeout
  int size = _hash->getSize();
  int counter = 0;
  while (counter < size) 
    {
      counter++;
      s = (*_iter).second;    // (*_iter).second is a pointer to the state
      if (!s)
	break;
      
      if (s->getStatus() == ACTIVE)
	{
	  if (_timeout == TIMEOUT)
	    // right now the timeout is based to To, the oldest tuple 
	    // contributing to the state
	    //	    if((_curr_ts - s->getTo()) > _unless_timeout_s)    // UNLESS timeout
	    if ((_curr_seconds - (s->getTo()).tv_sec) > _unless_timeout_s)
	      {
		_iter++;
		cout << "Closed for timeout" << endl;
		close(s);
		continue;
		}
	  //	  if (s->getTimer() != 0)
	  //	    if ((_curr_ts - s->getTimer()) > _slack_timeout )   // SLACK timeout
	  if ((s->getTimer()).tv_sec != 0)
	    if ((_curr_seconds - (s->getTimer()).tv_sec) > _slack_timeout)
	      {
		_iter++;
		cout << "Closed for slack" << endl;
		close(s);
		continue;
	      }
	}
      _iter++;
    }
}

void TumbleQBox::close(State *s) 
{
  char *sid = s->getSid()->key;
  char *atts = s->getAttributeVals();

  if (_output == LAST)
    emitTuple(s);
  s->setStatus(DONE);

  char *temp_string = new char[_sid_size + _group_by_size];   //e + 1];
  memcpy(temp_string, sid, _sid_size);
  memcpy(temp_string + _sid_size, atts, _group_by_size);

  // create a TrashState that will store sid, att_values, and the current ts
  TrashState *temp = new TrashState(temp_string, _sid_size + _group_by_size, _curr_ts);

  State* last_s = _last->getState(sid);
  //if ((s) && (last_s->keyEqual(s->getKey())))
  if (last_s == s) {
    _last->removeState(sid);            // update _list(sid) 
  }
  _hash->removeState(s->getKey()->key);    // remove from hash table
  _trash_hash->addTrashState(temp);   // add to trash
  delete[] temp_string;
  delete s;
}

void TumbleQBox::activateState(State *s)
{
  s->getAF()->init();
  s->setStatus(ACTIVE);
}

void TumbleQBox::incrState(State *s)
{

  s->getAF()->incr(_curr_tuple);
  s->setTimer(*(new Timestamp()));

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
  
bool TumbleQBox::checkForClose(State *s)
{
  if (_timeout == SATISFIES)
    if(_satisfies_pred->evaluate(_curr_tuple)) {
      close(s);
      return true;
    }
  return false;
}

bool TumbleQBox::checkInTrash(char *value)
{
  bool flag = false;
  TrashState *s;
  _trash_iter = _trash_hash->begin();

  // go through all the states in the trash and check UNIQUE FOR timeout
  // also see if value is in trash -> which means a new state should not 
  // be created : denoted by boolean flag
  int size = _trash_hash->getSize();
  int counter = 0;
  while (counter < size) 
    {
      counter++;
      s = (*_trash_iter).second;
      
      if (!s)
	break;
      
      //     if ((_curr_ts - s->getTimer()) > _unique_timeout)     // UNIQUE FOR timeout
      if ((_curr_seconds - (s->getTimer()).tv_sec) > _unique_timeout)
      	{
      	  _trash_iter++;
      	  _trash_hash->removeTrashState(s->getValues());
      	  delete s;
      	  continue;
      	}
      if (memcmp(value, s->getValues(), _key_size) == 0)
	flag = true;       // state for current tuple's sid+group_by_value was
                           // found in the trash -> do not create the state
      _trash_iter++;
    }
  return flag;
}

void TumbleQBox::emitTuple(State *s)
{
  // copy the timestamp to output_tuple
  Timestamp ts = s->getTo();
  memcpy(_output_tuple, &ts, getTsSize());      
  
  // copy the sid to output_tuple
  _output_tuple += getTsSize();
  memcpy(_output_tuple, s->getSid()->key, _sid_size);
  _output_tuple += _sid_size;
  
  // copy the attribute values to output_tuple
  memcpy(_output_tuple, s->getAttributeVals(), _group_by_size);
  _output_tuple += _group_by_size;
  
  // copy the output result to output_tuple
  char *result = s->getAF()->final();
  memcpy(_output_tuple, result, _af->getReturnedSize());
  _output_tuple += _af->getReturnedSize();

  _num_of_output_tuples++;

  // place on the output stream
  //char *out_ptr;    // = new char[_new_tuple_size];
  // out_ptr = _outStream[0]; // set to beginning of _outStream initially  
  // memcpy(out_ptr, output_tuple, _new_tuple_size);
  
  // *****************************************************
  // temporarily print to see the output_tuple
  const char *desc;
  if (_tuple_size == 25)
    desc = "tiii";
  else if (_tuple_size == 16)
    desc = "tiii";
  else
    desc = "tiii";
    
  printf("\n[TumbleQBox] output");
  printTuple(_output_tuple - _new_tuple_size, desc);
  printf ("\t  will be placed in the output stream.\n\n");
  // *****************************************************
}


void TumbleQBox::setWheneverPredicate(Predicate *p)
{
  _whenever_pred = p;
}

void TumbleQBox::setSatisfiesPredicate(Predicate *p)
{
  _satisfies_pred = p;
}

void TumbleQBox::setTimeoutS(Timestamp s)
{
  _unless_timeout_s = s.tv_sec;
}
