#include "StringConstant.H"

StringConstant::StringConstant (char *str) 
{
  int i = 0;
  while (str[i] != '\0')
    i++;
  _size = i;
  _str = new char[i];
  strcpy(_str,str);
}

StringConstant::~StringConstant() {}

char* StringConstant::evaluate(char *tuple) 
{
  char* result = new char[_size + 1];
  for (int i = 0; i < _size + 1; i++) // dont forget the null character!
    result[i] = _str[i];
  return result;
}

char* StringConstant::evaluate(char *tuple1, char *tuple2) 
{
  char* result = new char[_size + 1];
  for (int i = 0; i < _size + 1; i++) // dont forget the null character!
    result[i] = _str[i];
  return result;
}

int StringConstant::getReturnedSize()
{
  return _size;
}

