#include "BSortQBox.H"

/**
 * BSORT
 * All Timestamping stuff commented out Mar 26 03 since it seems we no longer
 * care about any timeouts in BSORT
 */


/**
 * Gets a modifier from the gui.
 * sortbyattrib~slack~list-of-groupbys
 * :0:i:16:4:~1~:0:i:12:4:,:0:i:8:4:
 *
 */
void BSortQBox::setBox(const char* modifier_str, HashForBufferList* buffer_hash) {
  //cout << "[BSortQBox] ENTERED SETBOX(string)" << endl;
  //cout << " MODIFIER_STR LOOKS LIKE THIS: " << endl;
  //printf("[%s]\n",modifier_str);


  // Gimme my hash back
  _buffer_hash = buffer_hash;


  // The sorting attrib you can make it right away
  _field_att = new FieldExt(modifier_str); // works cuz FieldExt parses very safely just what it needs
  //_att_type = _field_att->getType();
  // The slack - thanks strtol for doing the dirty work
  _slack = strtol(index(modifier_str,'~') + 1, NULL, 10);
  //_buffer_size = _slack + 1;
  // And now the group by stuff
  parseGroupBy(rindex(modifier_str,'~')+1); // go reverse to find the ~ this time
  // tuple size!
  _tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();

  //cout << " [BSortQBox] setBox() Parsing done! Tuple size " << _tuple_size << " and Slack is " << _slack << endl;

}


// group_by_atts may be null to indicate no grouping by
void BSortQBox::setBox(char *att, long slack, char *group_by_atts) 
{
  cout << " DONT USE ME ! IM NOT REAL! " << endl;
  abort();
  //_buffer_size = slack + 1;
  _slack = slack;
  
  _field_att = new FieldExt(att);
  //_att_type = _field_att->getType();
  
  //  gettimeofday(&_last_time_emitted, 0);   //************* need to save this in agg-state
  //******* if 0 is agg-state then set to curr_time as above
  
  parseGroupBy(group_by_atts);
  
  _buffer_hash = new HashForBufferList();
  _tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
}

Box_Out_T BSortQBox::doBox()
{
  _num_tuples_emitted = 0;
  _output_tuple_ptr = _outStream[0];

  //cout << "[BSortQBox] ****************************** BEGIN *****************************" << endl;
  for (int ii = 0; ii < _train_size[0]; ii++)
  
    {
      _curr_tuple = (char*) malloc(_tuple_size);
      memcpy(_curr_tuple, _inStream[0] + (ii * _tuple_size), _tuple_size);
      //cout << "[BSortQBox] TRAIN STEP " << ii << " : INPUT: ";
      //printTuple(_curr_tuple, "iiii");
      //cout << "[BSortQBox] Calling addToBuffer..." << endl;
      addToBuffer(_curr_tuple);
      //gettimeofday(&_curr_ts, NULL);
      //_curr_seconds = _curr_ts.tv_sec;

      // If the buffer for the current group is full...
      if (_buffer_hash->getBufferListSize(_group_by_values_str) == (_slack + 1))
	{ // Then emit the lowest value
	  // OPTIMIZATION: You could emit as soon as you are adding (addToBuffer)...
	  emitLowest(_buffer); // NOTE. the use of _buffer here depends on addToBuffer setting _buffer
	  //_buffer->setLastEmittedTime(_curr_ts);
	  ++_num_tuples_emitted;
	}
    }
  //cout << "[BSortQBox] ****************************** END *****************************" << endl;

  // This box always sucks every tuple
 return_val.kept_input_count = 0;
 return_val.output_tuples = _num_tuples_emitted; // DEPRECATED
 return_val.output_tuple_size = _tuple_size;

 // This is very important to maintain too!
 return_val.output_tuples_array = new int[1];
 return_val.output_tuples_array[0] = _num_tuples_emitted;

 return return_val;

}

