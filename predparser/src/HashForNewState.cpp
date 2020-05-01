#include "HashForNewState.H"

HashForNewState::HashForNewState(char *group_by_values, int group_by_size, char *group_by_values_str,  char order_att_type)
{
  _hash_with_states = hash_map<char*, NewState*, hash<const char*>, equal_string>();
  if (group_by_values_str != NULL) {
    _group_by_values_str = (char*) malloc(strlen(group_by_values_str)+1);
    strcpy(_group_by_values_str,group_by_values_str);
  } else {
    cout << "[HashForNewState] Not expecting group_by_values_str to be null - make it empty string!" << endl;
    exit(1);
  }
  _group_by_size = group_by_size;
  if (group_by_values != NULL) {
    _group_by_values = new char[group_by_size];
    memcpy(_group_by_values, group_by_values, group_by_size);
  } else {
    _group_by_values = NULL;
  }
  _first_tuple_done = false;
  _order_att_type = order_att_type;
  
  _values_more_than_threshold = vector<char*>();
  _curr_slack = 0;
  if (order_att_type == 'i')
    _curr_threshold = new char[sizeof(int)];
  else
    _curr_threshold = new char[sizeof(timeval)];
  
  _tuple_store = new vector<char*> ();
  _tuple_store_states = new vector<NewState*> ();
}

HashForNewState::HashForNewState(char order_att_type)
{
  _hash_with_states = hash_map<char*, NewState*, hash<const char*>, equal_string>();
  _first_tuple_done = false;
  _order_att_type = order_att_type;
  
  _values_more_than_threshold = vector<char*>();
  _curr_slack = 0;
  if (order_att_type == 'i')
    _curr_threshold = new char[sizeof(int)];
  else
    _curr_threshold = new char[sizeof(timeval)];
}
void HashForNewState::addState(NewState *s)
{

  _hash_with_states.insert(pair<char*, NewState*> (s->getValueStr(), s));

  /**
  int value_size = s->getValueSize();
  char *value = new char[value_size];
  memcpy(value, s->getValue(), value_size);
  cout << "[HashForNewState] ]]]]]]] ADDING A PAIR (val,s) (" << value << ","<<value_size<<")"<<endl;
  _hash_with_states.insert(pair<char*, NewState*> (value, s));
  */
  return;
}

void HashForNewState::removeState(NewState *s) 
{
  /**
  int value_size = s->getValueSize();
  char *value = new char[value_size];
  memcpy(value, s->getValue(), value_size);
  cout << "[HashForNewState] ]]]]]]]]] Value size is " << value_size << endl;
  cout << "[HashForNewState] ]]]]]]]]] getValue returns (" << s->getValue() << ")" << endl;
  */

  _hash_iter = _hash_with_states.find(s->getValueStr());
  //cout << "[HashForNewState] *****************************hash size before " << _hash_with_states.size() << endl;
  //cout << "[HashForNewState] **************************** removing " << *(int*) value << endl;
  //_hash_with_states.erase(value);

  if (_hash_iter != _hash_with_states.end())
    {
      //cout << "[HashForNewState] removing state from hash .." << endl;
      _hash_with_states.erase(_hash_iter);
    }
  else
    {
      //cout << "[HashForNewState] cannot remove state that does not exist for value " << s->getValueStr() << endl;
    }

  //free(value);
  return;
}

NewState* HashForNewState::getState(char *value)
{
  _hash_iter = _hash_with_states.find(value);

  if (_hash_iter != _hash_with_states.end())
      return (*_hash_iter).second;
  else
    {
      //cout << "[HashForNewState] No state for value " << value << endl;
      return NULL;
    }
}

void HashForNewState::removeState(char *value)
{
  NewState *temp = getState(value);
  if (temp == NULL)  
    {
      cout << "[HashForNewState] Warning - cannot remove state: does not exist " << value << endl;
    }
  else
    {
      removeState(temp);
    }
  return;
}

bool HashForNewState::getFirstTupleDone()
{
  return _first_tuple_done;
}

void HashForNewState::setFirstTupleDone()
{
  _first_tuple_done = true;
}

int HashForNewState::getHashSize() 
{
  return _hash_with_states.size();
}

char* HashForNewState::getLastVal()
{
  return _last_val;
}

void HashForNewState::setLastVal(char *val)
{
  _last_val = val;
}

int HashForNewState::getSlack()
{
  return _curr_slack;
}

void HashForNewState::increaseSlack()
{
  _curr_slack++;
}

void HashForNewState::setSlack(int new_slack)
{
  _curr_slack = new_slack;
}

