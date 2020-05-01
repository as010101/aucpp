#ifndef ACC2AF_H
#define ACC2AF_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class Acc2AF : public AggregateFunction {
public:
  Acc2AF(const char *att);
  virtual ~Acc2AF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  Acc2AF* makeNew();
private:
  bool _first;
  bool _acc;
  int _lane;
  int _car;
};

#endif //  ACC2AF_H
