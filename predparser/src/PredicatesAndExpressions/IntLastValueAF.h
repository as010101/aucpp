#ifndef INT_LAST_VALUE_AGGREGATE_FUNCTION_H
#define INT_LAST_VALUE_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class IntLastValueAF : public AggregateFunction
{
public:  
  IntLastValueAF(const char *att);
  virtual ~IntLastValueAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  IntLastValueAF* makeNew();

private:
  char *_att;
  FieldExt   *_field;
  int        _last;

};

#endif 
