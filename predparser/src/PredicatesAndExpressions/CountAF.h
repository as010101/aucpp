#ifndef COUNTAF_H
#define COUNTAF_H

#include "AggregateFunction.H"

class CountAF : public AggregateFunction
{
public:
  CountAF();
  virtual ~CountAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  CountAF* makeNew();

private:
  int   _cnt;
};

#endif 
