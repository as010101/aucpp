#ifndef FLOAT_MIN_AGGREGATE_FUNCTION_H
#define FLOAT_MIN_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class FloatMinAF : public AggregateFunction
{
public:  
  FloatMinAF(const char *att);
  virtual ~FloatMinAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  FloatMinAF* makeNew();

private:
  char       *_att;
  FieldExt   *_field;
  float      _min;
  bool       _set_min;
};

#endif 
