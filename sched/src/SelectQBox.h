//  SELECTION CLASS
//   by Christina Erwin and Andrew Flinders
//

#ifndef SELECTQBOX_H
#define SELECTQBOX_H

#include "QBox.H"

class SelectQBox : public QBox
{
public:
  SelectQBox()  { _boxType = SELECT_BOX; }
  ~SelectQBox() {}
  virtual Box_Out_T doBox();

  Predicate *_predicate;
}; 

#endif
