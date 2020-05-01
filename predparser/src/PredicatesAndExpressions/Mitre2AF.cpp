#include "Mitre2AF.H"

Mitre2AF::Mitre2AF(const char *att)
{
  _att = new char[strlen(att)+1];
  memcpy(_att,att,strlen(att)+1);
  _field = new FieldExt(att);
  _prn_field = new FieldExt(":0:i:0:4:");
  _angle = 0.00;
}

void Mitre2AF::init()
{
  _angle = 0.00;
}

void Mitre2AF::incr(char *tuple)
{
  _angle = *(float*) _field->evaluate(tuple);
  _prn = _prn_field->evaluate(tuple);
  _ts = new char[sizeof(timeval)];
  memcpy(_ts,tuple+12+14,sizeof(timeval));
}

char* Mitre2AF::final()
{
  char *result = new char[sizeof(int) + sizeof(timeval) + sizeof(float) + sizeof(float) + sizeof(int)];
  char *ptr = result;

  float lower = tan((90 - _angle) - MITRE2_RANGE_ANGLE);
  float upper = tan((90 - _angle) + MITRE2_RANGE_ANGLE);

  int dir;
  if (_angle <= 90 || _angle >= 270)
    dir = 1;    // UP == 1
  else
    dir = 0;   // DOWN == 0

  memcpy(ptr, _prn, sizeof(int));
  ptr += sizeof(int);
  memcpy(ptr,_ts,sizeof(timeval));
  ptr += sizeof(timeval);
  memcpy(ptr, &lower, sizeof(float));
  ptr += sizeof(float);
  memcpy(ptr, &upper, sizeof(float));
  ptr+= sizeof(float);
  memcpy(ptr, &dir, sizeof(int));
  
  return result;

}

char* Mitre2AF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int Mitre2AF::getReturnedSize()
{
  return (sizeof(int) + sizeof(timeval) + sizeof(float) + sizeof(float) + sizeof(int));
}

Mitre2AF* Mitre2AF::makeNew()
{
  return (new Mitre2AF(_att));
}
