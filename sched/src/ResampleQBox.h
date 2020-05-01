#ifndef RESAMPLE_QBOX_H
#define RESAMPLE_QBOX_H

#include "QBox.H"
#include "Predicate.H"
#include "Parse.H"
#include "FieldExt.H"
#include "BufferList.H"
//#include "NewStateList.H"
#include "HashForNewState.H"

class ResampleQBox : public QBox
{
public:
  ResampleQBox() {};  
  ~ResampleQBox() {};

  virtual Box_Out_T doBox();
  
  void setBox(const char* modifier_str, BufferList* left_buffer, BufferList* right_buffer, HashForNewState* statehash);
  void setBox(AggregateFunction *af, int size, char *left_order_att, int left_slack, char *right_order_att, int right_slack);

  void emitTuple(NewState *s);

private:
  
  AggregateFunction   *_resample_function;
  
  BufferList    *_left_buffer;
  BufferList    *_right_buffer;
  //  NewStateList  *_state_list;

  HashForNewState  *_state_hash;

  int    _window_size;

  int    _left_slack;
  int    _right_slack;
  
  char  *_left_order_att_value;
  char  *_left_order_att_str;
  char  _left_order_att_type;
  int   _left_order_att_size;

  char  *_right_order_att_value;
  char  *_right_order_att_str;
  char  _right_order_att_type;
  int   _right_order_att_size;

  FieldExt   *_left_field_att;
  FieldExt   *_right_field_att;

  char       *_left_curr_tuple;
  char       *_right_curr_tuple;
  Timestamp  _curr_ts;
  long       _curr_seconds;

  int      _left_tuple_size;
  int      _right_tuple_size;

  char     *_output_tuple;
  int      _output_tuple_size;
  int      _num_tuples_emitted;
  int      _hidden_size;

}; 

#endif
