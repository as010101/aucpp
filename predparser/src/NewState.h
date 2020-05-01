#ifndef NEW_STATE_H
#define NEW_STATE_H

#include "AggregateFunction.H"
#include "Timestamp.H"

class NewState
{
public:

  NewState(char *value, char type, char* value_str, AggregateFunction *af, Timestamp ts);
  ~NewState() {};

  char* getValue();
  char* getValueStr();
  AggregateFunction* getAF();
  Timestamp getTS();
  int getValueSize();
  int getValueStrLength();
  Timestamp getTo();
  void setTo(Timestamp new_ts);

private:
  char                 *_value;   // can be int or timestamp only
  char                 _value_type;
  int                  _value_size;
  char* _value_str; // c-string of value

  AggregateFunction    *_af;
  Timestamp            _ts;

  Timestamp            _to;

};

#endif

