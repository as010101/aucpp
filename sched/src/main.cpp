static char *cvs_id="@(#) $Id: main.C,v 1.62 2003/03/14 21:34:37 dpc Exp $";

#include <unistd.h>

//#define DEF_GLOBALS
//#include "global.H"
//#undef DEF_GLOBALS

#include "Scheduler.H"
//#include "shm_util.H"
//#include "sem_util.H"
#include "Shared.H"
#include "check_ip.H"
#include "Catalog.H"
#include "lex_stuff.H"
#include "StreamThread.H"
#include "NetThread.H"
//#include "InputReceiver.H"
#include "SysMon.H"
#include "QueueMon.H"
//#include "QoSMonitor.H"
#include <strstream.h>

#include <QueryNetwork.H>
#include "RandomPoint.H"

#include <CatalogManager.H>

#define NUM_WORKER_THREADS 1

#define BOX_SCHEDULING 1
#define APP_SCHEDULING 2

extern QueueMon* _queue_monitor;

void print_usage();

void print_usage()
{
	printf("Usage: run -D <database_directory> [-M] [-Q] [-N] [-r <rate>] [-h] [-H] [-B] [-A] [-P <priority_assignment>] [-T <traversal_type>] [-G] [-C <cost>]\n");
	printf("\t -C <cost>                  ...    specifies the per box cost\n");
	printf("\t -D <database_directory>    ...    specifies the catalog\n");
	printf("\t -M                         ...    run with sys monitor\n");
	printf("\t -Q                         ...    run with queue monitor\n");
	printf("\t -N                         ...    run with net app\n");
	printf("\t -r <rate>                  ...    set input rate for inputs (rate is a float)\n");
	printf("\t -h                         ...    print this message\n");
	printf("\t -H                         ...    print this message\n");
	printf("\t -L <scheduling_type>       ...    scheduling type (B=Box scheduling, A=Application Scheduling\n");
	printf("\t -P <priority_assignment>   ...    priority assigment type (RANDOM,RR,SLOPE,LQF)\n");
	printf("\t -T <traversal_type>        ...    box traversal type (MIN_COST,MIN_LATENCY,MIN_MEMORY)\n");
	printf("\t -G                         ...    use the automatic network generator\n");
}

