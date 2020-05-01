
#include "Measurement.H"
#include "global.H"
#include <math.h>

Measurement::Measurement(AppTupleMonitor * atmon)
{
	_atmon = atmon;
	_stop_type = -1; // no stop type is the default
	_mutex = new pthread_mutex_t;
	pthread_mutex_init(_mutex,0);
	_out_filename = new char[FILENAME_MAX];
	sprintf(_out_filename,"default.out");
	_num_box_calls = 0;
	_time_spent_in_worker_threads = 0.0;
	_to_memory_latency = 0.0;
	_to_memory_tuples_written = 0;
	_start_time = INT_MAX-10000000;
	_total_box_tuples_consumed = 0;
	_num_mallocs = 0;
	_time_loading_queues = 0.0;
	_time_unloading_queues = 0.0;
	_time_spent_in_stream_thread = 0.0;
	_time_spent_in_doBox = 0.0;
	_time_spent_executing_boxes = 0.0;
	_box_overhead = 0.0;
	_num_schedulable_boxes_total = 0;
	_num_schedulable_boxes_counter = 0;

	_out_step = 0;

	_disk_io = 0;
	_sm_reads = 0;
	_sm_writes = 0;
	_atmon = atmon;
	_stream_thread = NULL;
}
Measurement::~Measurement()
{
}
void Measurement::setNumApps(int num_apps)
{
	_num_apps = num_apps;
	_app_qos = new double[_num_apps];
	_app_qos_tuples_passed = new int[_num_apps];
	for ( int i = 0; i < _num_apps; i++ )
	{
		_app_qos[i] = 0.0;
		_app_qos_tuples_passed[i] = 0;
	}

}
void Measurement::setStopType(char* stop_type)
{
	if ( strcmp(stop_type,"TUPLES_WRITTEN") == 0 )
		_stop_type = TUPLES_WRITTEN;
	else if ( strcmp(stop_type,"TUPLES_WRITTEN_TO_MEMORY") == 0 )
		_stop_type = TUPLES_WRITTEN_TO_MEMORY;
	else if ( strcmp(stop_type,"TIME_STOP") == 0 )
		_stop_type = TIME_STOP;
	else if ( strcmp(stop_type,"TUPLES_GENERATED_STOP") == 0 )
		_stop_type = TUPLES_GENERATED_STOP;
	else if ( strcmp(stop_type,"IGNORE") == 0 ) 
	  _stop_type = STOP_IGNORE;
	else
	{
	  cout << "ERROR:[Measurementa] Bad Stop Type: " << stop_type << endl;
	  abort();
	}
}
int Measurement::getStopType()
{
	return(_stop_type);
}
void Measurement::testStopCond()
{
  // Fast return if you don't care for this 'feature'
  if (_stop_type == STOP_IGNORE) return;

	bool stop = false;
	pthread_mutex_lock(_mutex);
	switch(_stop_type)
	{
		case TUPLES_WRITTEN:
			if ( _total_tuples_written >= _sc._num_tuples_written )
				stop = true;
			break;
		case TUPLES_WRITTEN_TO_MEMORY:
			//printf("Stop: _to_memory_tuples_written: %i   _sc._num_tuples_written_to_memory: %i\n",_to_memory_tuples_written,_sc._num_tuples_written_to_memory);
			if ( _to_memory_tuples_written >= _sc._num_tuples_written_to_memory )
				stop = true;
			//printf("_to_memory_tuples_written: %i\n",_to_memory_tuples_written);
			break;
		case TIME_STOP:
			timeval t;
			gettimeofday(&t,NULL);
			if ( t.tv_sec > _start_time + _sc._experiment_time )
			{
				stop = true;
				//printf("GOT TO UUU _start_time: %i   experiment_time: %i  t.tv_sec: %ld \n",_start_time, _sc._experiment_time,t.tv_sec);
			}
			break;
		case TUPLES_GENERATED_STOP:
			if ( _stream_thread->getTuplesGenerated() >= _sc._num_tuples_generated )
			{
				if ( testStreamThreadStopped() )
				{
					// if there are no runnable boxes, then we can shut down
					if ( testRunnableBoxes() == false )
						stop = true;
				}
			}
			break;
		default:
		  cout << "ERROR:[Measurementa] Bad Stop Type: " << _stop_type << endl;
		  abort();
		  break;
	}
	pthread_mutex_unlock(_mutex);
	if ( stop == true )
	{
		//printf("GOT TO global .. setStop\n");
		__global_stop.setStop();

		// Let the Aurora API know that we'd like to wrap things up now...
		LockHolder lh(_atmon->_mtx);
		_atmon->_value._schedWantsShutdown = true;
		_atmon->_cond.broadcast();
	}

}
bool Measurement::testRunnableBoxes()
{
	int size = 0;
	pthread_mutex_lock(&__box_work_set_mutex);
		size = __box_work_set.size();
	pthread_mutex_unlock(&__box_work_set_mutex);

	//printf("Measurement::testRunnableBoxes(): size: %i app_count: %i\n",size,_app_count);
	if (size > 0)
			return true;
	else
			return false;
}
void Measurement::setStartTime()
{
	timeval t;
	gettimeofday(&t, NULL);
	_start_time = t.tv_sec;
}
stop_cond* Measurement::getStopCond()
{
	return(&_sc);
}
void Measurement::setStopCondTuplesWritten(int tuples_written)
{
	_sc._num_tuples_written = tuples_written;
}
void Measurement::setStopCondTuplesWrittenToMemory(int tuples_written)
{
	_sc._num_tuples_written_to_memory = tuples_written;
}
void Measurement::setStopCondTimeStop(int experiment_time)
{
	_sc._experiment_time = experiment_time;
}
void Measurement::setStopCondTuplesGenerated(int num_generated)
{
	_sc._num_tuples_generated = num_generated;
}

