#include "LRUpdateRelationQBox.H"
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

// All of these functions take the inTuple pointer, they will know the
// inTuple size.

void
LRUpdateRelationQBox::doUpdateAccts(void* tuple, int size) {
  Db *db = catbox->getAcctsDb();

  // Tuple: (car, time, LPos exp, LPos seg, LPos dir, LToll, Pos exp, Pos seg, Pos dir, Lane, toll)
  int *tuple_i = (int *)tuple;

  // Key is car_id, first 4 bytes.
  Dbt key(tuple, 4);

  // Get old balance, if any.
  Dbt old_row;
  db->get(NULL, &key, &old_row, 0);
  int old_balance;
  if (old_row.get_data() == NULL) {
    //cerr << "[LRUpdateRelationQBox::doUpdateAccts] retrieve at car_id " << tuple_i[0]
    //     << " null balance." << endl;
    old_balance = 0;
  } else {
    //cerr << "[LRUpdateRelationQBox::doUpdateAccts] retrieve at car_id " << tuple_i[0]
    //     << " old value " << printDbt(old_row) << endl;
    int *old_row_i = (int *)old_row.get_data();
    old_balance = old_row_i[7];
  }
  //cerr << "[LRUpdateRelationQBox::doUpdateAccts] retrieve at car_id " << tuple_i[0]
  //     << " old balance " << old_balance << endl;

  // Make table entry.
  int entry[9];
  entry[0] = tuple_i[0]; // Car id
  entry[1] = tuple_i[1]; // Time
  entry[2] = tuple_i[6]; // Pos exp
  entry[3] = tuple_i[7]; // Pos seg
  entry[4] = tuple_i[8]; // Pos dir
  entry[5] = tuple_i[9]; // LLane
  entry[6] = tuple_i[10]; // Last Toll gets Toll.
  entry[7] = old_balance - tuple_i[5]; // new balance is old balance minus last toll.
  entry[8] = 0; // Accident = false.

  Dbt data(entry, sizeof(entry));
      
  try {
    db->put(NULL, &key, &data, 0);
    //cerr << "[LRUpdateRelationQBox::doUpdateAccts] stored at key " << printDbt(key)
    //	 << " new balance " << entry[7] << endl;
  } catch (DbException& e) {
    cout << "LRUpdateRelationQBox::doUpdateAccts open DbException: ("
	 << e.get_errno() << ") " << e.what() << endl;
    assert(false);
  }
}

static int
time_to_day(int t) {
  // TODO: Get this right.
  return t/(24*60*60);
}

void
LRUpdateRelationQBox::doUpdateDaily(void* tuple, int size) {
  Db *db = catbox->getDailyDb();

  // Tuple: (car, time, LPos exp, LPos seg, LPos dir, LToll, Pos exp, Pos seg, Pos dir, Lane, toll)
  int *tuple_i = (int *)tuple;

  int day = time_to_day(tuple_i[1]);

  // Key is car_id, day, LPos exp)
  int key_i[3];
  key_i[0] = tuple_i[0];
  key_i[1] = day;
  key_i[2] = tuple_i[2];
  Dbt key(key_i, sizeof(key_i));

  // Get old balance, if any.
  Dbt old_row;
  db->get(NULL, &key, &old_row, 0);
  int old_daily_bal;
  if (old_row.get_data() == NULL) {
    //cerr << "[LRUpdateRelationQBox::doUpdateDaily] retrieve at key " << printDbt(key)
    //     << " null value" << endl;
    old_daily_bal = 0;
  } else {
    //cerr << "[LRUpdateRelationQBox::doUpdateDaily] retrieve at key " << printDbt(key)
    //     << " old value " << printDbt(old_row) << endl;
    int *old_row_i = (int *)old_row.get_data();
    old_daily_bal = old_row_i[3];
  }
  //cerr << "[LRUpdateRelationQBox::doUpdateDaily] retrieve at key " << printDbt(key)
  //     << " old balance " << old_daily_bal << endl;

  // Make table entry.
  int new_entry[4];
  new_entry[0] = tuple_i[0]; // Car id
  new_entry[1] = day; // Time
  new_entry[2] = tuple_i[2]; // Pos exp
  new_entry[3] = old_daily_bal - tuple_i[5]; // old balance minus last toll.

  Dbt data(new_entry, sizeof(new_entry));
      
  try {
    db->put(NULL, &key, &data, 0);
    //cerr << "[LRUpdateRelationQBox::doUpdateDaily] stored at key " << printDbt(key)
    //	 << " new balance " << new_entry[3] << endl;
      //<< printDbt(data) << endl;
  } catch (DbException& e) {
    cout << "LRUpdateRelationQBox::doUpdateDaily open DbException: ("
	 << e.get_errno() << ") " << e.what() << endl;
    assert(false);
  }
}

void
LRUpdateRelationQBox::doUpdateStats(void* tuple, int size) {
  // Tuple: (time, Pos exp, Pos seg, Pos dir, LAV, CAV, Cnt, toll)
  assert(size == 8*sizeof(int));

  Db *db = catbox->getStatsDb();
  
  {
    // Debugging stuff.
    int *tuple_i = (int*)tuple;
    cout << "Update stats: time " << tuple_i[0] << " pos ("
	 << tuple_i[1] << "," << tuple_i[2] << "," << tuple_i[3]
	 << ") LAV=" << tuple_i[4] << " CAV=" << tuple_i[5]
	 << " Cnt=" << tuple_i[6] << " Toll=" << tuple_i[7]
	 << endl;
  }

  // Key is first 4 ints.
  Dbt key(tuple, 4*sizeof(int));
  Dbt data(tuple, size);
  try {
    db->put(NULL, &key, &data, 0);
    //cerr << "[LRUpdateRelationQBox::doUpdateStats] stored at " << printDbt(key)
    // << " value " << printDbt(data) << endl;
  } catch (DbException& e) {
    cout << "LRUpdateRelationQBox::doUpdateStats put DbException: ("
	 << e.get_errno() << ") " << e.what() << endl;
    assert(false);
  }
}

