#include "GroupByHash.H"

GroupByHash::GroupByHash()
{
  _group_hash_map = hash_map<key_type*, GroupByState*, my_hash, eqstr>();

}

void GroupByHash::setKeySize(int key_size)
{
  _key_size = key_size;
  //  _group_hash_map = hash_map<key_type*, GroupByState*, my_hash, eqstr>();
}

/*
GroupByHash::GroupByHash(int key_size)
{
  _key_size = key_size;
  _group_hash_map = hash_map<key_type*, GroupByState*, my_hash, eqstr>();
}
*/

pair<key_type*, GroupByState*> GroupByHash::makePair(GroupByState *s)
{
  key_type* new_key = s->getKey();
  return pair<key_type*, GroupByState*> (new_key, s);
}

GroupByState* GroupByHash::getGroupByState(char *value)
{
  key_type* new_key = new key_type(value, _key_size);
  _iter = _group_hash_map.find(new_key);
  delete new_key;
  if (_iter != _group_hash_map.end())
    return (*_iter).second;
  else
    return NULL;
}

void GroupByHash::addGroupByState(GroupByState *s) 
{
  _group_hash_map.insert(makePair(s));
}

void GroupByHash::removeGroupByState(char *value)
{  
  key_type* new_key = new key_type(value, _key_size);
  _group_hash_map.erase(new_key);
  delete new_key;
}

int GroupByHash::getSize()
{
  return _group_hash_map.size();
}

hash_map<key_type*, GroupByState*, my_hash, eqstr>::iterator GroupByHash::begin()
{
  return  _group_hash_map.begin();
}

hash_map<key_type*, GroupByState*, my_hash, eqstr>::iterator GroupByHash::end()
{
  return  _group_hash_map.end();
}

hash_map<key_type*, GroupByState*, my_hash, eqstr> GroupByHash::getGroupByHashMap()
{
  return _group_hash_map;
}

