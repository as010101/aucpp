#include "IntAverageAF.H"

IntAverageAF::IntAverageAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
}

IntAverageAF::~IntAverageAF() 
{
	delete[] _att;
}

void IntAverageAF::init()
{
  _sum = 0;
  _num = 0;
}

void IntAverageAF::incr(char *tuple)
{
  int value =  *(int*) (_field->evaluate(tuple));
  _num++;
  _sum += value;
}

char* IntAverageAF::final()
{
  float average;
  if (_num == 0) average = 0.0;
  else average = (float) _sum / (float) _num;
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = average;
  return return_tuple;

}

char* IntAverageAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int IntAverageAF::getReturnedSize()
{
  return (sizeof(float));
}

IntAverageAF* IntAverageAF::makeNew()
{
  return (new IntAverageAF(_att));
}
