#include "GroupByState.H"

GroupByState::GroupByState(char *group_by_values, int size,  int sid)
{
  _group_by_values = new char[size];
  memcpy(group_by_values, group_by_values, size);
  
  _key = new key_type();
  _key->key = new char[size];
  memcpy(_key->key, group_by_values, size);
  _key->key_size = size; 

  _new_sid = sid;
}

GroupByState::~GroupByState()
{
  delete _key;
  delete[] _group_by_values;
  _new_sid = 0;
}

char* GroupByState::getGroupByValues()
{
  return _group_by_values;
}

int GroupByState::getNewSid()
{
  return _new_sid;
}

key_type* GroupByState::getKey()
{
  return _key;
}  
