#include "SortList.H"

SortList::SortList()
{
  _buffer = list<buffer_node>();
}

SortList::~SortList()
{
}

int SortList::size()
{
  return _buffer.size();
}

list<buffer_node>::iterator SortList::end()
{
  return _buffer.end();
}

list<buffer_node>::iterator SortList::begin()
{
  return _buffer.begin();
}

buffer_node SortList::front()
{
  return _buffer.front();
}

void SortList::push_front(buffer_node new_node)
{
    _buffer.push_front(new_node);
}

void SortList::pop_front()
{
    _buffer.pop_front();
}

buffer_node SortList::back()
{
  return _buffer.back();
}

void SortList::push_back(buffer_node new_node)
{
    _buffer.push_back(new_node);
}

void SortList::insert(list<buffer_node>::iterator iter, buffer_node node)
{
  _buffer.insert(iter, node);
}

/*
void SortList::setKeySize(int key_size)
{
  _key_size = key_size;
}


State* SortList::getState(char *value)
{
  key_type* new_key = new key_type(value, _key_size);
  _iter = _hash_map.find(new_key);
  delete new_key;
  if (_iter != _hash_map.end())
    return (*_iter).second;
  else
    return NULL;
}

void SortList::changeState(State *s)
{
  key_type* temp_key = s->getSid();
  removeState(temp_key->key);
  addState(s, temp_key->key, 0);
  //_iter = _hash_map.find(s->getSid());
  //if (_iter != _hash_map.end())
  //  (*_iter).second = s;
}

void SortList::addState(State *s, char *value, bool create_key) 
{
  if (create_key) {
    s->setKey(value, _key_size);
    _hash_map.insert(pair<key_type*, State*> (s->getKey(), s));
  }
  else
    _hash_map.insert(pair<key_type*, State*> (s->getSid(), s));
    

}

void SortList::removeState(char *value)
{  
  key_type* new_key = new key_type(value, _key_size);
  _hash_map.erase(new_key);
  delete new_key;
}

int SortList::getSize()
{
  return _hash_map.size();
}

hash_map<key_type*, State*, my_hash, eqstr>::iterator SortList::begin()
{
  return _hash_map.begin();
}

hash_map<key_type*, State*, my_hash, eqstr>::iterator SortList::end()
{
  return _hash_map.end();
}

hash_map<key_type*, State*, my_hash, eqstr> SortList::getSortListMap()
{
  return _hash_map;
}



// just for debugging
void SortList::printAll()
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
*/
