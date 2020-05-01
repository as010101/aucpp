#ifndef SLIDE_QBOX_H
#define SLIDE_QBOX_H

#include "State.H"
#include "AggregateFunction.H"
#include "Hash.H"
#include "TrashState.H"
#include "TrashHash.H"
#include "List.H"
#include "HashWithList.H"

#include "QBox.H"
#include "FieldExt.H"
#include "Predicate.H"

class SlideQBox : public QBox
{
public:
  SlideQBox();
  ~SlideQBox();
  Box_Out_T doBox();

  void timeOutStates(Timestamp t);
  void close(State *s);
  void activateState(State *s);
  void incrState(State *s);
  void checkForClose(State *s);
  bool checkInTrash(char *value);
  void emitTuple(State *s);
  void startTimers(List *lst, int value);
  void setWheneverPredicate(Predicate *p);
  void setSatisfiesPredicate(Predicate *p);
  void setTimeoutS(Timestamp s);
  void setState(AggregateState *agg);


private:
  AggregateFunction   *_af;
  FieldExt            **_fields;
  output_arg          _output;
  until_arg           _timeout;
  Predicate           *_whenever_pred;
  Predicate           *_satisfies_pred;

  Timestamp           _curr_ts;
  long                _curr_seconds;
  char                *_sid;
  char                *_group_by_values;
  char                *_curr_tuple;
  char                *_output_tuple;
  int                 _group_by_size;
  int                 _tuple_size;
  int                 _hidden_size;
  int                 _sid_size;
  int                 _key_size;
  int                 _num_atts;   // num of atts in group_by
  int                 _window_range;
  int                 _new_tuple_size;
  int                 _num_of_output_tuples;
  int                 _curr_pos;    // location in output streams

  bool           _value_type;   // use 1 for int; 0 for float
                                // remember float has a 'o'-looks like 0

  HashWithList        *_hash;     // stores all the Lists that the Slide 
                                  // box maintains
                                  // the Lists will be in ascending order of 
                                  // the group_by attribute

  TrashHash           *_trash_hash;    // stores the states so that new 
                                       // states will not be created until
                                       // the UNIQUE FOR has timed out

  //  Timestamp           _maxtime;    // max time of the tuples seen so far
  
  //  Timestamp           _unless_timeout_s;
  //  Timestamp           _slack_timeout;
  //  Timestamp           _unique_timeout;
  
  long _maxtime;
  long _unless_timeout_s;
  long _slack_timeout;
  long _unique_timeout;

  // iterators to go through the trash and normal hash table
  hash_map<key_type*, List*, my_hash, eqstr>::iterator         _iter;
  hash_map<key_type*, TrashState*, my_hash, eqstr>::iterator   _trash_iter;

};

#endif
