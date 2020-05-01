#ifndef MAX_AGGREGATE_FUNCTION_H
#define MAX_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class MaxAF : public AggregateFunction
{
public:  
  MaxAF(const char *att);
  virtual ~MaxAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  MaxAF* makeNew();

private:
  char       *_att;
  FieldExt   *_field;
  float      _max;
  bool       _set_max;
};

#endif 
