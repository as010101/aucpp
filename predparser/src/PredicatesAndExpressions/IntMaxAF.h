#ifndef INT_MAX_AGGREGATE_FUNCTION_H
#define INT_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class IntMaxAF : public AggregateFunction
{
public:  
  IntMaxAF(const char *att);
  virtual ~IntMaxAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  IntMaxAF* makeNew();

private:
  const char *_att;
  FieldExt   *_field;
  int        _max;
  bool       _set_max;
};

#endif 
