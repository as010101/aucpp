#ifndef SEG2AF_H
#define SEG2AF_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class Seg2AF : public AggregateFunction {
public:
  Seg2AF(const char *att);
  virtual ~Seg2AF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  Seg2AF* makeNew();
private:
  unsigned int          _cnt;
  unsigned int          _sum;
  unsigned int          _totcnt;
};

#endif //  SEG1AF_H