void Measurement::addMemRemainingVector(vector<MemRemaining> *mr)
{
	pthread_mutex_lock(_mutex);
	_mem_remaining_v.push_back(mr);
	pthread_mutex_unlock(_mutex);
}
void Measurement::incrementToMemoryLatency(double incr)
{
	pthread_mutex_lock(_mutex);
	_to_memory_latency += incr;
	pthread_mutex_unlock(_mutex);
}
void Measurement::incrementTotalBoxTuplesConsumed(int incr)
{
	pthread_mutex_lock(_mutex);
	_total_box_tuples_consumed += incr;
	pthread_mutex_unlock(_mutex);
}
void Measurement::incrementToMemoryTuplesWritten(int incr)
{
	pthread_mutex_lock(_mutex);
	_to_memory_tuples_written += incr;
	
	if ( _to_memory_tuples_written % 1000 == 0 )
		cout << " Written to memory so far " << _to_memory_tuples_written << endl;
	if ( (_to_memory_tuples_written - 1000) > _out_step)
	{
	   _out_step = _to_memory_tuples_written;

	   //fprintf(stderr,"_to_memory_tuples_written: %i  \n",_to_memory_tuples_written);
	   //fprintf(stderr,"_to_memory_tuples_written: %i  latency: %f\n",_to_memory_tuples_written,getToMemoryLatency());
	}
	
	pthread_mutex_unlock(_mutex);
}
void Measurement::incrementBoxOverhead(double incr)
{
	pthread_mutex_lock(_mutex);
	_box_overhead += incr;
	pthread_mutex_unlock(_mutex);
}

void Measurement::incrementSMReads()
{
	pthread_mutex_lock(_mutex);
	_sm_reads++;
	_disk_io++;
	pthread_mutex_unlock(_mutex);
}

int Measurement::getSMReads()
{
        int reads;
	pthread_mutex_lock(_mutex);
	reads = _sm_reads;
	pthread_mutex_unlock(_mutex);
	
	return reads;
}

int Measurement::getIOOperations()
{
	int io;
	pthread_mutex_lock(_mutex);
	io = _disk_io;
	pthread_mutex_unlock(_mutex);
	
	return io;
}

int Measurement::getSMWrites()
{
        int writes;
	pthread_mutex_lock(_mutex);
	writes = _sm_writes;
	pthread_mutex_unlock(_mutex);
	
	return writes;
}

