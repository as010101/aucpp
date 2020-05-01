#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <stdio.h>
#include <string>
#include "Timestamp.H"

using namespace std;

class Expression
{
public:
  virtual char* evaluate(char *tuple) = 0;
  virtual char* evaluate(char *tuple1, char *tuple2) = 0;
  virtual void setExpression(Expression *expr) {};
  virtual int getReturnedSize() = 0;
};

#endif 
