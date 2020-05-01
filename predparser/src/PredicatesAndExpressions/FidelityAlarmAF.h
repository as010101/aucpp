#ifndef FIDELITYAlarmAF_H
#define FIDELITYAlarmAF_H

#include "AggregateFunction.H"
#include "FieldExt.H"
#include <iostream>

class FidelityAlarmAF : public AggregateFunction {
public:
  FidelityAlarmAF(const char *att);
  virtual ~FidelityAlarmAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  FidelityAlarmAF* makeNew();
private:
  char* att_store; // to remember the att passed
  int shm_key;
};

#endif //  FIDELITYAlarmAF_H
