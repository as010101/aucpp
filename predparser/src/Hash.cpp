#include "Hash.H"

Hash::Hash()
{
  _hash_map = hash_map<key_type*, State*, my_hash, eqstr>();
}

Hash::~Hash()
{
}

void Hash::setKeySize(int key_size)
{
  _key_size = key_size;
}


State* Hash::getState(char *value)
{
  key_type* new_key = new key_type(value, _key_size);
  _iter = _hash_map.find(new_key);
  delete new_key;
  if (_iter != _hash_map.end())
    return (*_iter).second;
  else
    return NULL;
}

void Hash::changeState(State *s)
{
  key_type* temp_key = s->getSid();
  removeState(temp_key->key);
  addState(s, temp_key->key, 0);
  //_iter = _hash_map.find(s->getSid());
  //if (_iter != _hash_map.end())
  //  (*_iter).second = s;
}

void Hash::addState(State *s, char *value, bool create_key) 
{
  if (create_key) {
    s->setKey(value, _key_size);
    _hash_map.insert(pair<key_type*, State*> (s->getKey(), s));
  }
  else
    _hash_map.insert(pair<key_type*, State*> (s->getSid(), s));
    

}

void Hash::removeState(char *value)
{  
  key_type* new_key = new key_type(value, _key_size);
  _hash_map.erase(new_key);
  delete new_key;
}

int Hash::getSize()
{
  return _hash_map.size();
}

hash_map<key_type*, State*, my_hash, eqstr>::iterator Hash::begin()
{
  return _hash_map.begin();
}

hash_map<key_type*, State*, my_hash, eqstr>::iterator Hash::end()
{
  return _hash_map.end();
}

hash_map<key_type*, State*, my_hash, eqstr> Hash::getHashMap()
{
  return _hash_map;
}



// just for debugging
void Hash::printAll()
{
  _iter = _hash_map.begin();
  int size = _hash_map.size();;
  int counter = 0;
  while (counter < size) 
    {
      counter++;
      (*_iter).second->printAll();      
      _iter++;   
    }
}

