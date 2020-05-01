#include "JoinQBox.H"
#include "parseutil.H"

/**
 * Modifier here looks like
 * [predicate] ~ [window size] ~ [order-on left] ~ [slack left] ~ [order-on right] ~ [slack right]
 */
void JoinQBox::setBox(const char* modifier_str, BufferList* left_buffer, BufferList* right_buffer) {
  // Gimme my hashes back
  _left_buffer = left_buffer;
  _right_buffer = right_buffer;

  // Parsing time
  string modifier(modifier_str);
  vector<string> v = unpackString(modifier, "~");
  vector<string>::iterator pos = v.begin();
  
  // First is the predicate
  Parse *p = new Parse();
  _join_predicate = p->parsePred((*pos).c_str());

  // Next, window size
  ++pos;
  _buffer_size_before = _buffer_size_after = stringToInt((*pos));
  // Next, order-on left
  ++pos;
  _left_field_att = new FieldExt((*pos).c_str());
  _left_order_att_type = _left_field_att->getType();
  _left_order_att_size = _left_field_att->getReturnedSize();
  // Next, slack
  ++pos;
  _left_slack = stringToInt((*pos));
  
  // Next, order-on right
  ++pos;
  _right_field_att = new FieldExt((*pos).c_str());
  _right_order_att_type = _right_field_att->getType();
  _right_order_att_size = _right_field_att->getReturnedSize();
  // Next, slack right
  ++pos;
  _right_slack = stringToInt((*pos));

  // tuple size stuff, note how we access the different tuple descriptions
  _left_tuple_size = _tuple_descr_array[0]->getSize() + getTsSize() + getSidSize();
  _right_tuple_size = _tuple_descr_array[1]->getSize() + getTsSize() + getSidSize();
  
  _hidden_size = getTsSize() + getSidSize();
  
  _output_tuple_size = _hidden_size + (_left_tuple_size - _hidden_size) + (_right_tuple_size - _hidden_size);  
}
void JoinQBox::setBox(char *pred, int size, char *left_order_att, int left_slack, char *right_order_att, int right_slack)
{
  
  _buffer_size_before = size;
  _buffer_size_after = size;
  
  Parse *p = new Parse();
  _join_predicate = p->parsePred(pred);

  _left_slack = left_slack;
  _right_slack = right_slack;


  _left_field_att = new FieldExt(left_order_att);
  _left_order_att_type = _left_field_att->getType();
  _left_order_att_size = _left_field_att->getReturnedSize();

  _right_field_att = new FieldExt(right_order_att);
  _right_order_att_type = _right_field_att->getType();
  _right_order_att_size = _right_field_att->getReturnedSize();

  _left_buffer = new BufferList(_left_order_att_type);    // no group-by's anymore
  _right_buffer = new BufferList(_right_order_att_type);


  _left_tuple_size = _tuple_descr_array[0]->getSize() + getTsSize() + getSidSize();
  _right_tuple_size = _tuple_descr_array[1]->getSize() + getTsSize() + getSidSize();

  //  _left_tuple_size = 26;
  //_right_tuple_size = 26;
    
    _hidden_size = getTsSize() + getSidSize();
    
    _output_tuple_size = _hidden_size + (_left_tuple_size - _hidden_size) + (_right_tuple_size - _hidden_size);  
}


