#include "LRReadRelationQBox.H"
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

static int
time_to_day(int t) {
  // TODO: Get this right.
  return t/(24*60*60);
}

void 
LRReadRelationQBox::doCalcTollNewRead(void* in_tuple, int in_length,
				      void* out_stream, int *count, int *size) {
  // Should get 13 ints as the tuple data.
  assert(in_length == getTsSize() + getSidSize() + 13*sizeof(int));

  // Tuple: (car, time, lexp, lseg, ldir, llane, ltoll, acc, exp, seg, dir, lane, status
  void *in_tuple_data = ((char*)in_tuple + getTsSize() + getSidSize());
  int *tuple_i = (int *)in_tuple_data;

  *count = 0;

  // First get todays balance for this expressway out of the daily relation.
  int daily_balance;
  {
    Db *daily = catbox->getDailyDb();
    // Key is car_id, day, Pos exp)
    int day = time_to_day(tuple_i[1]);
    int key_i[3];
    key_i[0] = tuple_i[0];
    key_i[1] = day;
    key_i[2] = tuple_i[2];
    Dbt key(key_i, sizeof(key_i));

    Dbt row;
    daily->get(NULL, &key, &row, 0);
    if (row.get_data() == NULL) {
      daily_balance = 0;
    } else {
      int *row_i = (int *)row.get_data();
      daily_balance = row_i[3];
    }
  }
  //cerr << "LRReadRelationQBox::doCalcTollNewRead daily balance for carid " << tuple_i[0]
  //     << " is " << daily_balance << "." << endl;
  
  // Get the most up to date toll for this position.
  int toll;
  {
    // Looking for a row between time and time - 60 seconds.
    int max_time = tuple_i[1];
    int min_time = max_time - 60;

    Db *stats = catbox->getStatsDb();
    // Key is (time, pos exp, pos seg, pos dir)
    int key_i[4];
    key_i[0] = min_time;
    key_i[1] = tuple_i[8];
    key_i[2] = tuple_i[9];
    key_i[3] = tuple_i[10];
    Dbt key(key_i, sizeof(key_i));
    
    // Sort function in
    // RelationEnvironment::setupLinearRoadEnvironment will assure
    // that we sort first by pos*, then by time.
    // Pull down a cursor.
    Dbc *cursorp;
    stats->cursor(NULL, &cursorp, 0);
    
    // This should get us a cursor at the first key that is greater
    // than our key, which should be for a matching location and an
    // acceptable time.
    //cerr << "LRReadRelationQBox::doCalcTollNewRead getting at "
    //	 << printDbt(key) << endl;
    Dbt stat_row;
    cursorp->get(&key, &stat_row, DB_SET_RANGE);
    if (stat_row.get_data() == NULL) {
      //cerr << "LRReadRelationQBox::doCalcTollNewRead Got a null stat row." << endl;
      // No statistics, zero toll
      toll = 0;
    } else {
      int *db_key_i = (int *)stat_row.get_data();
      //cerr << "LRReadRelationQBox::doCalcTollNewRead read " << printDbt(stat_row)
      //	   << " at " << printDbt(key) << endl;
      if (db_key_i[0] >= min_time && db_key_i[0] <= max_time &&
	  db_key_i[1] == key_i[1] && db_key_i[2] == key_i[2] &&
	  db_key_i[3] == key_i[3]) {
	cerr << "doCalcTollNewRead: row is " << (max_time - db_key_i[0])
	     << " seconds old." << endl;
	cerr << "\tPosition (" << key_i[1] << "," << key_i[2] << "," << key_i[3]
	     << ") dbtime is " << db_key_i[0] << endl;
	toll = db_key_i[7];
      } else {
	cerr << "doCalcTollNewRead: row is " << (min_time - db_key_i[0])
	     << " seconds too old." << endl;
	cerr << "\tPosition (" << key_i[1] << "," << key_i[2] << "," << key_i[3]
	     << ") dbtime is " << db_key_i[0] << endl;
	// No statistics, zero toll
	//toll = 0;
	// Use old stats for demo.
	toll = db_key_i[7];
      }
    }
  }

  // We now have daily_balance and
  // stat_row = (time, exp, seg, dir, LAV, CAV, Cnt, Toll)
  // Build the output.
  int output[11];
  output[0] = tuple_i[0]; // car
  output[1] = tuple_i[1]; // time
  output[2] = tuple_i[2]; // lexp
  output[3] = tuple_i[3]; // lseg
  output[4] = tuple_i[4]; // ldir
  // No llane from input
  output[5] = tuple_i[6]; // Ltoll
  // No acc from input
  output[6] = tuple_i[8]; // exp
  output[7] = tuple_i[9]; // seg
  output[8] = tuple_i[10]; // dir
  output[9] = tuple_i[11]; // lane
  output[10] = toll; // toll
  
  //cerr << "LRReadRelationQBox::doReadPosReadAccts writing tuple: "
  //     << printCharArray((const char*)output, sizeof(output)) << endl;

  // Put output on out_stream
  // Copy the timestamp and streamid from our input.
  memcpy(out_stream, in_tuple, getTsSize() + getSidSize());
  out_stream = ((char*)out_stream) + (getTsSize() + getSidSize());
  // Copy the key data.
  memcpy(out_stream, output, sizeof(output));
  // Set size and count
  *size = sizeof(output) + getTsSize() + getSidSize();
  *count = 1;
}

