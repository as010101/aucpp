#ifndef FLOAT_LAST_VALUE_AGGREGATE_FUNCTION_H
#define FLOAT_LAST_VALUE_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class FloatLastValueAF : public AggregateFunction
{
public:  
  FloatLastValueAF(const char *att);
  virtual ~FloatLastValueAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  FloatLastValueAF* makeNew();

private:
  char       *_att;
  FieldExt   *_field;
  float      _last;

};

#endif 
