#include "WSortQBox.H"

WSortQBox::WSortQBox()
{
}

WSortQBox::~WSortQBox()
{
}

void WSortQBox::setState(AggregateState *agg)
{
  const char *atts = agg->group_by;
  _maxtime = agg->unless_timeout.tv_sec; 
  _tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
  _last_emitted = agg->last_emitted;
  if (!_last_emitted) {
    cout << "[WSortQBox] Allocating " << _tuple_size << " for _last_emitted " << endl;
    _last_emitted = new char[_tuple_size];
    agg->last_emitted = _last_emitted;
        //    _last_emitted[0] = -1;
    cout << "[WSortQBox] : Set value for _last_emitted to "<< _last_emitted << endl;
  }

    cout << "[WSortQBox] _last in setState() method" ; printTuple(_last_emitted, "iiiifcccc");

  // the attributes are delimited by ','
  int y = 0;
  int count = 0;
  _num_atts = 0;
  while (atts[y] != '\0') 
    { 
      if (atts[y] == ',')
	count++;
      y++;
    }
  _num_atts = count + 1;   // +1 for the attribute after the last ','

  FieldExt **left_atts = parseAtts(atts, _num_atts);
  FieldExt **right_atts = parseAtts(atts, _num_atts);

  _preds_compare = new Predicate*[_num_atts];
  _preds_equal = new Predicate*[_num_atts];

  for (int j = 0; j < _num_atts; j++)
    {
      left_atts[j]->setPort(0);
      right_atts[j]->setPort(1);
    }

  _buffer = agg->buffer;//list<buffer_node>();
  //  _num_tuples_emitted = 0;

  for (int i = 0; i < _num_atts; i++)
    {
      FieldExt *left = left_atts[i];
      FieldExt *right = right_atts[i];
      switch (left->getType())
	{
	case 'i':
	  {
	    _preds_compare[i] = new IntLTPredicate(left, right);
	    _preds_equal[i] = new IntEqualPredicate(left, right);
	    break;
	  }
	case 'f':
	  {
	    _preds_compare[i] = new FloatLTPredicate(left, right);
	    _preds_equal[i] = new FloatEqualPredicate(left, right);
	    break;
	  }
	case 's':
	  {
	    _preds_compare[i] = new StringLTPredicate(left, right);
	    _preds_equal[i] = new StringEqualPredicate(left, right);
	    break;
	  }
	case 't':
	  {
	    _preds_compare[i] = new TsLTPredicate(left, right);
	    _preds_equal[i] = new TsEqualPredicate(left, right);
	    break;
	  }
	}
    }
}

Box_Out_T WSortQBox::doBox()
{
  //  time_t curr_time;
  _num_tuples_emitted = 0;
  _output_tuple = _outStream[0];

  cout << "[WSortQBox] doBox has been called" << endl;

  for (int ii = 0; ii < _train_size[0]; ii++) {
    _curr_tuple_emitted = false;
    cout << "[WSortQBox] Allocating " << _tuple_size << " for _curr_tuple " << endl;
    _curr_tuple = new char[_tuple_size];
    memcpy(_curr_tuple, _inStream[0] + (ii * _tuple_size), _tuple_size);
    //    if (_tuple_size = 20)
    cout << "[WSortQBox] input: "; printTuple(_curr_tuple, "iiiifcccc");
    cout << "[WSortQBox] _last in doBox() method " << ii ; printTuple(_last_emitted, "iiiifcccc");

    cout << "buffer beginning of doBox() " << _buffer->size() << endl;

    //    time(&curr_time);
    //_curr_ts = curr_time;
    
    gettimeofday(&_curr_ts, 0);
    _curr_seconds = _curr_ts.tv_sec;

    timeOutBufferNodes(_curr_ts);
    
    // the last emitted tuple was bigger than the current one
    // discard tuple
    if (!_curr_tuple_emitted)
      {   
	if (bufferNodeCompare(_last_emitted, _curr_tuple) > 0)
	  //discard tuple;
	  cout << "[WSortQBox] discarded tuple" << endl;
	else
	  addToBuffer(_curr_tuple);
      }

  /*  // if buffer is empty, just add it
      else  if (_buffer->empty())  
      addToBuffer(_stream1);
      
      // else if the lowest tuple is smaller than the current tuple
      // add to the buffer
      else if (bufferNodeCompare(_buffer->back().tuple, _stream1) < 0)
      {
      //      emitLowest();
      addToBuffer(_stream1);
      }
      
      // else if the last_emitted is smaller than the current tuple
      // emit the current tuple
      else if (bufferNodeCompare(_last_emitted, _stream1) < 0)
      {
      addToBuffer(_stream1);
      //      emitTuple(_stream1);
      //      _num_tuples_emitted++;
      //      _last_emitted = _stream1;
      }
  */

    cout << " THE BUFFER " << endl << endl;
    
    printBuffer();
    
    cout << "buffer end of doBox() " << _buffer->size() << endl;
    
  }
  return_val.kept_input_count = 0;
  return_val.output_tuples = _num_tuples_emitted;
  return_val.output_tuple_size = _tuple_size; 
  //  return_val.currStates = _hash;

  // ADDED BY CJC TO ADDRESS VALGRIND'S COMPLAINT ABOUT AN UNINITIALIZED 
  // VARIABLE BEING USED IN AN IF(...) EXPRESSION IN WorkerThread.C.
  // IF THIS FIXUP IS AN ERROR, PLEASE CORRECT IT. -CJC, 3 MARCH 2003.
  return_val.kept_input_count_array = NULL;

  return return_val;
}

