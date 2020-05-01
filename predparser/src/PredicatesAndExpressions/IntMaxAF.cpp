#include "IntMaxAF.H"

IntMaxAF::IntMaxAF(const char *att)
{
  _att = att;
  _field = new FieldExt(att);
  _set_max = false;
}

IntMaxAF::~IntMaxAF() {}

void IntMaxAF::init()
{
  _set_max = false;
  _max = 0;
}

void IntMaxAF::incr(char *tuple)
{
  int tuple_value = *(int*) _field->evaluate(tuple);
  if (!_set_max) {
    _max = tuple_value;
    _set_max = true;
  }
  else if (tuple_value > _max)
    _max = tuple_value;
}

char* IntMaxAF::final()
{
  char *return_tuple = new char[sizeof(int)];
  *(int*)(return_tuple) = _max;
  return return_tuple;
}

char* IntMaxAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int IntMaxAF::getReturnedSize()
{
  return (sizeof(int));
}

IntMaxAF* IntMaxAF::makeNew()
{
  return (new IntMaxAF(_att));
}
