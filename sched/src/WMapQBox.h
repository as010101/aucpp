/**************************************************************/
/*	Name: Christina Marie Erwin (cherwin) &
/*            Andrew Flinders (awf)
/*	File: WMapQBox.H
/*	Asgn: SRC6
/*	Date: Thu Apr 25 15:48:50 EDT 2002
/**************************************************************/

#ifndef WMapQBox_Header
#define WMapQBox_Header

#include "QBox.H"

class WMapQBox : public QBox 
{
public:

  WMapQBox()  { _boxType = WMAP_BOX; }
  ~WMapQBox() {}
  virtual Box_Out_T doBox();

  Function *_func;
};

#endif
