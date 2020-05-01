#include "NewState.H"
#include <iostream>

// value_str BETTER BE A NUL TERMINATED C STRING!
NewState::NewState(char *value, char type, char* value_str, AggregateFunction *af, Timestamp ts)
{

  _value_size = 0;
  if (type == 'i')
    {
      _value_size = sizeof(int);
    }
  else
    {
      _value_size = sizeof(timeval);
    }

  _value = new char[_value_size];
  memcpy(_value, value, _value_size);    // the value can be INT and Timestamp only
  _value_type = type;
  _value_str = (char*) malloc(strlen(value_str) + 1);
  _value_str = strcpy(_value_str,value_str);
  _af = af;
  _ts = ts;

}

char* NewState::getValue()
{
  return _value;
}

char* NewState::getValueStr() {
  return _value_str;
}

AggregateFunction* NewState::getAF()
{
  return _af;
}

Timestamp NewState::getTS()
{
  return _ts;
}

int NewState::getValueSize()
{
  return _value_size;
}

int NewState::getValueStrLength()
{
  return strlen(_value_str);
}

Timestamp NewState::getTo()
{
  return _to;
}

void NewState::setTo(Timestamp new_ts)
{
  _to = new_ts;
}

