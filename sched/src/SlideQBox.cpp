#include "SlideQBox.H"

SlideQBox::SlideQBox()
{
}

SlideQBox::~SlideQBox()
{
}

void SlideQBox::setState(AggregateState *agg)
{
  AggregateFunction *af = agg->af;
  const char *group_by_atts = agg->group_by;
  int window_range = agg->window_range;
  output_arg output = agg->output;
  Timestamp slack_timeout = agg->slack_time;
  Timestamp unique_timeout = agg->unique_time;
  until_arg unless = agg->until;

  // initialize most of the variables
  _maxtime = 0; //NULL;
  //  _mintime = 0; //NULL;
  _window_range = window_range;
  _output = output;
  _timeout = unless;
  _slack_timeout = slack_timeout.tv_sec;
  _unique_timeout = unique_timeout.tv_sec;
  _af = af;
  _new_tuple_size = 0;
  _curr_pos = 0; 
  _curr_pos = 0; 
  _num_atts = 1;

  _fields = parseAtts(group_by_atts, _num_atts);

  _group_by_size = _fields[0]->getReturnedSize();
  switch (_fields[0]->getType())
    {
    case 'i':
      _value_type = 1;
      break;
    case 'f':
      _value_type = 0;
      break;
    default:
      cout << "[SlideQBox] wrong type for Slide GroupBy" << endl;
    }

  _new_tuple_size = _group_by_size;
  _group_by_values = new char[_group_by_size];

  // add the size of sid and output value
  _new_tuple_size += getSidSize() + _af->getReturnedSize(); //  size of output

  // add the size of timestamp --- which is an int
  _new_tuple_size += getTsSize();

  _sid_size = getSidSize();
  _key_size = _sid_size + _group_by_size;
  _hidden_size = getTsSize() + _sid_size; 
  _tuple_size = _tuple_descr->getSize() + _hidden_size;
  if (agg->output == WHENEVER)
    setWheneverPredicate(agg->whenever_pred);
  if (agg->until == SATISFIES)
    setSatisfiesPredicate(agg->satisfies_pred);
  if (agg->until == TIMEOUT)
    setTimeoutS(agg->unless_timeout);
  _hash = agg->list_hash;
  _trash_hash = agg->trash;
  _hash->setKeySize(_sid_size);
  _trash_hash->setKeySize(_key_size);
}

