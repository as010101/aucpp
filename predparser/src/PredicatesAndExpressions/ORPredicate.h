#ifndef OR_PREDICATE_H
#define OR_PREDICATE_H

#include "Predicate.H"

class ORPredicate : public Predicate
{
public:
  ORPredicate(Predicate *lhs, Predicate *rhs);
  virtual ~ORPredicate();
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Predicate   *_lhs;
  Predicate   *_rhs;
};
          
#endif
