#include "TrashState.H"
#include "tupleGenerator.H"

TrashState::TrashState(char *values, int size, Timestamp timer)
{
  _values = new char[size];
  memcpy(_values, values, size);

  _key = new key_type();
  _key->key = new char[size];
  memcpy(_key->key, values, size);
  _key->key_size = size;

  _timer = timer;
}

TrashState::~TrashState()
{
  delete _key;
  //Figure out later delete _key;
  delete[] _values;
  //delete &_timer; 
}

Timestamp TrashState::getTimer()
{
  return _timer;
}

char* TrashState::getValues()
{
  return _values;
}

key_type* TrashState::getKey()
{
  return _key;
}

void TrashState::setKey(char *key_value, int size)
{
  if (_key->key)
    delete[] _key->key;
  _key->key = new char[size];
  memcpy(_key->key, key_value, size);
  _key->key_size = size;
}

//just for debugging
void TrashState::printAll()
{
  printTuple(_key->key, "ii");
  cout << "   _timer = " << _timer.tv_sec << endl;
}
