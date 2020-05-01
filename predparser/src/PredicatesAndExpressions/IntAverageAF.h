#ifndef INTEGER_AVERAGE_AGGREGATE_FUNCTION_H
#define INTEGER_AVERAGE_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class IntAverageAF : public AggregateFunction
{
public:  
  IntAverageAF(const char *att);
  virtual ~IntAverageAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  IntAverageAF* makeNew();

private:
  char        *_att;
  int          _sum;
  int          _num;
  FieldExt    *_field;
};

#endif 
