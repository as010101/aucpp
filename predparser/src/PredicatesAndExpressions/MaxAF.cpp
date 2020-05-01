#include "MaxAF.H"

MaxAF::MaxAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);

  _set_max = false;
}

MaxAF::~MaxAF() 
{
	delete[] _att;
}

void MaxAF::init()
{
  _set_max = false;
  _max = 0;
}

void MaxAF::incr(char *tuple)
{
  float tuple_value = *(float*) _field->evaluate(tuple);
  if (!_set_max) {
    _max = tuple_value;
    _set_max = true;
  }
  else if (tuple_value > _max)
    _max = tuple_value;
}

char* MaxAF::final()
{
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = _max;
  return return_tuple;
}

char* MaxAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int MaxAF::getReturnedSize()
{
  return (sizeof(float));
}

MaxAF* MaxAF::makeNew()
{
  return (new MaxAF(_att));
}
