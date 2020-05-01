static char *cvs_id="@(#) $Id: WMapQBox.C,v 1.2 2003/03/26 19:06:26 cjc Exp $";
/*************************************************************************
 *    NAME: Christina Marie Erwin (cherwin) &
 *          Andrew Flinders (awf)
 *    FILE: WMapQBox.C
 *    DATE: Thu Apr 25 15:49:51 US/Eastern 2002
 *************************************************************************/
#include "WMapQBox.H"

/*************************************************************************
 * Function Name: WMapQBox::doBox
 * Parameters: 
 * Returns: int
 * Effects: 
 *************************************************************************/
Box_Out_T WMapQBox::doBox()
{
 
  fprintf(stderr, "WMapQBox::doBox() not coded yet!!! \n");

  return_val.kept_input_count = 0;
  return_val.output_tuples = 0;

  // ADDED BY CJC TO ADDRESS VALGRIND'S COMPLAINT ABOUT AN UNINITIALIZED 
  // VARIABLE BEING USED IN AN IF(...) EXPRESSION IN WorkerThread.C.
  // IF THIS FIXUP IS AN ERROR, PLEASE CORRECT IT. -CJC, 3 MARCH 2003.
  return_val.kept_input_count_array = NULL;

  return return_val;
}