void Measurement::incrementSMWrites()
{
	pthread_mutex_lock(_mutex);
	_sm_writes++;
	_disk_io++;
	pthread_mutex_unlock(_mutex);
}

double Measurement::getToMemoryLatency()
{
	double ret_val;
	pthread_mutex_lock(_mutex);
	ret_val = _to_memory_latency/(double)_to_memory_tuples_written;
	//printf("Measurement::getToMemoryLatency(): _to_memory_latency: %f  _to_memory_tuples_written: %i  ret_val: %f\n",_to_memory_latency, _to_memory_tuples_written, ret_val);
	pthread_mutex_unlock(_mutex);
	return ret_val;
}

void Measurement::incrementTuplesWritten(int incr)
{
	pthread_mutex_lock(_mutex);
	if ( _total_tuples_written % 100 == 0 )
		cout << " Written so far " << _total_tuples_written << endl;
	_total_tuples_written += incr;
	pthread_mutex_unlock(_mutex);
}
long Measurement::getTuplesWritten()
{
	long ret_val;
	pthread_mutex_lock(_mutex);
	ret_val = _total_tuples_written;
	pthread_mutex_unlock(_mutex);
	return ret_val;
}
void Measurement::incrementNumBoxCalls(int incr)
{
	pthread_mutex_lock(_mutex);
	_num_box_calls += incr;
	pthread_mutex_unlock(_mutex);

}
void Measurement::incrementTimeSpentInWorkerThread(double incr)
{
	pthread_mutex_lock(_mutex);
	_time_spent_in_worker_threads += incr;
	//printf("RRR incr: %f   _time_spent_in_worker_threads: %f\n",incr,_time_spent_in_worker_threads);
	pthread_mutex_unlock(_mutex);

}
void Measurement::incrementNumMallocs()
{
	pthread_mutex_lock(_mutex);
	_num_mallocs++;
	pthread_mutex_unlock(_mutex);
}
int Measurement::getNumMallocs()
{
	long ret_val;
	pthread_mutex_lock(_mutex);
	ret_val = _num_mallocs;
	pthread_mutex_unlock(_mutex);
	return ret_val;
}
void Measurement::incrementTimeLoadingQueuesWT(double incr)
{
	pthread_mutex_lock(_mutex);
	_time_loading_queues += incr;
	pthread_mutex_unlock(_mutex);
}
double Measurement::getTimeLoadingQueuesWT()
{
	double ret_val;
	pthread_mutex_lock(_mutex);
	ret_val = _time_loading_queues;
	pthread_mutex_unlock(_mutex);
	return ret_val;
}
void Measurement::incrementTimeUnloadingQueuesWT(double incr)
{
	pthread_mutex_lock(_mutex);
	_time_unloading_queues += incr;
	pthread_mutex_unlock(_mutex);
}
double Measurement::getTimeUnloadingQueuesWT()
{
	double ret_val;
	pthread_mutex_lock(_mutex);
	ret_val = _time_unloading_queues;
	pthread_mutex_unlock(_mutex);
	return ret_val;
}
void Measurement::incrementTimeSpentInStreamThread(double incr)
{
	pthread_mutex_lock(_mutex);
	_time_spent_in_stream_thread += incr;
	pthread_mutex_unlock(_mutex);

}
void Measurement::incrementTimeSpentInDoBox(double incr)
{
	pthread_mutex_lock(_mutex);
	_time_spent_in_doBox += incr;
	pthread_mutex_unlock(_mutex);

}
void Measurement::incrementTimeSpentExecutingBoxes(double incr)
{
	pthread_mutex_lock(_mutex);
	_time_spent_executing_boxes += incr;
	pthread_mutex_unlock(_mutex);

}
double Measurement::getTimeSpentExecutingBoxes()
{
	double ret_val;
	pthread_mutex_lock(_mutex);
	ret_val = _time_spent_executing_boxes;
	pthread_mutex_unlock(_mutex);
	return ret_val;
}

