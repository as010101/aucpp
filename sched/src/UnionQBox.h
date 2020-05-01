/**************************************************************/
/*	Name: Christina Marie Erwin (cherwin)
/*	File: UnionQBox.H
/*	Asgn: SRC6
/*	Date: Thu Aug 29 15:37:07 EDT 2002
/**************************************************************/

#ifndef UnionQBox_Header
#define UnionQBox_Header

#include "QBox.H"

class UnionQBox : public QBox 
{
public:
  UnionQBox()  { _boxType = UNION_BOX; }
  ~UnionQBox() {}
  virtual Box_Out_T doBox();
};

#endif
