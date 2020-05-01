#include "QBox.H"

QBox::~QBox() 
{ 
}

FieldExt** QBox::parseAtts(const char *group_by_atts, int num_atts)
{
  char **atts = new char*[num_atts];
  int y = 0;
  int j = 0;
  char *temp; 
  int k;
  int m;

  // atts[] will hold the char* for each of the attributes
  // this will enable us to form FieldExt classes to get the attribute values
  // from the tuple
  while (group_by_atts[y] != '\0')
    {
      k = y;
      m = 0;
      //      while (group_by_atts[k] != '~' && group_by_atts[k] != '\0')
      while (group_by_atts[k] != ',' && group_by_atts[k] != '\0')
	{
	  m++;        // count the number of characters in :0:i:8:4:
	  k++;        // k is temp pointer until , or \0 is encountered
	}
      temp = new char[m];   
      int i = 0;      
      //      while (group_by_atts[y] != '~' && group_by_atts[y] != '\0')
      while (group_by_atts[y] != ',' && group_by_atts[y] != '\0')
	{
	  temp[i] = group_by_atts[y];   // copy the group_by_atts to temp
	  i++;                          // which is later copied to atts[]
	  y++;
	}
      atts[j] = temp;
      if (group_by_atts[y] != '\0') 
	{
	  j++;          
	  y++;        // disregard the , in the middle  
	}
    }
  FieldExt **fields = new FieldExt*[num_atts];
  int a_offset;
  int a_size;
  int a_port;
  char a_type;
  char *curr_string; 
  
  // using the atts[] array find the offset and size of each attribute
  // then create FieldExt objects for attributes
  for (int i = 0; i < num_atts; i++) 
    {
      curr_string = atts[i];
      int x = 0;  
      while (curr_string[x] != ':')
	x++;
      x++;
      a_port = atoi(curr_string + x);  
      while (curr_string[x] != ':') 
	x++;
      x++;
      a_type = curr_string[x];
      x += 2;
      a_offset = atoi(curr_string + x);
      while (curr_string[x] != ':') 
	x++;
      x++;
      a_size = atoi(curr_string + x);
      fields[i] = new FieldExt(a_port, a_type, a_offset, a_size);
    }
  return fields;
}

int QBox::getSidSize()
{
  return TUPLE_STREAMID_SIZE;
}

int QBox::getTsSize()
{
  return TUPLE_TIMESTAMP_SIZE;
}

char* QBox::getSid(char *tuple)
{
  char* sid = (char*) malloc(TUPLE_STREAMID_SIZE);
  if (sid == NULL) {
    perror("[QBox] getSid: malloc for sid failed!");
    exit(1);
  }
  memcpy(sid, tuple + TUPLE_STREAMID_OFFSET, TUPLE_STREAMID_SIZE);
  return sid;
}

/** Dan says to keep it as an int. THIS SHOULD BE A CHAR* TO NOT TIE OURSELVES TO A DATA TYPE!!! **/
// NOTE: Eddie made this a ONE LINER. Woop! :)
Timestamp QBox::getTs(char *tuple)
{
  /**
     char* ts = malloc(TUPLE_TIMESTAMP_SIZE);
     if (ts == NULL) {
     perror("[QBox] getTs: malloc for ts failed!");
     exit(1);
     }
     // I wonder if you have to do this... oh no, you dont!
     //memcpy(ts, tuple + TUPLE_TIMESTAMP_OFFSET, TUPLE_TIMESTAMP_SIZE);
     //int return_val = *(int*)ts;
     //free(ts);
     */
  //  int return_val = *(int*)(tuple + TUPLE_TIMESTAMP_OFFSET);
  //  return return_val;

  //  long return_val = ((Timestamp*) tuple)->tv_sec;
  //  return return_val;
  long sec = ((Timestamp*) tuple)->tv_sec;
  long micro = ((Timestamp*) tuple)->tv_usec;
  Timestamp time = *(new Timestamp(sec, micro));
  return time;
}

// LoadShedder-related member functions BEGIN (tatbul@cs.brown.edu)
//
/*
 * This function decides which tuples to drop. 
 */
void QBox::applyLoadShedding()
{
	int i, j;
	int tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();

	srand(time(NULL));
	static unsigned int seed = 1;
	double rand_val;

	// initialize the flags
	//
	_ls_flag = (bool **)malloc(sizeof(bool *)*_numInputArcs);
	for (i = 0; i < _numInputArcs; i++)
	{
		if (_train_size[i] > 0)
		{
			_ls_flag[i] = (bool *)malloc(sizeof(bool)*_train_size[i]);

			for (j = 0; j < _train_size[i]; j++)
			{
				_ls_flag[i][j] = true;
			}
		}
	}

	// if no drop info for this box, return
	//
	if (_ls_info.empty())
	{
		//cout << "ls_info is empty" << endl;
		return;
	}

	for (i = 0; i < _numInputArcs; i++)
	{
		// if no drop at this arc, skip the loop
		//
		if (_ls_info.find(_inputArcId[i]) == _ls_info.end())
		{
			continue;
		}

		if (_ls_info[_inputArcId[i]]->drop_predicate)
		{
			// apply the predicate on _inStream[i]
			//
			for (j = 0; j < _train_size[i]; j++)
			{
				if (!(_ls_info[_inputArcId[i]]->drop_predicate->
										evaluate(_inStream[i] + j*tuple_size)))
				{
					_ls_flag[i][j]	= false;
				}
			}
		}
		else if (_ls_info[_inputArcId[i]]->drop_rate > 0)
		{
			// drop randomly from _inStream[i]
			//
			
			for (j = 0; j < _train_size[i]; j++)
			{
				rand_val = (double)rand_r(&seed)/(double)(RAND_MAX);

				if (rand_val <= _ls_info[_inputArcId[i]]->drop_rate)
				{
					_ls_flag[i][j]	= false;
				}
			}
		}
		else
		{
			// It must be the case that the drop predicate already evaluates to
			// false
			//
			for (j = 0; j < _train_size[i]; j++)
			{
				_ls_flag[i][j]  = false;
			}
		}
	}
}
//
// LoadShedder-related member functions END (tatbul@cs.brown.edu)
