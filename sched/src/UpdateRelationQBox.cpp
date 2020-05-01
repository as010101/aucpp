#include "UpdateRelationQBox.H"
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

Box_Out_T UpdateRelationQBox::doBox()
{
  // Prepare internal pointer to instream.
  char* inStream = _inStream[0];

  // Set tuple size.
  int tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
  assert(tuple_size > key_length);

  // Loop through all the tuples in the input stream
  for (int i = 0; i < _train_size[0]; i++)
    { 
      cerr << "[UpdateRelationQBox] INPUT:";
      printTuple(inStream + (i*tuple_size), _tuple_descr);
      void *tuple_data = inStream + (i*tuple_size) + getTsSize() + getSidSize();

      // Key is key_length, starting after timestamp and sid.
      Dbt key(tuple_data, key_length);
      Dbt data(tuple_data, _tuple_descr->getSize());
      
      int ret;
      try {
	ret = db->put(NULL, &key, &data, 0);
	cerr << "[UpdateRelationQBox] stored at key " << printDbt(key)
	     << " value " << printDbt(data) << endl;
	if (ret != 0) {
	  db->err(ret, "DB->put");
	  assert(false);
	}
      } catch (DbException& e) {
	cout << "UpdateRelationQBox::doBox open DbException: ("
	     << e.get_errno() << ") " << e.what() << endl;
	assert(false);
      }
      
      // Check that the right thing was stored.
      Dbt newData;
      db->get(NULL, &key, &newData, 0);
      cerr << "[UpdateRelationQBox] retrieve at key " << printDbt(key)
	   << " value " << printDbt(newData) << endl;
    } // end for loop over all tuples in the input stream

  return_val.kept_input_count = 0;
  // We have no output tuples.
  return_val.output_tuples_array = new int[1];
  return_val.output_tuples_array[0] = 0;
  return_val.output_tuples = 0;
  return_val.output_tuple_size = tuple_size;

  cerr << "[UpdateRelationQBox] *********** END ************* " << endl;
  
  return return_val;
}

void UpdateRelationQBox::setBox(Db *db, int key_length) {
  this->db = db;
  this->key_length = (size_t)key_length;
}
