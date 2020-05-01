#ifndef INTEGER_MIN_AGGREGATE_FUNCTION_H
#define INTEGER_MIN_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class IntMinAF : public AggregateFunction
{
public:  
  IntMinAF(const char *att);
  virtual ~IntMinAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  IntMinAF* makeNew();

private:
  char       *_att;
  FieldExt   *_field;
  int        _min;
  bool       _set_min;
};

#endif 
