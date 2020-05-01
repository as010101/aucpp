#ifndef UPDATE_RELATIONQBOX_H
#define UPDATE_RELATIONQBOX_H

#include "QBox.H"
#include <db_cxx.h>

class UpdateRelationQBox : public QBox
{
public:
  UpdateRelationQBox() {};

  ~UpdateRelationQBox() {};

  virtual Box_Out_T doBox();
  void setBox(Db *db, int key_length);

private:
  Db *db;
  size_t key_length;
}; 

#endif
