#include "NOTPredicate.H"

NOTPredicate::NOTPredicate(Predicate *child) 
{
  _child = child;
}

NOTPredicate::~NOTPredicate() {}

bool NOTPredicate::evaluate(char *tuple) 
{
  if (_child->evaluate(tuple) == true)
    return false;
  else
    return true;
}

bool NOTPredicate::evaluate(char *tuple1, char *tuple2) 
{
  if (_child->evaluate(tuple1, tuple2) == true)
    return false;
  else
    return true;
}

