#include "State.H"
#include "tupleGenerator.H"

State::State(const char *sid, int sid_size, char *att_values, int att_values_size, AggregateFunction *af, Timestamp to, Timestamp tn)
{
  _sid = new key_type(sid, sid_size);
  _attribute_values = new char[att_values_size];
  memcpy(_attribute_values, att_values, att_values_size);

  _key = new key_type();
  _key_set = 0;
  _af = af;
  _to = to;
  _tn = tn;
  _timer = *(new Timestamp()); 
}

State::~State()
{
  delete _key;
  delete _sid;
  delete[] _attribute_values;
  delete _af;
  //delete &_to;
  //delete &_tn;
}

char* State::getAttributeVals()
{
  return _attribute_values;
}

key_type* State::getSid()
{
  return _sid;
}

status State::getStatus()
{
  return _status;
}

AggregateFunction* State::getAF()
{
  return _af;
}

Timestamp State::getTo()
{
  return _to;
}

Timestamp State::getTn()
{
  return _tn;
}

Timestamp State::getTimer()
{
  return _timer;
}

key_type* State::getKey()
{
  return _key;
}

void State::setAttributeVals(char *att_values)
{
  _attribute_values = att_values;
}

void State::setAF(AggregateFunction *af)
{
  _af = af;
}

void State::setStatus(status st)
{
  _status = st;
}

void State::setTo(Timestamp to)
{
  _to = to;
}

void State::setTn(Timestamp tn)
{
  _tn = tn;
}

void State::setTimer(Timestamp t)
{
  _timer = t;
}

void State::setKey(char *key_value, int size)
{
  if (!_key_set) {
    _key->key = new char[size];
    memcpy(_key->key, key_value, size);
    _key->key_size = size;
    _key_set = 1;
  }
}
bool State::keyEqual(key_type* other)
{
  if (_key->key_size != other->key_size)
    return 0;
  return memcmp(_key->key, other->key, _key->key_size) == 0;
} 


//just for debugging
void State::printAll()
{
  cout << "\tsid ";
  printTuple(_sid->key, "i");
  cout << "\tatt ";
  printTuple(_attribute_values, "i");
  //cout << " _to = " << _to << "\n";
  cout << "\t_timer = " << _timer.tv_sec << "\n";  // toStringlateron
}
