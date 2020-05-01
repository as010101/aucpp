#ifndef AGGREGATE_STATE_H
#define AGGREGATE_STATE_H

#include "AggregateFunction.H"
#include "Hash.H"
#include "HashWithList.H"
#include "TrashHash.H"
#include "Predicate.H"
#include "GroupByHash.H"
#include "SortList.H"
#include "PredicatesAndExpressions/Timestamp.H"
#include "tupleGenerator.H"
#include <iostream>

enum output_arg 
{
  LAST, ALL, WHENEVER 
};

enum until_arg
{
  TIMEOUT, SATISFIES, NONE
};

//typedef int Timestamp;

class AggregateState
{
public:
  AggregateState();
  ~AggregateState(){};
  void printAll(); //FOR DEBUGGING ONLY

  AggregateFunction *af;
  char              *group_by;
  output_arg        output;
  until_arg         until; 
  Timestamp         unique_time;
  Timestamp         slack_time;
  int               window_range;
  Hash              *reg_hash;
  HashWithList      *list_hash;
  Hash              *last;
  TrashHash         *trash;
  GroupByHash       *group_hash;
  Timestamp         unless_timeout;
  Predicate         *whenever_pred;
  Predicate         *satisfies_pred;
  SortList          *buffer;
  char              *last_emitted;

private:


};

#endif
