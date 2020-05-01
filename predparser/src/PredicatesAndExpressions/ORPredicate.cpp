#include "ORPredicate.H"

ORPredicate::ORPredicate(Predicate *lhs, Predicate *rhs) 
{
  _lhs = lhs;
  _rhs = rhs;
}

ORPredicate::~ORPredicate() {}

bool ORPredicate::evaluate(char *tuple) 
{
  return _lhs->evaluate(tuple) || _rhs->evaluate(tuple);
}


bool ORPredicate::evaluate(char *tuple1, char *tuple2) 
{
  return _lhs->evaluate(tuple1, tuple2) && _rhs->evaluate(tuple1, tuple2);
}
