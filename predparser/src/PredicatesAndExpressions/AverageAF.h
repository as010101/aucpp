#ifndef AVERAGE_AGGREGATE_FUNCTION_H
#define AVERAGE_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class AverageAF : public AggregateFunction
{
public:  
  AverageAF(const char *att);
  virtual ~AverageAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  AverageAF* makeNew();

private:
  char        *_att;
  float       _sum;
  int         _num;
  FieldExt    *_field;
};

#endif 