void WSortQBox::timeOutBufferNodes(Timestamp t)
{
  list<buffer_node>::iterator iter;
  iter = _buffer->end();   // iter points to one past the last element
  iter--;     
  buffer_node temp_node;

  bool broken = false;
  while (iter != _buffer->begin())  // will miss the first element, deal later
    {
      temp_node = *iter;

      cout << "compared timestamps   _curr_ts = " << _curr_ts.tv_sec << "    ts is " << temp_node.ts.tv_sec << endl;

      //      if ((_curr_ts - temp_node.ts) > _maxtime)
      if ((_curr_ts.tv_sec - temp_node.ts.tv_sec) > _maxtime)
      {
	  removeUntilNode(temp_node);
	  broken = true;
	  break;
	}
      iter--;
    }
  
  // check for the first element now
  // two things may have happened but does not matter
  // first node may be the new one after tuples were emitted from above
  // in this case, the timeout will be false because it was false above when
  // this particular node was check -> so does not matter
  // first node may be checked for the first time in which case it may or may not
  // timeout -> handle accordingly
  temp_node = *_buffer->begin();
  //  if (temp_node.ts != 0)
  if (temp_node.ts.tv_sec != 0)
    {
      cout << "compared timestamps   _curr_ts = " << _curr_ts.tv_sec << "    ts is " << temp_node.ts.tv_sec << endl;
      
      //      if ((_curr_ts - temp_node.ts) > _maxtime)
      if ((_curr_ts.tv_sec - temp_node.ts.tv_sec) > _maxtime)
	removeUntilNode(temp_node);
    }
}

void WSortQBox::removeUntilNode(buffer_node elem)
{
  list<buffer_node>::iterator iter;
  iter = _buffer->begin();

  buffer_node temp_node = *iter;
 
  while (!(elem == temp_node))   //(&elem != &temp_node)
    {
      emitLowest();
      iter = _buffer->begin();

      temp_node = *iter;
      iter++;
    }
  // remove the node that caused the timeout
  if (elem == temp_node)
    emitLowest();
}

void WSortQBox::emitLowest()
{
  _num_tuples_emitted++;
  char *tuple = _buffer->front().tuple;
  if (!_curr_tuple_emitted && (bufferNodeCompare(_curr_tuple, tuple) < 0) &&  (bufferNodeCompare(_last_emitted, _curr_tuple) < 0))
    {
      _curr_tuple_emitted = true;
      emitTuple(_curr_tuple);
    }
  emitTuple(tuple);
  memcpy(_last_emitted, tuple, _tuple_size);;
  _buffer->pop_front();
  // delete the struct from memory
}

void WSortQBox::addToBuffer(char *tuple)
{
  char *temp_string = new char[_tuple_size];
  memcpy(temp_string, tuple, _tuple_size);
  buffer_node new_node = {temp_string, _curr_ts};

  // tuple is smaller than the first tuple is buffer  
  if (bufferNodeCompare(tuple, _buffer->front().tuple) <  0)
    {
      cout << "[WSortQBox] ****PUSH FRONT**** " ; printTuple(tuple, "iiiifcccc");
      _buffer->push_front(new_node);
    }
  // tuple is bigger than or equal to the last tuple in the buffer
  else if (bufferNodeCompare(tuple, _buffer->back().tuple) >= 0)
    {
      cout << "[WSortQBox] ****PUSH BACK **** " ; printTuple(tuple, "iiiifcccc");
    _buffer->push_back(new_node);
    }
  // none of the above
  else
    {
      list<buffer_node>::iterator iter = _buffer->begin();
      iter++;   // the first node has already been checked
      
      buffer_node temp_node;
      
      while (iter != _buffer->end())
	{
	  temp_node = *iter;
	  if (bufferNodeCompare(tuple, temp_node.tuple) < 0)
	    {
	      _buffer->insert(iter, new_node);
	      break;
	    }
	  iter++;
	}
    }
}

int WSortQBox::bufferNodeCompare(char *tuple1, char *tuple2)
{
  for (int i = 0; i < _num_atts; i++)
    {
      if (!tuple1)
	return -1;
      else if (*tuple1 == -1)
	return -1;
      else if (!tuple2)
	return 0;
      else if (_preds_compare[i]->evaluate(tuple1, tuple2))
	return -1;
      else if (!_preds_equal[i]->evaluate(tuple1, tuple2))
       	return 1;
    }
  return 0;
}

void WSortQBox::emitTuple(char *tuple)
{

  memcpy(_output_tuple, tuple, _tuple_size);
  _output_tuple += _tuple_size;
  // *****************************************************
  // temporarily print to see the output_tuple
  cout << "tuple size " << _tuple_size << endl;
  //  if (_tuple_size = 20) {
    const char *desc =  "iiiifcccc";
    cout << "[WSortQBox] output: ";
    printTuple(_output_tuple - _tuple_size, desc);
    printf ("\t  will be placed in the output stream -- Yahoo!\n\n");
  // *****************************************************
  //}
}




// for debugging
void WSortQBox::printBuffer()
{
  list<buffer_node>::iterator iter;
  iter = _buffer->begin();

  while (iter != _buffer->end())
    {
      cout << "\t[WSortQBox] " ; 
      printTuple((*iter).tuple, "iiiii");
      cout << " timestamp :  " << (*iter).ts.tv_sec << endl;
      iter++;
    }
  cout << endl;
}
