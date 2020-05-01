#ifndef FIDELITY2AF_H
#define FIDELITY2AF_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class Fidelity2AF : public AggregateFunction {
public:
  Fidelity2AF(const char *att);
  virtual ~Fidelity2AF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  Fidelity2AF* makeNew();
private:
  unsigned int          _cnt;
};

#endif //  FIDELITY2AF_H
