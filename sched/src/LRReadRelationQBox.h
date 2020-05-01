#ifndef LR_READ_RELATIONQBOX_H
#define LR_READ_RELATIONQBOX_H

#include "QBox.H"
#include <db_cxx.h>
#include "LRReadRelationBox.H"

class LRReadRelationQBox : public QBox
{
public:
  LRReadRelationQBox() {};

  ~LRReadRelationQBox() {};

  virtual Box_Out_T doBox();
  void setBox(LRReadRelationBox *catbox, int magic_number);

private:
  LRReadRelationBox *catbox;
  int magic_number;

  void LRReadRelationQBox::doCalcTollNewRead(void* in_tuple, int in_length,
					     void* out_stream, int *count,
					     int *size);
  void LRReadRelationQBox::doSegStatReadAcc(void* in_tuple, int in_length,
					    void* out_stream, int *count,
					    int *size);
  void LRReadRelationQBox::doReadPosReadAccts(void* in_tuple, int in_length,
					      void* out_stream, int *count,
					      int *size);
  void LRReadRelationQBox::doAccAlertReadAccts(void* in_tuple, int in_length,
					       void* out_stream, int *count,
					       int *size);

}; 

#endif
