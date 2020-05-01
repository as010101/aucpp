//static char *cvs_id="@(#) $Id: DropQBox.C,v 1.4 2003/03/26 19:06:25 cjc Exp $";
/*************************************************************************
 *    NAME: Christina Marie Erwin (cherwin) &
 *          Andrew Flinders (awf)
 *    FILE: DropQBox.C
 *    DATE: Thu Apr 25 15:40:15 US/Eastern 2002
 *************************************************************************/
#include "DropQBox.H"
#include <time.h>


DropQBox::~DropQBox() {}

/*************************************************************************
 * Function Name: DropQBox::doBox
 * Parameters: 
 * Returns: int
 * Effects: 
 *************************************************************************/

Box_Out_T DropQBox::doBox()
{
	fprintf(stdout, "Got to DropBox!\n");

	//int good_value_count = 0;
	return_val.output_tuples = 0;
	int _tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();

	char *outStream_ptr;
	outStream_ptr = _outStream[0]; // set to beginning of _outStream initially

	srand(time(NULL));
	static unsigned int seed = 1;
	double rand_val;

	printf("_drop_rate: %f\n",_drop_rate);
	// Loop through all the tuples in the input stream
	for (int ii = 0; ii < _train_size[0]; ii++)
	{
		// Grab the random value and force it to be a double between 0 & 1.
		// Compare this value to 1/period (Drop 1 in every 30 tuples)
		// If it's less than selectivity, drop tuple. Else, put it in outstream
		// rand_r is random number generator for multi-threaded applications
		//if (((double)rand_r(&seed) / (double)(RAND_MAX + 1)) < (1.0 / (_period)))
		rand_val = (double)rand_r(&seed) / (double)(RAND_MAX);
		printf("rand_val: %f\n",rand_val);
		if (rand_val > _drop_rate)
		{
			memcpy(outStream_ptr,
			_inStream[0] + (ii*_tuple_size),
			_tuple_size);

			// Move the pointer to the end of the outstream up to
			//   the proper position
			outStream_ptr += _tuple_size;

			return_val.output_tuples++;
		}
	}

	// ADDED BY CJC TO ADDRESS VALGRIND'S COMPLAINT ABOUT AN UNINITIALIZED 
	// VARIABLE BEING USED IN AN IF(...) EXPRESSION IN WorkerThread.C.
	// IF THIS FIXUP IS AN ERROR, PLEASE CORRECT IT. -CJC, 3 MARCH 2003.
	return_val.kept_input_count_array = NULL;

	return_val.kept_input_count = 0;
	return_val.output_tuple_size = _tuple_size;
	return return_val;

}


