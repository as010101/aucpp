#include "FloatFirstValueAF.H"

FloatFirstValueAF::FloatFirstValueAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
}

FloatFirstValueAF::~FloatFirstValueAF() 
{
	delete[] _att;
}

void FloatFirstValueAF::init()
{
	_seenFirst = 0;
}

void FloatFirstValueAF::incr(char *tuple)
{
	if (! _seenFirst)
		{
			_seenFirst = true;
			_first =  *(float*) _field->evaluate(tuple);
		}
}

char* FloatFirstValueAF::final()
{
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = _first;
  return return_tuple;
}

char* FloatFirstValueAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int FloatFirstValueAF::getReturnedSize()
{
  return (sizeof(float));
}

FloatFirstValueAF* FloatFirstValueAF::makeNew()
{
  return (new FloatFirstValueAF(_att));
}
