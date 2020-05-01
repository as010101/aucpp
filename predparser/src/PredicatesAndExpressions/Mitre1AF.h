#ifndef MITRE1AF_H
#define MITRE1AF_H

#include "AggregateFunction.H"
#include "FieldExt.H"

class Mitre1AF : public AggregateFunction
{
public:
  Mitre1AF(const char *att);
  virtual ~Mitre1AF() {};
  void init();
  void incr(char *tuple);
  char* final();
  char* evaluate(char *tuple);
  char* evaluate(char *tuple1, char*tuple2) {};
  int getReturnedSize();
  Mitre1AF* makeNew();

private:
  FieldExt *_field_lat;
  FieldExt *_field_long;
  float _old_com_lat;
  float _old_com_long;
  float _new_com_lat;
  float _new_com_long;
  float _diff_lat;
  float _diff_long;
  char *_time;
  const char *_att;

};

#endif
