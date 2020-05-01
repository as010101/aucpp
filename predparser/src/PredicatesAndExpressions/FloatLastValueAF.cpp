#include "FloatLastValueAF.H"

FloatLastValueAF::FloatLastValueAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
}

FloatLastValueAF::~FloatLastValueAF() 
{
	delete[] _att;
}

void FloatLastValueAF::init()
{
  _last = 0.00;
}

void FloatLastValueAF::incr(char *tuple)
{
  float tuple_value = *(float*) _field->evaluate(tuple);
  _last = tuple_value;
}

char* FloatLastValueAF::final()
{
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = _last;
  return return_tuple;
}

char* FloatLastValueAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int FloatLastValueAF::getReturnedSize()
{
  return (sizeof(float));
}

FloatLastValueAF* FloatLastValueAF::makeNew()
{
  return (new FloatLastValueAF(_att));
}
