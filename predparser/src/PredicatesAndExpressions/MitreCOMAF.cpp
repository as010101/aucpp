#include "MitreCOMAF.H"

MitreCOMAF::MitreCOMAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field_lat = new FieldExt(_att);
  _field_long = new FieldExt(":0:i:8:4:");
}

MitreCOMAF::~MitreCOMAF()
{
}

void MitreCOMAF::init()
{
  _sum_latitude = 0.0;
  _sum_longitude = 0.0;
  _num = 0;
}
 
#include "tupleGenerator.H"

void MitreCOMAF::incr(char *tuple)
{
   _num++;
  float value = *(int*) (_field_lat->evaluate(tuple));
  _sum_latitude += value;

   value = *(int*) (_field_long->evaluate(tuple));
  _sum_longitude += value;


}

char* MitreCOMAF::final()
{
  float average_lat;
  float average_long;
  if (_num == 0) 
    {
      average_lat = 0.0;
      average_long = 0.0;
    }
  else 
    {
      average_lat = _sum_latitude / _num;
      average_long = _sum_longitude / _num;
    }

  char *return_tuple = new char[sizeof(float) + sizeof(float)];
  char *ptr = return_tuple;

  memcpy(ptr, &average_lat, sizeof(float));
  ptr += sizeof(float);
  memcpy(ptr, &average_long, sizeof(float));

  return return_tuple;
}

char* MitreCOMAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int MitreCOMAF::getReturnedSize()
{
  return (sizeof(float) + sizeof(float));
}

MitreCOMAF* MitreCOMAF::makeNew()
{
  return (new MitreCOMAF(_att));
}
