#include "StreamThread.H"
#include "tupleGenerator.H"
#include "Measurement.H"

extern QueueMon* _queue_monitor;

int StreamThread::_tuples_generated = 0;
double StreamThread::_rate_multiplier = 1.0;
int StreamThread::_input_counter = 0;

pthread_cond_t    *StreamThread::_sched_wake_cond  = NULL;
pthread_mutex_t   *StreamThread::_sched_wake_mutex = NULL;

int NextArrival::operator <(const NextArrival &na) const
{
	return _next_t < na._next_t;
}


int NextArrival::operator ==(const NextArrival &na) const
{
	//printf("WTF3 call eq\n");
	return ( _arc_id == na._arc_id );
}


StreamThread::StreamThread(int arc_id, double interarrival,
int num_messages, int train_size,
pthread_mutex_t *tuple_count_mutex,
StreamThreadThrottle & throttle,
PtMutex & mtxInputPortInfoMap,
map<int, InputPortBoxInfo> & inputPortInfoMap ):
_throttle(throttle),
_mtxInputPortInfoMap( mtxInputPortInfoMap ),
_inputPortInfoMap( inputPortInfoMap )
{
	fd = NULL;

	//_arc_id = arc_id;
	_interarrival = interarrival * train_size;	  //this should prob. be done here
	_num_messages = num_messages;
	_train_size = train_size;
	_tuple_count_mutex = tuple_count_mutex;
	setStatus( 1, 0, 0 );
	_sched_wake_interval = 1;

	for (map<int, InputPortBoxInfo>::iterator pos = _inputPortInfoMap.begin();
		pos != _inputPortInfoMap.end(); ++pos)
	_queueIdToInputPortMap[ pos->first ] = pos->second._arcId;
	_num_tuples_to_generate = -1;

	_wait_mutex = new pthread_mutex_t;
	_wait_cond = new pthread_cond_t;
	pthread_mutex_init(_wait_mutex,NULL);
	pthread_cond_init(_wait_cond,NULL);
};

StreamThread::StreamThread(int input_count,
int max_arc_id,
pthread_mutex_t *tuple_count_mutex,
StreamThreadThrottle & throttle,
PtMutex & mtxInputPortInfoMap,
map<int, InputPortBoxInfo> & inputPortInfoMap ):
_throttle(throttle),
_mtxInputPortInfoMap( mtxInputPortInfoMap ),
_inputPortInfoMap( inputPortInfoMap )
{
	fd = NULL;
	_input_count = input_count;

	_interarrivals    = ( double* )malloc( sizeof( double ) * max_arc_id + 1);
	_destinations     = ( int* )malloc( sizeof( int ) * (max_arc_id + 1));
	_arc_ids          = ( int* )malloc( sizeof( int ) * (max_arc_id + 1));
	_train_sizes      = ( int* )malloc( sizeof( int ) * (max_arc_id + 1));
	_num_messages_arr = ( int* )malloc( sizeof( int ) * (max_arc_id + 1));
	_accumulator      = ( int* )malloc( sizeof( int ) * (max_arc_id + 1));
	bzero( _accumulator, sizeof( int ) * (max_arc_id+1) );

	_tuple_count_mutex = tuple_count_mutex;
	setStatus( 1, 0, 0 );
	_sched_wake_interval = 1;

	for (map<int, InputPortBoxInfo>::iterator pos = _inputPortInfoMap.begin();
		pos != _inputPortInfoMap.end(); ++pos)
	_queueIdToInputPortMap[ pos->first ] = pos->second._arcId;
	_num_tuples_to_generate = -1;

	_wait_mutex = new pthread_mutex_t;
	_wait_cond = new pthread_cond_t;
	pthread_mutex_init(_wait_mutex,NULL);
	pthread_cond_init(_wait_cond,NULL);
}


// This determines the current mode of work.
// send_to_out -- send the data to the input ports
// write_to_file -- write the data to file
// read_from_file -- read data from a data file.
void StreamThread::setStatus( int send_to_out, int write_to_file,
int read_from_file )
{
	status = 0;
	if ( send_to_out ) status = status | SEND_TO_OUT;
	if ( write_to_file ) status = status | WRITE_TO_FILE;
	if ( read_from_file ) status = status | READ_FROM_FILE;
}


