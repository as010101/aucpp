#include "ReadRelationQBox.H"
#include <ios>

static string printCharArray (const char *a, int len) {
  ostringstream oss;
  oss.flags(ios::hex);
  int v = *a;
  oss << v;
  for (int i = 1; i < len; i++) {
    v = *(a+i);
    oss << " " << v;
  }
  return oss.str();
}

static string printDbt (const Dbt &t) {
  return printCharArray((char *)t.get_data(), t.get_size());
}

Box_Out_T ReadRelationQBox::doBox()
{
  // Prepare internal pointer to instream and outstream.
  char* inStream = _inStream[0];
  char* outStream = _outStream[0];

  // Set tuple size.
  int in_tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
  assert(in_tuple_size > key_length);
  // Flag value while it is unset.
  int out_tuple_size = 0;

  // The output-tuples tracker
  return_val.output_tuples_array = new int[1];
  return_val.output_tuples_array[0] = 0;

  // Loop through all the tuples in the input stream
  for (int i = 0; i < _train_size[0]; i++)
    { 
      cerr << "[ReadRelationQBox] INPUT:";
      printTuple(inStream + (i*in_tuple_size), _tuple_descr);
      void *tuple_data = inStream + (i*in_tuple_size) + getTsSize() + getSidSize();

      // Key is key_length, starting after timestamp and sid.
      Dbt key(tuple_data, key_length);
      Dbt data;
      
      int ret;
      try {
	ret = db->get(NULL, &key, &data, 0);
	if (ret != 0) {
	  db->err(ret, "DB->put");
	  // Just move on.
	  // TODO: Make sure it was a non-present value. Decide what
	  // to do with that.
	  continue;
	}

	if (data.get_data() == NULL) {
	  cerr << "[ReadRelationQBox] retrieve at key " << printDbt(key)
	       << " null value " << endl;
	  continue;
	}

	cerr << "[ReadRelationQBox] retrieve at key " << printDbt(key)
	     << " value " << printDbt(data) << endl;

	// Got a tuple out. That will go on our output stream.
	if (out_tuple_size == 0) {
	  out_tuple_size = getTsSize() + getSidSize() + data.get_size();
	} else {
	  // Make sure size didn't change.
	  assert(out_tuple_size ==
		 (getTsSize() + getSidSize() + data.get_size()));
	}
	// Copy the timestamp and streamid from our input.
	memcpy(outStream, inStream + (i*in_tuple_size), getTsSize() + getSidSize());
	outStream += (getTsSize() + getSidSize());
	// Copy the key data.
	memcpy(outStream, data.get_data(), data.get_size());
	
	// Move the pointer to the end of the outstream up to the proper position
	outStream += out_tuple_size;
	
	// Track outputted tuple
	return_val.output_tuples_array[0]++;
      } catch (DbException& e) {
	cout << "ReadRelationQBox::doBox open DbException: ("
	     << e.get_errno() << ") " << e.what() << endl;
	// Just move on.
	// TODO: Make sure it was a non-present value. Decide what
	// to do with that.
      }
    } // end for loop over all tuples in the input stream

      return_val.kept_input_count = 0;
  // We have no output tuples.
  return_val.output_tuples = return_val.output_tuples_array[0]; // Deprecated?
  return_val.output_tuple_size = out_tuple_size;

  cerr << "[ReadRelationQBox] *********** END ************* " << endl;
  
  return return_val;
}

void ReadRelationQBox::setBox(Db *db, int key_length) {
  this->db = db;
  this->key_length = (size_t)key_length;
}
