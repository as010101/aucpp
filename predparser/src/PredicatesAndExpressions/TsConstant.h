#ifndef TIMESTAMP_CONSTANT_H
#define TIMESTAMP_CONSTANT_H

#include "Expression.H"

class TsConstant : public Expression
{
public:
  TsConstant (Timestamp mem);
  virtual ~TsConstant();
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  Timestamp    _mem;
  char         *_time;
  int          _size;
};

#endif
