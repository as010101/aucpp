#ifndef INTEGER_SUM_AGGREGATE_FUNCTION_H
#define INTEGER_SUM_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class IntSumAF : public AggregateFunction
{
public:  
  IntSumAF(const char *att);
  virtual ~IntSumAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  IntSumAF* makeNew();

private:
  char       *_att;
  int        _sum;
  FieldExt   *_field;
};

#endif 