void Measurement::addMeasurementType(int mt)
{
	switch(mt)
	{
		case TIME_SPENT_SCHEDULING:
		case TIME_SPENT_IN_WORKER_THREADS:
		case NUM_SCHEDULING_DECISIONS:
		case NUM_BOX_CALLS:
		case AVERAGE_LATENCY:
		case AVERAGE_QOS:
	    case DISK_IO:
	    case DISK_READS:
	    case DISK_WRITES:
	    case TOTAL_RUN_TIME:
    	case QOS_BOUND:
		case TIME_SCHEDULING_VS_WORKER_THREADS:
		case TIME_TOTAL_SCHED_WORKER_THREADS:
		case AVERAGE_TUPLE_TRAIN_SIZE:
		case NUM_MALLOCS:
		case TIME_LOADING_QUEUES:
		case TIME_UNLOADING_QUEUES:
		case TIME_SPENT_IN_STREAM_THREAD:
		case TIME_SPENT_IN_DO_BOX:
		case TIME_SPENT_EXECUTING_BOXES:
		case GENERAL_PROF_STATS:
		case SCHEDULER_PROF_STATS:
		case WORKERTHREAD_PROF_STATS:
		case BOX_OVERHEAD:
		case AVG_NUM_SCHEDULABLE_BOXES:
			break;
		default:
			printf("ERROR:[Measurement:addMeasurementType]: Bad Measurement Type\n");
			abort();
			break;
	}

	_measurement_types_set.insert( mt);

}
void Measurement::addXVariable(int x)
{
	switch(x)
	{
		case INPUT_RATE:
		case APP_DEPTH:
		case APP_WIDTH:
		case APP_COUNT:
		case BOX_COUNT:
    	case MEMORY_SIZE:
	    case QOS_BOUND:
		case BOX_COST:
		case BURST_SIZE:
		case TRAIN_SIZE:
		case BOX_SELECTIVITY:
		case K_SPANNER_VAL:
		case NUM_BUCKETS:
		case BEQ_SIZE:
			break;
		default:
			printf("ERROR:[Measurement:addXVariable]: Bad XVariable Type\n");
			abort();
			break;
	}

	_x_variable_set.insert( x);

}
void Measurement::outputMeasurements()
{
	cout << " OUTPUTTTING Measurements?? " <<_x_variable_set.size() << endl;
	set<int, less<int> >::iterator iter;

	for ( iter = _x_variable_set.begin();
			iter != _x_variable_set.end();
			iter++ )
	{
		switch(*iter)
		{

			case INPUT_RATE:
				print("input_rate",_input_rate);
				break;
			case APP_DEPTH:
				print("app_depth",_app_depth);
				break;
		    case MEMORY_SIZE:
                print("mem_size", _memory_size );
                break;
		    case QOS_BOUND:
				print("qos_bound", _qos_bound );
				break;
			case APP_WIDTH:
				print("app_width",fabs(_app_width));
				break;
			case APP_COUNT:
				print("app_count", (double)(_app_count) );
				break;
			case BOX_COUNT:
				print("box_count", (double)(_box_count) );
				break;
			case BOX_COST:
				print("box_cost",_box_cost);
				break;
			case BURST_SIZE:
				print("burst_size",_burst_size);
				break;
			case TRAIN_SIZE:
				print("train_size",_train_size);
				break;
			case BOX_SELECTIVITY:
				printf("ERROR:[Measurement::outputMeasurements]: Measurement Type Unavailable\n");
				abort();
				break;
			case K_SPANNER_VAL:
				print("k_spanner_val",_k_spanner_val);
				break;
			case NUM_BUCKETS:
				print("num_buckets",_num_buckets);
				break;
			case BEQ_SIZE:
				print("beq_size",_beq_size);
				break;
			default:
				printf("ERROR:[Measurement::outputMeasurements]: Bad Measurement Type\n");
				abort();
				break;
		}

	}
}

void Measurement::printPoint(const char *x_string, double x_val, double y_val )
{
	FILE *fp;

	char filename[FILENAME_MAX];

	sprintf( filename, "%s.%s.mem_remaining.dat", _out_filename, x_string );
	fp = fopen( filename, "a" );
	fprintf( fp, "%f %f\n", x_val, y_val );
	fclose( fp );
}

