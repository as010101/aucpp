#include "FloatMaxAF.H"

FloatMaxAF::FloatMaxAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);

  _set_max = false;
}

FloatMaxAF::~FloatMaxAF() 
{
	delete[] _att;
}

void FloatMaxAF::init()
{
  _set_max = false;
  _max = 0.00;
}

void FloatMaxAF::incr(char *tuple)
{
  float tuple_value = *(float*) _field->evaluate(tuple);
  if (!_set_max) {
    _max = tuple_value;
    _set_max = true;
  }
  else if (tuple_value > _max)
    _max = tuple_value;
}

char* FloatMaxAF::final()
{
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = _max;
  return return_tuple;
}

char* FloatMaxAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int FloatMaxAF::getReturnedSize()
{
  return (sizeof(float));
}

FloatMaxAF* FloatMaxAF::makeNew()
{
  return (new FloatMaxAF(_att));
}
