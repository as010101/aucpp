/**************************************************************/
/*	Name: Christina Marie Erwin (cherwin) & 
/*   	      Andrew Flinders (awf)
/*	File: MapBox.H
/*	Asgn: Boxes
/*	Date: Thu Apr 25 15:35:04 EDT 2002
/**************************************************************/

#ifndef MapQBox_Header
#define MapQBox_Header

#include "QBox.H"
#include <vector>

class MapQBox : public QBox 
{
public:
  MapQBox()  { _boxType = MAP_BOX; }
  ~MapQBox() {}
  virtual Box_Out_T doBox();

  vector<Expression*> *_expr;
  int _num_funcs;
};

#endif