Box_Out_T SlideQBox::doBox()
{
  //  time_t curr_time;
  //  int index;

  int value;    // int value (casted to int if group_by float) of the group_by attribute
  float x;      // float value of group_by - if group_by_type = float
  int value_for_state;   // the values for the different states affected - and will be updated
  int value_of_lowest_in_sid;   //lowest group_by value in the given sid

  char *temp_string;
  char *group_by;
  _num_of_output_tuples = 0;
  _output_tuple = _outStream[0];

  for (int ii = 0; ii < _train_size[0]; ii++)
    {
      _num_of_output_tuples = 0;
      _curr_tuple = new char[_tuple_size];
      memcpy(_curr_tuple, _inStream[0] + (ii * _tuple_size), _tuple_size);
      // extract the ts from the tuple is _curr_tuple
      Timestamp tuple_ts = getTs(_curr_tuple);  
      
      cout << "[SlideQBox] input " ; printTuple(_curr_tuple, "iiiifcccc");

      // extract the sid from the tuple in _curr_tuple
      _sid = getSid(_curr_tuple); 
      
      // get the current time from the system
      //      time(&curr_time);
      //      _curr_ts = curr_time;
      gettimeofday(&_curr_ts, 0);
      _curr_seconds = _curr_ts.tv_sec;

      // group_by_values will store the attribute value
      memcpy(_group_by_values, _fields[0]->evaluate(_curr_tuple), _group_by_size);
      
      // check to see if any of the states have timed out
      timeOutStates(tuple_ts);
      
      // temp_string will store sid+group_by_val for checking in the TrashHash
      temp_string = new char[_sid_size + _group_by_size];
      memcpy(temp_string, _sid, _sid_size);
      
      // retrieve the list for the sid of the current tuple
      List *lst = _hash->getList(_sid);   // get the list for the current sid
      group_by = new char[_group_by_size];
      
      // changes the group_by_value to int from a char*
      // the group_by attribute outputted by Slide will always be an int 
      if (_value_type)
	value = *(int*)(_group_by_values);   // if int get the value from the group_by_values
      else
	{
	  x = *(float*)(_group_by_values);   // if float, get the value
	  value = (int) x;                   // then cast it to int
	}
      
      if (!lst)  // if there is no list associated with the current sid
	{
	  lst = new List(_sid, _sid_size);
	  _hash->addList(lst);
	  State *s;
	  for (int i = 0; i < _window_range; i++)
	    {
	      // value_for_state will be the states that will be updated 
	      value_for_state = value - _window_range + i + 1;  // the value for the state affected
	      *(int*)(group_by) = value_for_state;
	      
	      memcpy(temp_string + _sid_size, group_by, _group_by_size);
	      
	      if (checkInTrash(temp_string))
		continue;
	      else
		{
		  s = new State(_sid, _sid_size, group_by, _group_by_size, (_af->makeNew()), _curr_ts, _curr_ts);
		  activateState(s);
		  lst->addState(s);
		  incrState(s);
		  checkForClose(s);
		}
	    }
	}
      else   // the list for the given sid exists
	{
	  // start appropriate timers for the slack timeout
	  startTimers(lst, value);
	  
	  for (int i = 0; i < _window_range; i++)
	    {
	      // value_for_state will be the states that will be updated 
	      value_for_state = value - _window_range + i + 1;
	      *(int*)(group_by) = value_for_state;
	      
	      // if the value for state is smaller than the smaller value in the list
	      // don't create any new states
	      if (lst->getSize() != 0)
		{
		  // see the smallest group_by in the given sid
		  // if the current value is bigger than the smallest for the state
		  // then this new state will not be created
		  value_of_lowest_in_sid = *(int*)(lst->first()->getAttributeVals());
		  if (value_for_state < value_of_lowest_in_sid)
		    continue;
		}
	      
	      memcpy(temp_string + _sid_size, group_by, _group_by_size);
	      
	      // get the state for the given group_by from the list
	      State *s = lst->getState(group_by, _group_by_size);
	      if (!s)  // there is no state in the list
		{
		  if (checkInTrash(temp_string))
		    continue;
		  else
		    {
		      s = new State(_sid, _sid_size, group_by, _group_by_size, (_af->makeNew()), _curr_ts, _curr_ts);
		      activateState(s);
		      lst->addState(s);
		      incrState(s);
		      checkForClose(s);
		    }
		}
	      else   // if the state is there
		{
		  incrState(s);
		  checkForClose(s);
		}
	    }
	}
      delete[] group_by;
      delete[] temp_string;
      delete[] _curr_tuple;
    }
  return_val.kept_input_count = 0;
  return_val.output_tuples = _num_of_output_tuples;
  return_val.output_tuple_size = _new_tuple_size;
  //  return_val.currStates = _hash;

  // ADDED BY CJC TO ADDRESS VALGRIND'S COMPLAINT ABOUT AN UNINITIALIZED 
  // VARIABLE BEING USED IN AN IF(...) EXPRESSION IN WorkerThread.C.
  // IF THIS FIXUP IS AN ERROR, PLEASE CORRECT IT. -CJC, 3 MARCH 2003.
  return_val.kept_input_count_array = NULL;

  return return_val;
}

void SlideQBox::timeOutStates(Timestamp t)
{
  // if (t > _maxtime)
  if (t.tv_sec > _maxtime)
    _maxtime = t.tv_sec;
  List *lst;
  State *s;
  _iter = _hash->begin();

  // go through all the states in the _hash too see if any state has timed out
  // check both UNLESS and SLACK timeout
  while (_iter != _hash->end())
    {
      lst = (*_iter).second;
      if (!lst)
	break;
      
      // go through the list that you just got from _hash
      list<State*>::iterator list_iter = lst->begin();
      while (list_iter != lst->end())
	{
	  s = *list_iter;
	  if (!s)
	    break;
	        
	  if (s->getStatus() == ACTIVE)
	    {
	      if (_timeout == TIMEOUT)
		// right now the timeout is based to To, the oldest tuple 
		// contributing to the state
		//		if((_curr_ts - s->getTo()) > _unless_timeout_s)    // UNLESS timeout
		if ((_curr_seconds - (s->getTo()).tv_sec) > _unless_timeout_s)
		  {
		    list_iter++;
		    close(s);
		    continue;
		  }
	      //	      if (s->getTimer() != 0)
	      // 		if ((_curr_ts - s->getTimer()) > _slack_timeout )   // SLACK timeout
	      if ((s->getTimer()).tv_sec != 0)
		if ((_curr_seconds - (s->getTimer()).tv_sec) > _slack_timeout)
		  {
		    list_iter++;
		    close(s);
		    continue;
		  }
	      list_iter++;
	    }
	}
      _iter++;
    }
}

