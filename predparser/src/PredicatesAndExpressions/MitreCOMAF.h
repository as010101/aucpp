#ifndef CENTER_OF_MASS_AGGREGATE_FUNCTION_H
#define CENTER_OF_MASS_AGGREGATE_FUNCTION_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class MitreCOMAF : public AggregateFunction
{
public:  
  MitreCOMAF(const char *att);
  virtual ~MitreCOMAF();
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char *tuple2) {};
  int getReturnedSize();
  MitreCOMAF* makeNew();

private:
  char        *_att;
  float       _sum_latitude;
  float       _sum_longitude;
  int         _num;
  FieldExt    *_field_lat;
  FieldExt    *_field_long;
};

#endif 
