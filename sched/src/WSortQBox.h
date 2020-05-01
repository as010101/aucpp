#ifndef W_SORT_QBOX_H
#define W_SORT_QBOX_H

#include "QBox.H"
#include "FieldExt.H"
#include <list>
#include "FunPred.H"

class WSortQBox : public QBox
{
public:
  WSortQBox();
  ~WSortQBox();
  Box_Out_T doBox();

  void timeOutBufferNodes(Timestamp t);
  void removeUntilNode(buffer_node elem);
  void emitLowest();
  void addToBuffer(char *tuple);
  int bufferNodeCompare(char *tuple1, char *tuple2);
  //  FieldExt** parseAtts(char *atts, int num_atts);
  void emitTuple(char *tuple);
  void setState(AggregateState* agg);
  void printBuffer();

private:
  SortList                 *_buffer;   //doubly linked list ASCENDING order
  //  FieldExt                 **_fields;
  Predicate                **_preds_compare;
  Predicate                **_preds_equal;
  //  Timestamp                _maxtime;
  long                     _maxtime;
  int                      _num_atts;
  Timestamp                _curr_ts;
  long                     _curr_seconds;
  char                     *_last_emitted;
  char                     *_curr_tuple;
  char                     *_output_tuple;
  int                      _num_tuples_emitted;
  int                      _tuple_size;
  bool                     _curr_tuple_emitted;

};


#endif