char* HashForNewState::getThreshold()
{
  return _curr_threshold;
}

void HashForNewState::setThreshold(char *new_threshold)
{
  _curr_threshold = new_threshold;
}

void HashForNewState::increaseThreshold()
{
  if (_order_att_type == 'i')
    {
      int value = *(int*) _curr_threshold;
      value++;
      *(int*) _curr_threshold = value;
    }
  else  // order on timestamp then   
    {
      timeval threshold_struct = *(timeval*) _curr_threshold;
      long value = threshold_struct.tv_sec;
      value += _time_increment;
      timeval new_struct = {value, threshold_struct.tv_usec};
      *(timeval*) _curr_threshold = new_struct;
      // don't know what to do here yet .....
    }
}

int HashForNewState::countAndClearVector() 
{
  _vector_iter = _values_more_than_threshold.begin();
  int count = 0;

  while (_vector_iter != _values_more_than_threshold.end())
    {
      if (_order_att_type == 'i')
	{
	  int threshold = *(int*) _curr_threshold;
	  int vector_value = *(int*) (*_vector_iter) ;
	  
	  if (vector_value <= threshold)
	    {
	      //cout << "[HashForNewState] removing from vector ........." << endl;
	      _vector_iter = _values_more_than_threshold.erase(_vector_iter);
	    }
	  else
	    {
	      count++;
	      _vector_iter++;
	    }
	}
      else     // order-on timestamp
	{
	  timeval threshold_struct = *(timeval*) _curr_threshold;
	  long threshold = threshold_struct.tv_sec;
	  timeval time_struct = *(timeval*) (*_vector_iter);
	  long vector_seconds = time_struct.tv_sec;
	  
	  if (vector_seconds <= threshold)
	    {
	      //	      cout << "[HashForNewState] removing from vector TIME ........." << endl;
	      _vector_iter = _values_more_than_threshold.erase(_vector_iter);
	    }
	  else
	    {
	      count++;
	      _vector_iter++;
	    }
	  // don't know what to do here yet .....
	} 
      
    }
  //  cout << "count = " << count << "vector.size() = " << _values_more_than_threshold.size() << endl;
  return count;
  //  return _values_more_than_threshold.size();
  
}

// NOTE TO CODER BY EDDIE: OH OH !!!!!!!!!!! these char*'s better be NUL TERMINATED!!!
// WHY DO I GET THE FEELING THEY ARE NOT NULL TERMINATED!!!??
// although ok, let me cut you some slack, YOU ARE LUCK THIS WORKS cuz you are just passing pointers.
void HashForNewState::addToVector(char *value)
{
  //cout << "[HashForNewState] adding to vector ........." << endl;
  _values_more_than_threshold.push_back(value);
}

char* HashForNewState::getGroupByValues()
{
  return _group_by_values;
}

int HashForNewState::getGroupBySize()
{
  return _group_by_size;
}

char* HashForNewState::getGroupByValuesStr()
{
  return _group_by_values_str;
}

void HashForNewState::setTimeIncrement(long num)
{
  _time_increment = num;
}

/**
int HashForNewState::getGroupBySizeStr()
{
  cout << "[HashForNewState] WARNING! SOMEONE JUST CALLED ME! CHECK WHO! " << endl;
  return strlen(_group_by_values_str);
}
*/
bool HashForNewState::isEmpty()
{
  return _hash_with_states.empty();
}

hash_map<char*, NewState*, hash<const char*>, equal_string> HashForNewState::getHash()
{
  return _hash_with_states;
}

hash_map<char*, NewState*, hash<const char*>, equal_string>::iterator HashForNewState::begin()
{
  return _hash_with_states.begin();
}

hash_map<char*, NewState*, hash<const char*>, equal_string>::iterator HashForNewState::end()
{
  return _hash_with_states.end();
}

vector<char*>* HashForNewState::getTupleStore()
{
  return _tuple_store;
}

vector<NewState*>* HashForNewState::getTupleStoreStates()
{
  return _tuple_store_states;
}

void HashForNewState::addToTupleStore(char *tuple, NewState *ns)
{
  _tuple_store->push_back(tuple);
  _tuple_store_states->push_back(ns);
}

void HashForNewState::clearTupleStore(int num)
{
  int loop = 0;
 
 if (num > _tuple_store->size())
    loop = _tuple_store->size();
  else
    loop = num;
  
  for (int i = 0; i < loop; i++)
    {
      _tuple_store->erase(_tuple_store->begin());
      _tuple_store_states->erase(_tuple_store_states->begin());
    }
}
