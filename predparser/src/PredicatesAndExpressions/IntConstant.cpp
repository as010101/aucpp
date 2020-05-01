#include "IntConstant.H"

IntConstant::IntConstant (int mem) 
{
  _mem = mem;
}

IntConstant::~IntConstant() {}

char* IntConstant::evaluate(char *tuple) 
{
  char* result = new char[sizeof(int)];
  *(int*)(result) = _mem;
  return  result;
}

char* IntConstant::evaluate(char *tuple1, char *tuple2) 
{
  char* result = new char[sizeof(int)];
  *(int*)(result) = _mem;
  return  result;
}

int IntConstant::getReturnedSize()
{
  return sizeof(int);
}
