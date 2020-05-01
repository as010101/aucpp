#include "TsConstant.H"

TsConstant::TsConstant (Timestamp mem) 
{
  _mem = mem;
  //  _size = (2 * sizeof(long)) + 2 + 1;  // timestamp is two longs
                                       // +2 for "(" and ")"
                                       // +1 for '\0'

  _size = sizeof(timeval);
  _time = new char[_size];

  //  sprintf(_time, "(%d, %d)", _mem.tv_sec, _mem.tv_usec);

  memcpy(_time, &mem, _size);

  //  _time[_size - 1] = '\0';
}

TsConstant::~TsConstant() {}

char* TsConstant::evaluate(char *tuple) 
{
  return _time;
}


char* TsConstant::evaluate(char *tuple1, char *tuple2) 
{
  return _time;
}

int TsConstant::getReturnedSize()
{
  return _size;
}

