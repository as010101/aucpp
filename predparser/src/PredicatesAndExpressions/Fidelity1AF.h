#ifndef FIDELITY1AF_H
#define FIDELITY1AF_H

#include "AggregateFunction.H"
#include "FieldExt.H"
#include <iostream>

class Fidelity1AF : public AggregateFunction {
public:
  Fidelity1AF(const char *att);
  virtual ~Fidelity1AF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  Fidelity1AF* makeNew();
private:
  unsigned int          _cnt;
  char* feed_exchange; // store the feed and exchange
};

#endif //  FIDELITY1AF_H
