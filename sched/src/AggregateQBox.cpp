#include "AggregateQBox.H"
#include "parseutil.H"


/**
 * Modifier here looks like
 * [target-attrib] '=' [agg function] '~' [agg attrib on] '~' [window size] '~'
 * [advance] '~' [timeout] '~' [sort-by attrib] '~' [slack] '~'
 * [comma-sep-list-of-group-by-attribs]
 *
 * [target-attrib] - internal gui stuff
 * [group-by-attribs] is optional
 * [window size] is "nn(space)VALUES" or "nn(space)TUPLES"
 *
 * Example:
 * n1=AVG~:0:i:0:4:~10~:0:i:0:4 > 100~1.0~:0:i:8:4:~1~
 */

void AggregateQBox::setBox(const char* modifier_str, HashForAggregate* hash_of_everything, unsigned int* tuple_counter, vector<char*> *tuple_store) {


  //cout << "[AggregateQBox] ENTERED SETBOX(string)" << endl;
  //cout << " MODIFIER_STR LOOKS LIKE THIS: [" << modifier_str << "]" <<endl;

  // Gimme my hash back
  _hash_of_everything = hash_of_everything;
  // and my counter back (if we even need it) - make the value into a char* now
  _tuple_counter = new char[sizeof(int)];
  memcpy(_tuple_counter,tuple_counter,sizeof(int));
  _tuple_counter_state = tuple_counter;

  // tuple store back
  _tuple_store = tuple_store;


  // Parsing time (eek)
  // Let's do it on C++ strings, hopefully simpler-lookin
  string modifier(modifier_str);

  // Let's try unpackString...
  vector<string> v = unpackString(modifier, "=~");
  // ..ohh nifty ok let's do it
  vector<string>::iterator pos = v.begin();

  // skip over the first (gui internal stuff), unless you want it for something.. (see fidelity alarm)
  const char* attribname_cstr = (*pos).c_str();
  ++pos;

  { // The aggregate function (yuk)
    string agg_str = *pos;
    const char* agg_cstr = (*pos).c_str(); // Wanna save a byte? you dont need c_str, just str since FieldExt doesnt care for the null
    // Get the aggregate function attribute
    ++pos;
    const char *s = (*pos).c_str();
    int str_len = strlen(s);
    char* agg_attrib_cstr = new char[str_len];
    memcpy(agg_attrib_cstr, (*pos).c_str(), str_len);
    //    const char* agg_attrib_cstr = (*pos).c_str();
    
    
    char agg_attrib_type = *(index(index(agg_attrib_cstr,':')+1,':')+1); // something a la :0:i:0:4:, I want just the i
    
    // Make the attribute

    // FIDELITY DEMO "HACK" FIDELITY1 means alarm raising, FIDELITYALARM is for the 100-counting (yes, messed up names)
    // Note: the FIDELITY_HACK code is NO LONGER NEEDED (since the code now correctly supports win size by tuples)

    if (agg_str == "FIDELITY1") {
      _af = new Fidelity1AF(agg_attrib_cstr);
    }
    else if (agg_str == "FIDELITYALARM") {
      _af = new FidelityAlarmAF(attribname_cstr);
    }
    else if (agg_str == "SEG1AF") {
      _af = new Seg1AF(NULL);
    }
    else if (agg_str == "SEG2AF") {
      _af = new Seg2AF(NULL);
    }
    else if (agg_str == "SEG3AF") {
      _af = new Seg3AF(NULL);
    }
    else if (agg_str == "ACC1AF") {
      _af = new Acc1AF(NULL);
    }
    else if (agg_str == "ACC2AF") {
      _af = new Acc2AF(NULL);
    }
    else if (agg_str == "MITRE1AF") {
      _af = new Mitre1AF(agg_attrib_cstr);
    }
    else if (agg_str == "MITRE2AF") {
      _af = new Mitre2AF(agg_attrib_cstr);
    }
    else if (agg_str == "CENTER_OF_MASS_AF") {
      _af = new MitreCOMAF(agg_attrib_cstr);
    }
    else if (agg_str == "AVG") {
      if (agg_attrib_type == 'i') _af = new IntAverageAF(agg_attrib_cstr);
      else if (agg_attrib_type == 'f') _af = new FloatAverageAF(agg_attrib_cstr);
      else {
	cout << "[AggregateQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ") at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else if (agg_str == "COUNT") _af = new CountAF();
    else if (agg_str == "SUM") {
      if (agg_attrib_type == 'i') _af = new IntSumAF(agg_attrib_cstr);
      else if (agg_attrib_type == 'f') _af = new FloatSumAF(agg_attrib_cstr);
      else {
	cout << "[AggregateQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ")  at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else if (agg_str == "MAX") {
      if (agg_attrib_type == 'i') _af = new IntMaxAF(agg_attrib_cstr);
      else if (agg_attrib_type == 'f') _af = new FloatMaxAF(agg_attrib_cstr);
      else {
	cout << "[AggregateQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ")  at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else if (agg_str == "MIN") {
      if (agg_attrib_type == 'i') _af = new IntMinAF(agg_attrib_cstr);
      else if (agg_attrib_type == 'f') _af = new FloatMinAF(agg_attrib_cstr);
      else {
	cout << "[AggregateQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ")  at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else if (agg_str == "DELTA") {
      if (agg_attrib_type == 'f') _af = new FloatDeltaAF(agg_attrib_cstr);
      else {
	cout << "[AggregateQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ")  at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else if (agg_str == "FIRSTVAL") {
      if (agg_attrib_type == 'f') _af = new FloatFirstValueAF(agg_attrib_cstr);
      else {
	cout << "[AggregateQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ")  at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else if (agg_str == "LASTVAL") {
      if (agg_attrib_type == 'i') _af = new IntLastValueAF(agg_attrib_cstr);
      else if (agg_attrib_type == 'f') _af = new FloatLastValueAF(agg_attrib_cstr);
      else {
	cout << "[AggregateQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ")  at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else {
      cout << "[AggregateQBox] Invalid Aggregate Function (" << agg_str << ") at line " << __LINE__ << endl;
      exit(1);
    }
  }

  // The window size
  ++pos;
  _window_size = stringToInt(*pos);

  // The window size type
  ++pos;
  if ((*pos) == "VALUES") {
    _window_by_tuples = false;
  } else { // should be "TUPLES"
    _window_by_tuples = true;
  }

  // The advance (lemme assume its an int for now)
  ++pos;


  ////////////////////
  //  cout << "***************" << *pos << "iiiii" << endl;
  // cout << ((*pos).at(0) == ':') << endl;
  // cout << (*pos).c_str() << endl;

  _int_advance = stringToInt(*pos);
  _how_to_advance = AggregateQBox::ADVANCE_INTEGER;
  ////////////////////////


  /**
   if (*pos).at(0) == ':')
   {
   _how_to_advance = AggregateQBox::ADVANCE_PREDICATE;
   Parse *p = new Parse();
   char *pred_string = (*pos).c_str();
   _advance_predicate = p->parsePred(pred_string);
   }
   else
   {
   _how_to_advance = AggregateQBox::ADVANCE_INTEGER;
   _int_advance = stringToINt(*pos);
   }
  */


  // The timeout (float)
  ++pos;
  _timeout = stringToDouble(*pos);

  // The order-by attribute (which could be the flag "order by tuple number!"
  ++pos;

  if (strcmp((*pos).c_str(),"TUPLENUM") == 0) {
    _ORDER_BY_TUPLENUM = true;
    // we'll fake a field extract object, to be used on the tuple_counter "tuple"
    _field_att = new FieldExt(":0:i:-12:4:"); // -12 cuz fieldext adds 12 (and yes, atoi can parse negative numbers)
  } else {
    _ORDER_BY_TUPLENUM = false;
    _field_att = new FieldExt((*pos).c_str());
  }
  _order_att_type = _field_att->getType();
  _order_att_size = _field_att->getReturnedSize(); // Careful this size does NOT include any needed +1 if a string

  // *******************************
  // when the order-by is time, need to know how to advance
  // HACK IS SET TO 1 FOR NOW ... INCREASE BY 1 MINUTE
  // -----anurag
  _order_time_increment = 1;

  // Slack
  ++pos;
  _slack = stringToInt(*pos);

  // Group-by-attrib lists (comma seperated)
  ++pos;
  if (pos != v.end()) { // Only if there are group-by attribs
    // parse em out
    parseGroupBy((char*) (*pos).c_str());
  } else {
    parseGroupBy((char*) NULL);
  }


  // Global stuff
  _tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
  _output_tuple_size = getTsSize() + getSidSize() + _group_by_size + _order_att_size + _af->getReturnedSize();
  if (_output_tuple_size > 512) {
    cout << "[AggregateQBox] Detected insane output tuple size ("<<_output_tuple_size<<") - verify output type is set correctly." << endl
	 << "[AggregateQBox] Sorry, but we're exiting now." << endl;
    exit(1);
  }

  /**
  cout << "--PARSING RESULTS--" << endl;
  cout << "Aggregate function: " << _af << endl;
  cout << "Window size: " << _window_size << endl;
  cout << "Advance (as int): " << _int_advance << endl;
  cout << "Timeout (seconds): " << _timeout << endl;
  cout << "Order-by attrib: " << _field_att << endl;
  cout << "Tuple-size: " << _tuple_size << endl;
  cout << "Output Tuple-size: " << _output_tuple_size << endl;
  */
}


Box_Out_T AggregateQBox::doBox()
{

  if (_train_size[0] > 1000) {
    cout << "AggregateQBox::doBox train size " << _train_size[0] << endl;
  }

  _num_tuples_emitted = 0;

  // Note, _outStream[0] wont have enough default memory for this
  // so lets realloc a bunch more (done in StreamThread.C)
  //_outStream[0] = (char*) realloc(_outStream[0], _output_tuple_size * 20); // make 20 something more sensible please
  _output_tuple = _outStream[0];

  for (int ii = 0; ii < _train_size[0]; ii++)
    {

      // da true first thing, if ordering by tuple number, increment the tuple number...

      if (_ORDER_BY_TUPLENUM) {
	++(*_tuple_counter_state);
	memcpy(_tuple_counter,_tuple_counter_state,sizeof(int));
      }

      _curr_tuple = (char*) malloc(_tuple_size);
      memcpy(_curr_tuple, _inStream[0] + (ii * _tuple_size), _tuple_size);
      
      //cout << "[AggregateQBox] INPUT: ";
      //printTuple(_curr_tuple, &_tuple_descr[0]);
      
      int gt_test = gettimeofday(&_curr_ts, NULL);
      assert(gt_test == 0);
      _curr_seconds = _curr_ts.tv_sec;

      // if ordering by tuple number, make the FieldExt object go to the "fake" tuple that's just the counter!
      _order_att_value = (_ORDER_BY_TUPLENUM) ? _field_att->evaluate(_tuple_counter) : _field_att->evaluate(_curr_tuple);

      // FOR WINDOW SIZE BY TUPLES

      /*
	DOCUMENTATION NOTES REGARDING WINDOW SIZE BY TUPLES
	You now emit *as soon as* you know the window is full (tuplestore size == window size),
	 you don't need to wait for the next tuple to arrive.
	This will be done like this until MAP boxes allow Functions, which may allow us to do
	 the things this change here now enables. (see fidelity demo)
      */
      
      if (_window_by_tuples)
	{
	  
	  // First thing - timeout states!
	  timeoutStatesForWinByTuples();

	  getGroupByValuesStr(_curr_tuple);
	  
	  HashForNewState *curr_hash_of_states = _hash_of_everything->getHash(_group_by_values_str);
	  int temp_size;
	  char* temp_str_value = _field_att->evaluateAsChar(_curr_tuple, temp_size);
	  temp_str_value = (char*) realloc(temp_str_value,temp_size+1); // need da null at the end
	  temp_str_value[temp_size] = '\0';
	  NewState *new_state = new NewState(_field_att->evaluate(_curr_tuple),
					     _order_att_type,
					     temp_str_value,
					     _af->makeNew(),
					     _curr_ts);
	  new_state->getAF()->init();
	  new_state->setTo(getTs(_curr_tuple));
	  
	  if (curr_hash_of_states == NULL)
	    {
	      print("No hash for group_by. creating .....");
	      curr_hash_of_states = new HashForNewState(_group_by_values_bytes, _group_by_size, _group_by_values_str,  _order_att_type);
	      if (_order_att_type == 't')
		{
		  curr_hash_of_states->setTimeIncrement(_order_time_increment);
		}
	      
	      _hash_of_everything->addHash(curr_hash_of_states);
	      //curr_hash_of_states->addToVector(_order_att_value);
	      new_state->getAF()->incr(_curr_tuple); // increment...
	      curr_hash_of_states->addToTupleStore(_curr_tuple,new_state); // store

	    }
	  else   // hash for the current group_by does exist
	    {
	      // add the current guy (not increment yet, we'll do it right after to everyone including me)
	      curr_hash_of_states->addToTupleStore(_curr_tuple,new_state);
	      // increment everyone (only in your group by!)
	      vector<char*> *tuple_store = curr_hash_of_states->getTupleStore();
	      vector<NewState*> *tuple_store_states = curr_hash_of_states->getTupleStoreStates();
	      vector<NewState*>::iterator dastates_iter;
	      for (dastates_iter = tuple_store_states->begin();
		   dastates_iter != tuple_store_states->end();
		     ++dastates_iter) {
		(*dastates_iter)->getAF()->incr(_curr_tuple);
	      }
	    }
	
	  // Now see if someone has to get booted
	  
	  if (curr_hash_of_states->getTupleStore()->size() == _window_size)
	    {
	      vector<NewState*> *tuple_store_states = curr_hash_of_states->getTupleStoreStates();
	      // WARNING: is this true? Only emit the front? Shouldn't it depend on ADVANCE??
	      ///          maybe not?
	      
	      // get the front state, that's the one we will emit
	      NewState *ns = tuple_store_states->front();
	      //		  curr_hash_of_states->setThreshold(_order_att_value);
	      
	      emitTuple(_group_by_values_bytes, ns);
	      curr_hash_of_states->clearTupleStore(_int_advance);
	    }
	}  // END WINDOW SIZE BY TUPLES

      else 
	{ 
	  int temp_size;
	  _order_att_str_value = _field_att->evaluateAsChar(_curr_tuple, temp_size);
	  _order_att_str_value = (char*) realloc(_order_att_str_value,temp_size+1); // need da null at the end
	  _order_att_str_value[temp_size] = '\0';
	  
      getGroupByValuesStr(_curr_tuple);
      
      HashForNewState *curr_hash_of_states = _hash_of_everything->getHash(_group_by_values_str);
      
      if (curr_hash_of_states == NULL)
	{
	  print("No hash for group_by. creating .....");
	  curr_hash_of_states = new HashForNewState(_group_by_values_bytes, _group_by_size, _group_by_values_str,  _order_att_type);
	  if (_order_att_type == 't')
	    {
	      curr_hash_of_states->setTimeIncrement(_order_time_increment);
	    }
	  
	  _hash_of_everything->addHash(curr_hash_of_states);
      	  curr_hash_of_states->addToVector(_order_att_value);

	  print("creating new state ...");
	  NewState *new_state = new NewState(_order_att_value, _order_att_type, _order_att_str_value, _af->makeNew(), _curr_ts);

	  curr_hash_of_states->addState(new_state);
	  new_state->getAF()->init();
	  new_state->setTo(getTs(_curr_tuple));
	  curr_hash_of_states->setThreshold(_order_att_value);

	  if (_order_att_type == 'i')
	    {
	      int value_int = *(int*) _order_att_value;
	      char *last = new char[_order_att_size];
	      *(int*)last = value_int + _window_size - 1;
	      curr_hash_of_states->setLastVal(last);
	      
	      char *char_val_for_state;
	      int j = 1;	  
	      int val_for_state = value_int - (j * _int_advance);
	      int tt = val_for_state + _window_size - 1;  // this is the final num in that state
	      
	      while (tt >= value_int) // dont do this if doing win size by tuples cuz
		//  on a new state, just create IT and no one else
		{
		  char_val_for_state = new char[_order_att_size];
		  *(int*) char_val_for_state = val_for_state;
		  
		  char* char_val_for_state_str = (char*) malloc(64);
		  int char_val_for_state_str_size = sprintf(char_val_for_state_str,"%d",val_for_state);
		  char_val_for_state_str = (char*) realloc(char_val_for_state_str,char_val_for_state_str_size+1);
		  char_val_for_state_str[char_val_for_state_str_size] = '\0';
		  
		  print("creating new state ...");
		  new_state = new NewState(char_val_for_state,
					   _order_att_type,
					   char_val_for_state_str,
					   _af->makeNew(),
					   _curr_ts);
		  curr_hash_of_states->addState(new_state);
		  new_state->getAF()->init();
		  new_state->setTo(getTs(_curr_tuple));
		  j++;
		  val_for_state = value_int - (j * _int_advance);
		  tt = val_for_state + _window_size - 1;  // this is the final num in that state
		}
	    }
	  else    // order-on timestamp
	    {
	      timeval value_time = *(timeval*) _order_att_value;
	      long value_time_long = value_time.tv_sec;
	      char* last = new char[_order_att_size];
	      *(long*) last = value_time_long + _window_size  - 1;
	      curr_hash_of_states->setLastVal(last);

	      char *char_val_for_state;

	      int j = 1;
	      long val_for_state = value_time_long - (j * _int_advance);
	      long tt = val_for_state + _window_size - 1;    // this is final long in that state
	      
	      while (tt >= value_time_long)
		{
		  char_val_for_state = new char[_order_att_size];
		  
		  timeval struct_for_state = {val_for_state, value_time.tv_usec};
		  *(timeval*) char_val_for_state = struct_for_state;

		  char *struct_for_state_str = (char*) malloc (64);
		  int str_size = sprintf(struct_for_state_str, "%d,%d", struct_for_state.tv_sec, struct_for_state.tv_usec);
		  struct_for_state_str = (char*) realloc(struct_for_state_str, str_size + 1);
		  struct_for_state_str[str_size] = '\0';

		  //cout << "[AggregateQBox] creating new state TIME ... " << endl;
		  new_state = new NewState (char_val_for_state,
					    _order_att_type,
					    struct_for_state_str,
					    _af->makeNew(),
					    _curr_ts);
		  curr_hash_of_states->addState(new_state);
		  new_state->getAF()->init();
		  new_state->setTo(getTs(_curr_tuple));
		  j++;
		  val_for_state = value_time_long - (j * _int_advance);
		  tt = val_for_state + _window_size - 1;   // this is final long in that state
		}
	    }
	}
      else   // hash for the current group_by does exist
	{
	  print("Retrieved old hash");
	  curr_hash_of_states->addToVector(_order_att_value);
	  NewState *new_state;

	  if (_order_att_type == 'i')
	    {
	      int value_int = *(int*) _order_att_value;
	      int last_val = *(int*) curr_hash_of_states->getLastVal();
	      int first_val_in_last_state = last_val - _window_size + 1;
	      char *char_value_for_state;
	      char *char_last_val;
	      int kk = 1;
	      int value_for_state = first_val_in_last_state + (kk * _int_advance);
	      
	      //	  cout << "**" << value_for_state << "**" << value_int << "**" << last_val << endl;
	      

	      while (last_val + _int_advance <= value_int || value_for_state <= value_int)
		{	      
		  char_value_for_state = new char[_order_att_size];
		  *(int*) char_value_for_state = value_for_state;
		  
		  char* char_val_for_state_str = (char*) malloc(64);
		  int char_val_for_state_str_size = sprintf(char_val_for_state_str,"%d",value_for_state);
		  char_val_for_state_str = (char*) realloc(char_val_for_state_str,char_val_for_state_str_size+1);
		  char_val_for_state_str[char_val_for_state_str_size] = '\0';
		  
		  print("creating new state ...");
		  new_state = new NewState(char_value_for_state, 
					   _order_att_type,
					   char_val_for_state_str,
					   _af->makeNew(),
					   _curr_ts);
		  curr_hash_of_states->addState(new_state);
		  new_state->getAF()->init();
		  new_state->setTo(getTs(_curr_tuple));
		  char_last_val = new char[_order_att_size];
		  *(int*) char_last_val = value_for_state + _window_size - 1;
		  curr_hash_of_states->setLastVal(char_last_val);
		  kk++;
		  first_val_in_last_state = value_for_state;
		  last_val = first_val_in_last_state + _window_size - 1;
		  value_for_state = first_val_in_last_state + _int_advance;   //(kk * _int_advance);
		}

	    }
	  else    // order-on timestamp
	    {
	      timeval value_time = *(timeval*) _order_att_value;
	      long value_time_long = value_time.tv_sec;
	      long last_val = *(long*) curr_hash_of_states->getLastVal();
	      long first_val_in_last_state = last_val - _window_size + 1;
	      char *char_value_for_state;
	      char *char_last_val;

	      int kk = 1;
	      long value_for_state = first_val_in_last_state + (kk * _int_advance);
	      
	      while (last_val + _int_advance <= value_time_long || value_for_state <= value_time_long)
		{
		  char_value_for_state = new char[_order_att_size];		  
		  timeval struct_for_state = {value_for_state, value_time.tv_usec};
		  *(timeval*) char_value_for_state = struct_for_state;

		  char *struct_for_state_str = (char*) malloc (64);
		  int str_size = sprintf(struct_for_state_str, "%d,%d", struct_for_state.tv_sec, struct_for_state.tv_usec);
		  struct_for_state_str = (char*) realloc(struct_for_state_str, str_size + 1);
		  struct_for_state_str[str_size] = '\0';

		  //		  cout << "[AggregateQBox] creating new state TIME ... " << endl;
		  new_state = new NewState (char_value_for_state,
					    _order_att_type,
					    struct_for_state_str,
					    _af->makeNew(),
					    _curr_ts);
		  curr_hash_of_states->addState(new_state);
		  new_state->getAF()->init();
		  new_state->setTo(getTs(_curr_tuple));
		  char_last_val = new char[_order_att_size];
		  *(long*) char_last_val = struct_for_state.tv_sec + _window_size - 1;
		  curr_hash_of_states->setLastVal(char_last_val);
		  kk++;
		  first_val_in_last_state = struct_for_state.tv_sec;
		  last_val = first_val_in_last_state + _window_size - 1;
		  value_for_state = first_val_in_last_state + _int_advance;
		}
	    }
	} // close no hash

      // Note - timeout, or increment first? zat is da question..
      incrStates(_order_att_value, _curr_tuple);

      // LINEAR ROAD HACK TODO: Fix.
      if (getenv("RUNNING_LINEAR_ROAD") != NULL ||
	  _timeout < 10000) {
	timeoutStates();
      }
      updateThresholdAndSlack(curr_hash_of_states, _order_att_value);
     

      
      /**
      cout << "[AggregateQBox] " << "curr t.A = " ; 
      printTuple(_order_att_value, "i");
      cout << "[AggregateQBox] " << "curr slack = " << curr_hash_of_states->getSlack() << endl; 
      cout << "[AggregateQBox] " << "curr threshold = " ; 
      printTuple(curr_hash_of_states->getThreshold(), "t");
      cout << "[AggregateQBox] " << "curr the last is = " ;
      printTuple(curr_hash_of_states->getLastVal(), "i");
      */
      

	}

    }
  
  // Ok, return time
  // This box always sucks every tuple
 return_val.kept_input_count = 0;
 return_val.output_tuples = _num_tuples_emitted;
 return_val.output_tuple_size = _output_tuple_size;

 return_val.output_tuples_array = new int[1];
 return_val.output_tuples_array[0] = _num_tuples_emitted;
 //cout << "AGG "<<_boxId<<" EMITTED " << _num_tuples_emitted << endl;
 return return_val;
 
}


void AggregateQBox::updateThresholdAndSlack(HashForNewState *curr_hash, char *att_value)
{
  int slack = curr_hash->getSlack();
  int threshold = *(int*) curr_hash->getThreshold();
  int int_att_value = *(int*) att_value;

  // DO NOT REMOVE THE LINES BELOW ---- anurag
  //  if ((int_att_value > threshold) && (slack <= (_slack - 1)))
  /**
  if ((int_att_value > threshold) && (slack <= _slack))
    {
      curr_hash->increaseSlack();
    }
  else
    {
      while ((int_att_value > threshold) && (slack > _slack))
  */

  if (int_att_value > threshold)
    {
      curr_hash->increaseSlack();
      slack = curr_hash->getSlack();
      while (slack > _slack)

	{
	  curr_hash->increaseThreshold();
	  threshold = *(int*) curr_hash->getThreshold();
	  slack = curr_hash->countAndClearVector();
	  curr_hash->setSlack(slack);
	  	   
	  // close and emit if any has timed-out
	  
	  hash_map<char*, NewState*, hash<const char*>, equal_string>::iterator  hash_iter;
	  
	  hash_iter = curr_hash->begin();
	  
	  NewState *temp_state;
	  int jj = 0;
	  int ii = curr_hash->getHashSize();

	  while (jj < ii)
	    //  while (hash_iter != curr_hash->end())
	    {
	      temp_state = (*hash_iter).second;
	      if (!temp_state)
		{
		  hash_iter++;
		  jj++;
		  continue;
		}


	      // CHANGE THIS TO MAKE IT TAG THE GUYS TO ERASE, THEN ERASE EM
	      int state_value = *(int*) temp_state->getValue();
	      
	      if (_order_att_type = 'i')
		{
		  if ((state_value + _window_size - 1) < threshold)
		    {
		      //cout << "              emitting from A               " << endl;
		      emitTuple(_group_by_values_bytes, temp_state);
		      closeState(curr_hash, temp_state); // WARNING - THIS MODIFIES curr_hash WHICH I THINK
		      // INVALIDATES ITERATORS
		      //  lame ass hack - make it start over
		      --ii; jj = 0; hash_iter = curr_hash->begin();
		    } else {
		      hash_iter++;
		      jj++;
		    }
		}
	      else   // order-on timestamp
		{
		  if ((state_value + _window_size - 1) < threshold)
		    {
		      emitTuple(_group_by_values_bytes, temp_state);
		      closeState(curr_hash, temp_state); // WARNING - THIS MODIFIES curr_hash WHICH I THINK
		      // INVALIDATES ITERATORS
		      //  lame ass hack - make it start over
		      --ii; jj = 0; hash_iter = curr_hash->begin();
		    } else {
		      hash_iter++;
		      jj++;
		    } 
		}
	    }
	}
    }
  //  cout << "***" << threshold << "***" << slack << endl;  
}

void AggregateQBox::timeoutStatesForWinByTuples() {

  _hash_iter = _hash_of_everything->begin();
  // for every possible group-by
  while (_hash_iter != _hash_of_everything->end())
    {
      HashForNewState *curr_hash = (*_hash_iter).second;
      vector<char*> *tuple_store = curr_hash->getTupleStore();
      vector<NewState*> *tuple_store_states = curr_hash->getTupleStoreStates();

      vector<char*>::iterator tuple_store_iter = tuple_store->begin();
      vector<NewState*>::iterator dastates_iter = tuple_store_states->begin();
      
      while (tuple_store_iter != tuple_store->end()) {

	if ((_curr_ts.tv_sec - (*dastates_iter)->getTS().tv_sec) > _timeout) {

	  NewState *new_state = (*dastates_iter);
	  getGroupByValuesStr(*tuple_store_iter);
	  //cout << " CANDIDATE TUPLE FOR TIMEOUT (groupby " << _group_by_values_str << ") time diff: " << (_curr_ts.tv_sec - new_state->getTS().tv_sec) << endl;

	  //cout << " note that tuple_store->size() is : " <<tuple_store->size() << endl;

	  
	  emitTuple(_group_by_values_bytes, new_state);
	  curr_hash->clearTupleStore(_int_advance);
	  // now restart iterators
	  tuple_store_iter = tuple_store->begin();
	  dastates_iter = tuple_store_states->begin();
	} else { // move on
	++tuple_store_iter;
	++dastates_iter;
	}
      }
      ++_hash_iter;
    }

}

void AggregateQBox::timeoutStates()
{
  hash_map<char*, NewState*, hash<const char*>, equal_string>::iterator  state_hash_iter;
  HashForNewState *curr_hash;
  NewState *temp_state;
  Timestamp state_ts;
  
  _hash_iter = _hash_of_everything->begin();
  while (_hash_iter != _hash_of_everything->end())
    {
      curr_hash = (*_hash_iter).second;
      
      state_hash_iter = curr_hash->begin();
      
      int ii = curr_hash->getHashSize();
      
      int jj = 0;
      while (jj < ii) {
	//      while (state_hash_iter != curr_hash->end())
	temp_state = NULL;
	temp_state = (*state_hash_iter).second;
	if (!temp_state)
	  {
	    state_hash_iter++;
	    jj++;
	    continue;
	  }
	state_ts = temp_state->getTS();
	//cout << " current time: " << _curr_ts.tv_sec << " comparing to state time: " << state_ts.tv_sec << " (for timeout " << _timeout << endl;
	if ((_curr_ts.tv_sec - state_ts.tv_sec) > _timeout)
	  {
	    //cout << "              EMITTING DUE TO A TIMEOUT!!!               " << endl;
	    emitTuple(curr_hash->getGroupByValues(), temp_state);
	    closeState(curr_hash, temp_state);// WARNING - THIS MODIFIES curr_hash WHICH I THINK
	    // INVALIDATES ITERATORS
	    //  lame ass hack - make it start over
	    --ii; jj = 0; state_hash_iter = curr_hash->begin();
	  } else {
	    state_hash_iter++;
	    jj++;
	  }
      }
      _hash_iter++;
    }
}

void AggregateQBox::incrStates(char *order_att_value, char *tuple)
{
  HashForNewState *curr_hash = _hash_of_everything->getHash(_group_by_values_str);

  hash_map<char*, NewState*, hash<const char*>, equal_string>::iterator  hash_iter;

  hash_iter = curr_hash->begin();

  int threshold = *(int*) curr_hash->getThreshold();
  int int_att_value = *(int*) order_att_value;

  // increment to be called only if the order-value is >= threshold
  // threshold says the smallest tuple that we care about ----anurag
  if (int_att_value >= threshold)   
    {      
      Timestamp tuple_ts = getTs(tuple);
      Timestamp state_to;
      
      NewState *temp_state;
      while (hash_iter != curr_hash->end())
	{
	  temp_state = (*hash_iter).second;
	  int state_value = *(int*) temp_state->getValue();
	  //int state_value = strtol(temp_state->getValue(),(char**)NULL,10);
	 
	  if (_order_att_type = 'i')
	    {
	      if (((int_att_value - _window_size + 1) <= state_value) &&
		  (state_value <= int_att_value))      // && (state_value >= threshold))
		{
		  //cout << "calling increment on state " << state_value << " and threshold =" << threshold << endl;
		  state_to = temp_state->getTo();
		  temp_state->getAF()->incr(tuple);
		  if (tuple_ts < state_to)
		    {
		      temp_state->setTo(tuple_ts);
		    }
		}
	      hash_iter++;
	    }
	  else   //order-on timestamp
	    {
	      if (((int_att_value - _window_size + 1) <= state_value) &&
		  (state_value <= int_att_value))      // && (state_value >= threshold))
		{
		  print("calling incr on state ");
		  state_to = temp_state->getTo();
		  temp_state->getAF()->incr(tuple);
		  if (tuple_ts < state_to)
		    {
		      temp_state->setTo(tuple_ts);
		    }
		}
	      hash_iter++;
	    }
	}
    } 
}

// If this returns VOID don't call it GETblabla
void AggregateQBox::getGroupByValuesStr(char *tuple)
{
  _group_by_values_str = NULL;
  if (_group_by_size == 0) {  // no group by
    // Since hash maps dont like looking for NULLS give them an empty string instead
    _group_by_values_str = "";
    _chars_needed_total = 1;
    _group_by_values_bytes = NULL;
    return;
  }
  _group_by_values_str = (char*) realloc(_group_by_values_str, (65 * _num_group_by_atts) + 1);
  _group_by_values_bytes = new char[_group_by_size];

  char *temp_ptr = _group_by_values_str;
  char *temp_ptr_2 = _group_by_values_bytes;

  int charsNeeded = 0;
  _chars_needed_total = 0;
  for (int ii = 0; ii < _num_group_by_atts; ii++) {
    if (ii > 0) { // put the 0x01 chararacter between values
      char sep = 0x01;
      memcpy(temp_ptr, &sep, 1);
      ++temp_ptr; 
      ++_chars_needed_total;
    }
    
    char* temp_group_by_values_str = _field_group[ii]->evaluateAsChar(tuple, charsNeeded);
    memcpy(temp_ptr, temp_group_by_values_str, charsNeeded);
    free(temp_group_by_values_str);
    temp_ptr += charsNeeded;
    _chars_needed_total += charsNeeded;

    memcpy(temp_ptr_2, _field_group[ii]->evaluate(tuple), _field_group[ii]->getReturnedSize());
    temp_ptr_2 += _field_group[ii]->getReturnedSize();
  }
  // Squeeze it back down, putting the \0 at the end too
  _group_by_values_str = (char*) realloc(_group_by_values_str, _chars_needed_total+1);
  _group_by_values_str[_chars_needed_total] = '\0';
  _chars_needed_total++;
}


void AggregateQBox::parseGroupBy(char *atts)
{
  
  if (atts == NULL || strcmp(atts,"") == 0) {
    // No group_by attributes means this operator basically groups everyone in the same place
    // OPTIMIZATION OPPORTUNITY: we could then not even use the hash...
    _num_group_by_atts = 0;
    _group_by_size = 0;
    return;
  }
  // We assume atts is null-terminated
  char *p = index(atts, ',');
  if (p == NULL) { // No match for "," means only one group by
    _num_group_by_atts = 1;
    _field_group = new FieldExt*[1];
    _field_group[0] = new FieldExt(atts);
    //   return;
  } else { // Let's figure out how many group bys
    int num_atts = 2; // At least two already
    p = index(p+1,','); // Resume search after the ,
    while (p != NULL) {
      num_atts++;
      p = index(p+1,',');
    }
    _field_group = new FieldExt*[num_atts];
    // Now that we know, create the FieldExts in _field_group
    p = atts;
    // Careful - the loop below goes by num_atts not by what p points to
    //            but that should be equivalent
    for (int i = 0; i < num_atts; i++) {
      _field_group[i] = new FieldExt(p);
      p = index(p+1,',');
      if (p != NULL) p++; // To skip the , (FieldExt doesnt want it)
    }
    _num_group_by_atts = num_atts;
  }

  // Make sure it is 0 to begin with!
  _group_by_size = 0;
  for (int i = 0; i < _num_group_by_atts; i++)
    {
      _group_by_size += _field_group[i]->getReturnedSize();
    }
  return;  
}


void AggregateQBox::emitTuple(char *group_by_values, NewState *s)
{
  // copy the timestamp
  char *temp = _output_tuple;
  Timestamp t = s->getTo();
  memcpy(_output_tuple, &t, getTsSize());
  _output_tuple += getTsSize();
  
  // copy sid
  int sid = 1;
  memcpy(_output_tuple, &sid, getSidSize());
  _output_tuple += getSidSize();
  
  // copy group_by
  memcpy(_output_tuple, group_by_values, _group_by_size);
  _output_tuple += _group_by_size;

  // copy state value
  if (!_ORDER_BY_TUPLENUM)
    {
      memcpy(_output_tuple, s->getValue(), s->getValueSize());
      _output_tuple += s->getValueSize();
    }

  // copy final from aggregate
  memcpy(_output_tuple, s->getAF()->final(), s->getAF()->getReturnedSize());
  _output_tuple += s->getAF()->getReturnedSize();

  // count it
  ++_num_tuples_emitted;

  //cout << "[AggregateQBox] OUTPUT: " ;
  //printTuple(temp, "tiiiccc");
}  

void AggregateQBox::closeState(HashForNewState *dahash, NewState *s)
{
  //print("Closing state!");
  dahash->removeState(s);

}

void AggregateQBox::print(char *msg)
{
  //cout << "[AggregateQBox] " << msg << endl;
}

void AggregateQBox::print(int msg)
{
  //cout << "[AggregateQBox] " << msg << endl;
}
