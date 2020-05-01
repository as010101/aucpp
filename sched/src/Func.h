/**************************************************************/
/*	Name: Christina Marie Erwin (cherwin)
/*	File: Func.H
/*	Asgn: SRC7
/*	Date: Mon Apr 29 13:36:38 EDT 2002
/**************************************************************/


/* 
   THIS ENTIRE UNIT IS FOR DEBUGGING PURPOSES ONLY. IT SIMULATES THE
   PRESENCE OF FUNCTIONS
*/
#ifndef Func_Header
#define Func_Header

#include "TupleDescription.H"
#include "Predicate.H"
#include <iostream>
#include <stdio.h>

void func1 (char* in_stream, char* out_stream, TupleDescription *td);
void func2 (char* in_stream, char* out_stream, TupleDescription *td);
void func3 (char* in_stream, char* out_stream, TupleDescription *td);

void func1 (char* in_stream, char* out_stream, TupleDescription *td)
{
/*
  // This value should be entered via the GUI; Hardcoding for now
  int field_num = 1;
  // End comment

  double temp;  // stores intermediate values
  u_type value;  // used to store the value by which the field should be skewed

  int field_type   = td->getFieldType(field_num);
  int field_offset = td->getFieldOffset(field_num);

  fprintf(stdout, "^^^^^^^^^^^^^^^^^^ In Func1 ^^^^^^^^^^^^^^^^^^\n");
  fprintf(stdout, "^^^ Function 1 converts value at index %i from fahrenheit to celsius\n", field_num);

  memcpy(out_stream,
	 in_stream,
	 td->getSize());
        
  temp = (double)(*(out_stream + field_offset));

  // Perform actual Celsius to Fahrenheit conversion.  Tf = ((9/5)*Tc)+32
  fprintf(stdout, "Celsius temperature %f => ", temp);

  temp = ((9.0/5.0) * temp) + 32.0;
  fprintf(stdout, "Fahrenheit temperature %f\n", temp);

  fprintf(stdout, "Value in i/s to change %i\n", *(in_stream + field_offset));

  switch (field_type)
    {
    case INT_TYPE:

      out_stream[field_offset] = int(temp);
      break;

    case FLOAT_TYPE:

      out_stream[field_offset] = float(temp);
      break;

    case DOUBLE_TYPE:

      out_stream[field_offset] = double(temp);
      break;

    default:
      fprintf(stderr, "In Func.H : Given type of %i not coded...\nExiting...\n", field_type);
      break;
    }

  fprintf(stdout, "Value in o/s after change %i\n", *(out_stream + field_offset));

  fprintf(stdout, "^^^^^^^^^^^^^^^^^^ Done In Func1 ^^^^^^^^^^^^^^^^^^\n");

*/
}

void func2 (char* in_stream, char* out_stream, TupleDescription *td)
{
/*
  // These two values should be entered via the GUI; Hardcoding for now
  int field_num = 2;
  int the_value = 2;
  // End comment

  u_type temp;  // stores intermediate values
  u_type value;  // used to store the value by which the field should be skewed

  int field_type   = td->getFieldType(field_num);
  int field_offset = td->getFieldOffset(field_num);


  fprintf(stdout, "^^^^^^^^^^^^^^^^^^ In Func2 ^^^^^^^^^^^^^^^^^^\n");
  fprintf(stdout, "^^^ Function 2 adds 2 to 3rd index\n");

  memcpy(out_stream,
	 in_stream,
	 td->getSize());

  switch (field_type)
    {
    case INT_TYPE:
      value._ival = int(the_value);
      break;

    case FLOAT_TYPE:

      value._fval = float(the_value);
      break;

    case DOUBLE_TYPE:

      value._dval = double(the_value);
      break;

    default:
      break;
    }
    
  fprintf(stdout, "Value in i/s to change %i\n", *(in_stream + field_offset));
  fprintf(stdout, "Value in o/s to change %i\n", *(out_stream + field_offset));

  switch (field_type)
    {
    case INT_TYPE:

      temp._ival = (int)(*(out_stream + field_offset));

      temp._ival += value._ival;

      out_stream[field_offset] = temp._ival;

      break;

    case FLOAT_TYPE:

      temp._fval = (float)(*(out_stream + field_offset));

      temp._fval += value._fval;

      out_stream[field_offset] = temp._fval;

      break;

    case DOUBLE_TYPE:

      temp._dval = (double)(*(out_stream + field_offset));

      temp._dval += value._dval;

      out_stream[field_offset] = temp._dval;

      break;

    default:
      fprintf(stderr, "In Func.H : Given type of %i not coded...\nExiting...\n", field_type);
      break;
    }

  fprintf(stdout, "Value in i/s after change %i\n", *(in_stream + field_offset));
  fprintf(stdout, "Value in o/s after change %i\n", *(out_stream + field_offset));

  fprintf(stdout, "^^^^^^^^^^^^^^^^^^ Done In Func2 ^^^^^^^^^^^^^^^^^^\n");
*/
}

void func3 (char* in_stream, char* out_stream, TupleDescription *td)
{
/*
  fprintf(stdout, "^^^^^^^^^^^^^^^^^^ In Func3 ^^^^^^^^^^^^^^^^^^\n");
  fprintf(stdout, "^^^ Function 3 drops the last field in the tuple... NEEDS WORK!!! \n");
  memcpy(out_stream,
	 in_stream,
	 td->getFieldOffset(td->getNumOfFields()));  // should the offset be subtracted by 1?!?! 

  // function td->getSize(fieldnum) would clean this up... 
  //  i.e. a function which got the size of the first 3 fields in the tuple
  //  instead i am finding up to which index I wish to copy... 
*/
}

#endif
/* 
   THIS ENTIRE UNIT IS FOR DEBUGGING PURPOSES ONLY. IT SIMULATES THE
   PRESENCE OF FUNCTIONS
*/
