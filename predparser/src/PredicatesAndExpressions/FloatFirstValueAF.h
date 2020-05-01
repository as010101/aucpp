#ifndef FLOAT_FIRST_VALUE_AGGREGATE_FUNCTION_H
#define FLOAT_FIRST_VALUE_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class FloatFirstValueAF : public AggregateFunction
{
public:  
  FloatFirstValueAF(const char *att);
  virtual ~FloatFirstValueAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  FloatFirstValueAF* makeNew();

private:
  char       *_att;
  FieldExt   *_field;

  float      _first;
	bool       _seenFirst;
};

#endif 
