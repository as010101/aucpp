#include "MapQBox.H"
#include "Func.H"

Box_Out_T MapQBox::doBox()
{
	// LoadShedder-related calls    BEGIN (tatbul@cs.brown.edu)
	//
	applyLoadShedding();
	//
	// LoadShedder-related calls    END (tatbul@cs.brown.edu)
	
	//printf("GOT TO MAPQBOX :: doBox() for %ld tuples\n", _train_size[0]);
	char *outStream_ptr;
	outStream_ptr = _outStream[0]; // set to beginning of _outStream initially
	int ii;  // looping variable
	int output_size_computed = false;
	return_val.output_tuple_size = 0;
	int _tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();

	// Loop through all the tuples in the input stream
	for (int tuple_ii = 0; tuple_ii < _train_size[0]; tuple_ii++)
	{

		// LoadShedder-related calls    BEGIN (tatbul@cs.brown.edu)
		//
		// skip this tuple if the load shedding flag is set to drop
		//
		if (!(_ls_flag[0][tuple_ii]))
		{
			//cout << "DROPPING TUPLE!!" << endl;
			//printTuple(_inStream[0] + (tuple_ii*_tuple_size), _tuple_descr);
			continue;
		}
		else
		{
			//cout << "PROCESSING TUPLE!!" << endl;
			//printTuple(_inStream[0] + (tuple_ii*_tuple_size), _tuple_descr);
		}
		//
		// LoadShedder-related calls    END (tatbul@cs.brown.edu)
	  
	  // Copy the timestamp and streamid
	  memcpy(outStream_ptr, _inStream[0] + (tuple_ii*_tuple_size), getTsSize() + getSidSize());
	  outStream_ptr += (getTsSize() + getSidSize());

	  //printf("GOT TO MAPQBOX :: doBox() A _num_expr: %i\n",_num_funcs);
	  //printf("GOT TO _expr.size(): %i\n",_expr->size());
		for (ii = 0; ii < _expr->size(); ii++)
		{
		  //printf("----------------------------\n********************\n\tsize of tuple is: %d\n\n\n",_tuple_descr->getSize());
		  //printf("GOT TO MAPQBOX (id %d, attribute %d) :: doBox() Expression: %x\n",_boxId,ii, _expr);
			//printf("GOT TO MAPQBOX :: doBox() expr[ii]->evaluate(_stream1 + (tuple_ii * _tuple_descr->getSize())): %p\n",
			       //     (*_expr)[ii]->evaluate(_inStream[0] + (tuple_ii * _tuple_descr->getSize())));
			//printf("GOT TO MAPQBOX :: doBox() expr[ii]->getReturnedSize(): %x\n",(*_expr)[ii]->getReturnedSize());
			memcpy(outStream_ptr, 
					(*_expr)[ii]->evaluate(_inStream[0] + (tuple_ii * _tuple_size)), 
					(*_expr)[ii]->getReturnedSize());

			outStream_ptr += (*_expr)[ii]->getReturnedSize();

			if (!output_size_computed) {
			  return_val.output_tuple_size += (*_expr)[ii]->getReturnedSize();
			  //printf("MAPQBOX TUPLE OUT SIZE  IS NOW %d\n", return_val.output_tuple_size);
			}


		}  // Loop through all the expr
		//printf("GOT TO MAPQBOX :: doBox() B\n");

	output_size_computed = true;

	}  // Loop through all the tuples in the input stream

	//printf("GOT TO MAPQBOX :: doBox() C\n");
	return_val.kept_input_count = 0;
	//return_val.kept_input_count_array = new int[_numInputArcs]; NOT TO BE USED UNLES YOU HAVE > 1 INPUT
	//return_val.kept_input_count_array[0] = 0;
	return_val.output_tuples = _train_size[0];
	// finalize adjusting the "returned output size" to account for the metadata
	return_val.output_tuple_size += (getTsSize() + getSidSize());

	// This is very important to maintain too!
	return_val.output_tuples_array = new int[1];
	return_val.output_tuples_array[0] = _train_size[0];


	//printf(" MAPQBOX OUTPUTTING TUPLES OF SIZE %d *************************************\n\n\n\n", return_val.output_tuple_size);
	//return_val.output_tuple_size = _tuple_descr->getSize(); WTF IS THIS LINE!?!??!?!

	return return_val;

}


