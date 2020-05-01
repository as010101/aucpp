#ifndef FLOAT_AVERAGE_AGGREGATE_FUNCTION_H
#define FLOAT_AVERAGE_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class FloatAverageAF : public AggregateFunction
{
public:  
  FloatAverageAF(const char *att);
  virtual ~FloatAverageAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  FloatAverageAF* makeNew();

private:
  char        *_att;
  float       _sum;
  int         _num;
  FieldExt    *_field;
};

#endif 
