#ifndef JOIN_QBOX_H
#define JOIN_QBOX_H

#include "QBox.H"
#include "Predicate.H"
#include "Parse.H"
//#include "HashForBufferList.H"
#include "FieldExt.H"
#include "BufferList.H"

class JoinQBox : public QBox
{
public:
  JoinQBox() {};  
  ~JoinQBox() {};

  virtual Box_Out_T doBox();
  
  Box_Out_T doBoxs(char *tuple);
  
  //  void setBox(char *pred, int size, char *left_order_att, int left_slack, char *left_group_by, 
  //	      char *right_order_att, int right_slack, char *right_group_by);

  void setBox(char *pred, int size, char *left_order_att, int left_slack, char *right_order_att, int right_slack);
  void setBox(const char* modifier_str, BufferList* left_buffer, BufferList* right_buffer);
  //  void parseLeftGroupBy(char *atts);
  //  void parseRightGroupBy(char *atts);
  
  //  void getLeftGroupByValuesStr(char *tuple);
  //  void getRightGroupByValuesStr(char *tuple);

  void emitTuple(char *left_tuple, char *right_tuple);

private:
  
  Predicate *_join_predicate;
   
  //  HashForBufferList   *_left_buffer;
  //  HashForBufferList   *_right_buffer;
  
  BufferList *_left_buffer;
  BufferList *_right_buffer;

  int    _buffer_size_before;
  int    _buffer_size_after;

  int    _left_slack;
  int    _right_slack;
  
  char  *_left_order_att_value;
  char  _left_order_att_type;
  int   _left_order_att_size;

  char  *_right_order_att_value;
  char  _right_order_att_type;
  int   _right_order_att_size;

  FieldExt   *_left_field_att;
  FieldExt   *_right_field_att;

  //  char  *_left_group_by_atts;
  //  char  *_left_group_by_values_str;
  //  char  *_left_group_by_values_bytes;
  //  int   _left_group_by_size_bytes;
  //  int   _left_chars_needed_total;
  //  int   _left_num_group_by_atts;

  //  char  *_right_group_by_atts;
  //  char  *_right_group_by_values_str;
  //  char  *_right_group_by_values_bytes;
  //  int   _right_group_by_size_bytes;
  //  int   _right_chars_needed_total;
  //  int   _right_num_group_by_atts;

  //  FieldExt   **_left_field_group;
  //  FieldExt   **_right_field_group;
 

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
