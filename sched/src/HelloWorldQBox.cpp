#include "HelloWorldQBox.H"

Box_Out_T HelloWorldQBox::doBox()
{
  // Prepare internal pointer to tuple outstream and instream.
  // There will be only one of each.
  char* outStream = _outStream[0];
  char* inStream = _inStream[0];

  // Set tuple size.
  tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();

  // The output-tuples tracker
  return_val.output_tuples_array = new int[1];
  return_val.output_tuples_array[0] = 0;

  // Loop through all the tuples in the input stream
  for (int i = 0; i < _train_size[0]; i++)
    { 
      cerr << "[HelloWorldQBox] INPUT:";
      printTuple(inStream + (i*tuple_size), _tuple_descr);
		  
      //*************** copy the entire tuple from the instream to the outstream
      memcpy(outStream,
	     inStream + (i*tuple_size),
	     tuple_size);

      // Move the pointer to the end of the outstream up to the proper position
      outStream += tuple_size;

      // Track outputted tuple
      return_val.output_tuples_array[0]++;

      // Print our message
      cout << "[HelloWorldQBox] " << message << endl;

    } // end for loop over all tuples in the input stream

  return_val.kept_input_count = 0;
  return_val.output_tuples = return_val.output_tuples_array[0]; // Deprecated?
  return_val.output_tuple_size = tuple_size;

  cerr << "[HelloWorldQBox] *********** END ************* " << endl;
  
  return return_val;
}

void HelloWorldQBox::setBox(const char* a_message) {
  setMessage(a_message);
}

/*
 * The message is a normal C string.
 */

void HelloWorldQBox::setMessage(const char* a_message) 
{
  cerr << "[HelloWorldQBox] setMessage(" << a_message << ")" << endl;
  message = a_message;
}

  