StreamThread::~StreamThread()
{
};

void StreamThread::addInput( int arc_id, double interarrival,int num_messages,
int train_size )
{
	
	//pair<set<NextArrival>::iterator,bool> res;
	// here's the place to add the inputs!
	// what do I do with num_messages??
	//printf("StreamThread: adding input %d with inter %f and counter %d and most importantly train szie %d\n", arc_id, interarrival, _input_counter, train_size );

	_arc_ids[ _input_counter ] = arc_id;
	// do I even need those?? NextArrival structures can keep track...?
	_interarrivals[ _input_counter ] = interarrival;
	_num_messages_arr[ _input_counter ] = num_messages;
	_train_sizes[ _input_counter ] = train_size;

	//printf("StreamThread innited arrays\n");

	NextArrival *na = new NextArrival;

	na->_interarrival = interarrival;
	na->_next_t = 0.0;
	na->_arc_id = arc_id;
	na->_num_messages_remain = num_messages;	  // even this is in na structure?
	// yes, I think so. it's not the same as total count, this one decreases!
	na->_seq_id = _input_counter;

	//printf("StreamThread inited NextArrival object\n");
	//printf(" WTF3 before %d insert arc %d\n", allInputs.size(), na->_arc_id );
	allInputs.insert( *na );
	//printf(" WTF3 after  %d\n", allInputs.size() );

	_input_counter++;

	//printf(" StreamThread: finished adding input %d\n", arc_id );
}


void StreamThread::setRate(double r)
{
	_interarrival = r;
}


double StreamThread::getRate(double r)
{
	return _interarrival;
}


double StreamThread::getRateMultiplier()
{
	double rm;
	pthread_mutex_lock(_tuple_count_mutex);
	rm =  _rate_multiplier;
	pthread_mutex_unlock(_tuple_count_mutex);
	return rm;
}


void StreamThread::setRateMultiplier(double rm)
{
	perror("StreamThread: rate multiplier is currently disabled due to reconstructions\n");
	exit( 1 );

	pthread_mutex_lock(_tuple_count_mutex);
	_rate_multiplier = rm;
	pthread_mutex_unlock(_tuple_count_mutex);
}


int StreamThread::getTuplesGenerated()
{
	int tg;
	pthread_mutex_lock(_tuple_count_mutex);
	tg =  _tuples_generated;
	pthread_mutex_unlock(_tuple_count_mutex);
	return tg;
}


void StreamThread::incrementTuplesGenerated(int incr)
{
	pthread_mutex_lock(_tuple_count_mutex);
	_tuples_generated += incr;
	pthread_mutex_unlock(_tuple_count_mutex);
}

		class PersonInfo
		{
				public:
				PersonInfo(int t, string n, int l) {
							unsigned int seed = time( NULL );
							_tag_id = t; 
							_name = n; 
							_current_location = l;
							_time_in_room = rand_r(&seed) % 5;}
				int _tag_id;
				string _name;
				int _current_location;
				int _time_in_room;
		};
		class SensorInfo
		{
				public:
				SensorInfo(int s, int l, int t) {_sensor_id = s; _sensor_location = l; _current_temperature = t;}
				int _sensor_id;
				int _sensor_location;
				int _current_temperature;
		};

