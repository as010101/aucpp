#ifndef STATE_H
#define STATE_H

#include "AggregateFunction.H"
#include "PredicatesAndExpressions/Timestamp.H"

enum status 
{ 
  DONE, ACTIVE
};

class key_type
{
public:
  key_type() {
    key = NULL;
    key_size = 0;
  };
  key_type(const char* values, int size) {
    key = new char[size];
    memcpy(key, values, size);
    key_size = size;
  };
  ~key_type() {
    delete[] key;
    key = NULL;
  };
  char* key;
  int key_size;
}; 

//typedef int Timestamp;


class State
{
public:
  State(const char *sid, int sid_size, char *att_values, int att_values_size, AggregateFunction *af, Timestamp to, Timestamp tn);
  ~State();

  char* getAttributeVals();
  key_type* getSid();
  status getStatus();
  AggregateFunction* getAF();
  Timestamp getTo();
  Timestamp getTn();
  Timestamp getTimer();
  key_type* getKey();
  bool _key_set;

  void setAttributeVals(char* att_values);
  void setAF(AggregateFunction *af);
  void setStatus(status st);
  void setTo(Timestamp to);
  void setTn(Timestamp tn);
  void setTimer(Timestamp t);
  void setKey(char *key_value, int size);
  bool keyEqual(key_type* other);

  void printAll();

private:
  key_type            *_key;
  key_type            *_sid;
  char                *_attribute_values;
  AggregateFunction   *_af;
  status              _status;
  Timestamp           _to;
  Timestamp           _tn;
  Timestamp           _timer;
};

#endif