/**
 * Due to reasons baffled in HashForBufferList.C and below, to create the "group by values string"
 *  used for hashing, one needs to collapse any numerical values into a "string representation"
 *  of it. Why? numericals may use a nul bytes (a byte where all 8 bits are 0), but then if you
 *  use that in the context of the hashing function, the nul byte (say decimal 0) is seen as
 *  the "end of string" destroying our intention of what we want hashed...
 
 * [original bug mentioning]
 * !BUG HASHNUL! If you just squish the bytes for each group_by value, and use it as a hash
 *  key, you get some region in memory (char*) that has some null bytes in it, and guess what!
 *  The hash function within hash_map stops at the first nul byte! (and in our representation,
 *   an integer, using 4 bytes, is represented as ("4 nul nul nul") so if you combine two values
 *   you get something like ("4 nul nul nul 2 nul nul nul"), supposed to be the key "4-2" but
 *   due to the nul bytes, the hash will only look at "4" so "4-3" hashes to the same place in the buffer!
 *  The real reason, is because __stl_hash_string(char *), obviously, stops at the first nul byte of the char*

 *
 * SOOO Take numericals (floats, ints, anything). Make them "strings". squeeze them together by
 *  putting a 0x01 byte in between each "string". For string, just use their chars.
 */
void BSortQBox::addToBuffer (char *tuple)
{
  // deal with the sorting attribute first
  int att_size = _field_att->getReturnedSize();
  char *att_value;
  char _att_type = _field_att->getType();
  if (_att_type == 's') {
    att_value = new char[att_size + 1]; // to accomodate the terminating null returned
    memcpy(att_value, _field_att->evaluate(tuple), att_size + 1);
  }
  else {
    att_value = new char[att_size];
    memcpy(att_value, _field_att->evaluate(tuple), att_size);
  }
  // WARNING: _field_att->evaluate() for a string returns
  //  a null-terminated string (so its actual bytes is getReturnedSize() + 1)
  //   SOOOO I fixed it -- see the above two calls to memcpy
  // NOTE!!!!!!!!!!! THIS, I THINK, REALLY SHOULD BE FIXED BY MAKING FIELDEXT
  //  RETURN (SIZE + 1) WHEN FIELDEXT IS DEALING WITH A STRING!!!
  // if this "fix" is done, rewrite the stuff above to just have the one call below
  //  - eddie
  //memcpy(att_value, _field_att->evaluate(tuple), att_size);
  node new_node = {att_value, tuple};

  // NEW _group_by_values_str: holds as explained before function, char representation of all
  //  values (numericals are made into chars by sprintf), with 0x01 in between, ending in \0
  //   Why the temp buffer starts at (64 + 1) * num of attribs? See FieldExt.C
  _group_by_values_str = (char*) realloc(_group_by_values_str,(65 * _num_group_by_atts) + 1);
  char* _group_by_values_str_i = _group_by_values_str; // temporary pointer
  int bytesNeeded = 0; int bytesNeededTotal = 0;
  for (int ii = 0; ii < _num_group_by_atts; ii++) {
    if (ii > 0) { // put the 0x01 chararacter between values
      char sep = 0x01;
      memcpy(_group_by_values_str_i,&sep,1);
      ++_group_by_values_str_i; ++bytesNeededTotal;
    }
    //_field_group[ii]->evaluateAsChar(tuple,bytesNeeded);
    // Careful here -- this is exploiting the fact that first the evaluateAsChar is evaluated,
    //  setting bytesNeeded which THEN is read for memcpy which does its job copying the memory
    //  and only AFTER will the rvalue returned by evaluateAsChar get destroyed
    //  WARNING -- the paragraph above describes something that DOESNT WORK. It just doesnt.
    //              so you need a temp, that you clean up right away anyways
    char* temp_group_by_values_str = _field_group[ii]->evaluateAsChar(tuple,bytesNeeded);
    memcpy(_group_by_values_str_i,temp_group_by_values_str,bytesNeeded);
    free(temp_group_by_values_str);
    _group_by_values_str_i += bytesNeeded;
    bytesNeededTotal += bytesNeeded;
  }
  // Squeeze it back down, putting the \0 at the end too
  _group_by_values_str = (char*) realloc(_group_by_values_str,bytesNeededTotal+1);
  _group_by_values_str[bytesNeededTotal] = '\0';
  if (_buffer_hash->isEmpty() || 
      _buffer_hash->getBufferList(_group_by_values_str) == NULL)   // nothing in the hash
    {
      _buffer = new BufferList(_group_by_values_str, bytesNeededTotal+1);
      // I SWAPPED THE POSITION OF THE NEXT TWO LINES -eddie
      //   because, amongst other things, I think the hash_map internally
      //   just created a copy? which would meant the push_front wouldn't do much, but perhaps I'm wrong
      _buffer->push_front(new_node);   // add node to BufferList
      _buffer_hash->addBufferList(_buffer);  // finally add the BufferList to hash
      // NOTE: Tempting to add _group_by_values_str to addBufferList call, but see comment in HashForBufferList
    }

  else 
    {
      if (_buffer_hash->getBufferListSize(_group_by_values_str) == 0)  
	{
	  _buffer = _buffer_hash->getBufferList(_group_by_values_str);
	  _buffer->push_front(new_node);     // nothing else in this list
	}
      else 
	{
	  // This whole "different adding needed depending on the attribute type
	  //  is only here because we need to compare the values of the types (which must be
	  //  cast from the (char*) buffer into whatever it needs to be - eddie
	  switch (_att_type)
	    {
	    case 'f':
	      {
		addFloatToBuffer(att_value, new_node);
		break;
	      }
	    case 'i':
	      {
		addIntToBuffer(att_value, new_node);
		break;
	      }
	    case 's':
	      {
		addStringToBuffer(att_value, new_node);
		break;
	      }
	    }
	}
  
    }
}
/**
 * FOR ALL THE addXXXToBuffer I wonder... couldn't you just insert anywhere then call sort ?
 */
