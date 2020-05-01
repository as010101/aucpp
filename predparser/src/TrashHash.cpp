#include "TrashHash.H"

TrashHash::TrashHash()
{
  _trash_hash_map = hash_map<key_type*, TrashState*, my_hash, eqstr>();
}

TrashHash::~TrashHash()
{
  _trash_hash_map.clear();
}

void TrashHash::setKeySize(int key_size)
{
  _key_size = key_size;
}

pair<key_type*, TrashState*> TrashHash::makePair(TrashState *s)
{
  //key_type* new_key = s->getKey();
  return pair<key_type*, TrashState*> (s->getKey(), s);
}

TrashState* TrashHash::getTrashState(char *value)
{
  key_type* new_key = new key_type(value, _key_size);
  _iter = _trash_hash_map.find(new_key);
  delete new_key;
  if (_iter != _trash_hash_map.end())
    return (*_iter).second;
  else
    return NULL;
}

void TrashHash::addTrashState(TrashState *s) 
{
  _trash_hash_map.insert(makePair(s));
}

void TrashHash::removeTrashState(char *value)
{  

  key_type* new_key = new key_type(value, _key_size);
  _trash_hash_map.erase(new_key);
  delete new_key;
}

int TrashHash::getSize()
{
  return _trash_hash_map.size();
}

hash_map<key_type*, TrashState*, my_hash, eqstr>::iterator TrashHash::begin()
{
  return  _trash_hash_map.begin();
}

hash_map<key_type*, TrashState*, my_hash, eqstr>::iterator TrashHash::end()
{
  return  _trash_hash_map.end();
}

// just for debugging
void TrashHash::printAll()
{
  _iter = _trash_hash_map.begin();
  int size = _trash_hash_map.size();;
  int counter = 0;
  while (counter < size) 
    {
      counter++;
      (*_iter).second->printAll();      
      _iter++;   
 }
}

