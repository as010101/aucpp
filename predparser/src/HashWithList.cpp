#include "HashWithList.H"

HashWithList::HashWithList()
{
  _hash_map = hash_map<key_type*, List*, my_hash, eqstr>();
}

HashWithList::~HashWithList()
{

}

void HashWithList::setKeySize(int key_size)
{
  _key_size = key_size;
}

List* HashWithList::getList(char *value)
{
  key_type *new_key = new key_type(value, _key_size);
  _iter = _hash_map.find(new_key);
  delete new_key;
  if (_iter != _hash_map.end())
    return (*_iter).second;
  else
    return NULL;
}

void HashWithList::addList(List *l)
{
  _hash_map.insert(pair<key_type*, List*> (l->getKey(), l));
}

void HashWithList::removeList(char *value)
{
  key_type *new_key = new key_type(value, _key_size);
  _hash_map.erase(new_key);
  delete new_key;
}

int HashWithList::getSize()
{
  return _hash_map.size();
}

hash_map<key_type*, List*, my_hash, eqstr>::iterator HashWithList::begin()
{
  return _hash_map.begin();
}

hash_map<key_type*, List*, my_hash, eqstr>::iterator HashWithList::end()
{
  return _hash_map.end();
}
