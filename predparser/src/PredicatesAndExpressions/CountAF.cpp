#include "CountAF.H"


CountAF::CountAF()
{
}

CountAF::~CountAF()
{
}

void CountAF::init()
{
  _cnt = 0;
}

void CountAF::incr(char *tuple)
{
  _cnt++;
}

char* CountAF::final()
{
  char *return_tuple = new char[sizeof(int)];
  *(int*)(return_tuple) = _cnt;
  return return_tuple;
}

char* CountAF::evaluate(char *tuple)
{
  incr(tuple);
  return final();
}

int CountAF::getReturnedSize()
{
  return (sizeof(int));
}

CountAF* CountAF::makeNew()
{
  CountAF *temp = new CountAF();
  return temp;
}