void Measurement::print(const char *x_string,double x_val)
{
	FILE *fp;

	set<int, less<int> >::iterator iter;

	char filename[FILENAME_MAX];

	for ( iter = _measurement_types_set.begin();
			iter != _measurement_types_set.end();
			iter++ )
	{
		switch(*iter)
		{
	    	case DISK_IO:
				sprintf(filename, "%s.%s.disk_io_operations.dat", _out_filename, x_string);
				fp = fopen( filename, "a" );
				fprintf(fp, "%f %d\n", x_val, _disk_io );
				fclose(fp);
				break;
	    	case DISK_READS:
				sprintf(filename, "%s.%s.disk_reads_operations.dat", _out_filename, x_string);
				fp = fopen( filename, "a" );
				fprintf(fp, "%f %d\n", x_val, _sm_reads );
				fclose(fp);
				break;
	    	case DISK_WRITES:
				sprintf(filename, "%s.%s.disk_writes_operations.dat", _out_filename, x_string);
				fp = fopen( filename, "a" );
				fprintf(fp, "%f %d\n", x_val, _sm_writes );
				fclose(fp);
				break;
	    	case TOTAL_RUN_TIME:
				sprintf(filename, "%s.%s.total_run_time.dat", 
						_out_filename, x_string);
				fp = fopen( filename, "a" );
				fprintf(fp, "%f %f\n", x_val, _total_running_time );
				fclose(fp);
				break;
			case TIME_TOTAL_SCHED_WORKER_THREADS:
				sprintf(filename,"%s.%s.time_total_sched_worker_threads.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_time_spent_scheduling+_time_spent_in_worker_threads);
				fclose(fp);
				break;
			case TIME_SCHEDULING_VS_WORKER_THREADS:
				sprintf(filename,"%s.%s.time_sched_vs_worker_threads.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_time_spent_scheduling/_time_spent_in_worker_threads);
				fclose(fp);
				break;
			case TIME_SPENT_SCHEDULING:
				sprintf(filename,"%s.%s.time_spent_scheduling.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_time_spent_scheduling);
				fclose(fp);
				break;
			case TIME_SPENT_IN_WORKER_THREADS:
				sprintf(filename,"%s.%s.time_spent_in_worker_threads.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_time_spent_in_worker_threads);
				fclose(fp);
				break;
			case NUM_SCHEDULING_DECISIONS:
				sprintf(filename,"%s.%s.num_scheduling_decisions.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %i\n",x_val,_num_scheduling_decisions);
				fclose(fp);
				break;
			case NUM_BOX_CALLS:
				sprintf(filename,"%s.%s.num_box_calls.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %i\n",x_val,_num_box_calls);
				fclose(fp);
				break;
			case AVERAGE_LATENCY:
				sprintf(filename,"%s.%s.average_latency.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				//fprintf(fp,"%f %f\n",x_val,_average_latency);
				fprintf(fp,"%f %f\n",x_val,getToMemoryLatency());
				fclose(fp);
				break;
			case AVERAGE_QOS:
				sprintf(filename,"%s.%s.average_qos.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_average_qos);
				/*{
					double total = 0.0;
					for ( int i = 0; i < _num_apps; i++ )
					{
						printf( "qos for app[%i]: %f\n",i,(_app_qos[i]/_app_qos_tuples_passed[i]));
						if ( _app_qos_tuples_passed[i] != 0 )
							total += (_app_qos[i]/_app_qos_tuples_passed[i]);
					}
					fprintf(fp,"%f %f\n",x_val,total/_num_apps);
					}*/
				fclose(fp);
				break;
			case AVERAGE_TUPLE_TRAIN_SIZE:
				sprintf(filename,"%s.%s.average_tuple_train_size.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,(double)_total_box_tuples_consumed/(double)_num_box_calls);
				fclose(fp);
				break;
			case NUM_MALLOCS:
				sprintf(filename,"%s.%s.num_mallocs.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %i\n",x_val,_num_mallocs);
				fclose(fp);
				break;
			case TIME_LOADING_QUEUES:
				sprintf(filename,"%s.%s.time_loading_queues.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_time_loading_queues);
				fclose(fp);
				break;
			case TIME_UNLOADING_QUEUES:
				sprintf(filename,"%s.%s.time_unloading_queues.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_time_unloading_queues);
				fclose(fp);
				break;
			case TIME_SPENT_IN_STREAM_THREAD:
				sprintf(filename,"%s.%s.time_spent_in_stream_thread.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_time_spent_in_stream_thread);
				fclose(fp);
				break;
			case TIME_SPENT_IN_DO_BOX:
				sprintf(filename,"%s.%s.time_spent_in_doBox.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_time_spent_in_doBox);
				fclose(fp);
				break;
			case TIME_SPENT_EXECUTING_BOXES:
				sprintf(filename,"%s.%s.time_spent_executing_boxes.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_time_spent_executing_boxes);
				fclose(fp);
				break;
			case SCHEDULER_PROF_STATS:
			case WORKERTHREAD_PROF_STATS:
				break;
			case GENERAL_PROF_STATS:
				sprintf(filename,"%s.%s.general_prof_stats.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f %f %f\n",x_val,
										getTimeSpentInStreamThread(),
										getTimeSpentInWorkerThread(),
										getTimeSpentScheduling());

				fclose(fp);
				break;
			case BOX_OVERHEAD:
				sprintf(filename,"%s.%s.box_overhead.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,_box_overhead);
				fclose(fp);
				break;
			case AVG_NUM_SCHEDULABLE_BOXES:
				sprintf(filename,"%s.%s.avg_num_schedulable_boxes.dat",_out_filename,x_string);
				fp = fopen(filename,"a");
				fprintf(fp,"%f %f\n",x_val,(double)_num_schedulable_boxes_total/(double)_num_schedulable_boxes_counter);
				fclose(fp);
				break;

			default:
				printf("ERROR:[Measurement::print]: Bad Measurement Type\n");
				abort();
				break;
		}

	}

}
void Measurement::setOutputFilename(char *filename)
{
	sprintf(_out_filename,"%s",filename);
}
void Measurement::setAverageLatency(vector<int> app_v)
{
	double total = 0.0;
	for ( int i = 0; i < app_v.size(); i++)
	{
		total += _qos->get_average_delay(app_v[i]);
		////cout << " Observed latency in " << i << " Is " << _qos->get_average_delay(app_v[i]) << endl;
	}
	_average_latency = total/app_v.size();
	//	cout << endl << " Average latency was found to be  " << _average_latency << endl;
}
void Measurement::setAverageQoS(vector<int> app_v)
{
	double total = 0.0;
	for ( int i = 0; i < app_v.size(); i++)
	{
		total += _qos->get_average_utility(app_v[i]);
		//cout << " OBSERVED utility of " << i << " IS " << _qos->get_average_utility(app_v[i]) << endl;
	}
	_average_qos = total/app_v.size();
	//cout << " OBSERVED utility of " << " IS " <<  _average_qos<< endl;
}
void Measurement::getMemRemainingGraph()
{
	double max_elapsed_time = 0.0;
	for ( int i = 0; i < _mem_remaining_v.size(); i++ )
	{
		//printf(" MMM %d \nMMM", i );
		for ( int j = 0; j < (*_mem_remaining_v[i]).size(); j++ )
		{
			if ( (*_mem_remaining_v[i])[j]._elapsed_time > max_elapsed_time )
				max_elapsed_time = (*_mem_remaining_v[i])[j]._elapsed_time;
			//printf( " %d : %f - %d,  ", j, (*_mem_remaining_v[i])[j]._elapsed_time, (*_mem_remaining_v[i])[j]._mem_remaining );
		}
	}

	max_elapsed_time = 15.0;
	double step = max_elapsed_time / 25.0;  // want to have 10? points in graph
			double old_value = 0.0;

	for ( double d = 0.0; d < max_elapsed_time; d = d+step )
	{
		double avg_val = 0.0;
		int points_counted = 0;

		// for each graph.
		for ( int i = 0; i < _mem_remaining_v.size(); i++ )
		{
			double before = 0.0, after = max_elapsed_time+1.0, fraction = -1.0;
			double mem_remaining_before = 0.0, mem_remaining_after = 0.0;
			// here we want to find the extrapolated point of each vector
			// and average the values. if no point defined, value not averaged

			mem_remaining_after = -1.0;
			// for each vector, find extrapolated point.
			for ( int j = 0; j < (*_mem_remaining_v[i]).size(); j++ )
			{
				if ( (*_mem_remaining_v[i])[j]._elapsed_time <= d )
				{
					before = (*_mem_remaining_v[i])[j]._elapsed_time;
					mem_remaining_before = (*_mem_remaining_v[i])[j]._mem_remaining;
				}
				else
				{
					after = (*_mem_remaining_v[i])[j]._elapsed_time;
					mem_remaining_after = (*_mem_remaining_v[i])[j]._mem_remaining;
					break;  // found the point after, can safely quite the loop
				}
			}

			if ( after > max_elapsed_time ) // point after d was not available.
				; // probably just ignore the missing points?
			else
			{
				// that is the position of d between before and after.
				fraction = ( d - before );///( after - before );

				//printf(" AVG val was %f", avg_val );
				if ( mem_remaining_before == -1.0 )
					avg_val += mem_remaining_before;
				else
					avg_val += ( mem_remaining_before + 
					fraction * ( (mem_remaining_after-mem_remaining_before) / 
								 ( after - before ) ) );  // thats the slope.
				points_counted++;
				
				//printf("      MMM added %f diff %f \n", ( mem_remaining_before + 
				//	fraction * ( (mem_remaining_after-mem_remaining_before) / 
				//				 ( after - before ) ) ),(mem_remaining_before-mem_remaining_after) );
				//printf("MMM: (d %f)   %f between %f and %f, avg val becomes %f\n", d, fraction, before, after, avg_val );
				//printf("MMM: MEM REMAINING %f --> %f\n", mem_remaining_before, mem_remaining_after );
			}
		}

		//printf("MMMM For time %f want to record %f, counted out %d\n", d, avg_val/(1.0*points_counted), points_counted );
		if ( points_counted == 0 )
			printPoint( "time", d, old_value );
		else
		{
			printPoint( "time", d, avg_val/(1.0*points_counted) );
			old_value = points_counted;
		}
	}
}
void Measurement::incrementQosPerTuple(int app, int num_tuples_passed, double qosPerTuple)
{
	pthread_mutex_lock(_mutex);

	_app_qos[app] += (qosPerTuple*num_tuples_passed);
	_app_qos_tuples_passed[app] += num_tuples_passed;

	pthread_mutex_unlock(_mutex);

}
void Measurement::incrementNumSchedulableBoxes(int incr)
{
	_num_schedulable_boxes_total += incr;
	_num_schedulable_boxes_counter++;

}
/*
double Scheduler::findYGivenXAndQos(double x,int qos_graph_num)
{
    double y=0.0;
    for ( int i = 0; i < _qos_graphs[qos_graph_num]._points.size()-1; i++ )
    {
        if ( x >= _qos_graphs[qos_graph_num]._points[i]->_x &&
                x < _qos_graphs[qos_graph_num]._points[i+1]->_x)
        {
                y = findYGivenLineAndX(_qos_graphs[qos_graph_num]._points[i]->_x,_qos_graphs[qos_graph_num]._points[i]->_y,
                        _qos_graphs[qos_graph_num]._points[i+1]->_x,_qos_graphs[qos_graph_num]._points[i+1]->_y,
                        x);
        }
        assert(_qos_graphs[qos_graph_num]._points[i]->_x != _qos_graphs[qos_graph_num]._points[i+1]->_x);
    }
    return y;
}
double Scheduler::findYGivenLineAndX(double x1, double y1, double x2, double y2, double x3)
{
    double y3;

    //printf("(x1,y1)(x2,y2)(x3):(%f,%f)(%f,%f)(%f)\n",x1,y1,x2,y2,x3);
    y3 = y1 - ( ((y1-y2)*(x3-x1))/(x2-x1) );

    return y3;
}
*/
