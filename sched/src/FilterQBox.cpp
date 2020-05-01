#include "FilterQBox.H"


/*
 * The modifier is a list of predicates, delimited by "~".
 * It must be a normal C-String, nul delimited.
 */

void FilterQBox::setPredicates(const char* modifier) 
{
  
  // A predicate parser
  Parse *parser = new Parse(); 
  // Temporarily holds things...
  list<Predicate*> temp_predicates;
  // Set the tuple size...
  _tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
  // Get the "use false output port or not" information
  string modifier_string(modifier);
  string::size_type idx = modifier_string.find('~');
  if (modifier_string.substr(idx+1) == "PASS") _useFalsePort = true;
  else _useFalsePort = false;
  // And now the predicate parsing
  string preds = modifier_string.substr(0,idx); // careful, 2nd arg is # of chars, not end index
  
  // UPDATE MAY 14 - the delimiter between predicates is "&" not ","
  idx = preds.find('&');
  if (idx == string::npos) { // No "&" means only one predicate
    _num_predicates = 1;
    _predicates = new Predicate*[_num_predicates];
    _predicates[0] = parser->parsePred(preds.c_str());
  } else {
    do {
      temp_predicates.push_back(parser->parsePred(preds.substr(0,idx).c_str()));
      preds = preds.substr(idx+1); // the rest after the comma
      idx = preds.find('&');
    } while (idx != string::npos);
    // Ok, that last predicate, need to push him too
    temp_predicates.push_back(parser->parsePred(preds.substr(0,idx).c_str()));
    // Now make the temp_predicates vector to the array like it should be
    _num_predicates = temp_predicates.size();
    _predicates = new Predicate*[_num_predicates];
    for (int i = 0; i < _num_predicates; i++) {
      _predicates[i] = temp_predicates.front();
      temp_predicates.pop_front();
    }
  }

  // Done!
  return;
}

Box_Out_T FilterQBox::doBox()
{
  // LoadShedder-related calls	BEGIN (tatbul@cs.brown.edu)
  // 
  applyLoadShedding();
  //
  // LoadShedder-related calls	END (tatbul@cs.brown.edu)
  
  //cout << "[FilterQBox] *********** BEGIN ************* " << endl;
  int good_value_count = 0;

  // Prepare internal pointer to tuple outstreams
  char** outStream_ptr = (char**) malloc(_num_output_ports * sizeof(char*));

  // Set all pointers to point to start of _outStreams
  for (int i = 0; i < _num_output_ports; i++) outStream_ptr[i] = _outStream[i];

  // The output-tuples tracker
  return_val.output_tuples_array = new int[_num_output_ports];
  for (int i = 0; i < _num_output_ports; i++) return_val.output_tuples_array[i] = 0;

  // _num_predicates number of outStream_ptr needed
  // something like outStream_ptr[0] to outStream_ptr[_num_predicates]
  // _outStream also needed for each
  //  FOR NOW ONLY ONE OUTPUT PORT
  //char *outStream_ptr;


  //outStream_ptr = _outStream[0]; // set to beginning of _outStream initially

  int jj = 0;
  bool done = false; 
  // Loop through all the tuples in the input stream
  for (int ii = 0; ii < _train_size[0]; ii++)
	  { 
	// LoadShedder-related calls    BEGIN (tatbul@cs.brown.edu)
	//
	// skip this tuple if the load shedding flag is set to drop
	//
	if (!(_ls_flag[0][ii]))
	{
		//cout << "DROPPING TUPLE!!" << endl;
		//printTuple(_inStream[0] + (ii*_tuple_size), _tuple_descr);
		continue;
	}
	else
	{
		//cout << "PROCESSING TUPLE!!" << endl;
		//printTuple(_inStream[0] + (ii*_tuple_size), _tuple_descr);
	}
	//
	// LoadShedder-related calls    END (tatbul@cs.brown.edu)

	    //cout << "[FilterQBox] INPUT:";
	    //printTuple(_inStream[0] + (ii*_tuple_size), _tuple_descr);
		  
	    jj = 0;
	    done = false;
	    while (jj < _num_predicates && !done)
	      {
		
		// Evaluate the predicate on the current tuple.
		// If the predicate returns true, add tuple to end of output stream. 
		// Otherwise, move on to next tuple.  
		if (_predicates[jj]->evaluate(_inStream[0] + (ii*_tuple_size)))
		  { 
		    //cout << "[FilterQBox] predicate["<<jj<<"] evaluated to true" << endl;
		    
		    //*************** copy the entire tuple from the instream to the outstream
		    memcpy(outStream_ptr[jj],
		    	   _inStream[0] + (ii*_tuple_size),
		    	   _tuple_size);
		    
		    //cout << "[FilterQBox] at predicate " << jj << " OUTPUT:";
		    //printTuple(outStream_ptr[jj], _tuple_descr);

		    // Move the pointer to the end of the outstream up to
		    //   the proper position
		    outStream_ptr[jj] += _tuple_size;
		    
		    // Track outputted tuple
		    good_value_count++;
		    return_val.output_tuples_array[jj]++;
		    done = true;
		    ++jj;
		  }
		else
		  {
			  //cout << "[FilterQBox] predicate["<<jj<<"] evaluated to false" << endl; 
		    if (jj == (_num_predicates - 1))
		      {
				  //cout << "[FilterQBox] all predicates evaluated to false!" << endl;
			if (_useFalsePort) {
				//cout << "[FilterQBox] Passing tuple to false port (assumed last output port)" << endl;
			//*************** copy the entire tuple from the instream to the outstream
			  memcpy(outStream_ptr[_num_output_ports-1],
				 _inStream[0] + (ii*_tuple_size),
				 _tuple_size);

			  //cout << "[FilterQBox] at predicate " << jj << " OUTPUT:";
			  ////printTuple(outStream_ptr[_num_output_ports-1], _tuple_descr);

			  // Move the pointer to the end of the outstream up to
			  //   the proper position
			  outStream_ptr[_num_output_ports-1] += _tuple_size;

			  // Track outputted tuple
			  good_value_count++;
			  return_val.output_tuples_array[_num_output_ports-1]++;

			} else {
				//cout << "[FilterQBox] False port not in use, bye bye tuple!" << endl;
			}

		      }
		    jj++;
		  }
	      } // end while loop for all predicates
	  } // end for loop over all tuples in the input stream

	return_val.kept_input_count = 0;
	return_val.output_tuples = good_value_count; // THESE ARE DEPRECATED
	return_val.output_tuple_size = _tuple_size;

	// hack for now...
	// Not needed since WorkerThread 1.26.6.2 changes 
	//return_val.kept_input_count_array = new int[1];
	//return_val.kept_input_count_array[0] = 0;
	//cout << "[FilterQBox] *********** END ************* " << endl;
	
	return return_val;
    
}


void FilterQBox::setBox(const char* modifier) {
  setPredicates(modifier);
}


  
