#ifndef SUM_AGGREGATE_FUNCTION_H
#define SUM_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class SumAF : public AggregateFunction
{
public:  
  SumAF(const char *att);
  virtual ~SumAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  SumAF* makeNew();

private:
  char       *_att;
  float      _sum;
  FieldExt   *_field;
};

#endif 