void
LRReadRelationQBox::doSegStatReadAcc(void* in_tuple, int in_length,
				     void* out_stream, int *count, int *size) {
  // Should get 7 ints as the tuple data.
  assert(in_length == getTsSize() + getSidSize() + 7*sizeof(int));
  // Tuple: (exp, seg, dir, time, LAV, CAV, Cnt)
  void *in_tuple_data = ((char*)in_tuple + getTsSize() + getSidSize());
  int *tuple_i = (int *)in_tuple_data;

  
  /*
   *cerr << "LRReadRelationQBox::doSegStatReadAcc called for "
   *   << "exp " << tuple_i[0] << " seg " << tuple_i[1]
   *   << " dir " << tuple_i[2] << " time " << tuple_i[3]
   *   << "." << endl;
   */
  // Look up if there is an accident.
  int accident_p = 0; //No accident.
  {
    /* TODO: Actually find accidents.
    // Looking for a row between time and time - 20 minutes.
    int max_time = tuple_i[3];
    int min_time = max_time - (20*60);

    // 
    assert(false);

    Db *stats = catbox->getAccDb();
    // Key is (time, pos exp, pos seg, pos dir)
    int key_i[4];
    key_i[0] = min_time;
    key_i[1] = tuple_i[0];
    key_i[2] = tuple_i[1];
    key_i[3] = tuple_i[2];
    Dbt key(key_i, sizeof(key_i));
    
    // Sort function in
    // RelationEnvironment::setupLinearRoadEnvironment will assure
    // that we sort first by pos*, then by time.
    // Pull down a cursor.
    Dbc *cursorp;
    stats->cursor(NULL, &cursorp, 0);
    
    // This should get us a cursor at the first key that is greater
    // than our key, which should be for a matching location and an
    // acceptable time.
    //cerr << "LRReadRelationQBox::doCalcTollNewRead getting at "
    //	 << printDbt(key) << endl;
    Dbt stat_row;
    cursorp->get(&key, &stat_row, DB_SET_RANGE);
    if (stat_row.get_data() == NULL) {
      //cerr << "LRReadRelationQBox::doCalcTollNewRead Got a null stat row." << endl;
      // No statistics, zero toll
      toll = 0;
    } else {
      int *db_key_i = (int *)stat_row.get_data();
      //cerr << "LRReadRelationQBox::doCalcTollNewRead read " << printDbt(stat_row)
      //	   << " at " << printDbt(key) << endl;
      if (db_key_i[0] >= min_time && db_key_i[0] <= max_time &&
	  db_key_i[1] == key_i[1] && db_key_i[2] == key_i[2] &&
	  db_key_i[3] == key_i[3]) {
	toll = db_key_i[7];
      } else {
	cerr << "LRReadRelationQBox::doCalcTollNewRead row did not match requirements."
	     << endl;
	// No statistics, zero toll
	toll = 0;
      }
    }
    */
  }

  int output[8];
  output[0] = tuple_i[3];
  output[1] = tuple_i[0];
  output[2] = tuple_i[1];
  output[3] = tuple_i[2];
  output[4] = tuple_i[4];
  output[5] = tuple_i[5];
  output[6] = tuple_i[6];
  output[7] = accident_p;
  
  //cerr << "LRReadRelationQBox::doReadPosReadAccts writing tuple: "
  //     << printCharArray((const char*)output, sizeof(output)) << endl;

  // Put output on out_stream
  // Copy the timestamp and streamid from our input.
  memcpy(out_stream, in_tuple, getTsSize() + getSidSize());
  out_stream = ((char*)out_stream) + (getTsSize() + getSidSize());
  // Copy the key data.
  memcpy(out_stream, output, sizeof(output));
  // Set size and count
  *size = sizeof(output) + getTsSize() + getSidSize();
  *count = 1;
}

