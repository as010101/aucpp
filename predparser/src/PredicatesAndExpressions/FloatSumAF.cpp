#include "FloatSumAF.H"

FloatSumAF::FloatSumAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
}

FloatSumAF::~FloatSumAF() 
{
	delete[] _att;
}

void FloatSumAF::init()
{
  _sum = 0;
}

void FloatSumAF::incr(char *tuple)
{
  float value = *(float*) (_field->evaluate(tuple));
  _sum += value;
}

char* FloatSumAF::final()
{
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = _sum;
  return return_tuple;
}

char* FloatSumAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int FloatSumAF::getReturnedSize()
{
  return (sizeof(float));
}

FloatSumAF* FloatSumAF::makeNew()
{
  return (new FloatSumAF(_att));
}
