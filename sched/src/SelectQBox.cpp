//static char *cvs_id="@(#) $Id: SelectQBox.C,v 1.11 2003/04/03 16:14:54 cjc Exp $";
//  SELECTION CLASS
//   by Christina Erwin and Andrew Flinders
//


#include "SelectQBox.H"

/*************************************************************** 
 * SelectQBox::doBox
 * Inputs  : 
 * Outputs : 
 ***************************************************************/
Box_Out_T SelectQBox::doBox()
{
	//printf("GOT TO SelectQBox::doBox()\n");
	int good_value_count = 0;
	int _tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();

	char *outStream_ptr;
	outStream_ptr = _outStream[0]; // set to beginning of _outStream initially

	// Loop through all the tuples in the input stream
	for (int ii = 0; ii < _train_size[0]; ii++)
	{ 
	  // Evaluate the predicate on the current tuple.
	  // If the predicate returns true, add tuple to end of output stream. 
	  // Otherwise, move on to next tuple.  
	  if (_predicate->evaluate(_inStream[0] + (ii*_tuple_size)))
		{
		  //fprintf(stdout, "SelectQBox.doBox - predicate evaluated to true\n");
		  //printf("outputptr: %p\n",outStream_ptr);
			
			// copy the entire tuple from the instream to the outstream
			memcpy(outStream_ptr,
			_inStream[0] + (ii*_tuple_size),
			_tuple_size);

			// Move the pointer to the end of the outstream up to
			//   the proper position
			outStream_ptr += _tuple_size;

			good_value_count++;
		}
	  //else
	  //fprintf(stdout,  "SelectQBox.doBox - predicate evaluated to false\n");

	} // end loop over all tuples in the input stream

	return_val.kept_input_count = 0;
	return_val.output_tuples = good_value_count;
	return_val.output_tuple_size = _tuple_size;

	// ADDED BY CJC TO ADDRESS VALGRIND'S COMPLAINT ABOUT AN UNINITIALIZED 
	// VARIABLE BEING USED IN AN IF(...) EXPRESSION IN WorkerThread.C.
	// IF THIS FIXUP IS AN ERROR, PLEASE CORRECT IT. -CJC, 3 MARCH 2003.
	return_val.kept_input_count_array = NULL;

	return return_val;
    
}
