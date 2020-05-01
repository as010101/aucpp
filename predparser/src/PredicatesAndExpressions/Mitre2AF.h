#ifndef MITRE2AF_H
#define MITRE2AF_H

#include "AggregateFunction.H"
#include "FieldExt.H"
#include <math.h>

#define MITRE2_RANGE_ANGLE 20
class Mitre2AF : public AggregateFunction
{
public:
  Mitre2AF(const char *att);
  virtual ~Mitre2AF() {};
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char*tuple2) {};
  int getReturnedSize();
  Mitre2AF* makeNew();

private:
  FieldExt *_field;
  FieldExt *_prn_field;
  char *_att;
  float    _angle;
  char *_prn;
  char *_ts;
};

#endif
