#ifndef AND_PREDICATE_H
#define AND_PREDICATE_H

#include "Predicate.H"

class ANDPredicate : public Predicate
{
public:
  ANDPredicate(Predicate *lhs, Predicate *rhs);
  virtual ~ANDPredicate();
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Predicate   *_lhs;
  Predicate   *_rhs;
};

#endif  
