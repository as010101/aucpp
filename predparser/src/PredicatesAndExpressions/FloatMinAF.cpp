#include "FloatMinAF.H"

FloatMinAF::FloatMinAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);

  _set_min = false;
}

FloatMinAF::~FloatMinAF() 
{
	delete[] _att;
}

void FloatMinAF::init()
{
  _set_min = false;
  _min = 0.00;
}

void FloatMinAF::incr(char *tuple)
{
  float tuple_value = *(float*) _field->evaluate(tuple);
  if (!_set_min) {
    _min = tuple_value;
    _set_min = true;
  }
  if (tuple_value < _min)
    _min = tuple_value;
}

char* FloatMinAF::final()
{
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = _min;
  return return_tuple;
}

char* FloatMinAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int FloatMinAF::getReturnedSize()
{
  return (sizeof(float));
}

FloatMinAF* FloatMinAF::makeNew()
{
  return (new FloatMinAF(_att));
}
