#include "IntMinAF.H"

IntMinAF::IntMinAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
  _set_min = false;
}

IntMinAF::~IntMinAF()
{
  delete[] _att;
}

void IntMinAF::init()
{
  _set_min = false;
  _min = 0;
}

void IntMinAF::incr(char *tuple)
{
  int tuple_value = *(int*) _field->evaluate(tuple);
  if (!_set_min) {
    _min = tuple_value;
    _set_min = true;
  }
  if (tuple_value < _min)
    _min = tuple_value;
}

char* IntMinAF::final()
{
  char *return_tuple = new char[sizeof(int)];
  *(int*)(return_tuple) = _min;
  return return_tuple;
}

char* IntMinAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int IntMinAF::getReturnedSize()
{
  return (sizeof(int));
}

IntMinAF* IntMinAF::makeNew()
{
  return (new IntMinAF(_att));
}