void
LRUpdateRelationQBox::doUpdateAcc(void* tuple, int size) {
  Db *acc_db = catbox->getAccDb();

  // Tuple: (car, time, Pos exp, Pos seg, Pos dir, Lane, Accident)
  int *tuple_i = (int *)tuple;

  // Accident should always be one.
  if (tuple_i[6] == 0) {
    cerr << "LRUpdateRelationQBox::doUpdateAcc Accident is false, this should not happen." << endl;
    assert(false);
  }

  // First write into the acc table. Key and data are the same.
  int acc_i[4];
  acc_i[0] = tuple_i[1];
  acc_i[1] = tuple_i[2];
  acc_i[2] = tuple_i[3];
  acc_i[3] = tuple_i[4];
  Dbt acc(acc_i, sizeof(acc_i));
  
  try {
    acc_db->put(NULL, &acc, &acc, 0);
    //cerr << "[LRUpdateRelationQBox::doUpdateacc] stored " << printDbt(acc) << endl;
      //<< printDbt(data) << endl;
  } catch (DbException& e) {
    cout << "LRUpdateRelationQBox::doUpdateAcc put DbException: ("
	 << e.get_errno() << ") " << e.what() << endl;
    assert(false);
  }
}
	  
void
LRUpdateRelationQBox::doUpdateAcctsWithAcc(void* tuple, int size) {
  Db *db = catbox->getAcctsDb();
  Db *sdb = catbox->getAcctsByPosDb();
  Dbc *cursorp;
  sdb->cursor(NULL, &cursorp, 0);

  // Tuple: (car, time, Pos exp, Pos seg, Pos dir, Lane, Accident)
  int *tuple_i = (int *)tuple;

  // Key is last 5 segments.
  int key_i[3];
  key_i[0] = tuple_i[2]; //posexp
  key_i[2] = tuple_i[4]; //posdir
  int first_seg, last_seg;
  if (tuple_i[4] == 0) {
    // TODO: Make sure this is correct.
    // Eastbound
    first_seg = max(0, tuple_i[3]-4);
    last_seg = tuple_i[3];
  } else {
    // Westbound
    // Going over the last segment is ok.
    first_seg = tuple_i[3];
    last_seg = tuple_i[3]+4;
  }

  for(key_i[1] = first_seg; key_i[1] <= last_seg; key_i[1]++) {
    Dbt key(key_i, sizeof(key_i));
    Dbt pkey;
    Dbt read_key, data;

    //cerr << "Doing fetch at key " << printDbt(key) << endl;
    int ret = cursorp->pget(&key, &pkey, &data, DB_SET);
    if (data.get_data() == NULL) {
      //cerr << "Got a null data. ret = " << ret << endl;
    } else {
      do {
	//cerr << "Retrieve at key " << printDbt(key)
	//     << " value " << printDbt(data) << endl;
	// Rewrite the accident part of the data.
	int *di = (int *)data.get_data();
	di[6] = 1;
	// Store the data.
	db->put(NULL, &pkey, &data, 0);
	// TODO: This might foul up the secondary index, or something.
	// Not sure.
      } while ((cursorp->pget(&read_key, &pkey, &data, DB_NEXT) == 0) &&
	       (memcmp(key.get_data(), read_key.get_data(), read_key.get_size()) == 0));
    }
  }
}
  

Box_Out_T LRUpdateRelationQBox::doBox()
{
  // Prepare internal pointer to instream.
  char* inStream = _inStream[0];

  // Set tuple size.
  int tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
  assert(tuple_size > magic_number);

  // Loop through all the tuples in the input stream
  for (int i = 0; i < _train_size[0]; i++)
    { 
      //cerr << "[LRUpdateRelationQBox " << magic_number << "] INPUT:";
      //printTuple(inStream + (i*tuple_size), _tuple_descr);
      void *tuple_data = inStream + (i*tuple_size) + getTsSize() + getSidSize();

      switch (magic_number) {
      case LR_UPDATE_ACCTS:
	doUpdateAccts(tuple_data, _tuple_descr->getSize());
	break;
      case LR_UPDATE_DAILY:
	doUpdateDaily(tuple_data, _tuple_descr->getSize());
	break;
      case LR_UPDATE_STATS:
	doUpdateStats(tuple_data, _tuple_descr->getSize());
	break;
      case LR_UPDATE_ACC:
	doUpdateAcc(tuple_data, _tuple_descr->getSize());
	doUpdateAcctsWithAcc(tuple_data, _tuple_descr->getSize());
	break;
      default:
	cerr << "[LRUpdateRelationQBox] unknown magic number "
	     << magic_number << endl;
      }
    }

  return_val.kept_input_count = 0;
  // We have no output tuples.
  return_val.output_tuples_array = new int[1];
  return_val.output_tuples_array[0] = 0;
  return_val.output_tuples = 0;
  return_val.output_tuple_size = tuple_size;

  //cerr << "[LRUpdateRelationQBox] *********** END ************* " << endl;
  
  return return_val;
}

void LRUpdateRelationQBox::setBox(LRUpdateRelationBox *catbox, int magic_number) {
  this->catbox = catbox;
  this->magic_number = magic_number;
}