Box_Out_T JoinQBox::doBox()
{


	// LoadShedder-related calls	BEGIN (tatbul@cs.brown.edu)
	//
	applyLoadShedding();
	//
	// LoadShedder-related calls	END (tatbul@cs.brown.edu)

  //cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$ START DOBOX $$$$$$$$$$$$$$$$$$$$$" << endl << endl;

  _num_tuples_emitted = 0;
  

  //_output_tuple = new char[_output_tuple_size];
  _output_tuple = _outStream[0];


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
	
      _left_curr_tuple = (char*) malloc(_left_tuple_size);
      memcpy(_left_curr_tuple, _inStream[0] + (ii * _left_tuple_size), _left_tuple_size);
      
      //      cout << "[JoinQBox] LEFT INPUT: ";
      //      printTuple(_left_curr_tuple, _tuple_descr_array[0]);

      gettimeofday(&_curr_ts, NULL);
      _curr_seconds = _curr_ts.tv_sec;

      _left_order_att_value = new char[_left_order_att_size];
      memcpy(_left_order_att_value, _left_field_att->evaluate(_left_curr_tuple), _left_order_att_size);
      

      _left_buffer->addToVector(_left_order_att_value);

      if (_right_buffer->size() == 0 )
	{
	  //cout << "[JoinQBox] creating new node for left buffer (empty right_buffer)" << endl;
	  node new_node = {_left_order_att_value, _left_curr_tuple};
	  if (_left_buffer->size() == 0)
	    _left_buffer->setThreshold(_left_order_att_value);
	  _left_buffer->push_back(new_node);
	  
	  continue;
	}
      else
	{	  
	  if (_left_buffer->size() == 0)
	    _left_buffer->setThreshold(_left_order_att_value);
	      	  
	  int left_threshold = *(int*) _left_buffer->getThreshold();
	  int left_value = *(int*) _left_order_att_value;
	  
	  if (left_value >= left_threshold)
	    {
	      int right_value = 0;
	      list<node>::iterator right_iter = _right_buffer->begin();

	      node temp_node;
	      while (right_iter != _right_buffer->end())
		{
		  temp_node = *right_iter;
		  right_value = *(int*) temp_node.att;

		  if (((abs (right_value - left_value)) <= _buffer_size_before) ||
		      ((abs (left_value - right_value)) <= _buffer_size_after))
		    {
		      if (_join_predicate->evaluate(_left_curr_tuple, temp_node.tuple))
			{
			  emitTuple(_left_curr_tuple, temp_node.tuple);
			}
		    }
		  right_iter++;
		} // close while loop

	      int right_threshold = *(int*) _right_buffer->getThreshold();
	      
	      if ((right_threshold - left_value) <= _buffer_size_after)
		{
		  //cout << "[JoinQBox] creating new node for left buffer" << endl;
		  node new_node = {_left_order_att_value, _left_curr_tuple};
		  if (_left_buffer->size() == 0)
		    _left_buffer->setThreshold(_left_order_att_value);
		  _left_buffer->push_back(new_node);
		}
  
	      left_threshold = *(int*) _left_buffer->getThreshold();

	      if (left_value > left_threshold)
		{
		  _left_buffer->increaseSlack();
		  int left_curr_slack = _left_buffer->getSlack();
		  
		  while (left_curr_slack > _left_slack)
		    {
		      _left_buffer->increaseThreshold();
		      left_threshold = *(int*) _left_buffer->getThreshold();
		      left_curr_slack = _left_buffer->countAndClearVector();
		      _left_buffer->setSlack(left_curr_slack);
		    }
		  
		  _right_buffer->erase(_left_buffer->getThreshold(), _buffer_size_before);
		  
		  
		}
	      
	    }  // close if left_value >= threshold
	}   // close when right_buffer exists
      

    
      /**
    cout << "*********left threshold = *" << *(int*)_left_buffer->getThreshold() << "*" << endl;
    cout << "*********left slack = *" << _left_buffer->getSlack() << "*" << endl;
    cout << "*********right threshold = *" << *(int*)_right_buffer->getThreshold() << "*" << endl;
    cout << "*********right slack = *" << _right_buffer->getSlack() << "*" << endl;
      */
    
    }  // WHEN TUPLE IS LEFT


    //******************************************************


  for (int ii = 0; ii < _train_size[1]; ii++)
    {

	// LoadShedder-related calls    BEGIN (tatbul@cs.brown.edu)
	//
	// skip this tuple if the load shedding flag is set to drop
	//
	if (!(_ls_flag[1][ii]))
	{
		//cout << "DROPPING TUPLE!!" << endl;
		//printTuple(_inStream[1] + (ii*_tuple_size), _tuple_descr);
		continue;
	}
	else
	{
		//cout << "PROCESSING TUPLE!!" << endl;
		//printTuple(_inStream[1] + (ii*_tuple_size), _tuple_descr);
	}
	//
	// LoadShedder-related calls    END (tatbul@cs.brown.edu)

      _right_curr_tuple = (char*) malloc(_right_tuple_size);
      memcpy(_right_curr_tuple, _inStream[1] + (ii * _right_tuple_size), _right_tuple_size);


      //      cout << "[JoinQBox] RIGHT INPUT: ";
      //      printTuple(_right_curr_tuple, _tuple_descr_array[1]);

      gettimeofday(&_curr_ts, NULL);
      _curr_seconds = _curr_ts.tv_sec;

      _right_order_att_value = new char[_right_order_att_size];
      memcpy(_right_order_att_value, _right_field_att->evaluate(_right_curr_tuple), _right_order_att_size);
    
      _right_buffer->addToVector(_right_order_att_value);

      if (_left_buffer->size() == 0 )
	{
	  //cout << "[JoinQBox] creating new node for right buffer (empty left_buffer)" << endl;
	  node new_node = {_right_order_att_value, _right_curr_tuple};
	  if (_right_buffer->size() == 0)
	    _right_buffer->setThreshold(_right_order_att_value);
	  _right_buffer->push_back(new_node);
	  continue;
	}
      else
	{
	  if (_right_buffer->size() == 0)
	    _right_buffer->setThreshold(_right_order_att_value);

	  int right_threshold = *(int*) _right_buffer->getThreshold();
	  int right_value = *(int*) _right_order_att_value;
	  
	  if (right_value >= right_threshold)
	    {
	      int left_value = 0;
	      list<node>::iterator left_iter = _left_buffer->begin();
	 
	      node temp_node;
	      while (left_iter != _left_buffer->end())
		{
		  temp_node = *left_iter;
		  left_value = *(int*) temp_node.att;

		  if (((abs (left_value - right_value)) <= _buffer_size_before) ||
		      ((abs (right_value - left_value)) <= _buffer_size_after))
		    {
		      //		      if (_join_predicate->evaluate(_right_curr_tuple, temp_node.tuple))

		      if (_join_predicate->evaluate(temp_node.tuple, _right_curr_tuple)) 
			{
			  emitTuple(temp_node.tuple, _right_curr_tuple);
			}
		    }
		  left_iter++;
		} // close while loop

	      int left_threshold = *(int*) _left_buffer->getThreshold();
	      
	      if ((left_threshold - right_value) <= _buffer_size_after)
		{
		  //cout << "[JoinQBox] creating new node for right buffer" << endl;
		  node new_node = {_right_order_att_value, _right_curr_tuple};
		  if (_right_buffer->size() == 0)
		    _right_buffer->setThreshold(_right_order_att_value);
		  _right_buffer->push_back(new_node);
		}

	      right_threshold = *(int*) _right_buffer->getThreshold();
	      if (right_value > right_threshold)
		{
		  _right_buffer->increaseSlack();
		  int right_curr_slack = _right_buffer->getSlack();
		  
		  while (right_curr_slack > _right_slack)
		    {
		      _right_buffer->increaseThreshold();
		      right_threshold = *(int*) _right_buffer->getThreshold();
		      right_curr_slack = _right_buffer->countAndClearVector();
		      _right_buffer->setSlack(right_curr_slack);
		    }
		  
		  _left_buffer->erase(_right_buffer->getThreshold(), _buffer_size_before);
		  
		  
		}
	      
	    }  // close if right_value >= threshold
	}   // close when left_buffer exists
      


      /**
    cout << "*********right threshold = *" << *(int*)_right_buffer->getThreshold() << "*" << endl;
    cout << "*********right slack = *" << _right_buffer->getSlack() << "*" << endl;
    cout << "*********left threshold = *" << *(int*)_left_buffer->getThreshold() << "*" << endl;
    cout << "*********left slack = *" << _left_buffer->getSlack() << "*" << endl;
      */
    }  // WHEN TUPLE IS RIGHT

  //cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$ END DOBOX $$$$$$$$$$$$$$$$$$$$$" << endl << endl;
  

  // return val stuff NOW
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