void
LRReadRelationQBox::doReadPosReadAccts(void* in_tuple, int in_length,
				       void* out_stream, int *count, int *size) {
  // Should get 8 ints as the tuple data.
  assert(in_length == getTsSize() + getSidSize() + 8*sizeof(int));

  Db *db = catbox->getAcctsDb();

  // Tuple: (type, time, car, speed, exp, lane, dir, X-position)
  void *in_tuple_data = ((char*)in_tuple + getTsSize() + getSidSize());
  int type, time, carid, speed, exp, seg, dir, lane;
  {
    int *tuple_i = (int *)in_tuple_data;

    type = tuple_i[0];
    time = tuple_i[1];
    carid = tuple_i[2];
    speed = tuple_i[3];
    exp = tuple_i[4];
    seg = tuple_i[7] / 5280;
    dir = tuple_i[6];
    lane = tuple_i[5];
  }

  // Print tuple data.
  /*
   *cout << "doReadPosReadAccts:" << " type " << type << " time " << time
   *   << " car " << carid << " speed " << speed << " exp " << exp
   *   << " lane " << lane << " dir " << dir << " seg " << seg << endl;
   */
  
  // Key is car id.
  Dbt key(&(carid), sizeof(int));

  // Default to null.
  const int null_row[9] = {LR_NULL, LR_NULL, LR_NULL,
			   LR_NULL, LR_NULL, LR_NULL,
			   LR_NULL, LR_NULL, LR_NULL};
  const int *row_i = null_row;
  Dbt row;
  int ret;
  try {
    ret = db->get(NULL, &key, &row, 0);
    if (ret != 0 && ret != DB_NOTFOUND) {
      db->err(ret, "DB->put");
      // Just move on.
      // TODO: Make sure it was a non-present value. Decide what
      // to do with that.
    }
    if (row.get_data() == NULL) {
      //cerr << "[LRReadRelationQBox] retrieve at key " << printDbt(key)
      //   << " null value " << endl;
      // Leave row_i null.
    } else {
      //cerr << "[ReadRelationQBox] retrieve at key " << printDbt(key)
      //	   << " value " << printDbt(row) << endl;
      row_i = (int*) row.get_data();
    }
  } catch (DbException& e) {
    cerr << "ReadRelationQBox::doBox open DbException: ("
	 << e.get_errno() << ") " << e.what() << endl;
    // Just move on.
    // TODO: Make sure it was a non-present value. Decide what
    // to do with that.
  }

  // Calculate status.
  int status;
  if (((row_i[2] == LR_NULL) && (row_i[3] == LR_NULL) &&
       (row_i[3] == LR_NULL))	// LastPos == NULL
      ||			// OR 
      ((row_i[2] != exp) || (row_i[3] != seg) ||
       (row_i[4] != dir))) // LastPos != Pos
    {
      //cout << "ReadRelationQBox::doReadPosReadAccts status is new." << endl;
      status = LR_STATUS_NEW;
    }
  else if (((row_i[2] == exp) && (row_i[3] == seg)
	    && (row_i[4] == dir)) // LastPos == Pos
	   &&		// AND
	   ((row_i[5] == lane) // LastLane == Lane
	    ||		// OR
	    (lane != 0))) // Lane != 0
    {
      //cout << "ReadRelationQBox::doReadPosReadAccts status is same." << endl;
      status = LR_STATUS_SAME;
    }
  else if (lane == 0) 	// Lane == 0
    {
      //cout << "ReadRelationQBox::doReadPosReadAccts status is exit." << endl;
      status = LR_STATUS_EXIT;
    }
  else
    {
      cerr << "Bad branch in ReadPosStatusF" << endl;
      //assert(false);
      status = LR_STATUS_EXIT;
    }

  int output[13];
  output[0] = carid; // car id.
  output[1] = time; // time.
  output[2] = row_i[2]; //LPos exp
  output[3] = row_i[3]; //LPos seg
  output[4] = row_i[4]; //LPos  dir
  output[5] = row_i[5]; //LLane
  output[6] = row_i[6]; //LToll
  output[7] = row_i[8]; //Acc
  output[8] = exp; // Pos exp
  output[9] = seg; // Pos seg
  output[10] = dir; // Pos dir
  output[11] = lane; // Lane
  output[12] = status; // Status

  //cerr << "LRReadRelationQBox::doReadPosReadAccts writing tuple: "
  //     << printCharArray((const char*)output, sizeof(output)) << endl;

  // Put output on out_stream
  // Copy the timestamp and streamid from our input.
  memcpy(out_stream, in_tuple, getTsSize() + getSidSize());
  out_stream = ((char*)out_stream) + (getTsSize() + getSidSize());
  // Copy the key data.
  memcpy(out_stream, output, sizeof(output));
  // Set size and count
  *size = sizeof(output) + getTsSize() + getSidSize();
  *count = 1;
}