void *StreamThread::run()
{
	unsigned int seed = time( NULL );			  //_arc_id;
		// map of names to IDs
		map<int,PersonInfo*> tag_id_to_name;
		tag_id_to_name[0]  = new PersonInfo(0,"Stan",rand_r(&seed) % 5);
		//tag_id_to_name[1]  = new PersonInfo(1,"Ugur",rand_r(&seed) % 5);
		//tag_id_to_name[2]  = new PersonInfo(2,"Mitch",rand_r(&seed) % 5);
		//tag_id_to_name[3]  = new PersonInfo(3,"Mike",rand_r(&seed) % 5);
		//tag_id_to_name[4]  = new PersonInfo(4,"Don",rand_r(&seed) % 5);
		//tag_id_to_name[5]  = new PersonInfo(5,"Nesime",rand_r(&seed) % 5);
		//tag_id_to_name[6]  = new PersonInfo(6,"Ying",rand_r(&seed) % 5);
		//tag_id_to_name[7]  = new PersonInfo(7,"Olga",rand_r(&seed) % 5);
		//tag_id_to_name[8]  = new PersonInfo(8,"Eddie",rand_r(&seed) % 5);
		//tag_id_to_name[9]  = new PersonInfo(9,"Anurag",rand_r(&seed) % 5);
		//tag_id_to_name[10] = new PersonInfo(10,"Magda",rand_r(&seed) % 5);
		//tag_id_to_name[11] = new PersonInfo(11,"Rich",rand_r(&seed) % 5);
		//tag_id_to_name[12] = new PersonInfo(12,"Bret",rand_r(&seed) % 5);

		// map of rooms to sensor IDs
		map<int,SensorInfo*> sensor_id_to_room;
		sensor_id_to_room[0] = new SensorInfo(0, 510, 65);
		sensor_id_to_room[1] = new SensorInfo(1, 529, 65);



	int temp;									  //n
	//int _queue;
	float r;
	timeval *t = new timeval;
	time_t start_t;
	//char buff[MAXBUFF + 1];
	TupleDescription *tuple_descr;
	int num_fields, field_offset;				  //, c=0;
	double last_time = 0.0, inter_time;
	NextArrival na;


	if ( (status & ( WRITE_TO_FILE | READ_FROM_FILE )) > 0 )
		;										  //sprintf( queueFilename, "input.%05d", _arc_id );

	if ( (status & WRITE_TO_FILE) > 0 )
		fd = fopen( queueFilename, "w+b" );

	if ( (status & READ_FROM_FILE) > 0 )
		fd = fopen( queueFilename, "rb" );

	if ( fd == NULL && ( (status&( WRITE_TO_FILE | READ_FROM_FILE)) > 0 ) )
	{
		;										  //printf("StreamThread: Arc %d input I/O Failure\n", _arc_id );
		exit( 1 );
	}

	// Wait until either the user wants us to generate load, or wants us to
	// shut down.
	// The purpose of this barrier is to make sure that all of the other
	// Scheduler objects have been initialized before we proceed.
	_throttle.awaitValueNotEquals(THROTTLE_STOPPED);

	int total_tuple_count = 0;
	timeval chunk_start;
	gettimeofday(&chunk_start,NULL);

	bool done = false;
	struct itimerval first;
	init_etime(&first);
	double secs;
	secs = get_etime(&first);

	SMInterface SM;
	SM.setITimerStart(&first);
	while ( (allInputs.size() > 0) && (! done))
	{
		// Wait until either the user wants us to generate load, or wants us to
		// shut down...
		StreamThreadThrottleState tState =
			_throttle.getValueWhenNotEquals(THROTTLE_STOPPED);

		if (tState == THROTTLE_DONE)
		{
			done = true;
			continue;
		}
		assert(tState == THROTTLE_RUNNING);

		if ( _num_tuples_to_generate > 0 )
		{
				if ( total_tuple_count >= _num_tuples_to_generate )
				{
						_measure->testStopCond();
						//printf("STREAMTHREAD FINISHED total_tuple_count: %i\n",total_tuple_count);
						done = true;
						continue;
				}
		}

		set<NextArrival>::iterator iter;
		for ( iter = allInputs.begin(); iter != allInputs.end(); iter++ )
		{
			//printf("WTF3 arc: %d nextt: %f inter: %f\n",(*iter)._arc_id, (*iter)._next_t, (*iter)._interarrival );
			;
		}

		// note that access to na is not mutexed, because there is only
		// one stream thread and na object is local to its run method
		na = *( allInputs.begin() );

		// take out the next arrival object.
		// REALLY has to be done like that, since allInputs is a multiset!
		//printf("WTF3 size now was %d\n", allInputs.size() );
		allInputs.erase( allInputs.begin() );
		//printf("WTF3 size now is %d\n", allInputs.size() );
		//printf("StreamThread(?) for arc: %i    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ %d\n", na._arc_id, _train_sizes[ na._seq_id ] );
		//_arc_id,_train_size);

		//SMInterface SM;
		//SMInterface SM;
		//SM.setITimerStart(&first);

		char* ptr;

		(void) time (&start_t);
		Arcs *arc = _catalog->getArc( na._arc_id );

		tuple_descr=arc->getTupleDescr();

		num_fields = tuple_descr->getNumOfFields();

		// tell the queue monitor (if in use) how many messages there will be
		if (_queue_monitor != NULL)
		{
			_queue_monitor->setNumMessages( _num_messages_arr[ na._seq_id ] );
			//_num_messages);
		}
		
		// printf("----------------------[StreamThread]: _interarrival: %f and as of now remain %d\n", na._interarrival, na._num_messages_remain);

		//	for(int i=0; i<(_num_messages/_train_size); i++)
		if ( na._num_messages_remain > 0 || ( na._num_messages_remain == -1 ) )
		{
			// If the queue monitor is in use, check if we are paused
			if (_queue_monitor != NULL)
			{
				while (!_queue_monitor->isRunning()) sleep(1);
			}
			for ( int tuple = 0; tuple < _train_sizes[ na._seq_id ]; tuple++ )
			{
				//printf("[StreamThread] Tuple %i/%i, train sz=%d tuple is %d\n", tuple, _train_sizes[ na._seq_id ], _train_sizes[ na._seq_id ], tuple );
				// Remember: tuples now implicitly include a special timestamp and streamid field

				if ( (status & SEND_TO_OUT) > 0 )
				{
					ptr = SM.enqueuePin( na._arc_id, 1 );//_train_sizes[na._seq_id] );
					//cout << " Called enqueuePin on " << na._arc_id << endl;
				}
				// why did I even have train size here? It's IN the train loop.
				else
					// the sizeof(int) comes from stream id (hard coded now!)
					{

					ptr = ( char * ) malloc( _train_size * (tuple_descr->getSize() +
						TUPLE_DATA_OFFSET + sizeof(int)));
					}

				assert(ptr != NULL);

				// get the current time stamp
				//(void) time (&t);

				// this is where we would read input from file!.
				// else do it the usual, oldfashioned way :)
				// New! More precision, seconds and microseconds
				if ( (status & READ_FROM_FILE) > 0 )
				{
					temp = fread( t, TUPLE_STREAMID_OFFSET, 1, fd );
					if ( temp <= 0 )
					{
												  //_arc_id );
						printf( "ST: Read failed miserably %d\n", -1 );
						exit( -1 );
					}
					//printf(" ST: READ: %d\n", t->tv_sec );
				}
				else
				if ( (status & SEND_TO_OUT) > 0 )
					if (gettimeofday(t, NULL) != 0)
				{
					perror("StreamThread: gettimeofday failed, setting timestamp for new tuple");
					exit(-1);
				}
				// do a little adjustment. accumulator array carries the
				// diffs that are too small to sleep on. Timestamp is adjusted
				// to make sure that timestamps compensate for non-slept time
				//else  (didn't work )
				//t->tv_usec += _accumulator[ na._arc_id ];

				//printf("YYY:StreamThreadArc%d: t1: %i  t2: %i\n", na._arc_id, t->tv_sec,t->tv_usec);

				if ( (status & WRITE_TO_FILE) > 0 )
				{
					if ( fwrite( t, TUPLE_STREAMID_OFFSET, 1, fd ) <= 0 )
						//printf(" SMI: WROTE: %ld\n", t->tv_sec );
						//else
					{
						printf("SMI: I/O write failure\n");
						exit( 1 );
					}
				}
				// and now place it in the tuple. NOTE the hard coded
				// assumption that the tuple has enough space in its timestamp
				// field for whatever space this variable 't' takes up
				memcpy(ptr, t, sizeof( timeval ));

				// Now, the streamid. For now, totally hard coded to be "1"
				int stream_id = 1;
				*((int*)(ptr+TUPLE_STREAMID_OFFSET)) = stream_id;

				//printf("%dSMI: Generating (or reading) a tuple [{ts, streamid}, %d fields] : {%d, %d} \n", na._arc_id, num_fields, t, stream_id);
				// To allow printing it via tupleGenerator
				string tupleFormat = "ti";		  // Start with the timestamps and streamids

				// random delay right now
				//sleep(rand_r(&seed)%8);
				for (int f=0; f<num_fields; f++)
				{
					field_offset = TUPLE_DATA_OFFSET + tuple_descr->getFieldOffset(f);
					// here we read the file... and go to next field.
					if ( (status & READ_FROM_FILE) > 0 )
					{
						fread( ptr+field_offset, tuple_descr->getFieldSize( f ), 1, fd );
						if ( temp <= 0 )
						{
							printf("ST: Read failed miserably %d\n",na._arc_id);
							exit( -1 );
						}
						//printf( "SMI: READ from file %d\n", *(ptr+field_offset ) );
						continue;				  // skip to end of filed generation
					}
												  // generate random tuple from 1 to 9
					//r = rand_r(&seed) % 100 + 1 + 0.01;
					r = rand_r(&seed) % 5 + 1 + 0.01;
					static int current_tag_id = 0;
					static int current_sensor_id = 0;
					switch(tuple_descr->getFieldType(f))
					{
						case INT_TYPE:
			/*
			  if (getenv("RUNNING_LINEAR_ROAD") != NULL) {
						    // LINEAR ROAD HACK HERE
						    if (f == 0) {
						      *((int*)(ptr+field_offset)) = 1; // TYPE
						    } else if (f == 1) {
						      int howmanycars = 1000;
						      char* lr_cars = getenv("LR_CARS");
						      if (lr_cars != NULL) howmanycars = atoi(lr_cars);
						      
						      *((int*)(ptr+field_offset)) = rand_r(&seed)%howmanycars; // CAR ID
						    } else if (f == 2) {
						      *((int*)(ptr+field_offset)) = t->tv_sec; // TIME
						    } else if (f == 3) {
						      *((int*)(ptr+field_offset)) = 1; // EXPRESSWAY (just 1 for now)
						    } else if (f == 4) {
						      *((int*)(ptr+field_offset)) = 1+rand_r(&seed)%3; // SEGMENT NUMBER (1 to 100)
						    } else if (f == 5) {
						      *((int*)(ptr+field_offset)) = rand_r(&seed)%2; // DIRECTION (0 or 1)
						    } else if (f == 6) {
						      *((int*)(ptr+field_offset)) = rand_r(&seed)%4; // LANE (0,1,2 or 3)
						    } else if (f == 7) {
						      *((int*)(ptr+field_offset)) = 40+rand_r(&seed)%56; // SPEED (from 40 to 95)
						    } else {
						      cerr << "There is no field 8 in Linear Road." << endl;
						    }
						  } else {
						    *((int*)(ptr+field_offset)) = (int)r;
						    tupleFormat += "i";
						    //printf(", %d",  (int)r);
						  }
						  break;

											
							if ( tuple_descr->getFieldName(f) == "Timestamp")
							{
								timeval tv;
								gettimeofday(&tv,NULL);
								r = tv.tv_sec;
							}
							if ( tuple_descr->getFieldName(f) == "TagId")
							{
								//r = rand_r(&seed) % tag_id_to_name.size();
								current_tag_id = (current_tag_id+1) % tag_id_to_name.size();
								r = current_tag_id;

							}
							if ( tuple_descr->getFieldName(f) == "SensorId")
							{
								current_sensor_id = (current_sensor_id + 1) % sensor_id_to_room.size();
								r = current_sensor_id;
							}
							if ( tuple_descr->getFieldName(f) == "SensorLocation")
							{
								r = sensor_id_to_room[current_sensor_id]->_sensor_location;
							}
							if ( tuple_descr->getFieldName(f) == "Location")
							{
								if ( tag_id_to_name[current_tag_id]->_time_in_room >= 5 )
								{
									r = sensor_id_to_room[rand_r(&seed)%sensor_id_to_room.size()]->_sensor_location;
									tag_id_to_name[current_tag_id]->_time_in_room = 0;
									tag_id_to_name[current_tag_id]->_current_location = r;
								}
								else
								{
									r = tag_id_to_name[current_tag_id]->_current_location;
									tag_id_to_name[current_tag_id]->_time_in_room++;
								}
								r = 529;
							}
							if ( tuple_descr->getFieldName(f) == "SensorTemperature")
							{
								r = sensor_id_to_room[current_sensor_id]->_current_temperature;
								if ( current_sensor_id != 0 )
								{
									static int num_temperature_reports = 0;
									if (num_temperature_reports > 5)
									{
										sensor_id_to_room[current_sensor_id]->_current_temperature += 100;
										r = sensor_id_to_room[current_sensor_id]->_current_temperature;
									}
									num_temperature_reports++;
								}
							}
							
							//if ( f = 0 )
								//r = 0;
							//if ( f == 1 )
								//r=529;
							

							*((int*)(ptr+field_offset)) = (int)r;
							tupleFormat += "i";
							//printf(", %d",  (int)r);
							break;
 */
						case FLOAT_TYPE:
							*((float*)(ptr+field_offset)) = (float)r;
							tupleFormat += "f";
							//printf(", %f",  (float)r);
							break;
						case DOUBLE_TYPE:
							*((double*)(ptr+field_offset)) = (double)r;
							tupleFormat += "f";
							//printf(", %f",  (double)r);
							break;
						case STRING_TYPE:
						{
							if (tuple_descr->getFieldName(f) == "Name")
							{
								int chars = tuple_descr->getFieldSize(f);
								int char_offset = 0;
								for ( int i = 0; i < tag_id_to_name[current_tag_id]->_name.size(); i++)
								{
									*(ptr + field_offset + char_offset) = tag_id_to_name[current_tag_id]->_name[i];
									char_offset++;
									tupleFormat += "c";
								}
								for (int si = tag_id_to_name[current_tag_id]->_name.size(); si < chars; si++)
								{
									char c = ' ';
									*(ptr + field_offset + char_offset) = c;
									char_offset++;
									tupleFormat += "c";
								}
							}
							else
							{
								int chars = tuple_descr->getFieldSize(f);
								int char_offset = 0;
								//printf(", ");
								for (int si = 0; si < chars; si++)
								{
									// This generates a char between a and d (d = ascii code 97)
									char c = (char) (rand() % 4) + 97;
									*(ptr + field_offset + char_offset) = c;
									//printf("%c",c);
									char_offset++;
									tupleFormat += "c";
								}
							}
						}
						break;
						case TIMESTAMP_TYPE:	  // a timeval
							timeval *ts = new timeval;
							gettimeofday(ts,NULL);
							memcpy(ptr+field_offset, t, sizeof( timeval ));
							tupleFormat += "t";
							break;
					}
					if ( (status & WRITE_TO_FILE) > 0 )
					{
						fwrite( ptr+field_offset, tuple_descr->getFieldSize( f ), 1, fd );
						//printf( "SMI: WROTE to file %d\n", *(ptr+field_offset ) );
					}
				}
				//printTuple(ptr,tupleFormat.c_str());
				incrementTuplesGenerated(1);

				if ( na._num_messages_remain > 0 )
					na._num_messages_remain--;

				total_tuple_count++;
				//if ( (total_tuple_count % _sched_wake_interval) == 0 )
				//{
				//}

				int step = 100;
				//int step = 1000;
				if ( (total_tuple_count % step) == 0 )
				{
					static double last_tuples_per_sec = 0;
					//if ( last_tuples_per_sec > 10000 )
						//step = 100000;
					//else if ( last_tuples_per_sec > 1000 )
						//step = 10000;
					if ( (total_tuple_count % step) == 0 )
					{
						timeval chunk_stop;
						gettimeofday(&chunk_stop,NULL);
						double total_time = (chunk_stop.tv_sec + (chunk_stop.tv_usec*1e-6)) -
							(chunk_start.tv_sec + (chunk_start.tv_usec*1e-6));
						last_tuples_per_sec = total_tuple_count/total_time;
						//if ( total_time > 1.0)
							//fprintf(stderr,"generating %f tuples per second : %f seconds per tuple .. so far: %i tuples generated\n",
								//total_tuple_count/total_time,total_time/total_tuple_count,total_tuple_count);
					}
				}

				if ( (status & SEND_TO_OUT) > 0 )
				{
					//printf(" StreamThread-------------------------------------->ENQUEUE, ST, into queue %d ( III%d )\n", na._arc_id, -1 );
					SM.enqueueUnpin(na._arc_id, ptr, 1);
					//printf(" StreamThread-------------------------------------->ENQUEUE2, ST, into queue %d ( III%d )\n", na._arc_id, -1 );
					// report one message generated

					//printf(" Signal to my box %d\n", _catalog->getArc( na._arc_id )->getDestId() );
					if ( _catalog->inTPBMode() )
						_catalog->TPBSignal( _catalog->getArc( na._arc_id )->getDestId() );
				}
				//printf("StreamThread: queue monitor is about to %f\n", _queue_monitor);
				if (_queue_monitor != NULL)
				{
					_queue_monitor->tickMessage();
				}
				SM.SM_free(ptr);
				if ( total_tuple_count % _sched_wake_interval == 0 )
				{
					//printf("StreamThread:SIGNALING the scheduler tuples sent: %i\n",total_tuple_count);
					pthread_mutex_lock(_sched_wake_mutex);
					pthread_cond_broadcast(_sched_wake_cond);
					pthread_mutex_unlock(_sched_wake_mutex);
				}

				// update the tuples count (for Stats Object)
				{
					//LockHolder lh(_mtxInputPortInfoMap);
					++ (( _inputPortInfoMap.find( _queueIdToInputPortMap[ na._arc_id ] ) )->second)._tuplesEnqSinceLastCheck;
					//++ (pos->second._tuplesEnqSinceLastCheck);
				}

			}

			// this is the sleeping part...
			/* 
						  double rm;
						 if ( i % 5 == 0 )
						{
							rm = getRateMultiplier();
							_interarrival = (_interarrival/rm);//*_train_size;
							}
			*/
			//printf(" StreamThread: about to compute sleeping time\n");
			//printf(" WTF3? ARC=%d, _interarr %f, Train=%d inter_time sleep %f\n", na._arc_id , na._interarrival, _train_sizes[ na._seq_id ], na._next_t - last_time );

			inter_time = na._next_t - last_time;

			struct timeval timeout;
			//timeout.tv_sec = (int)_interarrival;
			timeout.tv_sec = (int)inter_time;
			//timeout.tv_usec = (int)((_interarrival - (int)_interarrival) * 1000000);
			timeout.tv_usec = (int)((inter_time - (int)inter_time) * 1000000);

			// This appears to be a Richard / Eddie specific setting for inter-arrival time,
			// so I'm commenting it out for the general populance. -cjc
			// timeout.tv_sec = 0;
			// timeout.tv_usec = 500000;

 			// Old junk, forget stream thread rate control...
			//if (_queue_monitor != NULL) timeout.tv_usec = _queue_monitor->getusecRate();
			//
			//printf("arc*in %d _inter arrival: %f\n", na._arc_id, inter_time );
			// move the clock to the next arrival time.

			last_time = na._next_t;
			na._next_t += ( na._interarrival * _train_sizes[ na._seq_id ] );

			if ( na._num_messages_remain > 0 || na._num_messages_remain == -1 )
				allInputs.insert( na );

			//int precision_msec = 50000;
			//int precision_msec = 1000;
			int precision_msec = 100000;
			//int precision_msec = 1000000;

			//if ( inter_time > 0.0 )
			//cerr << "StreamThread: go to sleep..." << inter_time << " tim " << timeout.tv_usec << endl;

			if ( (1000000 * timeout.tv_sec + timeout.tv_usec)+_accumulator[ na._arc_id ] > precision_msec )
			{  // sleep soundly and safely. and reduce the accumulated time
				// we assume that precision is A LOT less than a second
				// something like .5 second might break this code. 
				timeout.tv_usec += _accumulator[ na._arc_id ];



			
				/*
				timeval wait_start_time;
				gettimeofday(&wait_start_time,NULL);
				while(1)
				{
					
					timespec ts;
					ts.tv_sec = wait_start_time.tv_sec + timeout.tv_sec;
					ts.tv_nsec = (wait_start_time.tv_usec + timeout.tv_usec) * 1000;
					//printf("StreamThread sleeping until : (%i)(%i)\n",ts.tv_sec,ts.tv_nsec);
					pthread_mutex_lock(_wait_mutex);
					int x = pthread_cond_timedwait(_wait_cond, _wait_mutex, &ts);
					pthread_mutex_unlock(_wait_mutex);
					
					if ( x == ETIMEDOUT )
					{
						printf("StreamThread: got ETIMEDOUT\n");
						break;
					}
					else
					{
						timeval wake_time;
						gettimeofday(&wake_time,NULL);
						//printf("StreamThread:GOT TO NOT A TIMEOUT time now: (%i)(%i)\n",wake_time.tv_sec,wake_time.tv_usec);
						if ( (wake_time.tv_sec + (wake_time.tv_usec*1e-6)) > 
										(ts.tv_sec + (ts.tv_nsec*1e-9)) )
						{
							break;
						}
					}
					
				}
			*/
				



				//printf("StreamThread: sleeping: (%i)(%i) seconds\n",timeout.tv_sec,timeout.tv_usec);
				select(0,0,0,0,&timeout);
				//printf("StreamThread: awake \n",timeout.tv_sec);



				//for ( int z = 0; z < _input_count; z++ )
				//	_accumulator[ z ] -= _accumulator[ na._arc_id ];
				_accumulator[ na._arc_id ] = 0;
			}
			else
				//for ( int z = 0; z < _input_count; z++ )
				_accumulator[ na._arc_id ] += timeout.tv_usec;
		}
		//_measure->incrementTimeSpentInStreamThread(get_etime(&first)-secs);
	}
	//printf("STREAMTHREAD allInputs.size(): %i done: %i\n",allInputs.size(),done);
	//printf("STREAMTHREAD FINISHED ... num_tuples_generated: %i\n",total_tuple_count);
	_measure->setTimeSpentInStreamThread(get_etime(&first)-secs);

	cout << "[StreamThread] is exiting spent " << (get_etime(&first)-secs) <<  endl;
	if ( (status & ( WRITE_TO_FILE )) > 0 ) fclose( fd );
	return (void *)0;
}


void *StreamThread::entryPoint(void *pthis)
{
	StreamThread *pt = (StreamThread*)pthis;
	pt->run();
	return (void *)0;
}


void StreamThread::start()
{
	pthread_create(&_thread, 0, entryPoint, this);
}


void StreamThread::join()
{
	pthread_join(_thread, NULL);
}


void StreamThread::init_etime(struct itimerval *first)
{
	first->it_value.tv_sec = 1000000;
	first->it_value.tv_usec = 0;
	//setitimer(ITIMER_PROF,first,NULL);
	setitimer(ITIMER_VIRTUAL,first,NULL);
}


double StreamThread::get_etime(struct itimerval *first)
{
	struct itimerval curr;
	//getitimer(ITIMER_PROF,&curr);
	getitimer(ITIMER_VIRTUAL,&curr);
	return (double)(
		(first->it_value.tv_sec + (first->it_value.tv_usec*1e-6)) -
		(curr.it_value.tv_sec + (curr.it_value.tv_usec*1e-6)));

	//return (double)(
	//(first->it_value.tv_sec - curr.it_value.tv_sec) +
	//(first->it_value.tv_usec - curr.it_value.tv_usec)*1e-6);
}
