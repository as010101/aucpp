#include "SumAF.H"

SumAF::SumAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
}

SumAF::~SumAF() 
{
	delete[] _att;
}

void SumAF::init()
{
  _sum = 0;
}

void SumAF::incr(char *tuple)
{
  float value = *(float*) (_field->evaluate(tuple));
  _sum += value;
}

char* SumAF::final()
{
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = _sum;
  return return_tuple;
}

char* SumAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int SumAF::getReturnedSize()
{
  return (sizeof(float));
}

SumAF* SumAF::makeNew()
{
  return (new SumAF(_att));
}
