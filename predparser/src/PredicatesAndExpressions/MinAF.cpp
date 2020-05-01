#include "MinAF.H"

MinAF::MinAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);

  _set_min = false;
}

MinAF::~MinAF() 
{
	delete[] _att;
}

void MinAF::init()
{
  _set_min = false;
  _min = 0;
}

void MinAF::incr(char *tuple)
{
  float tuple_value = *(float*) _field->evaluate(tuple);
  if (!_set_min) {
    _min = tuple_value;
    _set_min = true;
  }
  if (tuple_value < _min)
    _min = tuple_value;
}

char* MinAF::final()
{
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = _min;
  return return_tuple;
}

char* MinAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int MinAF::getReturnedSize()
{
  return (sizeof(float));
}

MinAF* MinAF::makeNew()
{
  return (new MinAF(_att));
}
