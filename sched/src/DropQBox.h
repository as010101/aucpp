/**************************************************************/
/*	Name: Christina Marie Erwin (cherwin) &
/*            Andrew Flinders (awf)
/*	File: DropQBox.H
/*	Asgn: SRC6
/*	Date: Thu Apr 25 15:39:10 EDT 2002
/**************************************************************/

#ifndef DropQBox_Header
#define DropQBox_Header

#include "QBox.H"

class DropQBox : public QBox 
{
public:
  DropQBox()  { _boxType = DROP_BOX; }
  virtual ~DropQBox();
  virtual Box_Out_T doBox();

  int _period;   // 1 out of _period should be dropped
  float _drop_rate; // fraction to drop
};

#endif
