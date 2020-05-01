#ifndef BSORTQBOX_H
#define BSORTQBOX_H

#include "QBox.H"
#include "BufferList.H"
#include "FieldExt.H"
#include "HashForBufferList.H"

class BSortQBox : public QBox
{
public:
  BSortQBox() {};
  ~BSortQBox() {};

  virtual Box_Out_T doBox();
  Box_Out_T doBoxs(char *tuple);

  void setBox(const char* modifier_str, HashForBufferList* buffer_hash);
  void setBox(char *att, long slack, char *group_by_atts);
  // something has to be passed to this method that will initialize everything
 
  void addToBuffer(char *tuple);
  void addFloatToBuffer(char *att_value, node new_node);
  void addIntToBuffer(char *att_value, node new_node);
  void addStringToBuffer(char *att_value, node new_node);

  void emitLowest(BufferList *buffer);
  void emitTuple(char *tuple);

  void parseGroupBy(char *atts);

private:
  HashForBufferList *_buffer_hash;
  BufferList     *_buffer; // this should be called "currentBuffer or something"
  //Timestamp      _last_time_emitted;
  char           *_group_by_atts;
  long           _slack;

  //int            _buffer_size;
  FieldExt       *_field_att;
  FieldExt       **_field_group;
  //char           _att_type;
  int            _num_group_by_atts;
  // This (nul-terminated) string is a concatenation (0x01 delimited)
  //of the group by attribute values from the current tuple.
  char           *_group_by_values_str;

  int            _num_tuples_emitted;
  int            _tuple_size;
  char           *_curr_tuple;
  //Timestamp      _curr_ts;
  //long           _curr_seconds;
  char           *_output_tuple_ptr;

};

#endif
