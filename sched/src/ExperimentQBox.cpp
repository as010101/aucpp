#include "ExperimentQBox.H"
#include "Measurement.H"
#include <sys/time.h>

Box_Out_T ExperimentQBox::doBox()
{
	return_val.output_tuples = 0;
	char *outStream_ptr;
	outStream_ptr = _outStream[0]; // set to beginning of _outStream initially
	Arcs *a;
	a = _catalog->getArc(_outputArcId[0]);
	int _tuple_size = _tuple_descr->getSize() + getTsSize() + getSidSize();
	double rand_val;
	//static unsigned int seed = 145;
	timeval t_seed;
	gettimeofday(&t_seed,NULL);
	unsigned int seed = t_seed.tv_usec;
	//timeval *t = new timeval;
	timeval t;
	double total_latency = 0.0;
	int good_value_count = 0;

	_window1 = 0;
	return_val.kept_input_count_array = new int[_numInputArcs];

	// Loop through all the input arcs
	for (int j = 0; j < _numInputArcs; j++)
	{
		if ( _train_size[j] <= 0 )
		{
			return_val.kept_input_count_array[j] = -1;
			continue;
		}
		else
		{
			return_val.kept_input_count_array[j] = 0;
		}
		Arcs *a_input;
		a_input = _catalog->getArc(_inputArcId[j]);
		if ( a_input->getIsInput() != true )
		{
			//return_val.kept_input_count_array[j] = _window1;
			//return_val.kept_input_count_array[j] = -1;
			return_val.kept_input_count_array[j] = 0;
		}
		/*timeval now;
		  gettimeofday(&now,NULL);*/
		//printf("Begin using time *******************************: %d %d\n", now.tv_sec, now.tv_usec);
		
		double time_used = use_time(_train_size[j]*_unitCost._cost_time);
		int num_tuples_used;
		//if ( time_used < (_train_size[j]*_unitCost._cost_time) )
		//	num_tuples_used = (int)(_train_size[j] * (time_used / (_train_size[j]*_unitCost._cost_time)));
		//else
			num_tuples_used = _train_size[j];
			//cout << "Number of tuples used " << num_tuples_used << endl;
		//for (int i = 0; i < _train_size[j]; i++)
		double *t_stamps = new double[num_tuples_used];
		int num_tuples_passed = 0;
		for (int i = 0; i < (num_tuples_used-_window1); i++)
		{
			
			/*
			if ( a_input->getIsInput() == true )
			{
					gettimeofday(&t, NULL);
					int v1 =  (int)*((int*)_inStream[j]);
					int v2 =  (int)*((int*)_inStream[j]+1);
					//fprintf(stderr,"starting latency: %f\n", ((t.tv_sec + (t.tv_usec * 1e-6)) - ( v1 + (v2*1e-6))));
					//memcpy(_inStream[j], &t, sizeof(timeval));
					v1 =  (int)*((int*)_inStream[j]);
					v2 =  (int)*((int*)_inStream[j]+1);
					//fprintf(stderr,"new starting latency: %f\n", ((t->tv_sec + (t->tv_usec * 1e-6)) - ( v1 + (v2*1e-6))));
			}
			*/
			/*
			int x1 =  (int)*((int*)_inStream[j]);
			if (x1 < 1036444)
			{
			  //				printf("YYY[in-arc %i][%i]:testing: ts1: %i  ts2: %i  _instream ptr: %p\n",a_input->getId(),i,(int)*((int*)(_inStream[j]+(i*_tuple_size))),(int)*((int*)(_inStream[j]+(i*_tuple_size))+1),_inStream[j]+(i*_tuple_size));
			  //				printf("GOT TO x1: %i\n",x1);
			  //				fprintf(stderr,"GOT TO x1: %i\n",x1);
				fflush(stdout);
				abort();
			}
			if ( x1 == 0 )
			{
			  //printf("YYY input arc: %i x1: %i\n",a_input->getId(),x1);
			}
			*/
			rand_val = (double)rand_r(&seed) / (double)(RAND_MAX);

			//assert(_train_size[j] <= _train_size[0]);
			if (rand_val <= _selectivity)
			{
				if ( _inputArcId[j] == NULL)
				{
				printf("E arc_id: %i _numInputArcs: %i  j: %i _tuple_size: %i(%i) outStream_ptr: %i  _inStream[j]: %i  _train_size[j]: %i num_tuples_used: %i  _window1: %i\n",_inputArcId[j],_numInputArcs,j,_tuple_size,_tuple_descr->getSize(),outStream_ptr,_inStream[j],_train_size[j],num_tuples_used,_window1);
				}
				memcpy(outStream_ptr,
					_inStream[j] + (i*_tuple_size),
					_tuple_size);
				outStream_ptr += _tuple_size;
				good_value_count++;

				return_val.output_tuples++;
				if ( a->getIsApp() == true )
				{
					gettimeofday(&t, NULL);
					//int x2 =  (int)*((int*)_inStream[j]);
					//					if ( x2 == 0 )
					//						printf("YYY x2: %i\n",x2);
					int v1 =  (int)*((int*)_inStream[j]);
					int v2 =  (int)*((int*)_inStream[j]+1);

					//cout << " ID " << a->getId() << " LATENCY found " << ((t.tv_sec + (t.tv_usec * 1e-6)) - ( v1 + (v2*1e-6))) << " RESULT OF " <<  t.tv_sec  << " " << (t.tv_usec * 1e-6)<< " MINUS " <<  v1 <<  " " << (v2*1e-6) << endl;
					double tuple_lat = ((t.tv_sec + (t.tv_usec * 1e-6)) - ( v1 + (v2*1e-6)));
					assert (tuple_lat > 0.0);
					total_latency += tuple_lat;

					t_stamps[num_tuples_passed] = (double)v1 + (double)(v2*1e-6);
					num_tuples_passed++;
					assert( total_latency > 0.0 );
				}
			}
		}
		if ( a_input->getIsInput() != true )
			return_val.kept_input_count = _window1;

		if ( _qos != NULL )
		{
	static double elapsed_time = 0.0;
	double secs;
	secs = get_etime(_itimer_start);
			//_qos->tuples_written( num_tuples_passed, t_stamps, a->getId() );
	elapsed_time += get_etime(_itimer_start) - secs;
			//printf("ExperimentBox:: going to _qos->tuples_written---------------------------------------------%f\n",elapsed_time);
		}

		if ( a->getIsApp() == true ) //&& a->getId() == 0)
		{
			_measure->incrementQosPerTuple(a->getAppNum(),num_tuples_passed,findYGivenXAndQos(total_latency/num_tuples_passed,a->getAppNum()));
		}

		delete[] t_stamps;
		_measure->incrementTotalBoxTuplesConsumed(_train_size[j]);
		static int max_train_size = 0;
		if ( _train_size[j] > max_train_size )
		{
			max_train_size = _train_size[j];
		}
	}

	if ( a->getIsApp() == true )
	{
		//Latency and tuples written are recorded by WorkerThread
		//_measure->incrementToMemoryLatency(total_latency);
//fprintf(stderr,"GOT TO ExperimentQBox::doBox()[app:%i] tuples: %i total_latency: %f  latency per tuple: %f\n",a->getAppNum(),return_val.output_tuples,total_latency,total_latency/(double)return_val.output_tuples);
		//_measure->incrementToMemoryTuplesWritten(return_val.output_tuples);
		if ( _measure->getStopType() == TUPLES_WRITTEN_TO_MEMORY )
		{
			_measure->testStopCond();
		}
	}

	return_val.total_latency = total_latency;
	return_val.kept_input_count = 0;
	return_val.output_tuple_size = _tuple_size;

	// This is very important to maintain too!
	return_val.output_tuples_array = new int[1];
	return_val.output_tuples_array[0] = good_value_count;



	return return_val;
  
}
double ExperimentQBox::use_time(double time_to_use)
{

	//printf("ExperimentQBox: time_to_use  %lf seconds\n",time_to_use);

	struct itimerval first;
	//init_etime(&first);
	//double secs;
	//secs = get_etime(&first);
	//double j = 0.0;
	//double elapsed_time;
	//double rand_val;


	//unsigned int seed = 1;
	//timeval t_seed;
	//gettimeofday(&t_seed,NULL);
	//unsigned int seed = t_seed.tv_usec;
	unsigned int seed = 1;

	double secs_per_rand = _measure->getSecondsPerRand();
	//printf("ExperimentQBox: secs_per_rand  %g seconds\n",secs_per_rand);
	//double secs_per_rand = 2.60260e-08;

	//double secs_per_rand = 2.414e-08;
	//double secs_per_rand = 2.78e-08;
	//double secs_per_rand = 1.09111e-07;
	//double secs_per_rand = 1.13e-07;
	//double secs_per_rand = 1.25e-07;
	//double secs_per_rand = 1.7e-07;
	//secs_per_rand = 1.7e-07;
	double num_rands = time_to_use/secs_per_rand;

	double elapsed_time = 0.0;
	double secs;
	secs = get_etime(_itimer_start);
	for ( int i = 0; i < num_rands; i++ )
	{
		//(double)rand_r(&seed) / (double)(RAND_MAX);
		drand48() / (double)(RAND_MAX);
	}
	elapsed_time = get_etime(_itimer_start) - secs;
	_measure->incrementTimeSpentExecutingBoxes(elapsed_time);
	//printf("ExperimentQBox:num_rands(%lf):::: %lf seconds\n",num_rands,elapsed_time);

	return (time_to_use);

	/*
	while(1)
	{
		elapsed_time = get_etime(&first)-secs;
		if ( elapsed_time < time_to_use )
		{
			j = j/10.0;
			j=j+2.3;
		}
		else
			break;
		_measure->testStopCond();
		if ( __global_stop.getStop() == true )
			break;
	}
	//return (get_etime(&first)-secs); // return elapsed time
	*/
}
void ExperimentQBox::init_etime(struct itimerval *first)
{
	first->it_value.tv_sec = 1000000;
	first->it_value.tv_usec = 0;
	//setitimer(ITIMER_PROF,first,NULL);
	setitimer(ITIMER_VIRTUAL,first,NULL);
}
double ExperimentQBox::get_etime(struct itimerval *first)
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

double ExperimentQBox::findYGivenLineAndX(double x1, double y1, double x2, double y2, double x3)
{
	double y3;

		y3 = y1 - ( ((y1-y2)*(x3-x1))/(x2-x1) );

	return y3;
}
double ExperimentQBox::findYGivenXAndQos(double x,int qos_graph_num)
{
    double y=0.0;
    for ( int i = 0; i < __qos_graphs[qos_graph_num]._points.size()-1; i++ )
    {
        if ( x >= __qos_graphs[qos_graph_num]._points[i]->_x &&
                x < __qos_graphs[qos_graph_num]._points[i+1]->_x)
        {
                y = findYGivenLineAndX(__qos_graphs[qos_graph_num]._points[i]->_x,__qos_graphs[qos_graph_num]._points[i]->_y,
                        __qos_graphs[qos_graph_num]._points[i+1]->_x,__qos_graphs[qos_graph_num]._points[i+1]->_y,
                        x);
        }
        assert(__qos_graphs[qos_graph_num]._points[i]->_x != __qos_graphs[qos_graph_num]._points[i+1]->_x);
    }
    return y;
}