int main(int argc, char** argv)
{
	int aflag=0;
	int bflag=0;
	bool queue_mon_flag = false;
	bool sys_mon_flag = false;
	bool net_app_flag = false;
	bool generate_network_flag = false;
	bool rate_flag = false;
	int c;
	istrstream *parser, *sub_parser;
	char *cvalue;
	double rate_control;
	string db;
	int sched_by = -1, mem_size = 0, tuple_size = 4;
	char *priority_assignment = NULL;
	_measure = new Measurement;
	bool experiment_mode = false;
	char *traversal_type = NULL;
	int num_applications = -1;
	int net_depth = 10, len = 0;
	double net_width = 0.9;
	double per_box_cost = -1.0;
	double per_box_selectivity = -1.0;
	bool top_k_spanner_flag = false;
	int k_val = 1, burst_size = 1;
	int num_worker_threads = NUM_WORKER_THREADS;
	double fixed_train_size = 1.0;
	int num_buckets = 50;

	RandomPoint ***qos_graphs;
	RandomPoint **cost, **selectivity;
	int currPoint = 0;
	char token[ 50 ], token2[ 50 ];
	char *box_costs = NULL, *box_selectivities = NULL, *qos_specs = NULL;
	
	printf("GOT TO A\n");
	while ((c = getopt (argc, argv, "A:B:b:C:D:EF:G:hHJ:K:NL:MO:P:Qr:S:s:T:U:V:W:X:Y:Z:z:")) != -1)
	{
	printf("GOT TO A1: %c\n",c);
	switch (c)
		{
			case 'A': // cost specifications.
				len = strlen( optarg );
				box_costs = ( char* ) malloc( len+1 );
				memcpy( box_costs, optarg, len );  // make a copy!
				break;
			case 'B': // selectivity specifications
				
				len = strlen( optarg );
				box_selectivities = ( char* ) malloc( len+1 );
				memcpy( box_selectivities, optarg, len );
				break;			
			case 'b':
				num_buckets = atoi(optarg);
				break;
			case 'C': // per box cost
				per_box_cost = atof(optarg);
				break;
			case 'D': // set catalog directory
				db = optarg;
				break;
			case 'E':
				experiment_mode = true;
				break;
			case 'F':
				fixed_train_size = atof(optarg);
				printf(" TRAIN SIZE! %f\n", fixed_train_size );
				break;
			case 'G':
				generate_network_flag = true;
				num_applications = atoi(optarg);
				_measure->setNumApps(num_applications);
				printf("num_applications: %i\n",num_applications);
				_measure->setAppCount( num_applications );
				break;
			case 'h': // print usage message (help)
			case 'H':
				print_usage();
				exit(0);
				break;
			case 'J':
				num_worker_threads = atoi(optarg);
				break;
			case 'K':
				printf("got to k1\n");
				top_k_spanner_flag = true;
				printf("got to k2 optarg: %s\n",optarg);
				k_val = atoi(optarg);
				printf("got to k3\n");
				break;
			case 'L': // scheduling type
				printf("optarg: %s\n",optarg);
				if ( strcmp(optarg,"A") == 0 )
					sched_by = APP_SCHEDULING;
				else if ( strcmp(optarg,"B") == 0 )
					sched_by = BOX_SCHEDULING;
				else
				{
					printf("Bad Scheduling Type\n");
					print_usage();
					exit(0);
				}
				break;
			case 'M': // turn SysMon on
				sys_mon_flag = true;
				break;
			case 'N': // turn net_app on
				net_app_flag = true;
				break;
			case 'O': // set outputfile prefix
				_measure->setOutputFilename(optarg);
				break;
			case 'P': // priority assignment type
				priority_assignment = optarg;
				break;
			case 'Q': // turn Queue Monitor on
				queue_mon_flag = true;
				break;
			case 'r': // set rate control (interarrival interval)
				rate_control = atof(optarg);
				rate_flag = true;
				printf("rate_control: %f\n",rate_control);
				break;
			case 'S': // set stop type
				_measure->setStopType(optarg);
				break;
			case 's': // set stop condition
				if ( _measure->getStopType() == TUPLES_WRITTEN )
					_measure->setStopCondTuplesWritten(atoi(optarg));
				else if ( _measure->getStopType() == TUPLES_WRITTEN_TO_MEMORY )
					_measure->setStopCondTuplesWrittenToMemory(atoi(optarg));
				else if ( _measure->getStopType() == TIME_STOP )
					_measure->setStopCondTimeStop(atoi(optarg));
				else
				{
					printf("ERROR:[Main] Bad Stop Type\n");
					exit(0);
				}
				break;
			case 'T': // traversal_type
				traversal_type = optarg;
				break;
			case 'U': // utility specifications
				  
				len = strlen( optarg );
				qos_specs = ( char* ) malloc( len+1 );
				memcpy( qos_specs, optarg, len );  // make a copy!
				break;
			case 'V': // network width
				burst_size = atoi( optarg );
				break;
			case 'W': // network width
				net_width = atof(optarg);
				break;
			case 'X': // network_depth
				net_depth = atoi(optarg);
				break;
			case 'Y': // memory size
				mem_size = atoi(optarg);
				break;
			case 'Z': // selectivity
				per_box_selectivity = atof(optarg);
				break;
			case 'z': // selectivity
				tuple_size          = atoi(optarg);
				break;
			
			case '?':
				if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr,
						"Unknown option character `\\x%x'.\n",
							optopt);
				return 1;
		}
	}

	if ( generate_network_flag )  // auto-generation has been specified
	{
		// continue initialization. should probably make a common function
		// eventually to shorten code:
		
		cost = ( RandomPoint ** )malloc( sizeof( RandomPoint **)*num_applications);
		if ( box_costs == NULL )  //defaults
			box_costs = "*0.12:0.03:N";
		printf(" main.C: DEBUG: box_costs %c\n", box_costs[ 0 ] );
		if ( box_costs[ 0 ] == '*' )
		{
			currPoint = 0;
			cout << strlen( box_costs ) << " DEBUGGY: " << box_costs << endl;
			parser = new istrstream( box_costs+1 );
			
			parser->getline( token, strlen( box_costs ), ':' );
			for ( int f = 0; f < num_applications; f++ )
			{
				cost[ currPoint++ ] = new RandomPoint( token );
				cout << "DEBUGGY: " << cost[ currPoint-1 ]->toString() << endl;
			}
			free( parser );
		}
		else
		{
			printf("DO NOT HANDLE MULTIPLE PTS yet %. \n");
			exit( 1 );
		}
		
		selectivity = ( RandomPoint ** )malloc( sizeof( RandomPoint **) 
												* num_applications );
		if ( box_selectivities == NULL )  //defaults
			box_selectivities = "*0.7:0.2:N";
		if ( box_selectivities[ 0 ] == '*' )
		{
			currPoint = 0;
			//memcpy( token2, optarg, 50 );
			parser = new istrstream( box_selectivities+1 );
			
			parser->getline( token, strlen( box_selectivities ), ':' );
			for ( int f = 0; f < num_applications; f++ )
				selectivity[ currPoint++ ] = new RandomPoint(token); 			    
			free( parser );
		}
		else
		{
			printf("DO NOT HANDLE MULTIPLE PTS yet. \n");
			exit( 1 );
		}
		
		// QOS specifications
		qos_graphs = ( RandomPoint *** )malloc( sizeof( RandomPoint **) * num_applications);
		for ( int z = 0; z < num_applications; z++ )
			qos_graphs[ z ] = ( RandomPoint ** )malloc( sizeof( RandomPoint *) * 6 ); // this is a single QoS, 6 points always.
		if ( qos_specs == NULL )   //defaults
			qos_specs = "*0.0:1.0:5.0:1.0:20.0:1.0";
		if ( qos_specs[ 0 ] == '*' )
		{
			for ( int f = 0; f < num_applications; f++ )
			{
				currPoint = 0;
				parser = new istrstream( qos_specs+1 );
				len--;
				while ( parser->good() != 0 )
				{
					parser->getline( token, strlen( qos_specs ), ':' );
					qos_graphs[ f ][ currPoint++ ] = new RandomPoint(token); 			    
				}
				free( parser );
			}
		}
		else
		{
			//for ( int f = 0; f < num_applications*6; f++ )
			
			int f=0;
				{
					currPoint = 0;
					parser = new istrstream( qos_specs );
					//len--;
					while ( parser->good() != 0 )
					{
						parser->getline( token, len, ':' );
						printf("qos token: %s\n",token);
						qos_graphs[ f/6 ][ (currPoint++)%6 ] = new RandomPoint(token); 			    
						f++;
					}
					free( parser );
				}
				//printf("DO NOT HANDLE MULTIPLE PTS yet. \n");
				//exit( 1 );
		}
	} // this whole block only applies to generated networks.
	printf("GOT TO B\n");

	errno=0;
	setbuf(stdout,NULL);


	CatalogManager catalog(db);

	QueryNetwork *q_net;
	if ( generate_network_flag == true )
	{
		// Automatic Network Generation Stuff
			//float br[] = {3.1, 2.6, 2.9};
			//float br[] = {1.5, 1.5, 1.5};
			//int dp[] = {6, 6, 6};
			// the last fraction denotes number of (randomly picked)
			// shared boxes.
			//QueryNetwork *q_net = catalog.generate( 3, dp, br, .20 );
			//This generates a query network instead of loading it. Directory 
			//param is ignored. 3 = # of Appl, br = branching, dp = depth.


		/*
	  		float br[] = { net_width };
			int dp[] = { net_depth };
			float sh[] = { 0.0 };
			float qos1[] = {	0.0,	1.0,
								0.1,	1.0,
								20.0,	0.0  };
			printf(" NUM APP %d\n", num_applications );
			float **qos = ( float** )malloc( sizeof( *qos ) * num_applications );
		*/
		
		/*
			float br[] = {	1.2,
							1.2,
							1.2,
	       					1.2,
							1.2};
			int dp[] = {5,
						5,
						5,
						5,
						5};
			float sh[] = { .1, .2, .1, .1, .9 }; //share coeff.
		*/
			float *br = new float[num_applications];
			int *dp = new int[num_applications];
			float *sh = new float[num_applications];
			for ( int i = 0; i < num_applications; i++ )
			{
				br[i] = net_width;
				dp[i] = net_depth;
				sh[i] = 0.0;
			}
			//float qos1[] = {0,1,10,1,20,0};
			//float qos2[] = {0,1,10.5,1,20,0};
			//float qos3[] = {0,1,10,1,20,0};
			//float qos4[] = {0,1,10.3,1,20.1,0};
			//float qos5[] = {0,1,10,1,20,0};
			//float **qos = ( float** )malloc( sizeof( *qos ) * 5 );
			//qos[ 0 ] = qos1;  qos[ 1 ] = qos2; 
			//qos[ 2 ] = qos3;  qos[ 3 ] = qos4; qos[ 4 ] = qos5; 
			printf("----------------------num_applications: %i\n",num_applications);
			q_net = catalog.generate( num_applications, dp, br, sh, qos_graphs, cost, selectivity, tuple_size );
			_measure->setQoSBound( catalog.getQoSBound() );
	}
	else
	{
		// Read network from catalog
			q_net = catalog.load();
	}


	cout << endl << catalog.getTypeManager().toString() << endl;
	cout << endl << q_net->toString() << endl;
	// q_net->unfreeze(); there is a memory leak issue with strstream
	// but the changes were lost
	errno = 0;
	_catalog = new Catalog;
	if ( experiment_mode == true )
		_catalog->setExperimentFlag(true);
	_catalog->loadFromDB(q_net);
	_catalog->printArcs();
	int shmid = initialize_shared_mem(SHM_KEY);
	int semid = initialize_semaphore(SEMKEY);

	for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
	{
		Boxes *b = _catalog->getBox(i);
		if ( b != NULL )
		{
			if ( per_box_cost >= 0 )
				b->setCost(per_box_cost);
			if ( per_box_selectivity >= 0 )
				b->setSelectivity(per_box_selectivity);
			printf("b->setSelectivity: %f\n",b->getSelectivity());
		}
	}

	printf("MAIN: GOT TO A\n");

	//shm_set_num_records_in_queue(3,10);
	//printf("recs in q3: %i\n",shm_get_num_records_in_queue(3));
	//printf("recs in q2: %i\n",shm_get_num_records_in_queue(2));
  
	//this had to move from SMI... turns out SMI is created in
	// multiple instances, and I needed one copy of this global stuff.
	int arcs = _catalog->getMaxArcId()+1;
	printf("DEBUG:SMI: Allocated 2X %d\n", sizeof( int ) * arcs );
	memory_in_use = 0;
	pthread_mutex_init( &memory_mutex, NULL );
	queueAccessTm = ( int *) malloc( sizeof( int ) * arcs );
	bzero( queueAccessTm, sizeof( int ) * arcs );
	//queueStatus = ( int *) malloc( sizeof( int ) * arcs );
	//bzero( queueStatus, sizeof( int ) * arcs );
	// m_queue = ( Queue * )(malloc( sizeof( Queue * ) * arcs ));
	m_queue = new Vector[arcs];
	tuplesOnDisk = new int[ arcs ];
	bzero( tuplesOnDisk, arcs * sizeof( int ) );

	pthread_mutex_init( &__box_work_set_mutex, NULL );

	printf("MAIN: GOT TO B\n");