void SlideQBox::close(State *s) 
{
  char *sid = s->getSid()->key;
  char *atts = s->getAttributeVals();

  if (_output == LAST)
    emitTuple(s);
  s->setStatus(DONE);

  char *temp_string = new char[_sid_size + _group_by_size];   //e + 1];
  memcpy(temp_string, sid, _sid_size);
  memcpy(temp_string + _sid_size, atts, _group_by_size);

  //  printf("Slide what %s", atts); 
  //    cout << "[SlideQBox] closing state ..... "; printTuple(temp_string, "iii");

  // create a TrashState that will store sid, att_values, and the current ts
  TrashState *temp = new TrashState(temp_string, _sid_size + _group_by_size, _curr_ts);

  List *temp_list = _hash->getList(sid); 
  temp_list->removeState(s);          // remove s from the list
  _trash_hash->addTrashState(temp);   // add to trash
  delete[] temp_string;
  delete s;
}

void SlideQBox::activateState(State *s)
{
  s->getAF()->init();
  s->setStatus(ACTIVE);
}

void SlideQBox::incrState(State *s)
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
  
void SlideQBox::checkForClose(State *s)
{
  if (_timeout == SATISFIES)
    if(_satisfies_pred->evaluate(_curr_tuple))
      close(s);
}

bool SlideQBox::checkInTrash(char *value)
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
      
      //      if ((_curr_ts - s->getTimer()) > _unique_timeout)     // UNIQUE FOR timeout
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

void SlideQBox::emitTuple(State *s)
{
  //  cout << "[SlideQBox] num output so far: " << _num_of_output_tuples << endl;
  // copy the timestamp to output_tuple
  Timestamp ts = s->getTo();
  memcpy(_output_tuple, &ts, getTsSize());     
  //  cout << "[SlideQBox] num output so far: " << _num_of_output_tuples << endl; 
  
  // copy the sid to output_tuple
  _output_tuple += getTsSize();
  memcpy(_output_tuple, s->getSid()->key, _sid_size);
  
  //  cout << "[SlideQBox] num output so far: " << _num_of_output_tuples << endl;
  // copy the attribute values to output_tuple
  _output_tuple += _sid_size;

  if (_value_type)   // if the group_by is an int, just copy it
    {
      cout << " output         is it really an int ????" << endl;
      memcpy(_output_tuple, s->getAttributeVals(), _group_by_size);
    }
  else   // if float, convert the int from state to float
    {
      int int_temp = *(int*) s->getAttributeVals();
      float float_temp = (float) int_temp;
      cout << "output      int value " << int_temp << "     float value " << float_temp << endl;
      memcpy(_output_tuple, &float_temp, _group_by_size);
    }
    
  // copy the output result to output_tuple
  _output_tuple += _group_by_size;
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
  //  char *desc =  "tiii";
  printf("\n");
  cout << "[SlideQBox] output " ; printTuple(_output_tuple - _new_tuple_size, "iiifi");
  printf ("\t  will be placed in the output stream -- Yahoo!\n\n");
  // *****************************************************
}

void SlideQBox::startTimers(List *lst, int value)
{
  list<State*>::iterator iter = lst->begin();
  State *temp;

  // go through all element in the list and start slack timers if not yet
  // if the current tuple is to update the state, then the timer is set 
  // to 0 in the incrState() method
  while (iter != lst->end())
    {
      temp = *iter;
      
      //     int x = *(int*) (temp->getAttributeVals());
      //     if (value >= x)
      // the above line is commented so that slack timer will start
      // in all the states not affected by the current tuple 
      // this might cause a bigger valued tuple to be emitted before 
      // smaller ones
      // if a 5 comes in the middle of all the 3's, then the 3 after
      // the 5 will start the slack timer in 5 and will timeout and be
      // emitted when the arriving tuples are in the midst of 3's.
      // output will then not necessarily be in order

      //	if (temp->getTimer() == 0)
      if ((temp->getTimer()).tv_sec == 0)  
	  temp->setTimer(_curr_ts);
      iter++;
    }

}

void SlideQBox::setWheneverPredicate(Predicate *p)
{
  _whenever_pred = p;
}

void SlideQBox::setSatisfiesPredicate(Predicate *p)
{
  _satisfies_pred = p;
}

void SlideQBox::setTimeoutS(Timestamp s)
{
  _unless_timeout_s = s.tv_sec;
}

