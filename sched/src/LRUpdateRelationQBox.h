#ifndef LR_UPDATE_RELATIONQBOX_H
#define LR_UPDATE_RELATIONQBOX_H

#include "QBox.H"
#include "LRUpdateRelationBox.H"
#include <db_cxx.h>

class LRUpdateRelationQBox : public QBox
{
public:
  LRUpdateRelationQBox() {};

  ~LRUpdateRelationQBox() {};

  virtual Box_Out_T doBox();
  void setBox(LRUpdateRelationBox *catbox, int magic_number);

private:
  LRUpdateRelationBox *catbox;
  int magic_number;

  void LRUpdateRelationQBox::doUpdateAccts(void* tuple, int size);
  void LRUpdateRelationQBox::doUpdateDaily(void* tuple, int size);
  void LRUpdateRelationQBox::doUpdateStats(void* tuple, int size);
  void LRUpdateRelationQBox::doUpdateAcc(void* tuple, int size);
  void LRUpdateRelationQBox::doUpdateAcctsWithAcc(void* tuple, int size);
}; 

#endif
