#ifndef SEG1AF_H
#define SEG1AF_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class Seg1AF : public AggregateFunction {
public:
  Seg1AF(const char *att);
  virtual ~Seg1AF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  Seg1AF* makeNew();
private:
  unsigned int          _cnt;
  unsigned int          _sum;
};

#endif //  SEG1AF_H