void BSortQBox::addFloatToBuffer(char *att_value, node new_node)
{
  float curr_att_value = *(float*) att_value;
  float buffer_att_value;

  _buffer = _buffer_hash->getBufferList(_group_by_values_str);

  buffer_att_value = *(float*) _buffer->front().att;
  if (curr_att_value < buffer_att_value)
    {
      _buffer->push_front(new_node);
    }
  else 
    {
      buffer_att_value = *(float*) _buffer->back().att;
      if (curr_att_value >= buffer_att_value)
	{
		//cout << "[BSortQBox] Buffer: PUSH BACK " << endl;
	  _buffer->push_back(new_node);
	}
      else
	{
	  list<node>::iterator iter = _buffer->begin();
	  iter++;    // first node already checked
	  
	  node temp_node;
	  while (iter != _buffer->end())
	    {
	      temp_node = *iter;
	      buffer_att_value = *(float*) temp_node.att;
	      if (curr_att_value < buffer_att_value)
		{
		  _buffer->insert(iter, new_node);
		  break;
		}
	      iter++;
	    }
	}
    }
}


void BSortQBox::addIntToBuffer(char *att_value, node new_node)
{
  int curr_att_value = *(int*) att_value;
  int buffer_att_value;

  _buffer = _buffer_hash->getBufferList(_group_by_values_str);

  assert(_buffer != NULL);
  assert(_buffer->size() > 0);
  assert(! _buffer->isEmpty());

  buffer_att_value = *(int*) _buffer->front().att;
  if (curr_att_value < buffer_att_value)
    {
      _buffer->push_front(new_node);
    }
  else 
    {
      buffer_att_value = *(int*) _buffer->back().att;
      if (curr_att_value >= buffer_att_value)
	{
	  _buffer->push_back(new_node);
	}
      else
	{
	  list<node>::iterator iter = _buffer->begin();
	  iter++;    // first node already checked
	  
	  node temp_node;
	  while (iter != _buffer->end())
	    {
	      temp_node = *iter;
	      buffer_att_value = *(int*) temp_node.att;
	      if (curr_att_value < buffer_att_value)
		{
		  _buffer->insert(iter, new_node);
		  break;
		}
	      iter++;
	    }
	}
    }
}



