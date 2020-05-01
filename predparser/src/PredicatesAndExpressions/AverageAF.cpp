#include "AverageAF.H"

AverageAF::AverageAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
}

AverageAF::~AverageAF()
{
	delete[] _att;
}

void AverageAF::init()
{
  _sum = 0;
  _num = 0;
}

void AverageAF::incr(char *tuple)
{
  float value = *(float*) (_field->evaluate(tuple));
  _num++;
  _sum += value;
}

char* AverageAF::final()
{
  float average;
  if (_num == 0) average = 0.0;
  else average = _sum / _num;
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = average;
  return return_tuple;
}

char* AverageAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int AverageAF::getReturnedSize()
{
  return (sizeof(float));
}

AverageAF* AverageAF::makeNew()
{
  return (new AverageAF(_att));
}
