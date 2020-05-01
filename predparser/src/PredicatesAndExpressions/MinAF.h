#ifndef MIN_AGGREGATE_FUNCTION_H
#define MIN_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class MinAF : public AggregateFunction
{
public:  
  MinAF(const char *att);
  virtual ~MinAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  MinAF* makeNew();

private:
  char       *_att;
  FieldExt   *_field;
  float      _min;
  bool       _set_min;
};

#endif 
