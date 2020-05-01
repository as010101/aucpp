#ifndef TUMBLE_QBOX_H
#define TUMBLE_QBOX_H

#include "QBox.H"
#include "FieldExt.H"

#include "State.H"
#include "AggregateFunction.H"
#include "Hash.H"
#include "TrashState.H"
#include "TrashHash.H"

#include "Predicate.H"

class TumbleQBox : public QBox
{
public:  
  TumbleQBox();

  ~TumbleQBox();
  Box_Out_T doBox();

  void timeOutStates(Timestamp t);
  void close(State *s);
  void activateState(State *s);
  void incrState(State *s);
  bool checkForClose(State *s);
  bool checkInTrash(char *value);
  void emitTuple(State *s);
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
  int                 _key_size;
  int                 _tuple_size;
  int                 _hidden_size;
  int                 _num_atts;   // num of atts in group_by
  int                 _new_tuple_size;
  int                 _num_of_output_tuples;
  int                 _curr_pos;    // location in output streams
  int                 _sid_size;

  Hash                *_hash;     // stores all the states that the Tumble 
                                  // box maintains

  TrashHash           *_trash_hash;    // stores the states so that new 
                                       // states will not be created until
                                       // the UNIQUE FOR has timed out

  Hash                *_last;     // stores the last states in each sid
                                  // that was updated
                                  // used for slack timeout

  //  Timestamp           _mintime;    // min time of the tuples seen so far
  //  Timestamp           _maxtime;    // max time of the tuples seen so far
  
  //  Timestamp           _unless_timeout_s;
  //  Timestamp           _slack_timeout;
  //  Timestamp           _unique_timeout;
  
  long    _maxtime;
  long    _unless_timeout_s;
  long    _slack_timeout;
  long    _unique_timeout;

  // iterators to go through the trash and normal hash table
  hash_map<key_type*, State*, my_hash, eqstr>::iterator        _iter;
  hash_map<key_type*, TrashState*, my_hash, eqstr>::iterator   _trash_iter;
};

#endif

