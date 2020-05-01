#ifndef HELLO_WORLDQBOX_H
#define HELLO_WORLDQBOX_H

#include "QBox.H"

class HelloWorldQBox : public QBox
{
public:
  HelloWorldQBox() {};  //  { _boxType = HELLO_WORLD_BOX; }

  ~HelloWorldQBox() {};

  virtual Box_Out_T doBox();
  void setBox(const char* a_message);
  void setMessage(const char* a_message);


private:
  string message;
  int tuple_size;
}; 

#endif
