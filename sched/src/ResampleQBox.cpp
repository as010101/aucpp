#include "ResampleQBox.H"
#include "parseutil.H"

/**
 * Modifier
 * [target attrib]~[agg func]~[agg attrib]~[wind size]~[order-on L]~[slack L]~[order-on R]~[slack R]
 */
void ResampleQBox::setBox(const char* modifier_str, BufferList* left_buffer, BufferList* right_buffer, HashForNewState* statehash) {
  // Gimme my hashes back
  _left_buffer = left_buffer;
  _right_buffer = right_buffer;
  _state_hash = statehash;

  // SET THESE NULLS (else bad for realloc)
  _left_order_att_str = NULL;
  _right_order_att_str = NULL;

  // Parsing time!
  string modifier(modifier_str);
  vector<string> v = unpackString(modifier, "=~");
  vector<string>::iterator pos = v.begin();
  
  // skip over first (its gui stuff really)
  ++pos;
  { // The aggregate function (yuk)
    string agg_str = *pos;
    //const char* agg_cstr = (*pos).c_str(); // Wanna save a byte? you dont need c_str, just str since FieldExt doesnt care for the null
    // Get the aggregate function attribute
    ++pos;
    const char* agg_attrib_cstr = (*pos).c_str();
    char agg_attrib_type = *(index(index(agg_attrib_cstr,':')+1,':')+1); // something a la :0:i:0:4:, I want just the i
    
    // Make the attribute
    if (agg_str == "AVG") {
      if (agg_attrib_type == 'i') _resample_function = new IntAverageAF(agg_attrib_cstr);
      else if (agg_attrib_type == 'f') _resample_function = new FloatAverageAF(agg_attrib_cstr);
      else {
	cout << "[ResampleQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ") at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else if (agg_str == "COUNT") _resample_function = new CountAF();
    else if (agg_str == "SUM") {
      if (agg_attrib_type == 'i') _resample_function = new IntSumAF(agg_attrib_cstr);
      else if (agg_attrib_type == 'f') _resample_function = new FloatSumAF(agg_attrib_cstr);
      else {
	cout << "[ResampleQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ")  at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else if (agg_str == "MAX") {
      if (agg_attrib_type == 'i') _resample_function = new IntMaxAF(agg_attrib_cstr);
      else if (agg_attrib_type == 'f') _resample_function = new FloatMaxAF(agg_attrib_cstr);
      else {
	cout << "[ResampleQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ")  at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else if (agg_str == "MIN") {
      if (agg_attrib_type == 'i') _resample_function = new IntMinAF(agg_attrib_cstr);
      else if (agg_attrib_type == 'f') _resample_function = new FloatMinAF(agg_attrib_cstr);
      else {
	cout << "[ResampleQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ")  at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else if (agg_str == "LASTVAL") {
      if (agg_attrib_type == 'i') _resample_function = new IntLastValueAF(agg_attrib_cstr);
      else if (agg_attrib_type == 'f') _resample_function = new FloatLastValueAF(agg_attrib_cstr);
      else {
	cout << "[ResampleQBox] Invalid Aggregate type (" << agg_attrib_type << ") for (" << agg_str << ")  at line " << __LINE__ << endl;
	exit(1);
      }
    }
    else {
      cout << "[ResampleQBox] Invalid Aggregate Function (" << agg_str << ") at line " << __LINE__ << endl;
      exit(1);
    }
  }

   // The window size
  ++pos;
  _window_size = stringToInt(*pos);

   // The order-on left attribute
  ++pos;
  _left_field_att = new FieldExt((*pos).c_str());
  _left_order_att_type = _left_field_att->getType();
  _left_order_att_size = _left_field_att->getReturnedSize();

  // slack left
  ++pos;
  _left_slack = stringToInt(*pos);

   // The order-on right attribute
  ++pos;
  _right_field_att = new FieldExt((*pos).c_str());
  _right_order_att_type = _right_field_att->getType();
  _right_order_att_size = _right_field_att->getReturnedSize();

  // slack right
  ++pos;
  _right_slack = stringToInt(*pos);

  // extra stuff
  _left_tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
  _right_tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
  
  _hidden_size = getTsSize() + getSidSize();
  
  _output_tuple_size = _hidden_size + _left_field_att->getReturnedSize() + _resample_function->getReturnedSize();

  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@222 HACK
  _output_tuple_size = _hidden_size + _left_field_att->getReturnedSize() + _resample_function->getReturnedSize() + sizeof(int);
  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@222 HACK  
}

void ResampleQBox::setBox(AggregateFunction *af, int size, 
			  char *left_order_att, int left_slack,
			  char *right_order_att, int right_slack)
{
  _resample_function = af;
  _window_size = size;

  _left_slack = left_slack;
  _right_slack = right_slack;
  
  _left_field_att = new FieldExt(left_order_att);
  _left_order_att_type = _left_field_att->getType();
  _left_order_att_size = _left_field_att->getReturnedSize();

  _right_field_att = new FieldExt(right_order_att);
  _right_order_att_type = _right_field_att->getType();
  _right_order_att_size = _right_field_att->getReturnedSize();

  _left_buffer = new BufferList(_left_order_att_type);
  _right_buffer = new BufferList(_right_order_att_type);
  //  _state_list = new NewStateList();
  _state_hash = new HashForNewState(_left_order_att_type);

  //***********************************************

  cout << " ^^^^^^^^^^^^^^^^^^ DONT CALL ME [resampleqbox] " << endl;
  _left_tuple_size = 26;
  _right_tuple_size = 26;
  
  _hidden_size = getTsSize() + getSidSize();
  
  _output_tuple_size = _hidden_size + _left_field_att->getReturnedSize() + _resample_function->getReturnedSize();

}


Box_Out_T ResampleQBox::doBox()
{

  //cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$ START DOBOX $$$$$$$$$$$$$$$$$$$$$" << endl << endl;

  _num_tuples_emitted = 0;
  
  //_output_tuple = new char[_output_tuple_size];
  _output_tuple = _outStream[0];

  for (int ii = 0; ii < _train_size[0]; ii++)
    {
      _left_curr_tuple = (char*) malloc(_left_tuple_size);
      memcpy(_left_curr_tuple, _inStream[0] + (ii * _left_tuple_size), _left_tuple_size);

      //cout << "[ResampleQBox] LEFT INPUT: ";
      //printTuple(_left_curr_tuple, _tuple_descr);


	gettimeofday(&_curr_ts, NULL);
	_curr_seconds = _curr_ts.tv_sec;

	_left_order_att_value = new char[_left_order_att_size];
	memcpy(_left_order_att_value, _left_field_att->evaluate(_left_curr_tuple), _left_order_att_size);
	
	// GET THE ORDER-ATT AS STRING
	int chars_needed = 0;
	_left_order_att_str = (char*) realloc(_left_order_att_str, 65 + 1);
	char *temp_char = _left_field_att->evaluateAsChar(_left_curr_tuple, chars_needed);
	memcpy(_left_order_att_str, temp_char, chars_needed);
	_left_order_att_str = (char*) realloc(_left_order_att_str, chars_needed + 1);
	_left_order_att_str[chars_needed] = '\0';

	// BEGIN
	_left_buffer->addToVector(_left_order_att_value);

	if (_left_buffer->size() == 0)
	  {
	    _left_buffer->setThreshold(_left_order_att_value);
	  }

	int left_value = *(int*) _left_order_att_value;
	int left_threshold = *(int*) _left_buffer->getThreshold();
	int left_curr_slack = _left_buffer->getSlack();
	
	if (left_value >= left_threshold)
	  {
	    //cout << "[ResampleQBox] creating new state for " << *(int*) _left_order_att_value << endl;
	    NewState *new_state = new NewState(_left_order_att_value, 
					       _left_order_att_type, 
					       _left_order_att_str, 
					       _resample_function->makeNew(), 
					       _curr_ts);
	    _state_hash->addState(new_state);
	    new_state->getAF()->init();
	    new_state->setTo(getTs(_left_curr_tuple));

	    if (left_value > left_threshold)
	      {
		_left_buffer->increaseSlack();
		left_curr_slack = _left_buffer->getSlack();
		
		while (left_curr_slack > _left_slack)
		  {
		    _left_buffer->increaseThreshold();
		    left_threshold = *(int*) _left_buffer->getThreshold();
		    left_curr_slack = _left_buffer->countAndClearVector();
		    _left_buffer->setSlack(left_curr_slack);
		  }
	      } // close if left_value > threshold

	    int right_value = 0;
	    list<node>::iterator right_iter = _right_buffer->begin();
	    
	    node temp_node;
	    while (right_iter != _right_buffer->end())
	      {
		temp_node = *right_iter;
		right_value = *(int*) temp_node.att;
		
		if (right_value < (left_threshold - _window_size))
		  {
		    right_iter = (_right_buffer->_buffer).erase(right_iter);
		  }
		else if (((left_value - _window_size) <= right_value) &&
			 (right_value <= (left_value + _window_size)))
		  {
		    NewState *temp_state = _state_hash->getState(_left_order_att_str);
		    Timestamp tuple_ts = getTs(_left_curr_tuple);
		    Timestamp state_to;

		    if (temp_state != NULL)
		      {
			//cout << "[ResampleQBox] incrementing state for " << _left_order_att_str << "=" << *(int*)_left_order_att_value << endl;
		      temp_state->getAF()->incr(_left_curr_tuple);
		      state_to = temp_state->getTo();
		      if (tuple_ts < state_to)
			{
			  temp_state->setTo(tuple_ts);
			}
		      }
		    right_iter++;
		  }
		else 
		  {
		    right_iter++;
		  }
	      } // close while 
	    
	    node new_node = {_left_order_att_value, _left_curr_tuple};
	    _left_buffer->push_back(new_node);
	  
	    
	  }  // if (x.A >= thrl)
    
	/**
    cout << "*********left threshold = *" << *(int*)_left_buffer->getThreshold() << "*" << endl;
    cout << "*********left slack = *" << _left_buffer->getSlack() << "*" << endl;
    cout << "*********right threshold = *" << *(int*)_right_buffer->getThreshold() << "*" << endl;
    cout << "*********right slack = *" << _right_buffer->getSlack() << "*" << endl;
	*/
    } // close left-for-loop
  

  //-----------------------------------------------------------


  for (int ii = 0; ii < _train_size[1]; ii++)
    {
      _right_curr_tuple = (char*) malloc(_right_tuple_size);
      memcpy(_right_curr_tuple, _inStream[1] + (ii * _right_tuple_size), _right_tuple_size);

      //cout << "[ResampleQBox] RIGHT INPUT: ";
      //printTuple(_right_curr_tuple, _tuple_descr);

	gettimeofday(&_curr_ts, NULL);
	_curr_seconds = _curr_ts.tv_sec;

	_right_order_att_value = new char[_right_order_att_size];
	memcpy(_right_order_att_value, _right_field_att->evaluate(_right_curr_tuple), _right_order_att_size);
	
	// GET THE ORDER-ATT AS STRING
	int chars_needed = 0;
	_right_order_att_str = (char*) realloc(_right_order_att_str, 65 + 1);
	char *temp_char = _right_field_att->evaluateAsChar(_right_curr_tuple, chars_needed);
	memcpy(_right_order_att_str, temp_char, chars_needed);
	_right_order_att_str = (char*) realloc(_right_order_att_str, chars_needed + 1);
	_right_order_att_str[chars_needed] = '\0';

	// BEGIN

	_right_buffer->addToVector(_right_order_att_value);

	if (_right_buffer->size() == 0)
	  {
	    _right_buffer->setThreshold(_right_order_att_value);
	  }

	int right_value = *(int*) _right_order_att_value;
	int right_threshold = *(int*) _right_buffer->getThreshold();
	int right_curr_slack = _right_buffer->getSlack();
	
	if (right_value >= right_threshold)
	  {

	    // eddie - don't delete the commented lines below
	    /*
	    cout << "[ResampleQBox] creating new state for " << *(int*) _right_order_att_value << endl;
	    NewState *new_state = new NewState(_right_order_att_value, 
					       _right_order_att_type, 
					       _right_order_att_str, 
					       _resample_function->makeNew(), 
					       _curr_ts);
	    _state_hash->addState(new_state);
	    new_state->getAF()->init();
	    */


	    if (right_value > right_threshold)
	      {
		_right_buffer->increaseSlack();
		right_curr_slack = _right_buffer->getSlack();
		
		while (right_curr_slack > _right_slack)
		  {
		    _right_buffer->increaseThreshold();
		    right_threshold = *(int*) _right_buffer->getThreshold();
		    right_curr_slack = _right_buffer->countAndClearVector();
		    _right_buffer->setSlack(right_curr_slack);
		  }
	      } // close if right_value > threshold

	    int left_value = 0;
	    list<node>::iterator left_iter = _left_buffer->begin();
	    
	    node temp_node;
	    while (left_iter != _left_buffer->end())
	      {
		temp_node = *left_iter;
		left_value = *(int*) temp_node.att;
		
		char *left_value_str = (char*) malloc(64);
		int nchars;
		nchars = sprintf(left_value_str, "%d", left_value);
		left_value_str = (char*) realloc(left_value_str, nchars);

		if (left_value < (right_threshold - _window_size))
		  {
		    NewState *temp_state = _state_hash->getState(left_value_str);
		    if (temp_state != NULL)
		      {
			//cout << "[ResampleQBox] calling final state for " << left_value << "=" << left_value_str << endl;
			temp_state->getAF()->final();
			emitTuple(temp_state);
			_state_hash->removeState(temp_state);
		      }
		    left_iter = (_left_buffer->_buffer).erase(left_iter);
		  }
		else if (((right_value - _window_size) <= left_value) &&
			 (left_value <= (right_value + _window_size)))
		  {
		    NewState *temp_state = _state_hash->getState(left_value_str);
		    Timestamp tuple_ts = getTs(_right_curr_tuple);
		    Timestamp state_to;
		    if (temp_state != NULL)
		      {
			//cout << "[ResampleQBox] incrementing state for " << left_value << "=" << left_value_str << endl;
			temp_state->getAF()->incr(_right_curr_tuple);
			state_to = temp_state->getTo();
			if (tuple_ts < state_to)
			  {
			    temp_state->setTo(tuple_ts);
			  }
		      }
		    left_iter++;
		  }
		else 
		  {
		    left_iter++;
		  }
	      } // close while 
	    
	    node new_node = {_right_order_att_value, _right_curr_tuple};
	    _right_buffer->push_back(new_node);
	  
	    
	  }  // if (x.A >= thrl)
    
	/**
    cout << "*********right threshold = *" << *(int*)_right_buffer->getThreshold() << "*" << endl;
    cout << "*********right slack = *" << _right_buffer->getSlack() << "*" << endl;
    cout << "*********left threshold = *" << *(int*)_left_buffer->getThreshold() << "*" << endl;
    cout << "*********left slack = *" << _left_buffer->getSlack() << "*" << endl;
	*/
    } // close right-for-loop

  //cout <<endl << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$ END DOBOX $$$$$$$$$$$$$$$$$$$$$" << endl << endl;
  
   return_val.kept_input_count = 0;
   return_val.output_tuples = _num_tuples_emitted;
   return_val.output_tuple_size = _output_tuple_size;

   // This is very important to maintain too!
   return_val.output_tuples_array = new int[1];
   return_val.output_tuples_array[0] = _num_tuples_emitted;

   return_val.kept_input_count_array = new int[2];
   return_val.kept_input_count_array[0] = 0;
   return_val.kept_input_count_array[1] = 0;

   return return_val;

}

void ResampleQBox::emitTuple(NewState *s)
{
  char *temp = _output_tuple;
  Timestamp t = s->getTo();
  
  //copy the timestamp
  memcpy(_output_tuple, &t, getTsSize());
  _output_tuple += getTsSize();

  // copy sid
  int sid = 10;
  memcpy(_output_tuple, &sid, getSidSize());
  _output_tuple += getSidSize();

  // order-by val from state
  memcpy(_output_tuple, s->getValue(), s->getValueSize());
  _output_tuple += s->getValueSize();

  // copy final from aggregate
  memcpy(_output_tuple, s->getAF()->final(), s->getAF()->getReturnedSize());
  _output_tuple += s->getAF()->getReturnedSize();

  //cout << "[ResampleQBox] OUTPUT: " ;
  //printTuple(temp, _tuple_descr);
  
  ++_num_tuples_emitted;
}