void BSortQBox::addStringToBuffer(char *att_value, node new_node)
{
  // the +1 here is because we will add the \0. WARNING if you change FieldExt
  // semantics to add the 1...
  int att_size = _field_att->getReturnedSize() + 1;  
  char *buffer_att_value = new char[att_size];

  _buffer = _buffer_hash->getBufferList(_group_by_values_str);

  // Can the new string go at the front?
  memcpy(buffer_att_value, _buffer->front().att, att_size);
  // WARNING!!! Shouldnt this check for equality too? I mean if its equal
  //  you could just stick it in front (or rather, right after)
  if (strcmp(att_value, buffer_att_value) < 0)
    {
      _buffer->push_front(new_node);
    }
  else 
    {
      // does it go all the way at the back?
      memcpy(buffer_att_value, _buffer->back().att, att_size);
      if (strcmp(att_value, buffer_att_value) >= 0)
	{
	  _buffer->push_back(new_node);
	}
      else
	{ // goes somewhere in the middle, figure it out
	  list<node>::iterator iter = _buffer->begin();
	  ++iter;    // first node already checked
	  
	  node temp_node;
	  while (iter != _buffer->end())
	    {
	      temp_node = *iter;
	      memcpy(buffer_att_value, temp_node.att, att_size);
	      if (strcmp(att_value, buffer_att_value) < 0)
		{
		  _buffer->insert(iter, new_node);
		  break;
		}
	      ++iter;
	    }
	}
    }
}




void BSortQBox::emitLowest(BufferList *buffer)
{
  // Euh... "The c++ stl reference" says pop_front() does NOT return the element...
  //  yet this works.. hmm
  // BAAAAAAAAAA ignore me. buffer isnt a vector, its a BufferList which DOES return it...
  char *tuple = buffer->pop_front().tuple;
  emitTuple(tuple);
}

void BSortQBox::emitTuple(char *tuple)
{
  //*******************
  memcpy(_output_tuple_ptr, tuple, _tuple_size);
  _output_tuple_ptr += _tuple_size;
  
  //cout << "[BSortQBox] OUTPUT TUPLE: " ;
  //printTuple(tuple, "ttiifcc") TEST 1
  //printTuple(tuple, "ttiifcci"); // TEST 2
  //printTuple(tuple, "iiii"); // Integration test
}

// SideEffect: creates _field_group with ptrs to FieldExt's for each groupby
//             and sets _num_group_by_atts. It should also set _group_by_size,
//             the # of bytes needed to store all values of the group by attrs
//             (here, DO NOT INCLUDE THE \0 in counting size of strings)
// example atts = ":0:i:0:4:~:0:i:0:4:~:0:i:0:4:"
//    variables set by parseGroupBy 
//                    _field_group (FieldExt)
//                    _group_by_size (int)
//                    _num_group_by_atts (int)
// April 2: GOOD BYE _GROUP_BY_SIZE (want the size? use strlen on _group_by_values_str)
// April 3 - its "," seperated, not ~ anymore
void BSortQBox::parseGroupBy(char *atts)
{
  _group_by_values_str = NULL;
  if (atts == NULL || strcmp(atts,"") == 0) { // Check both that it is "null" and the empty string (not the same)
    // No group_by attributes means this operator basically groups everyone in the same place
    // OPTIMIZATION OPPORTUNITY: we could then not even use the hash...
    _num_group_by_atts = 0;
    return;
  }
  // We assume atts is null-terminated
  char *p = index(atts, ',');
  if (p == NULL) { // No match for "~" means only one group by
    _num_group_by_atts = 1;
    _field_group = new FieldExt*[1];
    _field_group[0] = new FieldExt(atts);
    return;
  } else { // Let's figure out how many group bys
    int num_atts = 2; // At least two already
    p = index(p+1,','); // Resume search after the ~
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
      if (p != NULL) p++; // To skip the ~ (FieldExt doesnt want it)
    }
    _num_group_by_atts = num_atts;
    return;
  }
}
