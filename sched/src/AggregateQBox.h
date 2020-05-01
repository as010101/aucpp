#ifndef AGGREGATE_QBOX_H
#define AGGREGATE_QBOX_H

//#include <vector.h>
#include "QBox.H"
#include "AggregateFunction.H"
#include "Timestamp.H"
#include "FieldExt.H"
#include "HashForAggregate.H"
using __gnu_cxx::hash;
using __gnu_cxx::hash_map;
#include <vector.h>


// Tells what type the advance is, an integer or a predicate


class AggregateQBox : public QBox
{
enum advance_arg
  {
    ADVANCE_INTEGER, ADVANCE_PREDICATE
  };
public:

  bool FIDELITY_HACK;
  AggregateQBox() {}; 
  ~AggregateQBox() {};

  virtual Box_Out_T doBox();

  void setBox(const char* modifier_str, HashForAggregate* hash_of_everything, unsigned int* tuple_counter, vector<char*> *tuple_store);


  void initializeStates(char *value);

  void updateThresholdAndSlack(HashForNewState *hash, char *att_value);
  void timeoutStates();
  void timeoutStatesForWinByTuples();
  void incrStates(char *order_att_value, char *tuple);

  void getGroupByValuesStr(char *tuple);
  void parseGroupBy(char *group_by_atts);  

  void emitTuple(char *group_by_values, NewState *s);
  void closeState(HashForNewState *hash, NewState *s);

  void print(char *msg);
  void print(int msg);

private:

  AggregateFunction    *_af;
  int            _window_size;    // size of the windows
  //  char           *_order_att;
  int            _slack;
  char           *_group_by_atts;
  float           _timeout;
  bool _ORDER_BY_TUPLENUM;
  char* _tuple_counter; // used to track tuple # when doing ORDER ON TUPLE #
                        // note, this is a char* cuz it contains the byte memory for the value
  unsigned int* _tuple_counter_state; // needed to increment it (refers straight to the stored version of)
  
  advance_arg    _how_to_advance;
  int            _int_advance;   
  Predicate      *_advance_predicate;      

  FieldExt       *_field_att;
  FieldExt       **_field_group;
  
  char      *_curr_tuple;
  Timestamp _curr_ts;
  long      _curr_seconds;
  
  char      *_order_att_value;
  char      _order_att_type;
  int       _order_att_size;

  char* _order_att_str_value;

  long     _order_time_increment;

  char      *_group_by_values_str;  // to hash
  char      *_group_by_values_bytes;  // to store for later use
  int       _group_by_size;   // size in bytes
  int       _chars_needed_total;   // this is the size of the string for hashing
  
  char      *_output_tuple;
  int       _num_tuples_emitted;
  int       _num_group_by_atts;
  int       _tuple_size;
  int       _output_tuple_size;

 
  HashForAggregate   *_hash_of_everything;       // store this 
  vector<char*>   *_tuple_store;  // and this also store
  
  hash_map<char*, HashForNewState*, hash<const char*>, equal_string>::iterator  _hash_iter; 

  bool _window_by_tuples;

}; 

#endif
