#include "IntLastValueAF.H"

IntLastValueAF::IntLastValueAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
}

IntLastValueAF::~IntLastValueAF()
{
  delete[] _att;
}

void IntLastValueAF::init()
{
  _last = 0;
}

void IntLastValueAF::incr(char *tuple)
{
  int tuple_value = *(int*) _field->evaluate(tuple);
  _last = tuple_value;
}

char* IntLastValueAF::final()
{
  char *return_tuple = new char[sizeof(int)];
  *(int*)(return_tuple) = _last;
  return return_tuple;
}

char* IntLastValueAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int IntLastValueAF::getReturnedSize()
{
  return (sizeof(int));
}

IntLastValueAF* IntLastValueAF::makeNew()
{
  return (new IntLastValueAF(_att));
}