void
LRReadRelationQBox::doAccAlertReadAccts(void* in_tuple, int in_length,
					void* out_stream, int *count, int *size) {
  // Should get 6 ints as the tuple data.
  assert(in_length == getTsSize() + getSidSize() + 6*sizeof(int));

  Db *db = catbox->getAcctsByPosDb();
  Dbc *cursorp;
  db->cursor(NULL, &cursorp, 0);

  // Tuple: (time, Pos exp, Pos seg, Pos dir, accident)
  void *in_tuple_data = ((char*)in_tuple + getTsSize() + getSidSize());
  int *tuple_i = (int *)in_tuple_data;

  // Key is last 5 segments.
  int key_i[3];
  key_i[0] = tuple_i[1]; //posexp
  key_i[2] = tuple_i[3]; //posdir
  int first_seg, last_seg;
  if (tuple_i[3] == 0) {
    // TODO: Make sure this is correct.
    // Eastbound
    first_seg = max(0, tuple_i[2]-4);
    last_seg = tuple_i[2];
  } else {
    // Westbound
    // Going over the last segment is ok.
    first_seg = tuple_i[2];
    last_seg = tuple_i[2]+4;
  }

  *count = 0;
  *size = 0;

  for(key_i[1] = first_seg; key_i[1] <= last_seg; key_i[1]++) {
    Dbt key(key_i, sizeof(key_i));
    Dbt read_key, data;

    //cerr << "Doing fetch at key " << printDbt(key) << endl;
    int ret = cursorp->get(&key, &data, DB_SET);
    if (data.get_data() == NULL) {
      cerr << "Got a null data. ret = " << ret << endl;
    } else {
      do {
	//cerr << "Retrieve at key " << printDbt(key)
	//     << " value " << printDbt(data) << endl;
	// Create an output tuple with data.
	int *data_i = (int *)data.get_data();
	int output[4];
	output[0] = data_i[0]; // car id
	output[1] = data_i[2]; // pos exp
	output[2] = data_i[3]; // pos seg
	output[3] = data_i[4]; // pos dir
	
	// Put output on out_stream
	// Copy the timestamp and streamid from our input.
	memcpy(out_stream, in_tuple, getTsSize() + getSidSize());
	out_stream = ((char*)out_stream) + (getTsSize() + getSidSize());
	// Copy the key data.
	memcpy(out_stream, output, sizeof(output));
	// Set size and count
	*size = sizeof(output) + getTsSize() + getSidSize();
	*count++;
      } while ((cursorp->get(&read_key, &data, DB_NEXT) == 0) &&
	       (memcmp(key.get_data(), read_key.get_data(), read_key.get_size()) == 0));
    }
  }
}



