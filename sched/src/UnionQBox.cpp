#include "UnionQBox.H"

/**
 * Note: Union can only union same-schema streams
 *
 */
Box_Out_T UnionQBox::doBox()
{
	// LoadShedder-related calls	BEGIN (tatbul@cs.brown.edu)
	//
	applyLoadShedding();
	//
	// LoadShedder-related calls	END (tatbul@cs.brown.edu)
	
  int good_value_count = 0;
  
  char *outStream_ptr;
  outStream_ptr = _outStream[0]; // set to beginning of _outStream initially
  int _tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
  
  // Loop through all the input arcs
  for (int jj = 0; jj < _numInputArcs; jj++)
	{
	  
	  // Copy all of the tuples in each input stream straight into the
	  //   output stream. This call starts at the beginning of the current
	  //   input stream and grabs ((# of tuples) * (size of tuples)).
	  //printf("UBox : _train_size[%i] = %ld\n", jj, _train_size[jj]);
	  //printf("UBox : tuple size = %i\n", _tuple_descr->getSize());

		// LoadShedder-related modifications    BEGIN (tatbul@cs.brown.edu)
		//
		// had to change the original implementation from batch to
		// tuple-by-tuple
		//
		for (int ii = 0; ii < _train_size[jj]; ii++)
		{
			if (!(_ls_flag[jj][ii]))
			{
				//cout << "DROPPING TUPLE!!" << endl;
				//printTuple(_inStream[ii] + (jj*_tuple_size), _tuple_descr);
				continue;
			}
			else
			{
				//cout << "PROCESSING TUPLE!!" << endl;
				//printTuple(_inStream[ii] + (jj*_tuple_size), _tuple_descr);
			}
			memcpy(outStream_ptr,
				   _inStream[jj] + (ii*_tuple_size),
				   _tuple_size);

			outStream_ptr += _tuple_size;
			good_value_count++;
		}
		//
		// LoadShedder-related modifications    END (tatbul@cs.brown.edu)

	  // this was the original implementation
	  //
	  /* 
	  memcpy(outStream_ptr,
		 _inStream[jj],
		 _train_size[jj]*_tuple_size);

	  outStream_ptr += _train_size[jj]*(_tuple_size);
	  
	  good_value_count += _train_size[jj];
	  */
	}
  
  return_val.kept_input_count = 0;
  return_val.output_tuples = good_value_count;
  return_val.output_tuple_size = _tuple_size;

  return_val.kept_input_count_array = new int[_numInputArcs];
  for (int inp = 0; inp < _numInputArcs; inp++) return_val.kept_input_count_array[inp] = 0;

  // This is very important to maintain too!
  return_val.output_tuples_array = new int[1];
  return_val.output_tuples_array[0] = good_value_count;

  return return_val;
  
}
