#include "FloatConstant.H"

FloatConstant::FloatConstant (float mem) 
{
  _mem = mem;
}

FloatConstant::~FloatConstant() {}

char* FloatConstant::evaluate(char *tuple) 
{
  char* result = new char[sizeof(float)];
  *(float*)(result) = _mem;
  return result;
}


char* FloatConstant::evaluate(char *tuple1, char *tuple2) 
{
  char* result = new char[sizeof(float)];
  *(float*)(result) = _mem;
  return result;
}

int FloatConstant::getReturnedSize()
{
  return sizeof(float);
}