void JoinQBox::emitTuple(char *left_tuple, char *right_tuple)
{
  //_output_tuple = new char[_output_tuple_size];
 
  char *temp = _output_tuple;
  
  // copy the timestamp - smaller of the two
  Timestamp left_time = getTs(left_tuple);
  Timestamp right_time = getTs(right_tuple);

  if (left_time < right_time)
    {
      memcpy(_output_tuple, &left_time, getTsSize());
    }
  else
    {
      memcpy(_output_tuple, &right_time, getTsSize());
    }
  _output_tuple += getTsSize();

  // copy the sid
  int sid = 5;
  memcpy(_output_tuple, &sid, getSidSize());
  _output_tuple += getSidSize();

  // copy left_tuple
  char *temp_left = left_tuple;
  memcpy(_output_tuple, temp_left + _hidden_size, _left_tuple_size - _hidden_size);
  _output_tuple += (_left_tuple_size - _hidden_size);
 
  //   cout << "******************************* " << _left_tuple_size - _hidden_size << endl;

  // copy right_tuple
  char *temp_right = right_tuple;
  memcpy(_output_tuple, temp_right + _hidden_size, _right_tuple_size - _hidden_size);
  _output_tuple += (_right_tuple_size - _hidden_size);

  //  cout << "******************************* " << _right_tuple_size - _hidden_size << endl;

  //cout << "[JoinQBox] OUTPUT!" ;
  //printTuple(temp, "tiii");

  ++_num_tuples_emitted;

  //printTuple(_right_curr_tuple, "ttiifcci");

}

