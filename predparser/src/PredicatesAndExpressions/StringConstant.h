#ifndef STRING_CONSTANT_H
#define STRING_CONSTANT_H

#include "Expression.H"

class StringConstant : public Expression
{
public:
  StringConstant (char *str);
  virtual ~StringConstant();
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  char   *_str;
  int    _size;
};

#endif