// NOTE TO WHOMEVER CARES: AT THIS POINT, errno IS 4. THAT IS INTERRUPTED SYSTEM CALL.
// MAY WANT TO RESET THAT TO 0, OR FIND OUT WHO/WHY/WHAT IS SETTING IT 
	errno=0;

	pthread_mutex_t start_lock;
	pthread_cond_t  start_cond;
	pthread_attr_t  attr;

	pthread_cond_init(&start_cond, NULL);
	pthread_attr_init(&attr);
	pthread_mutex_init(&start_lock, NULL);


	//this has to come before the scheduler start, since SMI uses
	// that number for memory
	_measure->setMemorySize( mem_size );

	//Scheduler scheduler(10,NUM_WORKER_THREADS,num_buckets);
	Scheduler scheduler(10,num_worker_threads,num_buckets);



	// Jeff's Stuff
	vector<StreamThread *>	input_arcs;
	Arcs *a;
printf("GOT TO A numInputs : %i\n", _catalog->getNumInputs());
	pthread_mutex_t tuple_count_mutex;
	pthread_mutex_init(&tuple_count_mutex,NULL);
	StreamThread *st = NULL, *staticST = NULL;
	staticST = new StreamThread( _catalog->getNumInputs(), &start_lock,
								 &start_cond, &tuple_count_mutex );
	for ( int i = 0; i < _catalog->getNumInputs(); i++ )
	{
		Inputs *inputs = _catalog->getInput(i);
		printf("main .. F errno: %i\n",errno);
		if (net_app_flag == true)
		{
			NetThread *nt = new NetThread(inputs->getArcId(), inputs->getRate(), inputs->getRate(),burst_size,
								&start_lock, &start_cond, &tuple_count_mutex);
			st = (StreamThread*)nt;
		}
		else
		{
			st = NULL;
			//st = new StreamThread(inputs->getArcId(), inputs->getRate(), 1000,
			//					 &start_lock, &start_cond, &tuple_count_mutex);
			if (rate_flag == true)
			{
				// multiply rate_control by num inputs (its not a rate .. rather its
				// an interval (1/rate)
				//double per_input_interval = rate_control * (double)_catalog->getNumInputs();
				//double per_input_interval = rate_control;
				double per_input_interval = scheduler.getNetworkCost();
				printf("a per_input_interval: %f  rate_control: %f\n",per_input_interval,rate_control);
				//double input_rate = 1.0/per_input_interval;
				//input_rate = rate_control * input_rate;
				//per_input_interval = 1.0/input_rate;
				//per_input_interval = per_input_interval/rate_control;
				if ( rate_control > 0.0 )
				{
					per_input_interval = per_input_interval/rate_control;
					printf("b per_input_interval: %f\n",per_input_interval);
				}
				else
				{
					per_input_interval = fabs (rate_control);
					printf("c per_input_interval: %f\n",per_input_interval);
				}

				fprintf(stderr,"%i inputs => per_input_interval: %f   total_input_rate: %f\n",_catalog->getNumInputs(),per_input_interval, (1.0/per_input_interval)*_catalog->getNumInputs());
				//st = new StreamThread(inputs->getArcId(), per_input_interval, inputs->getNumTuples(), burst_size, 
				//&start_lock, &start_cond, &tuple_count_mutex);
				staticST->addInput( inputs->getArcId(), per_input_interval,
								   inputs->getNumTuples(), burst_size ); 
			}
			else
				//st = new StreamThread(inputs->getArcId(), inputs->getRate(), inputs->getNumTuples(), burst_size, 
				//&start_lock, &start_cond, &tuple_count_mutex);
				staticST->addInput( inputs->getArcId(), inputs->getRate(), 
								   inputs->getNumTuples(), burst_size ); 
		}
		
		printf("INPUT arc: %i   rate: %f   numtuples: %i\n", inputs->getArcId(), inputs->getRate(),inputs->getNumTuples());
		if ( st != NULL )
			input_arcs.push_back(st);
		//st->start();
	}
	for ( int i = 0; i < input_arcs.size(); i++ )
	{
		if ( net_app_flag == true)
		{
			((NetThread*)input_arcs[i])->start();
		}
		else
			input_arcs[i]->start();
	}

	staticST->setSchedulerWakeCondition( scheduler.getWakeCondition() );
	staticST->setSchedulerWakeMutex( scheduler.getWakeMutex() );

	// start generating input for all non-NetThread (whatever they are) inputs
	staticST->start();

	// This set the wake condition/mutex to all input threads
	// I am mystified as to why this was AFTER the inputthreads used to
	// start and still worked.
	/*for ( int i = 0; i < input_arcs.size(); i++ )
	{
		//if ( net_app_flag == false)
		{
			input_arcs[i]->setSchedulerWakeCondition(scheduler.getWakeCondition());
			input_arcs[i]->setSchedulerWakeMutex(scheduler.getWakeMutex());
		}
		//else
			// have to change NetThread at some point in the future for this
			;
			}*/

	QoSMonitor qm( 2, q_net );
	_qos = &qm;  // = NULL to turn off QoS reporting.
	//_qos = NULL; //to turn off QoS reporting.
	qm.run();  //not currently using/running as its own thread.

	if ( sys_mon_flag == true )
	{
		printf("GOT TO SysMon\n");
		SysMon *sm = new SysMon(_catalog->getMaxArcId()+1,input_arcs[0],&scheduler);
		sm->start(5);
	} 
	if ( queue_mon_flag == true ) 
	{
		//_queue_monitor = new QueueMon(_catalog->getMaxArcId()+1);
		_queue_monitor = new QueueMon(q_net, st, _catalog->getMaxArcId());
		_queue_monitor->start(5);
		sleep(2);
		// Block everyone right here until the viewer says "run". We'll just use busy-waiting right now
		while (!_queue_monitor->isRunning()) 
			sleep(1);
	}
		  
	printf("MAIN: GOT TO C\n");
		
	if ( sched_by == BOX_SCHEDULING ||  sched_by == APP_SCHEDULING )
	{
		// first set box and app types to random
		scheduler.setBoxScheduleType(BOX_RANDOM_TYPE);
		scheduler.setAppScheduleType(APP_RANDOM_TYPE);

		if ( priority_assignment == NULL )
		{
			printf("[MAIN]A Bad priority assignment\n");
			exit(0);
		}

		// next set the values
		if ( sched_by == BOX_SCHEDULING )
			scheduler.setSchedBy(SCHED_BY_BOX);
		else if ( sched_by == APP_SCHEDULING )
			scheduler.setSchedBy(SCHED_BY_APP);
		if ( strcmp(priority_assignment,"SLOPE") == 0 )
		{
			if ( sched_by == BOX_SCHEDULING )
				scheduler.setBoxScheduleType(SLOPE_SLACK_TYPE);
			else if ( sched_by == APP_SCHEDULING )
				scheduler.setAppScheduleType(APP_SLOPE_SLACK_TYPE);
		}
		else if ( strcmp(priority_assignment,"BUCKETING") == 0 )
		{
			if ( sched_by == BOX_SCHEDULING )
				scheduler.setBoxScheduleType(BOX_BUCKETING_TYPE);
			else if ( sched_by == APP_SCHEDULING )
				scheduler.setAppScheduleType(APP_BUCKETING_TYPE);
		}
		else if ( strcmp(priority_assignment,"RR") == 0 )
		{
			if ( sched_by == BOX_SCHEDULING )
				scheduler.setBoxScheduleType(BOX_RR_TYPE);
			else if ( sched_by == APP_SCHEDULING )
				scheduler.setAppScheduleType(APP_RR_TYPE);
		}
		else if ( strcmp(priority_assignment,"RANDOM") == 0 )
		{
			if ( sched_by == BOX_SCHEDULING )
				scheduler.setBoxScheduleType(BOX_RANDOM_TYPE);
			else if ( sched_by == APP_SCHEDULING )
				scheduler.setAppScheduleType(APP_RANDOM_TYPE);
		}
		else if ( strcmp(priority_assignment,"LQF") == 0 )
		{
			if ( sched_by == BOX_SCHEDULING )
				scheduler.setBoxScheduleType(BOX_LQF_TYPE);
			else if ( sched_by == APP_SCHEDULING )
				scheduler.setAppScheduleType(APP_LQF_TYPE);
		}
		else
		{
			printf("[MAIN]B Bad priority assignment\n");
			exit(0);
		}
	}
	else // default scheduling
	{
		scheduler.setSchedBy(SCHED_BY_BOX);
		scheduler.setBoxScheduleType(BOX_RR_TYPE);
		scheduler.setAppScheduleType(APP_RANDOM_TYPE);
	}
  
	//for ( int i=0; i < 10; i++ )
	{
		//printf("sleeping for 2 ..............\n");
		sleep(1);
		pthread_mutex_lock(&start_lock);
		printf("before cond broadcast\n");
		pthread_cond_broadcast(&start_cond);
		printf("after  cond broadcast\n");
		pthread_mutex_unlock(&start_lock);
	}

	printf("MAIN: GOT TO D\n");
	if ( traversal_type == NULL )
		scheduler.setBoxTraversalType(MIN_COST);
	else if ( strcmp(traversal_type,"MIN_COST") == 0 )
		scheduler.setBoxTraversalType(MIN_COST);
	else if ( strcmp(traversal_type,"MIN_LATENCY") == 0 )
		scheduler.setBoxTraversalType(MIN_LATENCY);
	else if ( strcmp(traversal_type,"MIN_MEMORY") == 0 )
		scheduler.setBoxTraversalType(MIN_MEMORY);
	else // default
		scheduler.setBoxTraversalType(MIN_COST);

	printf("MAIN: GOT TO E\n");
	if ( top_k_spanner_flag == true )
		scheduler.setTopKSpanner(k_val);
	scheduler.setFixedTrainSize(fixed_train_size);

	_measure->setStartTime();
    scheduler.start();

	_measure->addMeasurementType( TIME_TOTAL_SCHED_WORKER_THREADS );
	_measure->addMeasurementType( TIME_SCHEDULING_VS_WORKER_THREADS );
	_measure->addMeasurementType( TIME_SPENT_SCHEDULING );
	_measure->addMeasurementType( TIME_SPENT_IN_WORKER_THREADS );
	_measure->addMeasurementType( NUM_SCHEDULING_DECISIONS );
	_measure->addMeasurementType( NUM_BOX_CALLS );
	_measure->addMeasurementType( AVERAGE_LATENCY );
	_measure->addMeasurementType( AVERAGE_QOS );
	_measure->addMeasurementType( AVERAGE_TUPLE_TRAIN_SIZE );
	_measure->addMeasurementType( DISK_IO );
	_measure->addMeasurementType( DISK_READS );
	_measure->addMeasurementType( DISK_WRITES  );
	_measure->addMeasurementType( TOTAL_RUN_TIME );
	_measure->addMeasurementType( NUM_MALLOCS );
	_measure->addMeasurementType( TIME_LOADING_QUEUES );
	_measure->addMeasurementType( TIME_UNLOADING_QUEUES );

	_measure->addXVariable( INPUT_RATE );
	_measure->addXVariable( APP_DEPTH );
	_measure->addXVariable( APP_WIDTH );
	_measure->addXVariable( MEMORY_SIZE );
	_measure->addXVariable( BOX_COST );
	_measure->addXVariable( QOS_BOUND );
	_measure->addXVariable( APP_COUNT );
	_measure->addXVariable( K_SPANNER_VAL );
	_measure->addXVariable( BURST_SIZE );
	_measure->addXVariable( TRAIN_SIZE );

	_measure->setInputRate( rate_control );
	_measure->setAppDepth( net_depth );
	_measure->setAppWidth( net_width );
	_measure->setBoxCost( per_box_cost );
	_measure->setKSpannerVal( k_val );
	_measure->setBurstSize( burst_size );
	printf(" MEASURE setting TRAIN SIZE %f\n", fixed_train_size );
	_measure->setTrainSize( fixed_train_size );

	_measure->getMemRemainingGraph();
	/*
	        case APP_DEPTH:
	        case APP_WIDTH:
	        case BOX_COST:
	        case BOX_SELECTIVITY:    
	*/


	_measure->outputMeasurements();

  	cleanup(shmid,semid);

	return 0;
}

