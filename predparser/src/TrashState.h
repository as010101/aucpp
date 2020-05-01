#ifndef TRASH_STATE_H
#define TRASH_STATE_H

#include "State.H"

class TrashState
{
public:
  TrashState(char *values, int size, Timestamp timer);
  ~TrashState();
  Timestamp getTimer();
  char* getValues();
  key_type* getKey();
  void setKey(char *key_value, int size);

  void printAll();

private:
  key_type    *_key;
  char        *_values;
  Timestamp   _timer;
}; 

#endif
