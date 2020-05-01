#ifndef READ_RELATIONQBOX_H
#define READ_RELATIONQBOX_H

#include "QBox.H"
#include <db_cxx.h>

class ReadRelationQBox : public QBox
{
public:
  ReadRelationQBox() {};

  ~ReadRelationQBox() {};

  virtual Box_Out_T doBox();
  void setBox(Db *db, int key_length);

private:
  Db *db;
  size_t key_length;
}; 

#endif
