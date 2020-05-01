#include "FloatAverageAF.H"

FloatAverageAF::FloatAverageAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
}

FloatAverageAF::~FloatAverageAF()
{
	delete[] _att;
}

void FloatAverageAF::init()
{
  _sum = 0.00;
  _num = 0;
}

void FloatAverageAF::incr(char *tuple)
{
  float value = *(float*) (_field->evaluate(tuple));
  _num++;
  _sum += value;
}

char* FloatAverageAF::final()
{
  float average;
  if (_num == 0) average = 0.0;
  else average = _sum / (float) _num;
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = average;
  return return_tuple;

}

char* FloatAverageAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int FloatAverageAF::getReturnedSize()
{
  return (sizeof(float));
}

FloatAverageAF* FloatAverageAF::makeNew()
{
  return (new FloatAverageAF(_att));
}
