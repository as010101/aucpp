#include "IntSumAF.H"

IntSumAF::IntSumAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
}

IntSumAF::~IntSumAF()
{
  delete[] _att;
}

void IntSumAF::init()
{
  _sum = 0;
}

void IntSumAF::incr(char *tuple)
{
  int value = *(int*) (_field->evaluate(tuple));
  _sum += value;
}

char* IntSumAF::final()
{
  char *return_tuple = new char[sizeof(int)];
  *(int*)(return_tuple) = _sum;
  return return_tuple;
}

char* IntSumAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int IntSumAF::getReturnedSize()
{
  return (sizeof(int));
}

IntSumAF* IntSumAF::makeNew()
{
  return (new IntSumAF(_att));
}
