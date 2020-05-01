#include "RestreamQBox.H"
#include "tupleGenerator.H"


RestreamQBox::RestreamQBox() 
{
  _sid_size = getSidSize();
  _ts_size = getTsSize();
}

void RestreamQBox::setModifier(const char* input)
{
  printf("Got to Restream setModifier: %s\n", input);


  int hidden_size = _ts_size + _sid_size; 
  _tuple_size = _tuple_descr->getSize() + hidden_size;
  int y = 0;
  int count = 0;
  _num_atts = 0;
  while (input[y] != '\0') 
    { 
      if (input[y] == ',')
	count++;
      y++;
    }
  _num_atts = count + 1;   // +1 for the attribute after the last '~'

  _fields = parseAtts(input, _num_atts);

  _group_by_size = 0;
  for (int i = 0; i < _num_atts; i++)
    _group_by_size += _fields[i]->getReturnedSize();
  

  
  _group_hash->setKeySize(_group_by_size);
  //  _group_hash = new GroupByHash(_group_by_size);
}

Box_Out_T RestreamQBox::doBox() 
{
  char *output_tuple;
  //char *out_index;
  output_tuple = _outStream[0];
  int hidden_size = _ts_size + _sid_size; 
  char *att_values;
  int index;
  int temp_size;
  int new_sid;
  for (int ii = 0; ii < _train_size[0]; ii++)
    {
      if (_tuple_size == 15)
	printTuple(_inStream[0] + (ii*_tuple_size), "ticccf");
      att_values = new char[_group_by_size]; 
      index = 0;
      temp_size = 0;
      for (int i = 0; i < _num_atts; i++) 
	{
	  temp_size = _fields[i]->getReturnedSize();
	  printf("doBoxVal: %d\n", *(int*)_fields[i]->evaluate(_inStream[0] + (ii*_tuple_size)));
	  memcpy(att_values + index, 
		 _fields[i]->evaluate(_inStream[0] + (ii*_tuple_size)), 
		 temp_size);
	  index += temp_size;
	}
      
      GroupByState *s = _group_hash->getGroupByState(att_values);
      if (!s)
	{
	  new_sid = _group_hash->getSize();
	  s = new GroupByState(att_values, _group_by_size, new_sid);
	  _group_hash->addGroupByState(s);
	}
      else
	{
	  new_sid = s->getNewSid();
	}
      
      //output_tuple = new char[_tuple_size];
      
      memcpy(output_tuple, _inStream[0] + (ii* _tuple_size), _ts_size);    // copying the timestamp first
      
      output_tuple += _ts_size;
      memcpy(output_tuple, &new_sid, _sid_size);   // copying the new SID
      
      output_tuple += _sid_size;
      memcpy(output_tuple, _inStream[0] + hidden_size + (ii* _tuple_size), _tuple_size - hidden_size);
      
      output_tuple += _tuple_size - hidden_size;
      
      // *****************************************************
      // temporarily print to see the output_tuple
      if (_tuple_size == 24) {
	const char *desc =  "iiiifcccc"; //"tiicccci";
	cout << "[Restream]" ;
	printTuple(output_tuple-_tuple_size, desc);
	printf ("\t  will be placed in the output stream -- Yahoo!\n\n");
      // *****************************************************
      }
    }
  
  return_val.kept_input_count = 0;
  return_val.output_tuples = _train_size[0];
  return_val.output_tuple_size = _tuple_size;

  // ADDED BY CJC TO ADDRESS VALGRIND'S COMPLAINT ABOUT AN UNINITIALIZED 
  // VARIABLE BEING USED IN AN IF(...) EXPRESSION IN WorkerThread.C.
  // IF THIS FIXUP IS AN ERROR, PLEASE CORRECT IT. -CJC, 3 MARCH 2003.
  return_val.kept_input_count_array = NULL;

  return return_val;
}



void RestreamQBox::setHash(GroupByHash *hash)
{
  _group_hash = hash;
}