Box_Out_T LRReadRelationQBox::doBox()
{
  // Prepare internal pointer to instream and outstream.
  char* inStream = _inStream[0];
  char* outStream = _outStream[0];

  // Set tuple size.
  int in_tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
  // Flag value while it is unset.
  int out_tuple_size = 0;

  // The output-tuples tracker
  return_val.output_tuples_array = new int[1];
  return_val.output_tuples_array[0] = 0;

  // Loop through all the tuples in the input stream
  for (int i = 0; i < _train_size[0]; i++)
    { 
      //cerr << "[LRReadRelationQBox] INPUT:";
      //printTuple(inStream + (i*in_tuple_size), _tuple_descr);
      void *tuple = inStream + (i*in_tuple_size);

      int thecount, thesize;

      // Dispatch based on magic_number
      switch (magic_number) {
      case LR_CALC_TOLL_NEW_READ:
	doCalcTollNewRead(tuple, in_tuple_size,
			  outStream, &thecount, &thesize);
	break;
      case LR_SEG_STAT_READ_ACC:
	doSegStatReadAcc(tuple, in_tuple_size,
			 outStream, &thecount, &thesize);
	break;
      case LR_READ_POS_READ_ACCTS:
	doReadPosReadAccts(tuple, in_tuple_size,
			   outStream, &thecount, &thesize);
	break;
      case LR_ACC_ALERT_READ_ACCTS:
	doAccAlertReadAccts(tuple, in_tuple_size,
			    outStream, &thecount, &thesize);
	break;
      default:
	cerr << "[LRReadRelationQBox] unknown magic number "
	     << magic_number << endl;
      }

      // Got a tuple out. That will go on our output stream.
      if (out_tuple_size == 0) {
	out_tuple_size = thesize;
      } else {
	// Make sure size didn't change.
	assert(out_tuple_size == thesize);
      }
      
      //cerr << "[LRReadRelationQBox] thesize=" << thesize
      //	   << " thecount=" << thecount << endl;

      outStream += thesize * thecount;
      return_val.output_tuples_array[0] += thecount;
    } // end for loop over all tuples in the input stream

  return_val.kept_input_count = 0;
  // We have no output tuples.
  return_val.output_tuples = return_val.output_tuples_array[0]; // Deprecated?
  return_val.output_tuple_size = out_tuple_size;

  //cerr << "[LRReadRelationQBox] *********** END ************* " << endl;
  
  return return_val;
}

void LRReadRelationQBox::setBox(LRReadRelationBox* catbox, int magic_number) {
  this->catbox = catbox;
  this->magic_number = magic_number;
}
