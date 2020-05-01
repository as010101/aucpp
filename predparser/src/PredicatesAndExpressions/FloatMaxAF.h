#ifndef FLOAT_MAX_AGGREGATE_FUNCTION_H
#define FLOAT_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class FloatMaxAF : public AggregateFunction
{
public:  
  FloatMaxAF(const char *att);
  virtual ~FloatMaxAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  FloatMaxAF* makeNew();

private:
  char       *_att;
  FieldExt   *_field;
  float      _max;
  bool       _set_max;
};

#endif 
