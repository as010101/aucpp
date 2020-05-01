#include "FloatDeltaAF.H"

FloatDeltaAF::FloatDeltaAF(const char *att)
{
  _att = new char[strlen(att) + 1];
  strcpy(_att,att);
  _field = new FieldExt(_att);
}

FloatDeltaAF::~FloatDeltaAF() 
{
	delete[] _att;
}

void FloatDeltaAF::init()
{
	_seenFirst = 0;
}

void FloatDeltaAF::incr(char *tuple)
{
	_last =  *(float*) _field->evaluate(tuple);

	if (! _seenFirst)
		{
			_seenFirst = true;
			_first = _last;
		}
}

char* FloatDeltaAF::final()
{
  char *return_tuple = new char[sizeof(float)];
  *(float*)(return_tuple) = _last - _first;
  return return_tuple;
}

char* FloatDeltaAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int FloatDeltaAF::getReturnedSize()
{
  return (sizeof(float));
}

FloatDeltaAF* FloatDeltaAF::makeNew()
{
  return (new FloatDeltaAF(_att));
}
