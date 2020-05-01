#ifndef PREDICATE_H
#define PREDICATE_H

#include "Expression.H"
#include <stdlib.h>
#include "Timestamp.H"
//typedef int Timestamp;

class Predicate
{
public:
  virtual bool evaluate(char *tuple) = 0;
  virtual bool evaluate(char *tuple1, char *tuple2) = 0;
  virtual void setExpression(Expression *expr){};
};

#endif   
