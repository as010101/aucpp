#ifndef SEG3AF_H
#define SEG3AF_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class Seg3AF : public AggregateFunction {
public:
  Seg3AF(const char *att);
  virtual ~Seg3AF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  Seg3AF* makeNew();
private:
  unsigned int          _sum;
  unsigned int          _lav;
  unsigned int          _cnt;
};

#endif //  SEG1AF_H
