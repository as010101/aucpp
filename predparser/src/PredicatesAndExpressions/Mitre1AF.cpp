#include "Mitre1AF.H"

Mitre1AF::Mitre1AF(const char *att)
{
  _field_lat = new FieldExt(att);
  _field_long = new FieldExt(":0:f:12:4:");   // get the timestamp from the tuple also
  _old_com_lat = 0;  
  _old_com_long = 0;
  _new_com_lat = 0;
  _new_com_long = 0;
  _diff_lat = 0;
  _diff_long = 0;
  _att = att;
}

void Mitre1AF::init()
{
  _old_com_lat = 0;  
  _old_com_long = 0;
  _new_com_lat = 0;
  _new_com_long = 0;
  _diff_lat = 0;
  _diff_long = 0;
}

void Mitre1AF::incr(char *tuple)
{
  _old_com_lat = _new_com_lat;
  _old_com_long = _new_com_long;

  _new_com_lat = *(float*) _field_lat->evaluate(tuple);
  _new_com_long = *(float*) _field_long->evaluate(tuple);

}

char* Mitre1AF::final()
{
  _diff_lat = fabs(_new_com_lat - _old_com_lat);
  _diff_long = fabs(_new_com_long - _old_com_long);

  char *result = new char[sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float)];
  char *ptr = result;

  //  //copy timestamp
  // memcpy(ptr, _time, sizeof(timeval));
  //ptr += sizeof(timeval);

  //copy the COM
  memcpy(ptr, &_new_com_lat, sizeof(float));
  ptr += sizeof(float);
  memcpy(ptr, &_new_com_long, sizeof(float));
  ptr += sizeof(float);

  //copy the difference
  memcpy(ptr, &_diff_lat, sizeof(float));
  ptr += sizeof(float);
  memcpy(ptr, &_diff_long, sizeof(float));

  return result;
}

char* Mitre1AF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int Mitre1AF::getReturnedSize()
{
  return (sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float));
}

Mitre1AF* Mitre1AF::makeNew()
{
  return (new Mitre1AF(_att));
}
