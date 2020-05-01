#ifndef FLOAT_SUM_AGGREGATE_FUNCTION_H
#define FLOAT_SUM_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class FloatSumAF : public AggregateFunction
{
public:  
  FloatSumAF(const char *att);
  virtual ~FloatSumAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  FloatSumAF* makeNew();

private:
  char       *_att;
  float      _sum;
  FieldExt   *_field;
};

#endif 
