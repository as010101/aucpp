#include <Scheduler.H>
#include <Measurement.H>
#include <algorithm>
#include <set>
#include <values.h>

// LIFE WOULD BE EASIER IF ONE MASTER HEADER INCLUDED ALL OTHER BOXES
#include <AggregateBox.H>
#include <BSortBox.H>
#include <FilterBox.H>
#include <JoinBox.H>
#include <ResampleBox.H>
#include <HelloWorldBox.H>
#include <UpdateRelationBox.H>
#include <ReadRelationBox.H>
#include <LRUpdateRelationBox.H>
#include <LRReadRelationBox.H>
// The above are used to access the catalog manager "Box" subclasses when necessary

#include <RuntimeGlobals.H>	// tatbul@cs.brown.edu

extern QueueMon* _queue_monitor;

int Point::operator <(const Point & p) const
{
  return _x < p._x;
}
int BoxTraversal::operator <(const BoxTraversal & bt) const
{
  return _traversal_cost < bt._traversal_cost;
}
int BoxLatency::operator <(const BoxLatency & bl) const
{
  if ( _ss._slope == bl._ss._slope )
    return _ss._slack > bl._ss._slack; 
  else
    return _ss._slope < bl._ss._slope; 
}
int BoxMemory::operator <(const BoxMemory & bq) const
{
    return _memory_release < bq._memory_release; 
}
int BoxQueue::operator <(const BoxQueue & bq) const
{
	return _largest_input_queue_length < bq._largest_input_queue_length;
}


Scheduler::Scheduler(int max_queue_size,
					 int max_num_worker_threads, 
					 int num_buckets,
					 AppTupleMonitor * pTupleMon) :
	_tupleMon(pTupleMon)
{
	//cout << "Initializing scheduler with " << max_num_worker_threads << " Mode " << _catalog->inTPBMode();
	//cout << "begin Scheduler()" << endl;
	_beq_size = 10; // 10 is default .. this value can be set with setBEQSize()

	_num_applications = 0;
    _sort_list = new sortable_slope_struct[_catalog->getNumBoxes() + 1]; // AGAIN  (similar to below)
	_box_applications = new int[_catalog->getMaxBoxId() + 1]; // see below talk on memory violation why + 1
	_max_queue_size = max_queue_size;
	_max_num_worker_threads = max_num_worker_threads;
	_num_buckets = num_buckets;
	_num_scheduling_events = 0;
	_total_network_cost = 0.0;
	_num_beq_elements_pushed = 0;
	//_top_k_spanner_flag = true;
	_top_k_spanner_flag = false;
	_K = 1;
	_use_buckets_flag = false;
	_bucket_size = 0;
	_fixed_train_size = 1.0;
	_time_spent_copying_runnable_boxes = 0.0;
	_time_spent_verifying_lockable_boxes = 0.0;
	_time_spent_loading_candidate_boxes = 0.0;
	_time_spent_loading_execution_queue = 0.0;
	_time_spent_traverse_loadqueue = 0.0;
	_time_spent_create_boxlist = 0.0;
	_time_spent_loading_execution_queue_app = 0.0;
	_time_spent_loading_dequeue_pin_lists = 0.0;
	_time_spent_loading_pin_lists = 0.0;
	_time_spent_loadBox = 0.0;
	_time_spent_insert_locked_box_set = 0.0;
	_time_spent_shm_get_num_records = 0.0;
	_time_spent_findEligibleBox_Bucketing = 0.0;
    _time_spent_Bucketing_removeFromGrid = 0.0;
    _time_spent_Bucketing_finding_candidates = 0.0;
    _time_spent_Bucketing_finding_schedulable = 0.0;




	_execution_queue = new BoxExecutionQueue;

	_wait_mutex = new pthread_mutex_t;
	_app_schedule_mutex = new pthread_mutex_t;
	_box_schedule_mutex = new pthread_mutex_t;
	_sched_by_mutex = new pthread_mutex_t;
	_scheduling_ticks_mutex = new pthread_mutex_t;
	_box_traversal_type_mutex = new pthread_mutex_t;
	_wait_cond = new pthread_cond_t;
	pthread_mutex_init(_wait_mutex,NULL);
	pthread_mutex_init(_app_schedule_mutex,NULL);
	pthread_mutex_init(_box_schedule_mutex,NULL);
	pthread_mutex_init(_sched_by_mutex,NULL);
	pthread_mutex_init(_scheduling_ticks_mutex,NULL);
	pthread_mutex_init(_box_traversal_type_mutex,NULL);
	pthread_cond_init(_wait_cond,NULL);
	_box_qos_graphs = new map < int, qos_struct* >;


	/*
	//_app_schedule_type = APP_SLOPE_SLACK_TYPE;
	_app_schedule_type = APP_RR_TYPE;
	_box_schedule_type = SLOPE_SLACK_TYPE;
	_sched_by = SCHED_BY_APP;
	*/
	
	_app_schedule_type = APP_SLOPE_SLACK_TYPE;
	_box_schedule_type = BOX_RR_TYPE;
	//_box_schedule_type = BOX_RANDOM_TYPE;
	_sched_by = SCHED_BY_BOX;
	

	_bqm = new BoxQueueMapping[_catalog->getMaxBoxId()+1];
	for ( int k = 0; k <= _catalog->getMaxBoxId(); k++ )
	{
		_bqm[k]._box_id = -1;
		_bqm[k]._num_input_queues = 0;
		_bqm[k]._num_output_queues = 0;
		_bqm[k]._num_out_ports = 0;
        _bqm[k]._output_arcs_byport = NULL;
	}

	Boxes *b;
	Arcs *a;
	//fprintf(stderr,"GOT TO A\n");
	for ( int i = 0; i <= _catalog->getMaxArcId(); i++ )
	{
		//printf("testing arc: %i\n",i);
		if ( (a = _catalog->getArc(i)) != NULL )
		{
			int box_id_out = a->getDestId();
			int box_id_in = a->getSrcId();
			//printf("box_id_out: %i\n",box_id_out);
			//printf("box_id_in: %i\n",box_id_in);
			if ( (b = _catalog->getBox(box_id_out)) != NULL )
			{
				(_bqm[box_id_out]._input_queue_id).push_back(i);
				_bqm[box_id_out]._box_id = box_id_out;
				_bqm[box_id_out]._num_input_queues++;
			}
			else
			{
				//fprintf(stderr,"Found the Application ARC (%i)\n",i);
				_num_applications++;
				//loadApplicationTree(a->getId());
				_app_v.push_back(a->getId());
				a->setIsApp(true,_app_v.size()-1);
				AppToBoxMapping *abm = new AppToBoxMapping(i);
				abm->_boxes.push_back(a->getSrcId());
				_app_to_box_mappings.push_back(abm);
				// MEMORY VIOLATION HERE a->getSrcId() returns more than _catalog->getMaxBoxId() sometimes!
				// Solution : a->getSrcId() -1 , or create the array of size maxId + 1
				// but remains issue of threading if this is called BEFORE catalog is done with all its
				//  insertBox() calls (the array wont be initialized with the max id then!
				_box_applications[a->getSrcId()]=_app_to_box_mappings.size()-1;
				//printf("GOT TO B\n");
			}
			if ( (b = _catalog->getBox(box_id_in)) != NULL )
			{
				(_bqm[box_id_in]._output_queue_id).push_back(i);
				_bqm[box_id_in]._box_id = box_id_in;
				_bqm[box_id_in]._num_output_queues++;

				// this is to figure out how many ports a box has.
				// relies on sequential port numbering (and is a hack) --alexr
                if ( a->getSrcPortId()+1 > _bqm[box_id_in]._num_out_ports )
				{
					_bqm[box_id_in]._num_out_ports = a->getSrcPortId()+1;
					//printf(" For box %d set out ports to %d\n", box_id_in, _bqm[box_id_in]._num_out_ports );
				}
			}
			else
			{
				a->setIsInput(true);
			}
		}
	}

	for ( int i = 0; i < _app_v.size(); i++)
	{
		pthread_mutex_t *_tmp_mutex = new pthread_mutex_t;
		pthread_mutex_init(_tmp_mutex,NULL);
		_app_mutexes.push_back(_tmp_mutex);
	}
	//fprintf(stderr,"GOT TO B\n");
	for ( int i = 0; i < _app_v.size(); i++)
	{
		map<int, vector<int> *> *tmp_map = new map<int, vector<int> *>;
		_queue_to_upstream_queues_v.push_back(tmp_map);
		//fprintf(stderr,"GOT TO C1\n");
		loadApplicationTree(i,_app_v[i]);

		//fprintf(stderr,"GOT TO C2\n");

	/*
		for ( int x = 0; x < _catalog->getMaxArcId(); x++ )
		{
			Arcs *a = _catalog->getArc(_app_v[i]);
			if ( a != NULL )
			{
				map<int, vector<int>* > *map_ptr = _queue_to_upstream_queues_v[i];
				map<int,vector<int>*>::iterator iter;
				for ( iter = map_ptr->begin(); iter != map_ptr->end(); iter++ )
				{
					for ( int z = 0; z < ((*iter).second)->size(); z++ )
					{
					}
				}
			}
		}
		*/

		// Use the following code if you want to connect applications to sockets
		//
		//Arcs *a = _catalog->getArc(_app_v[i]);
		//a->setSockFD(MITRE_initOutputConnections());
		//int ival = _app_v[i];
		//if (write(a->getSockFD(), &ival, sizeof(ival)) < 0)  // MUST BE adjusted for the right length of stuff to send
		//{
			//perror("[MITRE] write failed");
			//exit(1);
		//}
	}
	//fprintf(stderr,"GOT TO D\n");
	generateAppBoxNodeMap();
	//printApplicationTrees();
	getPostOrderTraversals();
	getPreOrderTraversals();
	//fprintf(stderr,"GOT TO E\n");
	/*
	for ( int i = 0; i < _app_v.size(); i++ )
	{
		printf("Post-Order Traversal[%i]: ",i);
		for ( int j = 0; j < _post_order_traversals[i]->size(); j++ )
		{
			printf(" %i,",(*_post_order_traversals[i])[j]);
		}
		printf("\n");
	}
	*/
	/*
	for ( int i = 0; i < _app_v.size(); i++ )
	{
		printf("Pre-Order Traversal[%i]: ",i);
		for ( int j = 0; j < _pre_order_traversals[i]->size(); j++ )
		{
			printf(" %i,",(*_pre_order_traversals[i])[j]);
		}
		printf("\n");
	}
	*/

	//fprintf(stderr,"GOT TO F\n");
	for ( int i = 0; i < _app_v.size(); i++ )
	{
		//if (_traversal_type == MIN_LATENCY )
		{
			generateMinLatencyTraversals(i);
			//printMLTraversals();
		}
		//else if ( (_traversal_type == MIN_MEMORY )
		{
			generateMinMemoryTraversals(i);
			//printMMTraversals();
		}
		getAppCost(i);
	}
	//fprintf(stderr,"GOT TO G\n");
	loadBoxAppMapping();
	//fprintf(stderr,"GOT TO H\n");
	initQosGraphs();
	//printQosGraphs();


	//fprintf(stderr,"GOT TO I\n");
	loadBoxQosGraphs();

	//fprintf(stderr,"GOT TO J\n");
	for ( int k = 0; k <= _catalog->getMaxBoxId(); k++ )
	{
		multimap<int,int>::iterator iter;
		if(_bqm[k]._box_id >= 0)
		{
			//printf("BAM Box(%i):",k);
			for ( iter = _bam.lower_bound(k);
					iter != _bam.upper_bound(k);
					iter++)
			{
				//printf(" %i",(*iter).second);
			}
			//printf("\n");

		}
	}

	int max_actual_box_id = 0;
	for ( int k = 0; k <= _catalog->getMaxBoxId(); k++ )
	{
		if(_bqm[k]._box_id >= 0)
		{
			_actual_box_ids.push_back(k);
			if ( k > max_actual_box_id )
				max_actual_box_id = k;
		}
	}


	_runnable_boxes_bitarray = new BitShit(max_actual_box_id);
	//fprintf(stderr,"GOT TO K\n");
	initPriorityGrid();

	//fprintf(stderr,"GOT TO J\n");
	for ( int k = 0; k <= _catalog->getMaxBoxId(); k++ )
	{
		if(_bqm[k]._box_id >= 0)
		{
			if ( (_bqm[k]._input_queue_id).size() < 0 )
			{
					fprintf(stderr,"ERROR: BOX %i has no input queue\n",k);
					exit(0);
					assert( false );
			}
			if ( (_bqm[k]._output_queue_id).size() < 0 )
			{
					fprintf(stderr,"ERROR: BOX %i has no output queue\n",k);
					exit(0);
					assert( false );
			}
		}
	}



/* DON - REMOVE THIS (1) AND CLOSURE ONCE PROB BETWEEN FIXED
	for ( int k = 0; k < _app_to_box_mappings.size(); k++ )
	{
		int current_box = _app_to_box_mappings[k]->_boxes[0];
// DON THIS NEEDS TO CHANGE??? 
		int in_queue = _bqm[current_box]._input_queue_id_1;
		int in_queue_src = (_catalog->getArc(in_queue))->getSrcId();
		printf("GOT TO D\n");
		// note: this does not work for the general case .. it does not take branching of more than one into account
		while (_catalog->getBox(in_queue_src) != NULL )
		{
			(_app_to_box_mappings[k]->_boxes).push_back(in_queue_src);
			_box_applications[in_queue_src]=_app_to_box_mappings.size()-1;
			(_app_to_box_mappings[k]->_arcs).push_back(in_queue);
			current_box = in_queue_src;
// DON THIS NEEDS TO CHANGE???
			in_queue = _bqm[current_box]._input_queue_id_1;
			in_queue_src = (_catalog->getArc(in_queue))->getSrcId();
		}
		(_app_to_box_mappings[k]->_arcs).push_back(in_queue);
	}

	for ( int k = 0; k < _app_to_box_mappings.size(); k++ )
	{
		printf("Application(%i) Queue(%i)\n",k, _app_to_box_mappings[k]->_app_arc);
		for ( int i = 0; i < _app_to_box_mappings[k]->_boxes.size(); i++ )
		{
			printf("    box: %i\n",_app_to_box_mappings[k]->_boxes[i]);
			printf("    arc: %i\n",_app_to_box_mappings[k]->_arcs[i]);
		}
	}
*/ //DON - REMOVE THIS (1) AND BEGINNING ONCE PROB BETWEEN FIXED

	//printf("GOT TO X 1\n");

	if ( _catalog->inTPBMode() )
		_max_num_worker_threads = _catalog->getNumBoxes();

	_thread_pool = new WorkerThread* [_max_num_worker_threads];
	for (size_t i = 0; i < _max_num_worker_threads; ++i)
	{
		  _thread_pool[i] = new WorkerThread(i,
											 _wait_cond,_wait_mutex,_tupleMon);

		  _thread_pool[i]->start(_execution_queue);
		}
}

Scheduler::~Scheduler()
{

  for (size_t i = 0; i < _max_num_worker_threads; ++i)
    {
      delete _thread_pool[i];
    }

  delete[] _thread_pool;

}

void Scheduler::initPropsFromFile(const PropsFile & props)
	throw (exception)
{
	_max_queue_size = props.getInt("Scheduler.max_queue_size");
	_max_num_worker_threads = props.getInt("Scheduler.max_num_worker_threads");

	abort();

	//----------------------------------------------------------------------------

	if (props.isStringPropDefined("Scheduler.AppScheduleType"))
		{
			string s = props.getString("Scheduler.AppScheduleType");
			if (s == "APP_RANDOM_TYPE")
				{
					setAppScheduleType(APP_RANDOM_TYPE);
				}
			else if (s == "APP_RR_TYPE")
				{
					setAppScheduleType(APP_RR_TYPE);
				}
			else if (s == "APP_SLOPE_SLACK_TYPE")
				{
					setAppScheduleType(APP_SLOPE_SLACK_TYPE);
				}
			else if (s == "APP_LQF_TYPE")
				{
					setAppScheduleType(APP_LQF_TYPE);
				}
			else if (s == "APP_")
				{
					setAppScheduleType(APP_LQF_TYPE);
				}
			else
				{
					throw SmException(__FILE__, __LINE__, "Illegal value in the property file for 'Scheduler.AppScheduleType' property");
				}
		}

	//----------------------------------------------------------------------------

	if (props.isStringPropDefined("Scheduler.BoxScheduleType"))
		{
			string s = props.getString("Scheduler.BoxScheduleType");
			if (s == "SLOPE_SLACK_TYPE")
				{
					setBoxScheduleType(SLOPE_SLACK_TYPE);
				}
			else if (s == "BOX_RANDOM_TYPE")
				{
					setBoxScheduleType(BOX_RANDOM_TYPE);
				}
			else if (s == "BOX_RR_TYPE")
				{
					setBoxScheduleType(BOX_RR_TYPE);
				}
			else if (s == "BOX_LQF_TYPE")
				{
					setBoxScheduleType(BOX_LQF_TYPE);
				}
			else
				{
					throw SmException(__FILE__, __LINE__, "Illegal value in the property file for 'Scheduler.BoxScheduleType' property");
				}
		}

	//----------------------------------------------------------------------------

	if (props.isStringPropDefined("Scheduler.BoxTraversalType"))
		{
			string s = props.getString("Scheduler.BoxTraversalType");
			if (s == "MIN_COST")
				{
					setBoxTraversalType(MIN_COST);
				}
			else if (s == "MIN_LATENCY")
				{
					setBoxTraversalType(MIN_LATENCY);
				}
			else if (s == "MIN_MEMORY")
				{
					setBoxTraversalType(MIN_MEMORY);
				}
			else
				{
					throw SmException(__FILE__, __LINE__, "Illegal value in the property file for 'Scheduler.BoxTraversalType' property");
				}
		}

	//----------------------------------------------------------------------------

	if (props.isStringPropDefined("Scheduler.SchedBy"))
		{
			string s = props.getString("Scheduler.SchedBy");
			if (s == "SCHED_BY_APP")
				{
					setSchedBy(SCHED_BY_APP);
				}
			else if (s == "SCHED_BY_BOX")
				{
					setSchedBy(SCHED_BY_BOX);
				}
			else
				{
					throw SmException(__FILE__, __LINE__, "Illegal value in the property file for 'Scheduler.SchedBy' property");
				}
		}

	//----------------------------------------------------------------------------

	if (props.isIntPropDefined("Scheduler.TopKSpanner"))
		{
			setTopKSpanner(props.getInt("Scheduler.TopKSpanner"));
		}

	//----------------------------------------------------------------------------

	if (props.isIntPropDefined("Scheduler.FixedTrainSize"))
		{
			setFixedTrainSize(props.getInt("Scheduler.FixedTrainSize"));
		}

	//----------------------------------------------------------------------------

	bool doSetCost = props.isDoubleDefined("Scheduler.per_box_cost");
	bool doSetSel  = props.isDoubleDefined("Scheduler.per_box_selectivity");
	bool _thread_per_box_mode=props.getBoolWithDefault
		("Scheduler.one_thread_per_box", false);
	if ( _thread_per_box_mode )
		_catalog->initTPBSignalConditions();
	
	if (doSetCost || doSetSel)
	{
		double costVal;
		if (doSetCost)
		{
			costVal = props.getDouble("Scheduler.per_box_cost");
			//cout << "SETTING cost val to? " << props.getDouble("Scheduler.per_box_cost");
			if (costVal < 0)
			{
				throw SmException(__FILE__, __LINE__, "Property \"Scheduler."
								  "per_box_cost\" has a negative value");
			}
		}

		double selVal;
		if (doSetSel)
		{
			selVal = props.getDouble("Scheduler.per_box_selectivity");
			if (selVal < 0)
			{
				throw SmException(__FILE__, __LINE__, "Property \"Scheduler."
								  "per_box_selectivity\" has a negative value");
			}
		}

		for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
		{
			Boxes *b = _catalog->getBox(i);
			if ( b != NULL )
			{
				if (doSetCost)
				{
					b->setCost(costVal);
				}

				if (doSetSel)
				{
					b->setSelectivity(selVal);
				}
				//printf("b->setSelectivity: %f\n",b->getSelectivity());
			}
		}
	}
}

void Scheduler::generateAppBoxNodeMap()
{
	//map<int,AppNode*> *app_node_map = new map<int,AppNode*>;
	for ( int i = 0; i < _application_trees.size(); i++)
	{
		map<int,AppNode*> *app_node_map = new map<int,AppNode*>;
		//printf("ABNM[%i]:",i);
		traverse_loadAppBoxNodeMap(_application_trees[i], app_node_map);
		_app_box_node_v.push_back(app_node_map);
		//printf("\n");
	}
	/*
	for ( int i = 0; i < 20; i++ )
	{
		if ( (*_app_box_node_v[0]).find(i) != (*_app_box_node_v[0]).end() )
			printf("ABNM found box %i for app %i\n",i,0);
		if ( (*_app_box_node_v[1]).find(i) != (*_app_box_node_v[1]).end() )
			printf("ABNM found box %i for app %i\n",i,1);
	}
	map<int,AppNode*>::iterator iter;
	printf("ABNM traversing abnms\n");
	for ( iter = (*_app_box_node_v[0]).begin(); iter != (*_app_box_node_v[0]).end(); iter++)
			printf("ABNM found box %i for app %i\n",(*iter).first,0);
	for ( iter = (*_app_box_node_v[1]).begin(); iter != (*_app_box_node_v[1]).end(); iter++)
			printf("ABNM found box %i for app %i\n",(*iter).first,1);
	*/

}
void Scheduler::traverse_loadAppBoxNodeMap(AppNode *node, map<int,AppNode*> *app_node_map)
{
	for ( int i = 0; i < (node->_input_boxes)->size(); i++ )
	{
		if ((*(node->_input_boxes))[i] != NULL )
			traverse_loadAppBoxNodeMap((*(node->_input_boxes))[i], app_node_map);
	}
	//printf("(%i),",node->_box_id);
	(*app_node_map)[node->_box_id] = node;
}
double Scheduler::getToRootCost(int box_id)
{
	multimap<int,int>::iterator iter;
	double max_cost = 0.0;
	for ( iter = _bam.lower_bound(box_id);
			iter != _bam.upper_bound(box_id);
			iter++ )
	{
		double cost;
		BoxTraversal bt;
		AppNode *a = (*_app_box_node_v[(*iter).second])[box_id];
		cost = toRootTraversal(a,&bt._traversal,FULL_COST);
		if ( cost > max_cost )
			max_cost = cost;
	}
	Boxes *b;
	b = _catalog->getBox(box_id);
	b->getCost();
	return (max_cost - b->getCost());
	//return max_cost;

}

void Scheduler::loadBoxQosGraphs()
{
	// this function will load _box_qos_graphs with the average qos graphs
	// for each box (an average qos graph is the average of all the
	// qos graphs of the applications a box serves)
	
	for ( int k = 0; k <= _catalog->getMaxBoxId(); k++ )
	{
		qos_struct *qs = new qos_struct;
		multimap<int,int>::iterator mm_iter;
		if(_bqm[k]._box_id >= 0)
		{
			set<double> x_set;
			// for each application using this box
			for ( mm_iter = _bam.lower_bound(k);
					mm_iter != _bam.upper_bound(k);
					mm_iter++)
			{
				// find unique x values from qos graphs
				for ( int i = 0; i < _qos_graphs[(*mm_iter).second]._points.size(); i++ )
					x_set.insert(_qos_graphs[(*mm_iter).second]._points[i]->_x);

			}
			double to_root_cost = getToRootCost(k);
//fprintf(stderr,"to_root_cost(%i): %f\n",k,to_root_cost);
			set<double>::iterator s_iter;
			int x;
			if ( to_root_cost < 0.0 )
			{
				cerr << "[Scheduler] To root cost negative! Adjusting it \n";
				to_root_cost = 0.0;
			}
			// for each unique x value, find the average y
			for ( s_iter = x_set.begin(), x = 0;
					s_iter != x_set.end();
					s_iter++, x++ )
			{
				double total_y = 0.0;
				for ( mm_iter = _bam.lower_bound(k);
						mm_iter != _bam.upper_bound(k);
						mm_iter++)
				{
					//total_y += findYGivenXAndQos(x_val,(*mm_iter).second);
					total_y += findYGivenXAndQos(*s_iter,(*mm_iter).second);
				}
				////cout << " TO ROOT COST " << to_root_cost << " s_iter is " << *s_iter << endl << endl;
				double x_val = *s_iter - to_root_cost;
				if ( x_val <= 0.0 && x > 0)
					continue;
				if ( x_val < 0.0 )
					x_val = 0.0;
				qs->insertPoint(x_val,total_y/(double)_bam.count(k));

				//printf("BOX(%i) AVG QOS PT:(%f,%f)\n",k,x_val,total_y/(double)_bam.count(k));
			}
			if ( qs->_points.size() <= 0 )
			{
				// exiting was a bad, bad idea. This should notify of error
				// location
				assert( 0 );
				abort();
				exit(0);
			}
			//sort(qs->_points.begin(),qs->_points.end());
			(*_box_qos_graphs)[k] = qs;

		}
	}
	
}
void Scheduler::loadBoxAppMapping()
{
	for ( int i = 0; i < _application_trees.size(); i++ )
	{
		traverseAppTreeBAM(_application_trees[i],i);
	}
}
void Scheduler::traverseAppTreeBAM(AppNode *node,int app_num)
{
	for ( int i = 0; i < (node->_input_boxes)->size(); i++ )
	{
		if ((*(node->_input_boxes))[i] != NULL )
			traverseAppTreeBAM((*(node->_input_boxes))[i],app_num);
	}
	_bam.insert(pair<int,int>(node->_box_id,app_num));
}
void Scheduler::getPreOrderTraversals()
{
	vector<int> *po_traversal;
	for ( int i = 0; i < _application_trees.size(); i++ )
	{
		po_traversal = new vector<int>;
		traversePreOrder(_application_trees[i],i,po_traversal);
		_pre_order_traversals.push_back(po_traversal);
	}
}
void Scheduler::traversePreOrder(AppNode *node, int app_num, vector<int> *po_traversal)
{
	//if ( node->_node_type != ROOT_NODE )
		po_traversal->push_back(node->_box_id);
	for ( int i = 0; i < (node->_input_boxes)->size(); i++ )
	{
		if ((*(node->_input_boxes))[i] != NULL )
			traversePreOrder((*(node->_input_boxes))[i],app_num,po_traversal);
	}
}
void Scheduler::getPostOrderTraversals()
{
	//printf("GOT TO getPostOrderTraversals():\n");
	vector<int> *po_traversal;
	for ( int i = 0; i < _application_trees.size(); i++ )
	{
		//printf("GOT TO traversal[%i]:",i);
		po_traversal = new vector<int>;
		traversePostOrder(_application_trees[i],i,po_traversal);
		_post_order_traversals.push_back(po_traversal);
	}
	//printf("\n");
}
void Scheduler::traversePostOrder(AppNode *node, int app_num, vector<int> *po_traversal)
{
	for ( int i = 0; i < (node->_input_boxes)->size(); i++ )
	{
		if ((*(node->_input_boxes))[i] != NULL )
			traversePostOrder((*(node->_input_boxes))[i],app_num,po_traversal);
	}
	if ( node->_node_type != ROOT_NODE )
		po_traversal->push_back(node->_box_id);
	//printf(" %i,", node->_box_id);
}
void Scheduler::printApplicationTrees()
{
	printf("Application Trees (Post-Order)\n");
	printf("=============================\n");
	for ( int i = 0; i < _application_trees.size(); i++ )
	{
		printf("Application Trees (%i)",(_application_trees[i])->_box_id);
		traverseAppTreePrint(_application_trees[i]);
		printf("\n");
	}
}
void Scheduler::traverseAppTreePrint(AppNode *node)
{
	for ( int i = 0; i < (node->_input_boxes)->size(); i++ )
	{
		if ((*(node->_input_boxes))[i] != NULL )
			traverseAppTreePrint((*(node->_input_boxes))[i]);
	}
	printf(" %i",node->_box_id);
}
void Scheduler::loadApplicationTree( int curr_app, int arc_id )
{
	vector<BoxListObject> *box_list = new vector<BoxListObject>;
	int next_box_id = (_catalog->getArc(arc_id))->getSrcId();
	vector<int> *puq_v = NULL;
	vector<int> *upstream_queues_v = new vector<int>;
	AppNode *an = traverseApplicationTree(curr_app,next_box_id,arc_id,box_list,&puq_v);
	//printf("loadApplicationTree A\n");
	vector<AppNode*> *v = new vector<AppNode*>;
	v->push_back(an);
	for ( int j = 0; j < puq_v->size(); j++ )
	{
		upstream_queues_v->push_back((*puq_v)[j]);
	}
	(*_queue_to_upstream_queues_v[curr_app])[arc_id] = upstream_queues_v;
	AppNode *root = new AppNode((_catalog->getArc(arc_id))->getDestId(),ROOT_NODE,v,-1);
	//printf("loadApplicationTree B\n");
	BoxListObject blo;
	blo._box_id = an->_box_id;
	blo._node = root;
	box_list->push_back(blo);
	for ( int i = 0; i < v->size(); i++ )
		(*v)[i]->_parent = root;
	//printf("loadApplicationTree C\n");
	_application_trees.push_back(root);
}
AppNode* Scheduler::traverseApplicationTree(int curr_app, int box_id, int from_arc, vector<BoxListObject> *box_list, vector<int> **parent_upstream_queues)
{
	//	AppNode *node1 = NULL;
	//AppNode *node2 = NULL;

	//printf("XXX box_id: %i\n",box_id);
	vector<int> *upstream_queues_v = new vector<int>;

	int in_queue;
	int in_queue_src;
	vector<AppNode*> *v = new vector<AppNode*>;
	for ( int i = 0; i < _bqm[box_id]._input_queue_id.size(); i++ )
	{
		in_queue = _bqm[box_id]._input_queue_id[i];
		upstream_queues_v->push_back(in_queue);
		//printf("XXXAA adding to %i queue: %i\n",upstream_queues_v,in_queue);
		in_queue_src = (_catalog->getArc(in_queue))->getSrcId();
		if (_catalog->getBox(in_queue_src) != NULL && vectorFind(box_list,in_queue_src) != 1 )
		{
			vector<int> *puq_v = NULL; // parent upstream queues
			AppNode *a = traverseApplicationTree(curr_app,in_queue_src,in_queue,box_list,&puq_v);
			//printf("XXX B puq_v: %i\n",puq_v);
			//printf("traverseApplicationTree:pushing: %i\n",a->_box_id);
			v->push_back(a);
			BoxListObject blo;
			blo._box_id = a->_box_id;
			blo._node = a;
			box_list->push_back(blo);
			for ( int j = 0; j < puq_v->size(); j++ )
			{
				upstream_queues_v->push_back((*puq_v)[j]);
				//printf("XXXAB adding to %i queue: %i\n",upstream_queues_v,in_queue);
			}
			//(*_queue_to_upstream_queues_v[curr_app])[in_queue] = upstream_queues_v;
			(*_queue_to_upstream_queues_v[curr_app])[in_queue] = puq_v;
		}
		else
			v->push_back(NULL);
	}
	*parent_upstream_queues = upstream_queues_v;
	//printf("XXX A parent_upstream_queues: %i\n",*parent_upstream_queues);
	AppNode *an = new AppNode(box_id,BOX_NODE,v,from_arc);
	for ( int i = 0; i < v->size(); i++ )
	{
		if ( (*v)[i] != NULL )
			(*v)[i]->_parent = an;
	}
	return an;

}
int Scheduler::vectorFind(vector<BoxListObject> *v,int box_id)
{
	//printf("vectorFind::looking for: %i\n",box_id);
	for ( int i = 0; i < v->size(); i++ )
	{
		//printf("vectorFind::found: %i\n",(*v)[i]);
		if ( (*v)[i]._box_id == box_id )
		{
			//printf("vectorFind::A\n");
			return (1);
		}
	}
	//printf("vectorFind::B\n");
	return(0);
}
void Scheduler::start()
{
	//_beq_size = 100000;
	//_beq_size = 200;
	//_beq_size = 10;
	//_beq_size = 1;

	if ( _sched_by == SCHED_BY_APP )
	{
		if ( _beq_size > _app_to_box_mappings.size())
			 _beq_size = _app_to_box_mappings.size();
	}
	else
	{
		if ( _catalog->getNumBoxes() < _beq_size )
			_beq_size = _catalog->getNumBoxes();
	}


	int size;

	double total_time_refilling_queue = 0.0;
	timeval start_time;
	gettimeofday(&start_time,NULL);
	init_etime(&_first);
	double secs;
	secs = get_etime(&_first);
	int num_actual_scheduling_events = 0;
	while( waitForEvent() )
	{
		/*timeval now;
		gettimeofday(&now,NULL);
		printf("                                    Scheduler begins execution::  TIME: %d %d\n", now.tv_sec, now.tv_usec);*/

		_num_scheduling_events++;
		setPriorities();	// set box priorities
		
	    _execution_queue->lock();
	    size = _execution_queue->size();
		_execution_queue->unlock();

		if ( size < _beq_size )
		{
			int num_to_schedule = _beq_size - size;
			//if ( num_to_schedule > 0 )
			if ( num_to_schedule > (int)(_beq_size/2.0) )
			{
				double t1 = get_etime(&_first);
				int num_pushed;
				if ( (num_pushed = refillQueue(num_to_schedule)) > 0 )		// refill the box execution queue
				{
					num_actual_scheduling_events += num_pushed;
				}
				double t2 = get_etime(&_first);
				total_time_refilling_queue += (t2-t1);
			}
		}
		
		//printf("Total Scheduling Time: %lf seconds %i (%i) events\n",get_etime(&_first)-secs,_num_scheduling_events,num_actual_scheduling_events);
	    _execution_queue->lock();
	    size = _execution_queue->size();
		_execution_queue->unlock();
		//printf("GOT TO execution_queue size: %i\n",size);
	}
	timeval end_time;
	gettimeofday(&end_time,NULL);
	double total_running_time = end_time.tv_sec + (double)(start_time.tv_usec*1e-6)-
								start_time.tv_sec + (double)(start_time.tv_usec*1e-6);
	cout << " Scheduler, total running time " << total_running_time << endl;
	_measure->setTotalRunTime( total_running_time );
	for( int i = 0; i < _max_num_worker_threads; i++ )
	{
		_execution_queue->signal();
		int status;
		// wait for WorkerThread completions by using the Pthread join routine
		printf("joining WorkerThread[%i] \n",i);
		pthread_join(_thread_pool[i]->getThread(), (void **)&status);
		printf("WorkerThread[%i] finished\n",i);
	}
	//_measure->setNumSchedulingDecisions(_num_scheduling_events);
	_measure->setNumSchedulingDecisions(num_actual_scheduling_events);
	//_measure->setNumSchedulingDecisions(_num_beq_elements_pushed);
	double time_spent_scheduling = get_etime(&_first)-secs;
	_measure->setTimeSpentScheduling(time_spent_scheduling);

	if (_qos != NULL )
	{
		// calculate average delays.
		_measure->setAverageLatency(_app_v);
		// calculate average QoS.
		_measure->setAverageQoS(_app_v);
	}

	//printf("Scheduler Exiting...\n");

	//dpc 
	//printf("Scheduler total_time_refilling_queue: %f\n", total_time_refilling_queue);
	//dpc printf("Scheduler: _time_spent_copying_runnable_boxes: %f\n", _time_spent_copying_runnable_boxes);
	//dpc printf("Scheduler: _time_spent_verifying_lockable_boxes: %f\n", _time_spent_verifying_lockable_boxes);
	//dpc printf("Scheduler: _time_spent_loading_candidate_boxes: %f\n", _time_spent_loading_candidate_boxes);
	//dpc printf("Scheduler: _time_spent_loading_execution_queue: %f\n", _time_spent_loading_execution_queue);
	//dpc printf("Scheduler: cumulative: %f\n",_time_spent_copying_runnable_boxes +
    //dpc                                      _time_spent_verifying_lockable_boxes +
    //dpc                                      _time_spent_loading_candidate_boxes +
    //dpc                                      _time_spent_loading_execution_queue);
	//dpc printf("Scheduler: _time_spent_traverse_loadqueue: %f\n",_time_spent_traverse_loadqueue);
	//dpc printf("			_time_spent_loading_dequeue_pin_lists: %f\n", _time_spent_loading_dequeue_pin_lists);
	//dpc printf("				_time_spent_shm_get_num_records: %f\n", _time_spent_shm_get_num_records);
	//dpc printf("			_time_spent_insert_locked_box_set: %f\n", _time_spent_insert_locked_box_set);
	//dpc printf("			_time_spent_loading_pin_lists: %f\n", _time_spent_loading_pin_lists);
	//dpc printf("			_time_spent_loadBox: %f\n", _time_spent_loadBox);
	//dpc printf("Scheduler: _time_spent_create_boxlist: %f\n", _time_spent_create_boxlist);
	//dpc printf("Scheduler: _time_spent_loading_execution_queue_app: %f\n", _time_spent_loading_execution_queue_app);
	//dpc printf("SCHEDULER: time_spent_scheduling: %f\n",time_spent_scheduling);
	//dpc printf("SCHEDULER: _time_spent_findEligibleBox_Bucketing: %f\n",_time_spent_findEligibleBox_Bucketing);
    //dpc printf("SCHEDULER: _time_spent_Bucketing_removeFromGrid %f\n",_time_spent_Bucketing_removeFromGrid);
    //dpc printf("SCHEDULER: _time_spent_Bucketing_finding_candidates %f\n",_time_spent_Bucketing_finding_candidates);
    //dpc printf("SCHEDULER: _time_spent_Bucketing_finding_schedulable %f\n",_time_spent_Bucketing_finding_schedulable);

	pthread_mutex_lock(__global_stop.getSchedFinishedMutex());
	pthread_cond_broadcast(__global_stop.getSchedFinishedWaitCond());
	pthread_mutex_unlock(__global_stop.getSchedFinishedMutex());
}

int Scheduler::waitForEvent()
{
	// This function waits for one of three possible events
	//		 1. a timeout .. the scheduler will periodically
	//			reset priorities
	//		2. the box execution queue running low
	//		3. a signal that an ad-hoc query has been initiated

	//  Eddie adds support for the queue monitor: busy-wait (for now) if queue monitor indicates we must pause
	if (_queue_monitor != NULL) {
		while (!_queue_monitor->isRunning()) sleep(1);
	}

	int ret_val = 1, x = 0;
	struct timespec to;
	memset(&to, 0, sizeof to);
	//to.tv_sec = time(0) + 5; // 1 seconds from now
	//to.tv_sec = time(0) + 1; // 1 seconds from now

	//to.tv_sec = time(0); // 1 seconds from now
	//to.tv_nsec = 1000000000; // 1 second
	//to.tv_nsec = 0; // 0 second
	timeval t_now;
	gettimeofday(&t_now,NULL);
	to.tv_sec = t_now.tv_sec + 2;
	to.tv_nsec = t_now.tv_usec * 1000;
	//to.tv_nsec = t_now.tv_usec;

	if ( _execution_queue->size() == 0 && __box_work_set.size() > 0 && _candidates_v.size() > 0)
	{
		//printf("Scheduler not waiting since there are candidate boxes to schedule\n");
	}
	else
	{
		timeval tv;
		gettimeofday(&tv,NULL);
		//printf("Scheduler waiting at %i %i _execution_queue->size(): %i __box_work_set.size(): %i _candidates_v.size(): %i\n",tv.tv_sec,tv.tv_usec,_execution_queue->size(),__box_work_set.size(),_candidates_v.size());
		if ( _catalog->getNumBoxes() == _num_beq_elements_pushed &&
			 _catalog->inTPBMode() )
		{
			cout << " Scheduler going to sleep for a while, all boxes have been assigned." << endl;
			to.tv_sec += 10;
		}

		pthread_mutex_lock(_wait_mutex);
		int x = pthread_cond_timedwait(_wait_cond, _wait_mutex, &to);
		pthread_mutex_unlock(_wait_mutex);
		if ( x == ETIMEDOUT )
		{
			timeval t_now1;
			gettimeofday(&t_now1,NULL);
			int gettime_now = t_now1.tv_sec;
			 //printf("[Scheduler] ETIMEDOUT - Scheduler awoke at %i %i (normally) to reschedule num runnable boxes: %i\n",t_now1.tv_sec,t_now1.tv_usec,__box_work_set.size());
		}
		else
		{
			timeval tv;
			gettimeofday(&tv,NULL);
			//printf("Scheduler received signal at %i %i\n",tv.tv_sec,tv.tv_usec);
		}
		
		pthread_cond_t *st_wake_cond;
		pthread_mutex_t *st_wake_mutex;
		st_wake_mutex = (_measure->getStreamThread())->getWakeMutex();
		st_wake_cond = (_measure->getStreamThread())->getWakeCondition();
		//signal StreamThread
		pthread_mutex_lock(st_wake_mutex);
		pthread_cond_broadcast(st_wake_cond);
		pthread_mutex_unlock(st_wake_mutex);
		
	}

	_measure->testStopCond();
	if ( __global_stop.getStop() == true )
		ret_val = 0;

	return ret_val;
}
void Scheduler::setPriorities()
{
	// set box priorities based on some algorithm
}

int  Scheduler::refillQueue(int num_to_schedule)
{
	// refill the box execution queue
	// randomly select boxes from shared memory
	// which have enough records to execute.

	static int rq_count = 0;

	rq_count++;
	//pthread_mutex_lock(&__box_work_set_mutex);
	//fprintf(stderr,"rq_count: %i  __box_work_set.size(): %i\n",rq_count,__box_work_set.size());
	//pthread_mutex_unlock(&__box_work_set_mutex);

	
	int num_pushed = 0;
	
	int sched_by = _sched_by;
	int box_sched_type = _box_schedule_type;
	int app_sched_type = _app_schedule_type;

	static int apps_scheduled = 0;
	

	if ( sched_by == SCHED_BY_APP )
	{
		for ( int i = 0; i < num_to_schedule; i++ )
		{
			QueueElement *qe = new QueueElement();
			int temp_id;
			//pthread_mutex_lock(_app_schedule_mutex);

			if ( app_sched_type == APP_RR_TYPE )
			{
				temp_id = findEligibleApplication_RR();
			}
			else if ( app_sched_type == APP_RANDOM_TYPE )
				temp_id = findEligibleApplication_Random();
			else if ( app_sched_type == APP_SLOPE_SLACK_TYPE )
				temp_id = findEligibleApplication_BoxLatency();
			else if ( app_sched_type == APP_BUCKETING_TYPE )
				temp_id = findEligibleApplication_Bucketing();
			else if ( app_sched_type == APP_LQF_TYPE )
				temp_id = findEligibleApplication_LQF();
			else
			{
				fprintf(stderr,"BAD app schedule type\n");
				exit(0);
				assert( false );
			}

			//assert ( temp_id >=0 && temp_id < _application_trees.size());

			if ( temp_id >= 0 )
				assert(temp_id < _application_trees.size());
			if (  temp_id >=0 && temp_id < _application_trees.size() )
			{
				qe->_app_mutex = _app_mutexes[temp_id];
				qe->_app_num = temp_id;
				apps_scheduled++;
				map<int,int> *queue_tuples_map = new map<int,int>;
				set<int> *locked_boxes_set = new set<int>;
				BoxList_T *bl = qe->getBoxList();
				double t_q_start = get_etime(&_first);
				if ( _top_k_spanner_flag == true )
				{
					vector<int> *top_k_boxes_v;
					top_k_boxes_v = getTopKBoxes(temp_id,_K);
					if ( top_k_boxes_v->size() <= 0 )
					{
						pthread_mutex_unlock(_app_mutexes[temp_id]);
						continue;
					}
					
					
					createTopKSpannerBoxList( temp_id, _K, top_k_boxes_v, locked_boxes_set,qe);
				}
				else
				{
					AppNode *start_node = (*(_application_trees[temp_id]->_input_boxes))[0];

					int out_tuples = 0;
					out_tuples = traverse_LoadQueue(start_node, start_node->_out_arc_id, qe, bl,_application_trees[temp_id], queue_tuples_map, locked_boxes_set);
					assert (out_tuples != 0);
				}
				double t_q_stop = get_etime(&_first);
				_time_spent_traverse_loadqueue += t_q_stop - t_q_start;
				double t_r_start = get_etime(&_first);
				switch(_traversal_type)
				{
					case MIN_COST:
						createMinCostBoxList(_post_order_traversals[temp_id],bl,locked_boxes_set);
						break;
					case MIN_LATENCY:
						if ( _top_k_spanner_flag == false )
							loadMinLatencyTraversal(temp_id,qe,queue_tuples_map,locked_boxes_set);
						createMinLatencyBoxList(_ML_traversals[temp_id],bl,locked_boxes_set);
						break;
					case MIN_MEMORY:
						createMinMemoryBoxList(_MM_traversals[temp_id],bl,locked_boxes_set);
						break;
					default:
						fprintf(stderr,"ERROR: Bad Traversal Type\n");
						exit(0);
						assert( false );
				}
				//pthread_mutex_unlock(_app_schedule_mutex);
				double t_r_stop = get_etime(&_first);
				_time_spent_create_boxlist += t_r_stop - t_r_start;

			}
			BoxList_T *bl = qe->getBoxList();
			//BoxList_T::iterator iter;
			if ( temp_id >= 0 )
				assert(bl->size() > 0);
			if ( temp_id < 0 )
				assert(bl->size() == 0);
				
			double t_s_start = get_etime(&_first);
			if(bl->size() > 0)
			{
				//fprintf(stderr,"scheduling box list\n");
				_execution_queue->lock();
				//printf(" push box here%d\n", (*((qe->getBoxList())->begin()))->_boxId );
				/*timeval now;
				gettimeofday(&now,NULL);
				printf("                                    Scheduler SIGNALS exec queue::  TIME: %d %d\n", now.tv_sec, now.tv_usec);
				*/
				_execution_queue->push(qe);
				_execution_queue->signal();
				_execution_queue->unlock();
				_num_beq_elements_pushed++;
				num_pushed++;
			}
			else
			{
				//fprintf(stderr,"xxxxx\n");
				delete qe;
			}
			double t_s_stop = get_etime(&_first);
			_time_spent_loading_execution_queue_app += t_s_stop - t_s_start;
		}
	}
	else
	{
		int temp_id;
		//pthread_mutex_lock(_box_schedule_mutex);
		if (box_sched_type == SLOPE_SLACK_TYPE)
			temp_id = findEligibleBox_SlopeSlack();
		else if (box_sched_type == BOX_BUCKETING_TYPE)
		{
			double t_a_start = get_etime(&_first);
			temp_id = findEligibleBox_Bucketing1(num_to_schedule);
			//temp_id = findEligibleBox_Bucketing(num_to_schedule);
			double t_a_stop = get_etime(&_first);
			_time_spent_findEligibleBox_Bucketing += t_a_stop - t_a_start;
		}
		else if (box_sched_type == BOX_RANDOM_TYPE)
			temp_id = findEligibleBox_Random();
		else if (box_sched_type == BOX_RR_TYPE)
			temp_id = findEligibleBox_RR();
		else if (box_sched_type == BOX_LQF_TYPE)
			temp_id = findEligibleBox_LQF();
		else
		{
			fprintf(stderr,"BAD box schedule type\n");
			assert( false );
			exit(0);
		}

		//checkAppForTuples(0);
		//testBoxWorkSet();
		if ( temp_id != -1 )
		{
			double t_a_start = get_etime(&_first);
			for ( int i = 0; i < num_to_schedule && i < _candidates_v.size(); i++ )
			{
				QueueElement *qe = new QueueElement();
				//cout << " Considering candidate " << _candidates_v[i]._box_id << " num to sched? " << num_to_schedule << " size " << _candidates_v.size() << endl;
				loadSingleBox(_candidates_v[i]._box_id,qe);
				BoxList_T *bl = qe->getBoxList();
				//printBoxList(bl);
				if(bl->size() > 0)
				{
					_execution_queue->lock();
					//printf(" push box %d AND signal\n", (*((qe->getBoxList())->begin()))->_boxId );
					_execution_queue->push(qe);
					_execution_queue->signal();
					_execution_queue->unlock();
					_num_beq_elements_pushed++;
					num_pushed++;
				}
			}
			double t_a_stop = get_etime(&_first);
			_time_spent_loading_execution_queue += t_a_stop - t_a_start;
		}
	}
	return num_pushed;
						
}
void Scheduler::printBoxList(BoxList_T* bl)
{
  return;
	BoxList_T::iterator iter;
	printf("B BoxList: ");
	for ( iter = bl->begin(); iter != bl->end(); iter++ )
	{
		int box_id = (*iter)->_boxId;
		printf("%i, ",box_id);
		printf("[");
		multimap<int,int>::iterator iter;
		for ( iter = _bam.lower_bound(box_id);
			iter != _bam.upper_bound(box_id);
			iter++ )
		{
			printf("%i,",(*iter).second);
		}
		printf("]");
	
	}
	printf("\n");
}

void Scheduler::loadSingleBox(int box_id,QueueElement *qe)
{
	int max_train_size = 0;
	Boxes *b = _catalog->getBox(box_id);
	if ( b->lockBoxTry() == 0 )
	{
	//printf("LOCKED ......................................... box: %i\n",box_id);
		queue_allocation qa;
		int total_input_tuples = 0;
		for ( int i = 0; i < _bqm[box_id]._num_input_queues; i++ )
		{
			qa._arc_id = _bqm[box_id]._input_queue_id[i];
			qa._num_tuples = shm_get_num_records_in_queue(_bqm[box_id]._input_queue_id[i]);
			if (qa._num_tuples > 0)
			{

				if ( (_fixed_train_size * qa._num_tuples) < 1.0 )
					qa._num_tuples = 1;
				else
					qa._num_tuples = (int)(_fixed_train_size * qa._num_tuples);
			}
			if ( qa._num_tuples > max_train_size )
				max_train_size = qa._num_tuples;

			qe->_dequeuePin_list.push_back(qa);
			shm_set_state(qa._arc_id,1);
			total_input_tuples += qa._num_tuples;// should use selectivity instead
		}
		for ( int i = 0; i < _bqm[box_id]._num_output_queues; i++ )
		{
			qa._arc_id = _bqm[box_id]._output_queue_id[i];
			qa._num_tuples = total_input_tuples + shm_get_num_records_in_queue(_bqm[box_id]._output_queue_id[i]);
			if ( (b->getType() == AGGREGATE_BOX) ||
			     (b->getType() == JOIN_BOX) ||
			     (b->getType() == LR_READ_RELATION_BOX) ||
			     (b->getType() == RESAMPLE_BOX) ) qa._num_tuples*=30;

			    //(b->getType() == TUMBLE_BOX) ||
			    //(b->getType() == WSORT_BOX) || 
			    //(b->getType() == SLIDE_BOX) ||
			    //(b->getType() == XSECTION_BOX))
			    
			qe->_enqueuePin_list.push_back(qa);
			shm_set_state(qa._arc_id,1);
		}
		QBox *temp_box;
		//int train_size = total_input_tuples;
		int train_size = max_train_size;
		temp_box = loadBox(box_id,train_size);
		BoxList_T *bl = qe->getBoxList();
		bl->push_back(temp_box);
	}
}
vector<int>* Scheduler::getTopKBoxes(int app,int k)
{
	int bucket_size;
	if ( _use_buckets_flag == true )
	{
		bucket_size = _bucket_size;
	}
	else
		bucket_size = _candidates_v.size();
	vector<int> *top_k_boxes_v = new vector<int>;
	int count = 0;
	for ( int i = 0; i < bucket_size; i++ )
	{
		int box_id = _candidates_v[i]._box_id;
		if ( (*_app_box_node_v[app]).find(box_id) != (*_app_box_node_v[app]).end() )
		{
			// found box in application
			if ( checkPathToRootUnlocked(app,box_id) == true)
			{
				top_k_boxes_v->push_back(box_id);
				count++;
			}
		}
		if ( count >= k )
			break;
	}

	return top_k_boxes_v;
}
bool Scheduler::checkPathToRootUnlocked(int app, int box_id)
{
	//verify that all boxes in path to root are not locked
	bool test = true;
	AppNode *curr_node = (*_app_box_node_v[app])[box_id];
	while( curr_node->_node_type != ROOT_NODE )
	{
		Boxes *b = _catalog->getBox(curr_node->_box_id);
		if ( b->lockBoxTry() == 0 )
		{
			b->unlockBox();
		}
		else
		{
			test = false;
			break;
		}
		curr_node = curr_node->_parent;
	}
	return test;

}
void Scheduler::createTopKSpannerBoxList( int app, int k, vector<int> *top_k_boxes, set<int> *locked_boxes_set,QueueElement *qe)
{
	set<int> local_box_set;
	set<int> leaf_set;
	// put all boxes int top-k spanner in to local_box_set
	// figure out which boxes are leaves and put leaf boxes in leaf_set
	for ( int i = 0; i < top_k_boxes->size(); i++ )
	{
		int box_id = (*top_k_boxes)[i];
		local_box_set.insert(box_id);
		leaf_set.insert(box_id);

		AppNode *curr_node = (*_app_box_node_v[app])[box_id];
		while( curr_node->_parent->_node_type != ROOT_NODE )
		{
			AppNode *parent = curr_node->_parent;
			if ( leaf_set.find(parent->_box_id) != leaf_set.end() )
			{
				leaf_set.erase(parent->_box_id);
			}
			local_box_set.insert(parent->_box_id);
			curr_node = parent;
		}
	}

	set<int>::iterator iter;

	//lock all the boxes in local_box_set
	for ( iter = local_box_set.begin(); iter != local_box_set.end(); iter++ )
	{
		Boxes *b = _catalog->getBox(*iter);
		if ( b->lockBoxTry() != 0 )
		{
			fprintf(stderr,"ERROR in createTopKSpannerBoxList()\n");
			assert(false); // assertion should never happen since all boxes should be available
			exit(0);
		}
		(*locked_boxes_set).insert(*iter);
	}
	
	set<int> dequeuePin_set;
	set<int> enqueuePin_set;
	set<int> dequeueEnqueuePin_set;
	map<int,int> queue_sizes;
	//calculate queue sizes starting at leaves
	for ( iter = leaf_set.begin(); iter != leaf_set.end(); iter++ )
	{
		int curr_leaf = *iter;
		int cumulative_path_tuples = 0;
		for ( int i = 0; i < _bqm[curr_leaf]._num_input_queues; i++ )
		{
			int q_id = _bqm[curr_leaf]._input_queue_id[i];
			dequeuePin_set.insert(q_id);
			int num_tuples = shm_get_num_records_in_queue(q_id);
			queue_sizes[q_id] = num_tuples;
			cumulative_path_tuples += num_tuples;
		}

		AppNode *curr_node = (*_app_box_node_v[app])[curr_leaf];
		while( curr_node->_parent->_node_type != ROOT_NODE )
		{
			// add out arc to dequeueEnqueuePinList
			int q_id = curr_node->_out_arc_id;
			dequeueEnqueuePin_set.insert(q_id);
			int num_tuples = 0;
			if ( queue_sizes.find(q_id) == queue_sizes.end() )
			{
				num_tuples = shm_get_num_records_in_queue(q_id);
				queue_sizes[q_id] = num_tuples;
				cumulative_path_tuples += num_tuples;
			}
			queue_sizes[q_id] += (cumulative_path_tuples-num_tuples);

			// add other ouput arcs to enqueuePinList
			for ( int i = 0; i < _bqm[curr_node->_box_id]._num_output_queues; i++ )
			{
				if ( _bqm[curr_node->_box_id]._output_queue_id[i] != curr_node->_out_arc_id )
				{
					int q_id = _bqm[curr_node->_box_id]._output_queue_id[i];
					enqueuePin_set.insert(q_id);
					int num_tuples = 0;
					if ( queue_sizes.find(q_id) == queue_sizes.end() )
					{
						num_tuples = shm_get_num_records_in_queue(q_id);
						queue_sizes[q_id] = num_tuples;
					}
					queue_sizes[q_id] += (cumulative_path_tuples-num_tuples);
				}
			}
			curr_node = curr_node->_parent;
		}
		assert( curr_node->_parent->_node_type == ROOT_NODE );
		for ( int i = 0; i < _bqm[curr_node->_box_id]._num_output_queues; i++ )
		{
			int q_id = _bqm[curr_node->_box_id]._output_queue_id[i];
			enqueuePin_set.insert(q_id);
			int num_tuples = 0;
			if ( queue_sizes.find(q_id) == queue_sizes.end() )
			{
				num_tuples = shm_get_num_records_in_queue(q_id);
				queue_sizes[q_id] = num_tuples;
			}
			queue_sizes[q_id] += (cumulative_path_tuples-num_tuples);
		}
	}

	createPinLists(qe,&dequeuePin_set,&dequeueEnqueuePin_set,&enqueuePin_set,&queue_sizes);
}
void Scheduler::createPinLists(QueueElement *qe, set<int> *dequeuePinList, set<int> *dequeueEnqueuePinList, set<int> *enqueuePinList, map<int,int> *queue_sizes)
{
	set<int>::iterator iter;

	timeval t_now;
	double time_now;
	gettimeofday(&t_now,NULL);
	time_now = t_now.tv_sec + (t_now.tv_usec*1e-06);
	double total_latency = 0.0;
	for ( iter = dequeuePinList->begin(); iter != dequeuePinList->end(); iter++ )
	{
		queue_allocation qa;
		qa._arc_id = *iter;
		qa._num_tuples = (*queue_sizes)[qa._arc_id];
		qe->_dequeuePin_list.push_back(qa);
		shm_set_state(qa._arc_id,1);
		total_latency += (time_now - shm_get_average_timestamp(qa._arc_id));
	}
	//fprintf(stderr,"dequeuePinList: average latency: %f\n",total_latency/dequeuePinList->size()); 

	for ( iter = dequeueEnqueuePinList->begin(); iter != dequeueEnqueuePinList->end(); iter++ )
	{
		queue_allocation qa;
		qa._arc_id = *iter;
		qa._num_tuples = (*queue_sizes)[qa._arc_id];
		qe->_dequeueEnqueuePin_list.push_back(qa);
		shm_set_state(qa._arc_id,1);
	}
	for ( iter = enqueuePinList->begin(); iter != enqueuePinList->end(); iter++ )
	{
		queue_allocation qa;
		qa._arc_id = *iter;
		qa._num_tuples = (*queue_sizes)[qa._arc_id];
		qe->_enqueuePin_list.push_back(qa);
		shm_set_state(qa._arc_id,1);
	}
}
/*
void Scheduler::createTopKSpannerBoxList( int app, int k, vector<int> to_root_path_v, vector<int> *traversal_v, BoxList_T *bl, set<int> *locked_boxes_set)
{
	QBox *temp_box;
	
	if ( k > locked_boxes_set->size()) 
	{
		bl->empty();
		bl->erase(bl->begin(),bl->end());
		// load box traversal
		for ( int i = 0; i < (*traversal_v).size(); i++ )
		{
			int box_id = (*traversal_v)[i];
			if ( locked_boxes_set->find( box_id ) != locked_boxes_set->end() )
			{
				int train_size = 0; // this value is reset by the WorkerThread based on the number of resident tuples.
				temp_box = loadBox(box_id,train_size);
				bl->push_back(temp_box);

			}
		}
		return;
	}
	int count = 0;
	vector<int> local_box_priority_v;
	for ( int i = 0; i < _candidates_v.size(); i++ )
	{
		if ( locked_boxes_set->find(_candidates_v[i]._box_id) != locked_boxes_set->end() )
		{
			local_box_priority_v.push_back(_candidates[i]._box_id);
			count++;
		}
		if ( count >= k )
			break;
	}
	set<int> local_app_set;
	for ( int i = 0; i < local_box_priority_v.size(); i++ )
	{
		BoxTraversal bt;
		int box_id = local_box_priority_v[i];
		toRootTraversal((*_app_box_node_v[app])[box_id],&bt._traversal,PARTIAL_COST);
		for ( int j = 0; j < bt._traversal.size(); j++ )
		{
			local_app_set.insert(bt._traversal[j]);
		}
	}
	for ( int i = 0; i < bl->size(); i++ )
	{
		Boxes *b;
		if ( local_app_set.find(bl[i]) == local_app_set.end() )
		{
			b = _catalog->getBox(bl[i]);
			b->unlockBox();
			locked_boxes_set.erase(bl[i]);
		}
	}
	new vector<int> new_traversal;
	!!!
	for ( int i = 0; i < traversal_v->size(); i++ )
	{
		if ( local_app_set.find(traversal_v[i]) != local_app_set.end() )
			new_traversal.push_back(;
	}

	for each box in traversal_v
	{
		if box is in local_app_set
			new_traversal.push_back(box)
	}

	bl->erase(bl->begin(),bl->end());
	for ( int 0 = 0; i < new_traversal.size(); i++ )
	{
		int train_size = 0;
		tmp_box = loadBox(new_traversal[i],train_size);
		bl->push_back(temp_box);
	}
		
}
*/
void Scheduler::createMinCostBoxList( vector<int> *traversal_v, BoxList_T *bl, set<int> *locked_boxes_set)
{
	QBox *temp_box;
	
	bl->empty();
	bl->erase(bl->begin(),bl->end());
	// load box traversal
	for ( int i = 0; i < (*traversal_v).size(); i++ )
	{
		int box_id = (*traversal_v)[i];
		if ( locked_boxes_set->find( box_id ) != locked_boxes_set->end() )
		{
			int train_size = 0; // this value is reset by the WorkerThread based on the number of resident tuples.
			temp_box = loadBox(box_id,train_size);
			bl->push_back(temp_box);
		}
	}
}
void Scheduler::createMinLatencyBoxList( vector<int> *traversal_v, BoxList_T *bl, set<int> *locked_boxes_set)
{
	QBox *temp_box;
	
	bl->empty();
	bl->erase(bl->begin(),bl->end());
	// load box traversal
	for ( int i = 0; i < (*traversal_v).size(); i++ )
	{
		int box_id = (*traversal_v)[i];
		if ( locked_boxes_set->find( box_id ) != locked_boxes_set->end() )
		{
			int train_size = 0; // this value is reset by the WorkerThread based on the number of resident tuples.
			temp_box = loadBox(box_id,train_size);
			bl->push_back(temp_box);

		}
	}
}
void Scheduler::createMinMemoryBoxList( vector<int> *traversal_v, BoxList_T *bl, set<int> *locked_boxes_set)
{
	QBox *temp_box;
	
	bl->empty();
	bl->erase(bl->begin(),bl->end());
	// load box traversal
	for ( int i = 0; i < (*traversal_v).size(); i++ )
	{
		int box_id = (*traversal_v)[i];
		if ( locked_boxes_set->find( box_id ) != locked_boxes_set->end() )
		{
			int train_size = 0; // this value is reset by the WorkerThread based on the number of resident tuples.
			temp_box = loadBox(box_id,train_size);
			bl->push_back(temp_box);
		}
	}
}
//void Scheduler::loadMinLatencyTraversal(int app_num, QueueElement *qe, map<int,int> **queue_tuples_map, set<int> **locked_boxes_set )
void Scheduler::loadMinLatencyTraversal(int app_num, QueueElement *qe, map<int,int> *queue_tuples_map, set<int> *locked_boxes_set)
{
	//map<int,int> *qt_map = new map<int,int>;
	//*queue_tuples_map = new map<int,int>;
	//*locked_boxes_set = new set<int>;
	//map<int,int> *queue_tuples_map = new map<int,int>;
	//set<int> *locked_boxes_set = new set<int>;


	for ( int i = 0; i < qe->_dequeueEnqueuePin_list.size(); i++ )
	{
		// load queue dequeueEnqueuePin allocations
		vector<int> *q_v;
		q_v = (*_queue_to_upstream_queues_v[app_num])[qe->_dequeueEnqueuePin_list[i]._arc_id];
		int max_upstream_queue_size = -1;
		for ( int z = 0; z < q_v->size(); z++ )
		{
			int q_id = (*q_v)[z];
			int q_size = (*queue_tuples_map)[q_id];
			if ( q_size > max_upstream_queue_size)
				max_upstream_queue_size = q_size;
		}
		
	}



}
int Scheduler::traverse_LoadQueue(AppNode* node, int out_arc, QueueElement *qe, BoxList_T *bl, AppNode *parent, map<int,int> *queue_tuples_map, set<int> *locked_boxes_set)
{
	if ( node == NULL )
	{
		int output_tuples = 0;
		output_tuples = shm_get_num_records_in_queue(out_arc);
		(*queue_tuples_map)[out_arc] = output_tuples;
		queue_allocation qa;
		qa._arc_id = out_arc;
		qa._num_tuples = output_tuples;
		qe->_dequeuePin_list.push_back(qa);
		shm_set_state(qa._arc_id,1);
		//printf("LEAF (arc_id: %i): output_tuples: %i\n",out_arc,output_tuples);
		return output_tuples;
	}
	Boxes *b = _catalog->getBox(node->_box_id);
	double t_v_start = get_etime(&_first);
	if ( b->lockBoxTry() != 0 )
	{
		double t_z_start = get_etime(&_first);
		int output_tuples = 0;
		output_tuples = shm_get_num_records_in_queue(out_arc);
		(*queue_tuples_map)[out_arc] = output_tuples;
		queue_allocation qa;
		qa._arc_id = out_arc;
		qa._num_tuples = output_tuples;
		qe->_dequeuePin_list.push_back(qa);
		shm_set_state(qa._arc_id,1);
		double t_z_stop = get_etime(&_first);
		_time_spent_shm_get_num_records += t_z_stop - t_z_start;
		return output_tuples;
	}
	double t_v_stop = get_etime(&_first);
	_time_spent_loading_dequeue_pin_lists += t_v_stop - t_v_start;
	//printf("LOCKED ......................................... box: %i\n",node->_box_id);
	double t_y_start = get_etime(&_first);
	(*locked_boxes_set).insert(node->_box_id);
	double t_y_stop = get_etime(&_first);
	_time_spent_insert_locked_box_set += t_y_stop - t_y_start;

	vector<int> input_tuples_v;
	int total_input_tuples = 0;
	fflush(stdout);
	for ( int i = 0; i < _bqm[node->_box_id]._num_input_queues; i++ )
	{
		assert ((*(node->_input_boxes)).size() > i);

		int num_in_tuples = traverse_LoadQueue((*(node->_input_boxes))[i], _bqm[node->_box_id]._input_queue_id[i], qe, bl, node, queue_tuples_map, locked_boxes_set);
		input_tuples_v.push_back(num_in_tuples);
		total_input_tuples += num_in_tuples;// should use selectivity instead
		//printf("input_tuples(arc_id: %i): %i\n",_bqm[node->_box_id]._input_queue_id[i],num_in_tuples);
	}

	if ( total_input_tuples == 0 )
	{
		b->unlockBox();
		return 0;
	}

	queue_allocation qa;
	qa._arc_id = node->_out_arc_id;
	int curr_out_tuples = shm_get_num_records_in_queue(out_arc);
	(*queue_tuples_map)[out_arc] = curr_out_tuples;
	qa._num_tuples = total_input_tuples + curr_out_tuples;
	if ( (b->getType() == AGGREGATE_BOX) ||
	     (b->getType() == JOIN_BOX) ||
	     (b->getType() == LR_READ_RELATION_BOX) ||
	     (b->getType() == RESAMPLE_BOX) ) qa._num_tuples*=30;

	    //(b->getType() == TUMBLE_BOX) ||
	    //(b->getType() == SLIDE_BOX) ||
	    //(b->getType() == XSECTION_BOX)
	    //(b->getType() == WSORT_BOX)
	    

	double t_w_start = get_etime(&_first);
	if ( parent->_node_type == ROOT_NODE )
	{
		qe->_enqueuePin_list.push_back(qa);
		shm_set_state(qa._arc_id,1);
	}
	else
	{
		if ( qa._num_tuples == 0 )
		{
			printf("********* Scheduler:: qa._num_tuples == 0 ..     qa._arc_id: %i\n", qa._arc_id);
			abort();
		}
		qe->_dequeueEnqueuePin_list.push_back(qa);
		shm_set_state(qa._arc_id,1);
	}

	if ( _bqm[node->_box_id]._num_output_queues > 1 )
	{
		for ( int i = 0; i < _bqm[node->_box_id]._num_output_queues; i++ )
		{
			if ( _bqm[node->_box_id]._output_queue_id[i] != node->_out_arc_id )
			{
				qa._arc_id = _bqm[node->_box_id]._output_queue_id[i];
				int out_tuples = shm_get_num_records_in_queue(_bqm[node->_box_id]._output_queue_id[i]);
				(*queue_tuples_map)[out_arc] = out_tuples;
				qa._num_tuples = total_input_tuples + out_tuples;
				if ( (b->getType() == AGGREGATE_BOX) ||
				     (b->getType() == JOIN_BOX) ||
				     (b->getType() == LR_READ_RELATION_BOX) ||
				     (b->getType() == RESAMPLE_BOX) ) qa._num_tuples*=30;

				    //(b->getType() == TUMBLE_BOX) ||
				    //(b->getType() == SLIDE_BOX) ||
				    //(b->getType() == XSECTION_BOX)
				    //(b->getType() == WSORT_BOX)
				    			  
				qe->_enqueuePin_list.push_back(qa);
				shm_set_state(qa._arc_id,1);
			}
		}
	}
	double t_w_stop = get_etime(&_first);
	_time_spent_loading_pin_lists += t_w_stop - t_w_start;

	double t_x_start = get_etime(&_first);
	QBox *temp_box;
	int train_size = total_input_tuples;
	temp_box = loadBox(node->_box_id,train_size);
	bl->push_back(temp_box);
	double t_x_stop = get_etime(&_first);
	_time_spent_loadBox += t_x_stop - t_x_start;

	return total_input_tuples + curr_out_tuples;;
	//return total_input_tuples + shm_get_num_records_in_queue(out_arc);;
		

}
QBox *Scheduler::loadBox(int box_id, int train_size)
{
	QBox *temp_box;
	Boxes *b = _catalog->getBox(box_id);

	switch (b->getType())
	  {
	    /** Gone! - eddie
	       case SELECT_BOX:
	       temp_box = new SelectQBox();
	       break;
	    */
	  case FILTER_BOX:
	    temp_box = new FilterQBox();
	    break;
	  case BSORT_BOX:
	    temp_box = new BSortQBox();
	    break;
	  case JOIN_BOX:
	    temp_box = new JoinQBox();
	    break;
	  case RESAMPLE_BOX:
	    temp_box = new ResampleQBox();
	    break;
	  case MAP_BOX:
	    temp_box = new MapQBox();
	    break;
	  
	  case RESTREAM_BOX:
	    temp_box = new RestreamQBox();
	    break;
  
	  case UNION_BOX:
	    temp_box = new UnionQBox();
	    break;  
	    
	    //case TUMBLE_BOX:
	    //temp_box = new TumbleQBox();
	    //break;  
	  case AGGREGATE_BOX: // Replaces tumble, slide, xsection...
	    temp_box = new AggregateQBox();
	    break;
	    
	    //case SLIDE_BOX:
	    //temp_box = new SlideQBox();
	    //break;  
	    
	    //case XSECTION_BOX:
	    //temp_box = new XSectionQBox();
	    //break;  
	    
	    //case WSORT_BOX:
	    //temp_box = new WSortQBox();
	    //break;  
	    // bye bye! bsort took you over - eddie

	  case DROP_BOX:
	    temp_box = new DropQBox();
	    break;  

	    case EXPERIMENT_BOX:
	    temp_box = new ExperimentQBox();
	    break;   
	    
	  case HELLO_WORLD_BOX:
	    temp_box = new HelloWorldQBox();
	    break;
	  case UPDATE_RELATION_BOX:
	    temp_box = new UpdateRelationQBox();
	    break;
	  case READ_RELATION_BOX:
	    temp_box = new ReadRelationQBox();
	    break;
	  case LR_UPDATE_RELATION_BOX:
	    temp_box = new LRUpdateRelationQBox();
	    break;
	  case LR_READ_RELATION_BOX:
	    temp_box = new LRReadRelationQBox();
	    break;
	    
	  default:
	    fprintf(stderr,"Aurora ERROR bad box type in scheduler\n");
		assert( false );
			exit(0);
			break;
	}

	temp_box->_boxId = box_id;
	//printf("GOT TO findEligibleBox(): %i\n",findEligibleBox());
	temp_box->_numInputArcs = _bqm[temp_box->_boxId]._num_input_queues;
	temp_box->_inputArcId = new long[temp_box->_numInputArcs];
	for (int g = 0; g < temp_box->_numInputArcs; g++)
		temp_box->_inputArcId[g] = _bqm[temp_box->_boxId]._input_queue_id[g];

	temp_box->_numOutputArcs = _bqm[temp_box->_boxId]._num_output_queues;
	temp_box->_outputArcId = new long[temp_box->_numOutputArcs];
	for (int g = 0; g < temp_box->_numOutputArcs; g++)
		temp_box->_outputArcId[g] = _bqm[temp_box->_boxId]._output_queue_id[g];

	temp_box->_num_output_ports = _bqm[temp_box->_boxId]._num_out_ports;

	// to be initialized below
	temp_box->_tuple_descr_array = NULL;
	temp_box->_output_arcs_byport = NULL;

	Arcs *aa;
	Boxes *bb;
	for ( int i = 0; i <= _catalog->getMaxArcId(); i++ )
	{
		if ( (aa = _catalog->getArc(i)) != NULL )
		{
			int box_id_in = aa->getSrcId();
            int box_id_out = aa->getDestId();

			if ( (bb = _catalog->getBox(box_id_in)) != NULL && temp_box->_boxId == box_id_in ) //not an input arc
			{
				if ( temp_box->_output_arcs_byport == NULL )
					// its ok to use _bqm to get number of ports.
					temp_box->_output_arcs_byport = new vector<long>[ _bqm[box_id_in]._num_out_ports ];
				//printf(" Adding an arc %d on box %d  and src port id of arc %d\n", i, temp_box->_boxId, aa->getSrcPortId() );

				temp_box->_output_arcs_byport[ aa->getSrcPortId() ].push_back( i );
			}
		}
	}

	//Predicate *p;
	b = _catalog->getBox(temp_box->_boxId);

	Arcs *a;
	a = _catalog->getArc(temp_box->_inputArcId[0]);
	temp_box->_tuple_descr = new TupleDescription(*(a->getTupleDescr()));

	// The array of tuple descriptions (for all inputs)
	temp_box->_tuple_descr_array = new TupleDescription*[temp_box->_numInputArcs];
	for (int ti = 0; ti < temp_box->_numInputArcs; ++ti)
	  temp_box->_tuple_descr_array[ti] = new TupleDescription(*(_catalog->getArc(temp_box->_inputArcId[ti])->getTupleDescr()));
	

	temp_box->_selectivity = b->getSelectivity();
	//temp_box->_selectivity = 1.0;
	temp_box->_unitCost._cost_time = b->getCost();


	//int yy;

	switch (b->getType())
	  {
	    /**
	       // SELECT box Gone! it's filter box now! - eddie
	       case SELECT_BOX:
	       
	       temp_box->_window1 = b->getWinSize();	    
	       ((SelectQBox*)temp_box)->_predicate = b->getPredicate();
	       
	       break;
	    */
	  case FILTER_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((FilterQBox*) temp_box)->setBox(b->getModifier());
	    break;
	  case BSORT_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((BSortQBox*) temp_box)->setBox(b->getModifier(), ((BSortBox*) b->_db_box)->getBufferHash());
	    break;
	  case JOIN_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((JoinQBox*) temp_box)->setBox(b->getModifier(), ((JoinBox*) b->_db_box)->getLeftBufferList(), ((JoinBox*) b->_db_box)->getRightBufferList());
	    break;
	  case RESAMPLE_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((ResampleQBox*) temp_box)->setBox(b->getModifier(), ((ResampleBox*) b->_db_box)->getLeftBufferList(), ((ResampleBox*) b->_db_box)->getRightBufferList(), ((ResampleBox*) b->_db_box)->getHash());
	    break;
	  case MAP_BOX:
	    temp_box->_window1 = b->getWinSize();
	    //vector<Expression*> *e = b->getExpression();
	    //printf("GOT TO Scheduler Expression: %x\n",b->getExpression());
	    ((MapQBox*)temp_box)->_expr = b->getExpression();
	    ((MapQBox*)temp_box)->_num_funcs = 1;
	    break;
	    
	  case RESTREAM_BOX:
	    
	    temp_box->_window1 = b->getWinSize();
	    ((RestreamQBox*)temp_box)->setHash(b->getState()->group_hash);
	    ((RestreamQBox*)temp_box)->setModifier(b->getModifier());
	    //printf("GOT TO Restream Box: %s\n",b->getModifier());
	    break;
	    
	    /**
	  case TUMBLE_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((TumbleQBox*)temp_box)->setState(b->getState());
	    //printf("GOT TO Tumble Box: %s\n",b->getModifier());
	    break;
	    */
	    // Replaced tumble:
	  case AGGREGATE_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((AggregateQBox*) temp_box)->setBox(b->getModifier(),
						((AggregateBox*) b->_db_box)->getHash(),
						((AggregateBox*) b->_db_box)->getTupleCounter(),
						((AggregateBox*) b->_db_box)->getTupleStore());
						
	    break;
	    
	    /**
	  case SLIDE_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((SlideQBox*)temp_box)->setState(b->getState());
	    //printf("GOT TO Slide Box: %s\n",b->getModifier());
	    break;
	    
	  case XSECTION_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((XSectionQBox*)temp_box)->setState(b->getState());
	    //printf("GOT TO Slide Box: %s\n",b->getModifier());
	    break;
	    */

	    // Bye bye! you're gone - bsort took you over - eddie
	    //case WSORT_BOX:
	    
	    //temp_box->_window1 = b->getWinSize();
	    //((WSortQBox*)temp_box)->setState(b->getState());
	    //printf("GOT TO Slide Box: %s\n",b->getModifier());
	    //break;
	    
	  case UNION_BOX:
	    temp_box->_window1 = b->getWinSize();
	    break;
	    
	  case DROP_BOX:
	    
	    temp_box->_window1 = b->getWinSize();	    
	    ((DropQBox*)temp_box)->_drop_rate = b->getDropRate();
	    break;

	  case EXPERIMENT_BOX:
	    temp_box->_window1 = b->getWinSize();
	    break;

	  case HELLO_WORLD_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((HelloWorldQBox*) temp_box)->setBox(b->getModifier());
	    break;
	  case UPDATE_RELATION_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((UpdateRelationQBox*) temp_box)->setBox(
	      ((UpdateRelationBox *)b->_db_box)->getDb(),
	      ((UpdateRelationBox *)b->_db_box)->getKeyLength());
	    break;
	  case READ_RELATION_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((ReadRelationQBox*) temp_box)->setBox(
	      ((ReadRelationBox *)b->_db_box)->getDb(),
	      ((ReadRelationBox *)b->_db_box)->getKeyLength());
	    break;
	  case LR_UPDATE_RELATION_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((LRUpdateRelationQBox*) temp_box)->setBox(
	      ((LRUpdateRelationBox *)b->_db_box),
	      ((LRUpdateRelationBox *)b->_db_box)->getMagicNumber());
	    break;
	  case LR_READ_RELATION_BOX:
	    temp_box->_window1 = b->getWinSize();
	    ((LRReadRelationQBox*) temp_box)->setBox(
	      ((LRReadRelationBox *)b->_db_box),
	      ((LRReadRelationBox *)b->_db_box)->getMagicNumber());
	    break;

	  default:
	    fprintf(stderr,"Aurora ERROR bad box type in scheduler\n");
	    assert(false);
	    break;
	  }

	//Arcs *a;
	//a = _catalog->getArc(temp_box->_inputArcId1);
	//temp_box->_tuple_descr = new TupleDescription(*(a->getTupleDescr()));
	//printf("\n\n\n\n\nDan's test: %d\n\n\n\n\n\n", temp_box->_tuple_descr->getSize());
	    
	// LoadShedder-related calls BEGIN (tatbul@cs.brown.edu)
	//
	(temp_box->_ls_info).clear(); 
	RuntimeGlobals::getLoadShedder()->fillLSInfo(box_id, &(temp_box->_ls_info));
	//
	// LoadShedder-related calls END (tatbul@cs.brown.edu)
	
	    return temp_box;
	    
	    
}

/*
int Scheduler::findEligibleBox()
{
	Arcs *a;
	Boxes *b;
	for ( int i = 0; i < _catalog->getMaxArcId(); i++ )
	{
		a = _catalog->getArc(i);
		if ( a != NULL )
		{
			b = _catalog->getBox(a->getDestId());
			if ( b != NULL)
			{
				if ( shm_get_num_records_in_queue(i) >= b->getWinSize() )
					return b->getId();
			}
		}

	}
	return 1;
}
*/


/*

int Scheduler::findEligibleBox()
{
//TODO .. pick a random box
//do not pick a box that is in the box execution queue
//printf("GOT TO findEligibleBox()\n");
	Arcs *a;
	Boxes *b;
	for ( int i = 0; i <= _catalog->getMaxArcId(); i++ )
	{
	//printf("GOT TO i: %i\n",i);
		a = _catalog->getArc(i);
		if ( a != NULL )
		{
		//printf("a->getDestId(): %i\n",a->getDestId());
			b = _catalog->getBox(a->getDestId());
			if ( b != NULL)
			{
				if ( shm_get_num_records_in_queue(i) >= b->getWinSize() )
				{
				//printf("GOT TO A locking b->getId():%i\n",b->getId());
					if ( b->lockBoxTry() == 0 )
					{
					//printf("LOCKED ......................................... box: %i\n",b->getId());
					//printf("num_records_in queue: %i\n",shm_get_num_records_in_queue(i));
						return b->getId();
					}
				}
			}
		}
		//printf("GOT TO X 2\n");

	}
	//printf("GOT TO Y--------------------------------------------------------------------------\n");
	return -1;
}
*/


/*
int Scheduler::findEligibleBox()
{
	// RANDOM selection of eligible box

	vector<int> candidates_v;
	Boxes *b;
	static unsigned int seed = 1;

//printf("GOT TO Candidate Boxes:");
	for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
	{
		if (  _bqm[i]._box_id >= 0 )
		{
			b = _catalog->getBox(i);
			int winsize = b->getWinSize();
// DON THIS NEEDS TO CHANGE??? UNLESS IT WILL REMAIN COMMENTED OUT!
			if ( (_bqm[i]._num_input_queues == 1 && 
					shm_get_num_records_in_queue(_bqm[i]._input_queue_id_1) >= winsize) ||
				(_bqm[i]._num_input_queues == 2 &&
					shm_get_num_records_in_queue(_bqm[i]._input_queue_id_1) >= winsize &&
					shm_get_num_records_in_queue(_bqm[i]._input_queue_id_2) >= winsize) )
			{
//printf(" %i",i);
				candidates_v.push_back(i);
			}
		}
	}
//printf("\n");

	if ( candidates_v.size() > 0 )
	{
		for ( int k = 0; k < candidates_v.size(); k++ )
		{
			int random_index = rand_r(&seed) % candidates_v.size();
			int box_id = candidates_v[random_index];
			b = _catalog->getBox(box_id);
//printf("GOT TO trying to lock box: %i\n",box_id);
			if ( b->lockBoxTry() == 0 )
			{
			//printf("LOCKED ......................................... box: %i\n",b->getId());
			//printf("GOT TO random_index: %i   box_id: %i ##############################################\n",random_index,box_id);
				return (box_id);
			}
		}
	}

	return -1;
	
}
*/

/*
static  int slopecompare(const sortable_slope_struct *i, const sortable_slope_struct *j)
{
	sortable_slope_struct* si = const_cast<sortable_slope_struct*>(i);
	sortable_slope_struct* sj = const_cast<sortable_slope_struct*>(j);
	     if (si->slope < sj->slope)
			    return (1);
		 if (si->slope > sj->slope)
			return (-1);
		  return (0);
}
*/

int Scheduler::findEligibleBox_SlopeSlack_nolock()
{
	// SLOPE selection of eligible box .. independent of box lock status
	
	perror("THIS IS A BAD FUNCTION\n");
	// ERROR use gettime of day instead of time
	assert( false );
	exit(0);
	time_t t;
	(void) time (&t);

	_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
	assert(_candidates_v.size() == 0);
	Boxes *b;
	int winsize;
	int full_window;

	for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
	{
		if (  _bqm[i]._box_id >= 0 )
		{
			b = _catalog->getBox(i);
			winsize = b->getWinSize();
			//if ( b->lockBoxTry() == 0 )
			{
				double total_input_latency = 0;
				full_window = 1;
				for ( int k = 0; k < _bqm[i]._num_input_queues; k++ )
				{
					total_input_latency += (double)t - shm_get_average_timestamp(_bqm[i]._input_queue_id[k]);
					//printf("GOT TO AVG TIME STAMP(%i): %f\n",_bqm[i]._input_queue_id[k],shm_get_average_timestamp(_bqm[i]._input_queue_id[k]));
					//printf("GOT TO AVG TIME STAMP(%i) a : %f\n",i,shm_get_sum_timestamp(_bqm[i]._input_queue_id[k]));
					if (shm_get_num_records_in_queue(_bqm[i]._input_queue_id[k]) < winsize)
					{
						full_window = 0;
						break;
					}
				}
				if ( full_window == 1 )
				{
					BoxLatency bl;
					double average_input_latency = total_input_latency/(double)_bqm[i]._num_input_queues;
					bl._box_id = i;
					bl._ss=findSlopeAt(average_input_latency,i);

					_candidates_v.push_back(bl);
				}
				else
				{
					BoxLatency bl;
					double average_input_latency = MAXDOUBLE; // put it at the end of the list
					bl._box_id = i;
					bl._ss=findSlopeAt(average_input_latency,i);

					_candidates_v.push_back(bl);
				}
				//b->unlockBox();
			}
		}
	}
	sort(_candidates_v.begin(),_candidates_v.end());
	//printf("findEligibleBox:: CANDIDATE BOXES:");
	//for ( int i = 0; i < _candidates_v.size(); i++ )
	//{
	//	printf(" %i(%f,%f)",_candidates_v[i]._box_id,_candidates_v[i]._ss._slope,_candidates_v[i]._ss._slack);
		//printf(" %i[%i](%f,%f)",_candidates_v[i]._box_id,shm_get_average_timestamp(_candidates_v[i]._box_id),_candidates_v[i]._ss._slope,_candidates_v[i]._ss._slack);
	//}
	//printf("\n");
	if ( _candidates_v.size() == 0 )
		return -1;
	else
		return(_candidates_v[0]._box_id);



	

	return -1;
	
}

int Scheduler::findEligibleBox_SlopeSlack()
{
	// SLOPE selection of eligible box
	
	//time_t t;
	//(void) time (&t);
	double time_now;
	timeval t_now;
	gettimeofday(&t_now,NULL);
	time_now = t_now.tv_sec + (t_now.tv_usec*1e-06);

	_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
	assert(_candidates_v.size() == 0);
	Boxes *b;
	int winsize;
	int full_window;

	set<int>::iterator iter;
	vector<int> runnable_box_v;
	pthread_mutex_lock(&__box_work_set_mutex);
	for ( iter = __box_work_set.begin(); iter != __box_work_set.end(); iter++ )
		runnable_box_v.push_back(*iter);
	pthread_mutex_unlock(&__box_work_set_mutex);

	for ( int j = 0; j < runnable_box_v.size(); j++ )
	//for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
	{
		int i = runnable_box_v[j];
		if ( i <= _catalog->getMaxBoxId())
		{
			if (  _bqm[i]._box_id >= 0 )
			{
				b = _catalog->getBox(i);
				winsize = b->getWinSize();
				//if ( i == 0 )
				//	fprintf(stderr,"GOT TO findEligibleBox_SlopeSlack() testing box: %i\n",i);
				if ( b->lockBoxTry() == 0 )
				{
					//if ( i == 0 )
					//	fprintf(stderr,"GOT TO findEligibleBox_SlopeSlack() box: %i now locked\n",i);
					double total_input_latency = 0.0;
					full_window = 0;
					assert(_bqm[i]._num_input_queues > 0);
					for ( int k = 0; k < _bqm[i]._num_input_queues; k++ )
					{
						if (shm_get_num_records_in_queue(_bqm[i]._input_queue_id[k]) >= winsize)
							total_input_latency += time_now - shm_get_average_timestamp(_bqm[i]._input_queue_id[k]);
						//printf("GOT TO AVG TIME STAMP(%i): %f\n",_bqm[i]._input_queue_id[k],shm_get_average_timestamp(_bqm[i]._input_queue_id[k]));
						//printf("GOT TO AVG TIME STAMP(%i) a : %f\n",i,shm_get_sum_timestamp(_bqm[i]._input_queue_id[k]));
						if (shm_get_num_records_in_queue(_bqm[i]._input_queue_id[k]) >= winsize)
						{
							full_window = 1;
						}
						//fprintf(stderr,"shm_get_num_records[%i]: %i  shm_get_sum_timestamp: %f time_now: %f\n",_bqm[i]._input_queue_id[k],shm_get_num_records_in_queue(_bqm[i]._input_queue_id[k]),shm_get_sum_timestamp(_bqm[i]._input_queue_id[k]),time_now);
					}
					if ( full_window == 1 )
					{
						BoxLatency bl;
						double average_input_latency = total_input_latency/(double)_bqm[i]._num_input_queues;
						bl._box_id = i;
						bl._ss=findSlopeAt(average_input_latency,i);
						//fprintf(stderr,"GOT TO box: %i til: %f niq: %i  average_input_latency :%f  slope: %f   slack: %f\n",i,total_input_latency,_bqm[i]._num_input_queues,average_input_latency,bl._ss._slope,bl._ss._slack);
						if ( average_input_latency > 0.4 )
						{
							//fprintf(stderr,"GOT TO average_input_latency > 0.4  slope: %f   slack: %f\n",bl._ss._slope,bl._ss._slack);
						}

						//if ( i == 0 )
						//	fprintf(stderr,"GOT TO findEligibleBox_SlopeSlack() box: %i added to _candidates_v\n",i);

						_candidates_v.push_back(bl);
					}
					b->unlockBox();
				}
				else
				{
					//printf("GOT TO findEligibleBox_SlopeSlack() box: %i locked\n",i);
				}
			}
		}
	}
	//testCandidates();
	// sort in ascending order of slope and slack
	sort(_candidates_v.begin(),_candidates_v.end());
	// then reverse, because we really want highest slope to be first
	reverse(_candidates_v.begin(),_candidates_v.end());
	/*
	printf("findEligibleBox_SlopeSlack:: CANDIDATE BOXES:");
	for ( int i = 0; i < _candidates_v.size(); i++ )
	{
		printf(" %i[",_candidates_v[i]._box_id);
		multimap<int,int>::iterator iter;
		for ( iter = _bam.lower_bound(_candidates_v[i]._box_id);
				iter != _bam.upper_bound(_candidates_v[i]._box_id);
				iter++ )
		{
			printf("%i,",(*iter).second);
		}
		printf("](%f,%f)",_candidates_v[i]._ss._slope,_candidates_v[i]._ss._slack);
		//printf(" %i(%f,%f)",_candidates_v[i]._box_id,_candidates_v[i]._ss._slope,_candidates_v[i]._ss._slack);
		//printf(" %i[%i](%f,%f)",_candidates_v[i]._box_id,shm_get_average_timestamp(_candidates_v[i]._box_id),_candidates_v[i]._ss._slope,_candidates_v[i]._ss._slack);
	}
	printf("\n");
	*/
	if ( _candidates_v.size() == 0 )
		return -1;
	else
		return(_candidates_v[0]._box_id);



	

	return -1;
	
}
int Scheduler::findEligibleBox_Bucketing(int num_to_schedule)
{
	static double chunk1total = 0.0;
	static double chunk2total = 0.0;
	//static double chunk3total = 0.0;
	//static double chunk4total = 0.0;
	timeval chunk_start,chunk_stop;
	gettimeofday(&chunk_start,NULL);
	double time_now;
	timeval t_now;
	gettimeofday(&t_now,NULL);
	time_now = t_now.tv_sec + (t_now.tv_usec*1e-06);
	int ul_count = 0;

	_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
	assert(_candidates_v.size() == 0);
	Boxes *b;
	int winsize;
	int full_window;

	_runnable_boxes_bitarray->clear();
	set<int>::iterator iter;
	vector<int> runnable_box_v;
	int box_id;
	pthread_mutex_lock(&__box_work_set_mutex);
	for ( iter = __box_work_set.begin(); iter != __box_work_set.end(); iter++ )
	{
		box_id = *iter;
		if ( box_id > _catalog->getMaxBoxId() ||  _bqm[box_id]._box_id < 0 )
				continue;
		runnable_box_v.push_back(box_id);
		_runnable_boxes_bitarray->setBit(box_id);
	}
	pthread_mutex_unlock(&__box_work_set_mutex);

	int tmp_count = 0;
	BoxLatency bl;
	GridElement *ge;
	GridElement *loaded_ge;
	int urgent_count=0;
	vector<GridElement*> ge_list;
	vector<UrgencyStruct> *urgency_list_v = _priority_grid->getUrgencyList();
	GRID_ELEMENT_BOX_ITER iter1;
	gettimeofday(&chunk_stop,NULL);
	chunk1total+= (chunk_stop.tv_sec + (chunk_stop.tv_usec*1e-6)) -
				(chunk_start.tv_sec + (chunk_start.tv_usec*1e-6));
	for ( int i = 0; i < (*urgency_list_v).size(); i++ )
	{
		ul_count++;
		box_id = (*urgency_list_v)[i]._box_id;
		if ( i > 0 )
		{
			ge = (*urgency_list_v)[i]._ge;
			if ( ge != (*urgency_list_v)[i-1]._ge && ge->_current_assigned_count > 0)
			//if ( ge_list.size() > 0 ) // if no boxes have been loaded, then no need
			{
				//if ( ge != ge_list[ge_list.size()-1] && ge->_current_assigned_count > 0)
				{
					urgent_count += ge->_current_assigned_count;
					for ( iter1 = ge->_box_list.begin();
							iter1 != ge->_box_list.end();
							iter1++ )
					{
						box_id = *iter1;
						bl._box_id = box_id;
						_candidates_v.push_back(bl);
						if ( _candidates_v.size() >= num_to_schedule )
								break;
					}
					if ( _candidates_v.size() >= num_to_schedule )
							break;
				}
			}
		}
		if ( _runnable_boxes_bitarray->testBit(box_id) == 1 )
		{
			_runnable_boxes_bitarray->unsetBit(box_id);
			b = _catalog->getBox(box_id);
			winsize = b->getWinSize();
	gettimeofday(&chunk_start,NULL);
			if ( b->lockBoxTry() == 0 )
			{
				double total_input_latency = 0.0;
				full_window = 0;
				assert(_bqm[box_id]._num_input_queues > 0);
				for ( int k = 0; k < _bqm[box_id]._num_input_queues; k++ )
				{
					int arc_id = _bqm[box_id]._input_queue_id[k];
					int num_records = shm_get_num_records_in_queue(arc_id);
					if (num_records >= winsize)
					{
						total_input_latency += time_now - shm_get_average_timestamp(arc_id);
						full_window = 1;
					}
				}
				if ( full_window == 1 )
				{
					tmp_count++;
					double average_input_latency = total_input_latency/(double)_bqm[box_id]._num_input_queues;
	//fprintf(stderr,"GOT TO B ul_count: %i tmp_count: %i \n",ul_count,tmp_count);
					loaded_ge = _grid_pointers->loadPriorityAssignment(box_id,average_input_latency);
					ge_list.push_back(loaded_ge);
					loaded_ge->_current_assigned_count++;
					if ( loaded_ge == (*urgency_list_v)[i]._ge )
					{
						urgent_count++;
						box_id = (*urgency_list_v)[i]._box_id;
						bl._box_id = box_id;
						_candidates_v.push_back(bl);
						if ( _candidates_v.size() >= num_to_schedule )
								break;
					}
				}
				b->unlockBox();
			}
			else
			{
				//printf("GOT TO findEligibleBox_Bucketing() box: %i locked\n",box_id);
			}
	gettimeofday(&chunk_stop,NULL);
	chunk2total+= (chunk_stop.tv_sec + (chunk_stop.tv_usec*1e-6)) -
				(chunk_start.tv_sec + (chunk_start.tv_usec*1e-6));
		}
		if ( urgent_count >= num_to_schedule )
		{
			fprintf(stderr,"ERROR Bucketing .. should not get here\n");
		}

	}
	//vector<GridElement*>::iterator iter2;
	for ( int i = 0; i < ge_list.size(); i++ )
	{
		ge_list[i]->_current_assigned_count = 0;
	}

	//gettimeofday(&chunk_stop,NULL);
	//chunk2total+= (chunk_stop.tv_sec + (chunk_stop.tv_usec*1e-6)) -
				//(chunk_start.tv_sec + (chunk_start.tv_usec*1e-6));

	//fprintf(stderr,"runnable_box_v.size(): %i chunk1total: %f chunk2total: %f ul_count: %i/%i \n",runnable_box_v.size(),chunk1total,chunk2total,ul_count,(*urgency_list_v).size());
	//fprintf(stderr,"\n");
	for ( int i = 0; i < _candidates_v.size(); i++ )
	{
		_grid_pointers->removeFromGrid(_candidates_v[i]._box_id);
	}
	if ( _candidates_v.size() == 0 )
	{
		//fprintf(stderr,"runnable_box_v.size(): %i\n", runnable_box_v.size());
		//fprintf(stderr,"no candidates\n");
		return -1;
	}
	else
		return(_candidates_v[0]._box_id);

}

int Scheduler::findEligibleBox_Bucketing1(int num_to_schedule)
{
	static double chunk1total = 0.0;
	static double chunk2total = 0.0;
	static double chunkxtotal = 0.0;
	double time_now;
	timeval t_now;
	timeval chunk_start,chunk_stop;
	timeval chunk_x_start,chunk_x_stop;

	_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
	assert(_candidates_v.size() == 0);
	Boxes *b;
	int winsize;
	int full_window;

	set<int>::iterator iter;
	vector<int> runnable_box_v;
	pthread_mutex_lock(&__box_work_set_mutex);
	for ( iter = __box_work_set.begin(); iter != __box_work_set.end(); iter++ )
		runnable_box_v.push_back(*iter);
	pthread_mutex_unlock(&__box_work_set_mutex);

	
	/*
	static int runnable_count = 0;
	static int counter = 0;
	counter++;
	runnable_count += runnable_box_v.size();
	static double max_running_average = 0.0;
	if ( ((double)runnable_count/(double)counter) > max_running_average)
	{
		max_running_average = (double)runnable_count/(double)counter;
		printf("max_running_average: %f _runnable_box_v.size(): %i\n",max_running_average,runnable_box_v.size());
	}
	*/
	

	_measure->incrementNumSchedulableBoxes(runnable_box_v.size());

	gettimeofday(&chunk_start,NULL);
	double t_a_start = get_etime(&_first);
	for ( int j = 0; j < runnable_box_v.size(); j++ )
	{
		int box_id = runnable_box_v[j];
		if ( box_id > _catalog->getMaxBoxId() ||  _bqm[box_id]._box_id < 0 )
				continue;

	gettimeofday(&t_now,NULL);
	time_now = t_now.tv_sec + (t_now.tv_usec*1e-06);

		b = _catalog->getBox(box_id);
		winsize = b->getWinSize();
		if ( b->lockBoxTry() == 0 )
		{
			double total_input_latency = 0.0;
			full_window = 0;
			assert(_bqm[box_id]._num_input_queues > 0);
			for ( int k = 0; k < _bqm[box_id]._num_input_queues; k++ )
			{
				int num_records = shm_get_num_records_in_queue(_bqm[box_id]._input_queue_id[k]);
				if (num_records >= winsize)
				{
					gettimeofday(&t_now,NULL);
					time_now = t_now.tv_sec + (t_now.tv_usec*1e-06);
					total_input_latency += time_now - shm_get_average_timestamp(_bqm[box_id]._input_queue_id[k]);
					if ((time_now - shm_get_average_timestamp(_bqm[box_id]._input_queue_id[k])) < 0.0)
					{
						printf("MAXDOUBLE: %f\n",MAXDOUBLE);
						printf (" time_now: %f shm_get_average_timestamp(): %f\n",time_now,shm_get_average_timestamp(_bqm[box_id]._input_queue_id[k]));
						printf ("(time_now - shm_get_average_timestamp(_bqm[box_id]._input_queue_id[k])): %f\n",(time_now - shm_get_average_timestamp(_bqm[box_id]._input_queue_id[k])));
						assert(time_now - shm_get_average_timestamp(_bqm[box_id]._input_queue_id[k]) >= 0.0);
					}
					full_window = 1;
					//cout << " Time now! " << time_now << " Timestamp acquired " << shm_get_average_timestamp(_bqm[box_id]._input_queue_id[k]) << "  for a queue " << _bqm[box_id]._input_queue_id[k] << " DIFF " << (time_now - shm_get_average_timestamp(_bqm[box_id]._input_queue_id[k])) << endl;
						gettimeofday(&t_now,NULL);
						//cout << " MORE DIFFs " << time_now - (t_now.tv_sec + (t_now.tv_usec*1e-06)) << endl;
					if ( total_input_latency < 0 )
					{
						assert( false );
						abort();
						exit( 0 );
					}
				}
			}
			if ( full_window == 1 )
			{
				gettimeofday(&chunk_x_start,NULL);
				double average_input_latency = total_input_latency/(double)_bqm[box_id]._num_input_queues;
				_grid_pointers->loadPriorityAssignment(box_id,average_input_latency);
				//fprintf(stderr,"loading box: %i\n",box_id);
				gettimeofday(&chunk_x_stop,NULL);
				chunkxtotal+= (chunk_x_stop.tv_sec + (chunk_x_stop.tv_usec*1e-6)) -
							(chunk_x_start.tv_sec + (chunk_x_start.tv_usec*1e-6));
			}
			else
			{
				//fprintf(stderr,"weirdness!!\n");
				//for ( int k = 0; k < _bqm[box_id]._num_input_queues; k++ )
				  //fprintf(stderr,"box_id:%i num_records[%i]: %i\n",box_id,k,shm_get_num_records_in_queue(_bqm[box_id]._input_queue_id[k]));
			}
			b->unlockBox();
		}
		else
		{
			//printf("GOT TO findEligibleBox_Bucketing() box: %i locked\n",box_id);
		}
	}
	double t_a_stop = get_etime(&_first);
	_time_spent_Bucketing_finding_schedulable += t_a_stop - t_a_start;
	gettimeofday(&chunk_stop,NULL);
	chunk1total+= (chunk_stop.tv_sec + (chunk_stop.tv_usec*1e-6)) -
				(chunk_start.tv_sec + (chunk_start.tv_usec*1e-6));
	//_priority_grid->printGrid();
	BoxLatency bl;
	GridElement *ge;
	GRID_ELEMENT_BOX_ITER iter1;
	int box_id;
	int count;
	//fprintf(stderr,"selecting from grid:");
	
	
	//fprintf(stderr,"_priority_grid_bucket_vector->size(): %i\n",_priority_grid_bucket_vector->size());
	gettimeofday(&chunk_start,NULL);
	double t_b_start = get_etime(&_first);
	for ( int i = 0; i < _priority_grid_bucket_vector->size(); i++ )
	{
		ge = (*_priority_grid_bucket_vector)[i];
		
		for ( iter1 = ge->_box_list.begin();
				iter1 != ge->_box_list.end();
				iter1++ )
		{
			box_id = *iter1;
			bl._box_id = box_id;
			_candidates_v.push_back(bl);
			count++;
			//fprintf(stderr,"(%i,%i)",slack_bucket,slope_bucket);
			if ( _candidates_v.size() >= num_to_schedule )
					break;
		}
		if ( _candidates_v.size() >= num_to_schedule )
			break;
	}
	double t_b_stop = get_etime(&_first);
	_time_spent_Bucketing_finding_candidates += t_b_stop - t_b_start;
	gettimeofday(&chunk_stop,NULL);
	chunk2total+= (chunk_stop.tv_sec + (chunk_stop.tv_usec*1e-6)) -
				(chunk_start.tv_sec + (chunk_start.tv_usec*1e-6));

	//fprintf(stderr,"runnable_box_v.size(): %i chunk1total: %f   chunk2total: %f chunkxtotal: %f\n",runnable_box_v.size(),chunk1total,chunk2total,chunkxtotal);
	
	/*
	for ( int slack_bucket = 0; slack_bucket < _num_buckets; slack_bucket++ )
	{
		for ( int slope_bucket = _num_buckets-1; slope_bucket >= 0; slope_bucket-- )
		{
			ge = _priority_grid->getGridElement(slack_bucket,slope_bucket);
			count = 0;
			//if (ge->_box_list.size() > 0)
				//fprintf(stderr,"size:%i",ge->_box_list.size());
			for ( iter1 = ge->_box_list.begin();
					iter1 != ge->_box_list.end();
					iter1++ )
			{
				box_id = *iter1;
				bl._box_id = box_id;
				_candidates_v.push_back(bl);
				count++;
				//fprintf(stderr,"(%i,%i)",slack_bucket,slope_bucket);
				if ( _candidates_v.size() >= num_to_schedule )
						break;
			}
			if ( _candidates_v.size() >= num_to_schedule )
				break;
		}
		if ( _candidates_v.size() >= num_to_schedule )
			break;
	}
	*/
	//fprintf(stderr,"\n");
	//printf("_candidates_v: ");
	//for ( int i = 0; i < _candidates_v.size(); i++ )
	//{
	//	printf("%i ",_candidates_v[i]._box_id);
	//}
	double t_c_start = get_etime(&_first);
	for ( int i = 0; i < _candidates_v.size(); i++ )
	{
		_grid_pointers->removeFromGrid(_candidates_v[i]._box_id);
	}
	double t_c_stop = get_etime(&_first);
	_time_spent_Bucketing_removeFromGrid += t_c_stop - t_c_start;
	if ( _candidates_v.size() == 0 )
	{
		//fprintf(stderr,"runnable_box_v.size(): %i\n", runnable_box_v.size());
		//fprintf(stderr,"no candidates\n");
		return -1;
	}
	else
	{
		//cout << " returning " << (_candidates_v[0]._box_id) << endl;
		return(_candidates_v[0]._box_id);
	}

}
int Scheduler::findEligibleBox_LQF_nolock()
{
	// LQF selection of eligible box
	//
	
	vector<BoxQueue> candidates_bq_v;
	
	candidates_bq_v.erase(candidates_bq_v.begin(), candidates_bq_v.end());
	assert(candidates_bq_v.size() == 0);
	Boxes *b;
	int winsize;
	int full_window;

	for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
	{
		if (  _bqm[i]._box_id >= 0 )
		{
			b = _catalog->getBox(i);
			winsize = b->getWinSize();
			//if ( b->lockBoxTry() == 0 )
			{
				//double total_input_latency = 0;
				full_window = 1;
				int largest_queue = 0;
				int largest_queue_size = 0;
				int tmp_num_tuples = 0;
				for ( int k = 0; k < _bqm[i]._num_input_queues; k++ )
				{
					tmp_num_tuples = shm_get_num_records_in_queue(_bqm[i]._input_queue_id[k]);
					if ( tmp_num_tuples > largest_queue_size )
					{
						largest_queue_size = tmp_num_tuples;
						largest_queue = _bqm[i]._input_queue_id[k];
					}
					if (tmp_num_tuples < winsize)
					{
						full_window = 0;
						break;
					}
				}
				//if ( full_window == 1 )
				{
					BoxQueue bq;
					bq._box_id = i;
					//bq._largest_input_queue_length;

					candidates_bq_v.push_back(bq);
				}
				//b->unlockBox();
			}
		}
	}
	sort(candidates_bq_v.begin(),candidates_bq_v.end());
	//printf("findEligibleBox:: CANDIDATE BOXES:");
	_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
	for ( int i = 0; i < candidates_bq_v.size(); i++ )
	{
		BoxLatency bl;
		bl._box_id = candidates_bq_v[i]._box_id;
		_candidates_v.push_back(bl);
		//printf(" %i(%f,%f)",candidates_bq_v[i]._box_id,candidates_bq_v[i]._largest_input_queue_length);
	}
	//printf("\n");
	if ( candidates_bq_v.size() == 0 )
		return -1;
	else
		return(candidates_bq_v[0]._box_id);

	return -1;
	
}
int Scheduler::findEligibleBox_LQF()
{
	// LQF selection of eligible box
	//
	
	vector<BoxQueue> candidates_bq_v;
	
	candidates_bq_v.erase(candidates_bq_v.begin(), candidates_bq_v.end());
	assert(candidates_bq_v.size() == 0);
	Boxes *b;
	int winsize;
	int full_window;

	set<int>::iterator iter;
	vector<int> runnable_box_v;
	pthread_mutex_lock(&__box_work_set_mutex);
	for ( iter = __box_work_set.begin(); iter != __box_work_set.end(); iter++ )
		runnable_box_v.push_back(*iter);
	pthread_mutex_unlock(&__box_work_set_mutex);

	for ( int j = 0; j < runnable_box_v.size(); j++ )
	//for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
	{
		int i = runnable_box_v[j];
		if ( i <= _catalog->getMaxBoxId())
		{
			if (  _bqm[i]._box_id >= 0 )
			{
				b = _catalog->getBox(i);
				winsize = b->getWinSize();
				if ( b->lockBoxTry() == 0 )
				{
					//double total_input_latency = 0;
					full_window = 1;
					int largest_queue = 0;
					int largest_queue_size = 0;
					int tmp_num_tuples = 0;
					for ( int k = 0; k < _bqm[i]._num_input_queues; k++ )
					{
						tmp_num_tuples = shm_get_num_records_in_queue(_bqm[i]._input_queue_id[k]);
						if ( tmp_num_tuples > largest_queue_size )
						{
							largest_queue_size = tmp_num_tuples;
							largest_queue = _bqm[i]._input_queue_id[k];
						}
						if (tmp_num_tuples < winsize)
						{
							full_window = 0;
							break;
						}
					}
					if ( full_window == 1 )
					{
						BoxQueue bq;
						bq._box_id = i;
						//bq._largest_input_queue_length;

						candidates_bq_v.push_back(bq);
					}
					b->unlockBox();
				}
			}
		}
	}
	sort(candidates_bq_v.begin(),candidates_bq_v.end());
	//printf("findEligibleBox:: CANDIDATE BOXES:");
	_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
	for ( int i = 0; i < candidates_bq_v.size(); i++ )
	{
		BoxLatency bl;
		bl._box_id = candidates_bq_v[i]._box_id;
		_candidates_v.push_back(bl);
		//printf(" %i(%f,%f)",candidates_bq_v[i]._box_id,candidates_bq_v[i]._largest_input_queue_length);
	}
	//printf("\n");
	if ( candidates_bq_v.size() == 0 )
		return -1;
	else
		return(candidates_bq_v[0]._box_id);

	return -1;
	
}
int Scheduler::findEligibleBox_Random_nolock()
{
	// Random selection of eligible box
	
	//time_t t;
	//(void) time (&t);

	_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
	assert(_candidates_v.size() == 0);
	Boxes *b;
	int winsize;
	int full_window;

	for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
	{
		if (  _bqm[i]._box_id >= 0 )
		{
			b = _catalog->getBox(i);
			winsize = b->getWinSize();
			//if ( b->lockBoxTry() == 0 )
			{
				full_window = 0;
				for ( int k = 0; k < _bqm[i]._num_input_queues; k++ )
				{
					//printf("GOT TO AVG TIME STAMP(i): %f\n",i,shm_get_average_timestamp(_bqm[i]._input_queue_id[k]));
					//printf("GOT TO AVG TIME STAMP(i) a : %f\n",i,shm_get_sum_timestamp(_bqm[i]._input_queue_id[k]));
					// have to fix this for JOIN(join) .. works for UNION
					if (shm_get_num_records_in_queue(_bqm[i]._input_queue_id[k]) >= winsize)
					{
						full_window = 1;
						break;
					}
				}
				//if ( full_window == 1 )
				{
					BoxLatency bl;
					bl._box_id = i;

					_candidates_v.push_back(bl);
				}
				//b->unlockBox();
			}
		}
	}
	//printf("findEligibleBox:: CANDIDATE BOXES:");
	//for ( int i = 0; i < _candidates_v.size(); i++ )
	//{
	//	printf(" %i",_candidates_v[i]._box_id);
	//}
	//printf("\n");
	if ( _candidates_v.size() == 0 )
		return -1;
	else
	{
		random_shuffle(_candidates_v.begin(),_candidates_v.end());
		return(_candidates_v[0]._box_id);
	}



	

	return -1;
	
}
int Scheduler::findEligibleBox_Random()
{
	// Random selection of eligible box
	
	//time_t t;
	//(void) time (&t);

	_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
	assert(_candidates_v.size() == 0);
	Boxes *b;
	int winsize;
	int full_window;
	int num_to_find = 10;

	set<int>::iterator iter;
	vector<int> runnable_box_v;
	set<int> test_set;
	pthread_mutex_lock(&__box_work_set_mutex);
	for ( iter = __box_work_set.begin(); iter != __box_work_set.end(); iter++ )
	{
		runnable_box_v.push_back(*iter);
		test_set.insert(*iter);
	}
	pthread_mutex_unlock(&__box_work_set_mutex);

	int box_id;
	static unsigned int seed = 1;
	while( _candidates_v.size() < num_to_find && test_set.size() > 0)
	{
		int random_index = rand_r(&seed) % runnable_box_v.size();
		box_id = runnable_box_v[random_index];
		if ( box_id <= _catalog->getMaxBoxId())
		{
			if (  _bqm[box_id]._box_id >= 0 )
			{
				b = _catalog->getBox(box_id);
				winsize = b->getWinSize();
				if ( b->lockBoxTry() == 0 )
				{
					full_window = 0;
					for ( int k = 0; k < _bqm[box_id]._num_input_queues; k++ )
					{
						//printf("GOT TO AVG TIME STAMP(%i): %f\n",box_id,shm_get_average_timestamp(_bqm[box_id]._input_queue_id[k]));
						//printf("GOT TO AVG TIME STAMP(%i) a : %f\n",box_id,shm_get_sum_timestamp(_bqm[box_id]._input_queue_id[k]));
						// have to fix this for JOIN(join) .. works for UNION
						if (shm_get_num_records_in_queue(_bqm[box_id]._input_queue_id[k]) >= winsize)
						{
							full_window = 1;
							break;
						}
					}
					if ( full_window == 1 )
					{
						BoxLatency bl;
						bl._box_id = box_id;

						_candidates_v.push_back(bl);
					}
					b->unlockBox();
				}
			}
		}
		test_set.erase(box_id);
	}
	if ( _candidates_v.size() == 0 )
		return -1;
	else
	{
		return(_candidates_v[0]._box_id);
	}
/*
	for ( int j = 0; j < runnable_box_v.size(); j++ )
	//for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
	{
		int i = runnable_box_v[j];
		if ( i <= _catalog->getMaxBoxId())
		{
			if (  _bqm[i]._box_id >= 0 )
			{
				b = _catalog->getBox(i);
				winsize = b->getWinSize();
				if ( b->lockBoxTry() == 0 )
				{
					full_window = 0;
					for ( int k = 0; k < _bqm[i]._num_input_queues; k++ )
					{
						printf("GOT TO AVG TIME STAMP(i): %f\n",i,shm_get_average_timestamp(_bqm[i]._input_queue_id[k]));
						printf("GOT TO AVG TIME STAMP(i) a : %f\n",i,shm_get_sum_timestamp(_bqm[i]._input_queue_id[k]));
						// have to fix this for JOIN(join) .. works for UNION
						if (shm_get_num_records_in_queue(_bqm[i]._input_queue_id[k]) >= winsize)
						{
							full_window = 1;
							break;
						}
					}
					if ( full_window == 1 )
					{
						BoxLatency bl;
						bl._box_id = i;

						_candidates_v.push_back(bl);
					}
					b->unlockBox();
				}
			}
		}
	}
	printf("findEligibleBox:: CANDIDATE BOXES:");
	for ( int i = 0; i < _candidates_v.size(); i++ )
	{
		printf(" %i(%f,%f)",_candidates_v[i]._box_id,_candidates_v[i]._ss._slope,_candidates_v[i]._ss._slack);
	}
	printf("\n");
	if ( _candidates_v.size() == 0 )
		return -1;
	else
	{
		random_shuffle(_candidates_v.begin(),_candidates_v.end());
		return(_candidates_v[0]._box_id);
	}
*/


	

	return -1;
	
}
int Scheduler::findEligibleBox_RR_nolock()
{
	// Random selection of eligible box
	
	//time_t t;
	//(void) time (&t);

	_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
	assert(_candidates_v.size() == 0);
	Boxes *b;
	int winsize;
	int full_window;

	for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
	{
		if (  _bqm[i]._box_id >= 0 )
		{
			b = _catalog->getBox(i);
			winsize = b->getWinSize();
			//printf("findEligibleBox:: testing box %i \n",i);
			//if ( b->lockBoxTry() == 0 )
			{
				full_window = 0;
				for ( int k = 0; k < _bqm[i]._num_input_queues; k++ )
				{
					//printf("GOT TO AVG TIME STAMP(i): %f\n",i,shm_get_average_timestamp(_bqm[i]._input_queue_id[k]));
					//printf("GOT TO AVG TIME STAMP(i) a : %f\n",i,shm_get_sum_timestamp(_bqm[i]._input_queue_id[k]));
					// have to fix this for JOIN(join) .. works for UNION
					if (shm_get_num_records_in_queue(_bqm[i]._input_queue_id[k]) >= winsize)
					{
						full_window = 1;
						break;
					}
				}
				//if ( full_window == 1 )
				{
					BoxLatency bl;
					bl._box_id = i;

					_candidates_v.push_back(bl);
				}
				//b->unlockBox();
			}
		}
	}
	//shm_print("SCHEDULER");
	//printf("findEligibleBox:: CANDIDATE BOXES:");
	//for ( int i = 0; i < _candidates_v.size(); i++ )
	//{
	//	printf(" %i",_candidates_v[i]._box_id);
	//}
	//printf("\n");
	if ( _candidates_v.size() == 0 )
		return -1;
	else
	{
		static int last_box = 0;
		//printf("findEligibleBox:: PREVIOUS CANDIDATE: %i\n",last_box);

		int new_box = -1;
		int new_box_index = 0;
		for ( int i = 0; i < _candidates_v.size(); i++ )
		{
			if ( _candidates_v[i]._box_id > last_box )
			{
				new_box = _candidates_v[i]._box_id;
				new_box_index = i;
				break;
			}
		}
		if ( new_box < 0 )
			new_box = _candidates_v[0]._box_id;
		else
		{
			vector<BoxLatency> tmp;
			int rr_index = new_box_index;
			for ( int i = 0; i < _candidates_v.size(); i++ )
			{
				tmp.push_back(_candidates_v[rr_index]);
				rr_index++;
				if ( rr_index >= _candidates_v.size() )
					rr_index = 0;
			}
			_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
			_candidates_v = tmp;
			assert(tmp.size() == _candidates_v.size());
			
		}

		last_box = new_box;
		//printf("findEligibleBox:: CANDIDATE SELECTION: %i\n",new_box);
		return(new_box);
	}

	return -1;
	
}
int Scheduler::findEligibleBox_RR()
{
	// Random selection of eligible box
	
	//time_t t;
	//(void) time (&t);
	double t_a_start = get_etime(&_first);
	
	int test_counter = 0;

	_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
	assert(_candidates_v.size() == 0);
	Boxes *b;
	int winsize;
	int full_window;

	set<int>::iterator iter;
	vector<int> runnable_box_v;
	pthread_mutex_lock(&__box_work_set_mutex);
	for ( iter = __box_work_set.begin(); iter != __box_work_set.end(); iter++ )
		runnable_box_v.push_back(*iter);
	pthread_mutex_unlock(&__box_work_set_mutex);

	double t_a_stop = get_etime(&_first);
	_time_spent_copying_runnable_boxes += t_a_stop - t_a_start;
	//for ( int i = 0; i <= _catalog->getMaxBoxId(); i++ )
	//for ( int j = 0; j < _actual_box_ids.size(); j++ )
	double t_b_start = get_etime(&_first);

	//cout << " NO RUNNABLE BOXES? " << runnable_box_v.size() << endl << endl;
	for ( int j = 0; j < runnable_box_v.size(); j++ )
	{
		//int i = _actual_box_ids[j];
		int i = runnable_box_v[j];
		//cout << " TRY " << j << "/" << runnable_box_v.size() << " and box id " << i << " TRUE BOX ID " << _bqm[i]._box_id << endl;
		if ( i <= _catalog->getMaxBoxId())
		{
			if (  _bqm[i]._box_id >= 0 )
			{
				b = _catalog->getBox(i);
				winsize = b->getWinSize();
				if ( b->lockBoxTry() == 0 )
				{
					full_window = 0;
					for ( int k = 0; k < _bqm[i]._num_input_queues; k++ )
					{
						//printf("GOT TO AVG TIME STAMP(%i): %f\n",i,shm_get_average_timestamp(_bqm[i]._input_queue_id[k]));
						//printf("GOT TO AVG TIME STAMP(%i) a : %f\n",i,shm_get_sum_timestamp(_bqm[i]._input_queue_id[k]));
						test_counter++;
						//fprintf(stderr,"test_counter: %i\n",test_counter);
						// have to fix this for JOIN(join) .. works for UNION
						if (shm_get_num_records_in_queue(_bqm[i]._input_queue_id[k]) >= winsize)
						{
							full_window = 1;
							break;
						}
					}
					if ( full_window == 1 )
					{
						BoxLatency bl;
						bl._box_id = i;

						_candidates_v.push_back(bl);
					}
					b->unlockBox();
				}
				else
				{
					/*timeval now;
					gettimeofday(&now,NULL);
					printf("findEligibleBox:: box %i could NOT be LOCKED -- TIME: %d %d -- QUEUE: %d\n",i, now.tv_sec, now.tv_usec, shm_get_num_records_in_queue(_bqm[i]._input_queue_id[0]));
					*/
				}
			}
		}
	}
	double t_b_stop = get_etime(&_first);
	_time_spent_verifying_lockable_boxes += t_b_stop - t_b_start;
	//shm_print("SCHEDULER");
	
	/**
	if (_candidates_v.size() > 0) {
	  printf("findEligibleBox:: CANDIDATE BOXES:");
	  for ( int i = 0; i < _candidates_v.size(); i++ )
	    {
	      printf(" %i",_candidates_v[i]._box_id);
	    }
	  printf("\n");
	}
	*/
	if ( _candidates_v.size() == 0 )
		return -1;
	else
	{
		double t_c_start = get_etime(&_first);
		static int last_box = 0;
	//printf("findEligibleBox:: PREVIOUS CANDIDATE: %i\n",last_box);

		int new_box = -1;
		int new_box_index = 0;
		for ( int i = 0; i < _candidates_v.size(); i++ )
		{
			if ( _candidates_v[i]._box_id > last_box )
			{
				new_box = _candidates_v[i]._box_id;
				new_box_index = i;
				break;
			}
		}
		if ( new_box < 0 )
			new_box = _candidates_v[0]._box_id;
		else
		{
			vector<BoxLatency> tmp;
			int rr_index = new_box_index;
			for ( int i = 0; i < _candidates_v.size(); i++ )
			{
				tmp.push_back(_candidates_v[rr_index]);
				rr_index++;
				if ( rr_index >= _candidates_v.size() )
					rr_index = 0;
			}
			_candidates_v.erase(_candidates_v.begin(), _candidates_v.end());
			_candidates_v = tmp;
			assert(tmp.size() == _candidates_v.size());
			
		}

		last_box = new_box;
		//printf("findEligibleBox:: CANDIDATE SELECTION: %i\n",new_box);
		double t_c_stop = get_etime(&_first);
		_time_spent_loading_candidate_boxes += t_c_stop - t_c_start;
		return(new_box);
	}

	return -1;
	
}
bool Scheduler::isAppRunnable(int app_num)
{
	// simple .. app is runnable if there is at lease one tuple in one of its queues and the 
	// corresponding box is lockable
	int box_id;
	for ( int j = 0; j < (*_post_order_traversals[app_num]).size(); j++ )
	{
		box_id = (*_post_order_traversals[app_num])[j];
		for ( int i = 0; i < _bqm[box_id]._num_input_queues; i++ )
		{
			if ( shm_get_num_records_in_queue(_bqm[box_id]._input_queue_id[i]) > 0 )
			{
				Boxes *b = _catalog->getBox(box_id);
				if ( b->lockBoxTry() == 0 )
				{
					b->unlockBox();
					return true;
				}
			}
		}
	}
	return false;
}
int Scheduler::findEligibleApplication_RR()
{
	if ( _top_k_spanner_flag == true )
		//findEligibleBox_RR_nolock();
		findEligibleBox_RR();
	static int app = -1;
	//int num_tried = 0;
	for ( int i = 0; i < _num_applications; i++ )
	{
		app++;
		if ( app >= _num_applications )
			app = 0;
		if (pthread_mutex_trylock(_app_mutexes[app]) == 0 )
		{
			if ( isAppRunnable(app) == true )
			{
			
				return app; 
			}
			else
				pthread_mutex_unlock(_app_mutexes[app]);
		}
	}
	return -1;
}
int Scheduler::findEligibleApplication_Random()
{
	if ( _top_k_spanner_flag == true )
		findEligibleBox_Random();
		//findEligibleBox_Random_nolock();
	//static unsigned int seed = 1;
	vector<int> eligible_apps;
	for ( int i = 0; i < _num_applications; i++ )
		eligible_apps.push_back(i);
	random_shuffle(eligible_apps.begin(),eligible_apps.end());
	for ( int i = 0; i < eligible_apps.size(); i++ )
	{
		if (pthread_mutex_trylock(_app_mutexes[eligible_apps[i]]) == 0 )
		{
			if ( isAppRunnable(eligible_apps[i]) == true )
				return eligible_apps[i]; 
			else
				pthread_mutex_unlock(_app_mutexes[eligible_apps[i]]);
		}
	}
	return -1;
}
int Scheduler::findEligibleApplication_Bucketing()
{
	int temp_id;
	//if ( _use_buckets_flag == true )
	//	temp_id = findEligibleBox_SlopeSlack_nolock();
	//else
		temp_id = findEligibleBox_Bucketing(_actual_box_ids[_actual_box_ids.size()-1]);

	if ( temp_id == -1 )
		return -1;
	
	multimap<int,int>::iterator iter;
	vector<int> eligible_apps;
	for ( iter = _bam.lower_bound(temp_id);
			iter != _bam.upper_bound(temp_id);
			iter++ )
	{
		eligible_apps.push_back((*iter).second);
	}
	if ( eligible_apps.size() > 1 )
	{
		random_shuffle(eligible_apps.begin(),eligible_apps.end());
		// if there is no sharing, should never get here
		fprintf(stderr,"BAD place to be if there is no sharing\n");
		assert( false );
		exit(0);
	}
	for ( int i = 0; i < eligible_apps.size(); i++ )
	{
		if (pthread_mutex_trylock(_app_mutexes[eligible_apps[i]]) == 0 )
		{
			if ( isAppRunnable(eligible_apps[i]) == true )
				return eligible_apps[i]; 
			else
				pthread_mutex_unlock(_app_mutexes[eligible_apps[i]]);
		}
	}
	return -1;

	
}
int Scheduler::findEligibleApplication_BoxLatency()
{
	int temp_id;
	//if ( _use_buckets_flag == true )
	//	temp_id = findEligibleBox_SlopeSlack_nolock();
	//else
		temp_id = findEligibleBox_SlopeSlack();

	if ( temp_id == -1 )
		return -1;
	
	multimap<int,int>::iterator iter;
	vector<int> eligible_apps;
	for ( iter = _bam.lower_bound(temp_id);
			iter != _bam.upper_bound(temp_id);
			iter++ )
	{
		eligible_apps.push_back((*iter).second);
	}
	if ( eligible_apps.size() > 1 )
	{
		random_shuffle(eligible_apps.begin(),eligible_apps.end());
		// if there is no sharing, should never get here
		fprintf(stderr,"BAD place to be if there is no sharing\n");
		assert( false );
		exit(0);
	}
	for ( int i = 0; i < eligible_apps.size(); i++ )
	{
		if (pthread_mutex_trylock(_app_mutexes[eligible_apps[i]]) == 0 )
		{
			if ( isAppRunnable(eligible_apps[i]) == true )
				return eligible_apps[i]; 
			else
				pthread_mutex_unlock(_app_mutexes[eligible_apps[i]]);
		}
	}
	return -1;

	
}
int Scheduler::findEligibleApplication_LQF()
{
	int temp_id;
	if ( _use_buckets_flag == true )
		temp_id = findEligibleBox_LQF_nolock();
	else
		temp_id = findEligibleBox_LQF();
	if ( temp_id == -1 )
		return -1;
	

	multimap<int,int>::iterator iter;
	vector<int> eligible_apps;
	for ( iter = _bam.lower_bound(temp_id);
			iter != _bam.upper_bound(temp_id);
			iter++ )
	{
		eligible_apps.push_back((*iter).second);
	}
	random_shuffle(eligible_apps.begin(),eligible_apps.end());
	for ( int i = 0; i < eligible_apps.size(); i++ )
	{
		if (pthread_mutex_trylock(_app_mutexes[eligible_apps[i]]) == 0 )
		{
			if ( isAppRunnable(eligible_apps[i]) == true )
				return eligible_apps[i]; 
			else
				pthread_mutex_unlock(_app_mutexes[eligible_apps[i]]);
		}
	}
	return -1;
	
}

void Scheduler::printQosGraphs()
{
	printf("QoS graphs\n");
	printf("==========\n");
	for ( int i = 0; i < _app_v.size(); i++ )
	{
		printf("app[%i]: ",i);
		for ( int j = 0; j < _qos_graphs[i]._points.size(); j++ )
		{
			printf("(%f,%f)",_qos_graphs[i]._points[j]->_x,_qos_graphs[i]._points[j]->_y);
		}
		printf("\n");
	}
}
void Scheduler::initQosGraphs()
{

	_qos_graphs = new qos_struct[_app_v.size()];
	QueryNetwork *q_net = _catalog->getQNet();
	ApplicationMap &app_map = q_net->getApplications();

	for ( int i = 0; i < _app_v.size(); i++ )
	{
		for (ApplicationMapIter iter = app_map.begin(); iter != app_map.end(); iter++)
		{
			if(((*iter).second)->getIncomingArcId() == _app_v[i])
			{
				//printf("AAA: FOUND 2 app[%i]:arc_id: %i\n",i,((*iter).second)->getIncomingArcId());
				// found match;
				QoS* qos = ((*iter).second)->getQoS();
				QoSEntryVector& qos_graph = qos->getGraph();
				//printf("AAA: qos_graph.size(): %i\n",qos_graph.size());
				for ( int j = 0; j < qos_graph.size(); j++ )
				{
					_qos_graphs[i].insertPoint(qos_graph[j]->x, qos_graph[j]->utility);
					//printf("AAA x: %f   utility: %f\n",qos_graph[j]->x,qos_graph[j]->utility);
				}
			}

		}
	}

	__qos_graphs = _qos_graphs;

	/*
	_qos_graphs[0].insertPoint(0.0,1.0);
	_qos_graphs[0].insertPoint(0.1,1.0);
	_qos_graphs[0].insertPoint(100.0,0.0);

	_qos_graphs[1].insertPoint(0.0,1.0);
	_qos_graphs[1].insertPoint(3.0,1.0);
	_qos_graphs[1].insertPoint(100.0,0.0);

	_qos_graphs[2].insertPoint(0.0,1.0);
	_qos_graphs[2].insertPoint(10.,1.0);
	_qos_graphs[2].insertPoint(100.0,0.0);

	_qos_graphs[3].insertPoint(0.0,1.0);
	_qos_graphs[3].insertPoint(0.1,1.0);
	_qos_graphs[3].insertPoint(100.0,0.0);

	_qos_graphs[4].insertPoint(0.0,1.0);
	_qos_graphs[4].insertPoint(0.1,1.0);
	_qos_graphs[4].insertPoint(100.0,0.0);

	_qos_graphs[5].insertPoint(0.0,1.0);
	_qos_graphs[5].insertPoint(0.1,1.0);
	_qos_graphs[5].insertPoint(100.0,0.0);

	_qos_graphs[6].insertPoint(0.0,1.0);
	_qos_graphs[6].insertPoint(0.1,1.0);
	_qos_graphs[6].insertPoint(100.0,0.0);

	_qos_graphs[7].insertPoint(0.0,1.0);
	_qos_graphs[7].insertPoint(0.1,1.0);
	_qos_graphs[7].insertPoint(100.0,0.0);

	_qos_graphs[8].insertPoint(0.0,1.0);
	_qos_graphs[8].insertPoint(0.1,1.0);
	_qos_graphs[8].insertPoint(100.0,0.0);

	_qos_graphs[9].insertPoint(0.0,1.0);
	_qos_graphs[9].insertPoint(0.1,1.0);
	_qos_graphs[9].insertPoint(100.0,0.0);
	*/

}
SlopeSlack Scheduler::findSlopeAt(double current_latency,int box_id)
{
	//printf("findSlopeAt: box: %i  current_latency: %f\n",box_id,current_latency);
	SlopeSlack ss;
	//ss._slope = MAXDOUBLE;
	ss._slope = 0.0;
	ss._slack = 0.0;
	//if ( box_id == 0 )
	//	fprintf(stderr,"findSlopeAt .. box_id: %i  current_latency: %f\n",box_id,current_latency);
	for ( int i = 0; i < (*_box_qos_graphs)[box_id]->_points.size()-1; i++ )
	{
		if ( current_latency > (*_box_qos_graphs)[box_id]->_points[i]->_x &&
				current_latency < (*_box_qos_graphs)[box_id]->_points[i+1]->_x)
		{
			ss._slack = (*_box_qos_graphs)[box_id]->_points[i+1]->_x - current_latency;
			if ( (*_box_qos_graphs)[box_id]->_points[i]->_y == 
					(*_box_qos_graphs)[box_id]->_points[i+1]->_y)
				ss._slope = 0.0;
			else
				ss._slope = - ( (*_box_qos_graphs)[box_id]->_points[i]->_y - (*_box_qos_graphs)[box_id]->_points[i+1]->_y ) / 
						( (*_box_qos_graphs)[box_id]->_points[i]->_x - (*_box_qos_graphs)[box_id]->_points[i+1]->_x );
		}
		assert((*_box_qos_graphs)[box_id]->_points[i]->_x != (*_box_qos_graphs)[box_id]->_points[i+1]->_x);
	}
	//if ( box_id == 0 )
	//	fprintf(stderr,"findSlopeAt .. box_id: %i   slope: %f  slack: %f\n",box_id,ss._slope,ss._slack);
	return ss;
}
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
void Scheduler::setAppScheduleType(int type)
{
	pthread_mutex_lock(_app_schedule_mutex);
	_app_schedule_type = type;
	pthread_mutex_unlock(_app_schedule_mutex);
}
int Scheduler::getAppScheduleType()
{
	int ret_val;
	pthread_mutex_lock(_app_schedule_mutex);
	ret_val = _app_schedule_type;
	pthread_mutex_unlock(_app_schedule_mutex);
	return ret_val;
}
void Scheduler::setBoxScheduleType(int type)
{
	pthread_mutex_lock(_box_schedule_mutex);
	_box_schedule_type = type;
	pthread_mutex_unlock(_box_schedule_mutex);
}
int Scheduler::getBoxScheduleType()
{
	int ret_val;
	pthread_mutex_lock(_box_schedule_mutex);
	ret_val = _box_schedule_type;
	pthread_mutex_unlock(_box_schedule_mutex);
	return ret_val;
}
void Scheduler::setSchedBy(int type)
{
	pthread_mutex_lock(_sched_by_mutex);
	_sched_by = type;
	pthread_mutex_unlock(_sched_by_mutex);
}
int Scheduler::getSchedBy()
{
	int ret_val;
	pthread_mutex_lock(_sched_by_mutex);
	ret_val = _sched_by;
	pthread_mutex_unlock(_sched_by_mutex);
	return ret_val;
}
long Scheduler::getSchedulingTicks()
{
	long ret_val;
	pthread_mutex_lock(_scheduling_ticks_mutex);
	ret_val = _total_ticks;
	pthread_mutex_unlock(_scheduling_ticks_mutex);
	return ret_val;
}
void Scheduler::setBoxTraversalType(int type)
{
	pthread_mutex_lock(_box_traversal_type_mutex);
	_traversal_type = type;
	pthread_mutex_unlock(_box_traversal_type_mutex);
}
int Scheduler::getBoxTraversalType()
{
	int ret_val;
	pthread_mutex_lock(_box_traversal_type_mutex);
	ret_val = _traversal_type;
	pthread_mutex_unlock(_box_traversal_type_mutex);
	return ret_val;
}
int Scheduler::MITRE_initOutputConnections() 
{
	printf("[MITRE] Establishing communication links with output GUI...\n"); 

	// Blank out the sockets
	int s1;

	static const char *server_host_name = "localhost";  // CHANGE to whatever it should be 
	static char server_can_host_name[64];
	static struct sockaddr_in server_in; // server socket addr
	server_in.sin_family = AF_INET;

	if ((server_in.sin_addr.s_addr = 
				resolve_name(const_cast<char *>(server_host_name), server_can_host_name, 64)) == 0)
	{
		printf("[MITRE] Unknown host %s!\n", server_host_name);
		assert( false );
		exit(1);
	}

	server_in.sin_port = htons(6402); // The port create the sockets
	if (((s1 = socket(PF_INET, SOCK_STREAM, 0)) < 0))
	{
		perror("[MITRE] socket failed");
		assert( false );
		exit(1);
	}

	printf("[MITRE] Socket endpoints %d created\n", s1);                     

	// setup the connections
	if ((connect(s1, (struct sockaddr *) &server_in, sizeof(server_in)) < 0))
	{
		perror("[MITRE] connect failed");
		assert( false );
		exit(1);
	}
	printf("[MITRE] Outgoing communications established\n");



	return s1;
}                                                               
u_long Scheduler::resolve_name(char* namep, char canonp[], int canonl) 
{ 
	struct hostent *hp;
	u_long inetaddr;
	int n;

	if (isdigit(*namep)) 
	{
		inetaddr = (u_long) inet_addr(namep);
		*canonp = '\0';
		return (inetaddr);
	}
	else 
	{
		if (NULL == (hp = gethostbyname(namep)))
			return(0);
		n = ((n = strlen(hp->h_name)) >= canonl) ? canonl-1 : n;
		memcpy(canonp, hp->h_name, n);
		canonp[n] = '\0';
		return (*(u_long *) hp->h_addr);
	}
}  
void Scheduler::init_etime(struct itimerval *first)
{
	// This is just to shut-up Valgrind. I don't know whether or not the
	// Linux API actually requires this intialization. -cjc, 17 Mar 2003...
	first->it_interval.tv_sec = 0;
	first->it_interval.tv_usec = 0;

	first->it_value.tv_sec = 1000000;
	first->it_value.tv_usec = 0;
	//setitimer(ITIMER_PROF,first,NULL);
	setitimer(ITIMER_VIRTUAL,first,NULL);
}
double Scheduler::get_etime(struct itimerval *first)
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
void Scheduler::getAppCost(int application)
{
	//vector<int> *traversal;
	double app_cost = 0.0;
	int i = application;
	{
		traverseAppCost(_application_trees[i],i,&app_cost);
		//printf("TRT: final App Cost: %f\n",app_cost);
		_app_costs.push_back(app_cost);
		_total_network_cost += app_cost;
	}
}
void Scheduler::traverseAppCost(AppNode *node,int app_num, double *app_cost)
{
	for ( int i = 0; i < (node->_input_boxes)->size(); i++ )
	{
		if ((*(node->_input_boxes))[i] != NULL )
		{
			traverseAppCost((*(node->_input_boxes))[i],app_num,app_cost);
		}
		else // go to leaf
		{
			BoxTraversal bt;
			bt._traversal_cost = toRootTraversal(node,&bt._traversal,PARTIAL_COST);
			// since this is per application, we have to divide by the number of output arcs for the box
			*app_cost += bt._traversal_cost;
			
			//printf("\nLEAF cost to root: %f   app_cost: %f\n",bt._traversal_cost,*app_cost);
		}
	}
}
void Scheduler::generateMinLatencyTraversals(int application)
{
	vector<int> *traversal;
	//printf("GOT TO ML_Traversals.size(): %i   %i\n",_ML_traversals.size(),_application_trees.size());
	//for ( int i = 0; i < _application_trees.size(); i++ )
	int i = application;
	{
		vector< BoxTraversal > to_root_traversals;
		traverseAppMinLatency(_application_trees[i],i,&to_root_traversals);
		sort(to_root_traversals.begin(),to_root_traversals.end());
		traversal = createSingleTraversal(&to_root_traversals);
		_ML_traversals.push_back(traversal);
	}
}
void Scheduler::traverseAppMinLatency(AppNode *node,int app_num, vector<BoxTraversal> *to_root_traversals)
{
	BoxTraversal bt;
	if ( node->_node_type != ROOT_NODE )
	{
		bt._traversal_cost = toRootTraversal(node,&bt._traversal,FULL_COST);
		to_root_traversals->push_back(bt);
	}
	for ( int i = 0; i < (node->_input_boxes)->size(); i++ )
	{
		if ((*(node->_input_boxes))[i] != NULL )
		{
			traverseAppMinLatency((*(node->_input_boxes))[i],app_num,to_root_traversals);
		}
	}
}
double Scheduler::toRootTraversal(AppNode *node, vector<int> *traversal, int cost_type)
{
	Boxes *b = NULL;
	Boxes *prev_b = NULL;
	double latency_cost = 0.0;
	double output_cost = 0.0;
	double output_selectivity = 0.0;
	//double partial_cost = 0.0; // used for application sharing
	double box_cost = 0.0;
	AppNode *curr_node;
	AppNode *traverse_node;
	curr_node = node;
	//do
	double mult_selectivity = 1.0;

	while(curr_node->_node_type != ROOT_NODE)
	{
		b = _catalog->getBox(curr_node->_box_id);
		if (b == NULL) {
		  cerr << "Expected to find a box with id " << curr_node->_box_id
		       << " but none was there" << endl;
		  assert(false);
		}
		
		latency_cost += (b->getCost()/b->getSelectivity());
		//partial_cost += ((b->getCost()/b->getSelectivity())/((double)_bqm[curr_node->_box_id]._num_output_queues));

		if ( prev_b != NULL )
			mult_selectivity *= prev_b->getSelectivity();
		box_cost += (b->getCost()*mult_selectivity)/((double)_bqm[curr_node->_box_id]._num_output_queues);
		

		output_selectivity = b->getSelectivity();
		traverse_node = curr_node;
		while(traverse_node->_parent->_node_type != ROOT_NODE)
		{
			traverse_node = traverse_node->_parent;
			Boxes *b1 = _catalog->getBox(traverse_node->_box_id);
			output_selectivity *= b1->getSelectivity();
		}
		output_cost += (b->getCost()/output_selectivity);


		traversal->push_back(curr_node->_box_id);

		prev_b = b;
		curr_node = curr_node->_parent;
	}
	//while(curr_node->_node_type != ROOT_NODE);
	//traversal->push_back(curr_node->_box_id);
	if ( cost_type == FULL_COST )
		//return latency_cost;
		return output_cost;
	else if ( cost_type == PARTIAL_COST )
		//return partial_cost;
		return box_cost;
	else
	{
		fprintf(stderr,"BAD COST TYPE in toRootTraversal()\n");
		assert( false );
		exit(0);
	}
}
void Scheduler::generateMinMemoryTraversals(int application)
{
	vector<int> *traversal = new vector<int>;
	vector<BoxMemory> bm_list;
	Boxes *b;
	for ( int i = 0; i < _post_order_traversals[application]->size(); i++ )
	{
		BoxMemory bm;
		bm._box_id = (*_post_order_traversals[application])[i];
		b = _catalog->getBox(bm._box_id);
		Arcs *a = _catalog->getArc(_bqm[bm._box_id]._input_queue_id[0]);
		TupleDescription *td = a->getTupleDescr();
		int tuple_size = td->getSize() + TUPLE_TIMESTAMP_SIZE + TUPLE_STREAMID_SIZE;
		bm._memory_release = (tuple_size * ( 1.0 - b->getSelectivity()))/b->getCost();
		bm_list.push_back(bm);
	}
	random_shuffle(bm_list.begin(),bm_list.end());
	sort(bm_list.begin(),bm_list.end());
	reverse(bm_list.begin(),bm_list.end());
	//printf("MM start: ");
	//for ( int i = 0; i < bm_list.size(); i++ )
	//{
	//	printf("%i, ",bm_list[i]._box_id);
	//}
	//printf("\n");
	map<int,int> mm_priority_map;
	for ( int i = 0; i < bm_list.size(); i++ )
	{
		mm_priority_map[bm_list[i]._box_id] = i;  // assign min-memory priorities to boxes
	}
	set<int> avail_boxes_set;
	for ( int i = 0; i < bm_list.size(); i++ )
	{
		avail_boxes_set.insert(bm_list[i]._box_id);
	}
	map<int,AppNode*> *map_ptr = _app_box_node_v[application];
	int i = 0;
	//printf("MM traversal: ");
	while ( i < bm_list.size() )
	{
		if ( avail_boxes_set.find(bm_list[i]._box_id) != avail_boxes_set.end() )
		{
			traversal->push_back(bm_list[i]._box_id);
			//printf("%i, ",bm_list[i]._box_id);
			avail_boxes_set.erase(bm_list[i]._box_id);
			AppNode* parent_node = (*map_ptr)[bm_list[i]._box_id]->_parent;
			if ( parent_node != NULL && parent_node->_node_type != ROOT_NODE )
			{
				if ( avail_boxes_set.find(parent_node->_box_id) == avail_boxes_set.end() )
				{
					avail_boxes_set.insert(parent_node->_box_id);
				}
				i = -1;
			}
		}
		i++;
	}
	//printf("\n");
	_MM_traversals.push_back(traversal);
	/*
	  for ( int i = 0; i < bm_list.size(); i++ )
	{
		int box_id = bm_list[i]._box_id;
		traversal->push_back(box_id);
		map<int,AppNode*> *map_ptr = _app_box_node_v[application];
		AppNode* parent_node = (*map_ptr)[box_id]->_parent;
		
		if ( parent_node != NULL )
		{
			if ( i < (bm_list.size()-1) )
			{
				while ( (mm_priority_map[parent_node->_box_id] >= mm_priority_map[bm_list[i+1]._box_id]) &&
						parent_node->_node_type != ROOT_NODE )
				{
					traversal->push_back(parent_node->_box_id);
					parent_node = (*map_ptr)[parent_node->_box_id]->_parent;
				}
			}
			else
			{
				while ( parent_node->_node_type != ROOT_NODE )
				{
					traversal->push_back(parent_node->_box_id);
					parent_node = (*map_ptr)[parent_node->_box_id]->_parent;
				}
			}
		}
	}
	_MM_traversals.push_back(traversal);
	*/
}
vector<int>* Scheduler::createSingleTraversal(vector< BoxTraversal > *to_root_traversals)
{
	vector<int>	*traversal = new vector<int>;

	for ( int i = 0; i < to_root_traversals->size(); i++ )
	{
		for ( int j = 0; j < (*to_root_traversals)[i]._traversal.size(); j++ )
		{
			traversal->push_back((*to_root_traversals)[i]._traversal[j]);
		}
	}
	return traversal;
}
void Scheduler::printMMTraversals()
{
	for ( int i = 0; i < _MM_traversals.size(); i++ )
	{
		printf("MM Traversal[%i]: ",i);
		for ( int j = 0; j < _MM_traversals[i]->size(); j++ )
		{
			printf(" %i,", (*_MM_traversals[i])[j]);
		}
		printf("\n");
	}
}
void Scheduler::printMLTraversals()
{
	for ( int i = 0; i < _ML_traversals.size(); i++ )
	{
		printf("ML Traversal[%i]: ",i);
		for ( int j = 0; j < _ML_traversals[i]->size(); j++ )
		{
			printf(" %i,", (*_ML_traversals[i])[j]);
		}
		printf("\n");
	}
}
int Scheduler::checkAppForTuples(int app)
{
	int total_tuples = 0;
	//fprintf(stderr,"runnable boxes for app[%i]: ",app);
	for ( int i = 0; i < _post_order_traversals[app]->size(); i++)
	{
		int box_id = (*_post_order_traversals[app])[i];
		int num_in_tuples = 0;
		for ( int j = 0; j < _bqm[box_id]._num_input_queues; j++ )
		{
			num_in_tuples += shm_get_num_records_in_queue(_bqm[box_id]._input_queue_id[j]);
		}
		if ( num_in_tuples > 0 )
			fprintf(stderr,"%i(%i,%f,%f), ",box_id,num_in_tuples,_candidates_v[i]._ss._slope,_candidates_v[i]._ss._slack);
			//printf("app: %i runnable box: %i  tuples: %i\n",app,box_id,num_in_tuples);
		total_tuples += num_in_tuples;
	}
	fprintf(stderr,"\n");
	return total_tuples;
}
void Scheduler::testBoxWorkSet()
{
	set<int>::iterator iter;

	pthread_mutex_lock(&__box_work_set_mutex);
	fprintf(stderr,"box_work_set: ");
	for ( iter = __box_work_set.begin(); iter != __box_work_set.end(); iter++ )
		fprintf(stderr,"%i, ", *iter);
	fprintf(stderr,"\n");


	fprintf(stderr,"boxes from shm: ");
	for ( int i = 0; i < _actual_box_ids.size(); i++ )
	{
		int box_id = _actual_box_ids[i];
		for ( int j = 0; j < _bqm[box_id]._num_input_queues; j++ )
		{
			if ( shm_get_num_records_in_queue(_bqm[box_id]._input_queue_id[j]) > 0 )
			{
				fprintf(stderr,"%i, ", box_id);
				if ( __box_work_set.find(box_id) != __box_work_set.end() )
				{
					break;
				}
				else
				{
					fprintf(stderr,"ERROR :: __box_work_set is wrong could not find box id: %i\n",box_id);
					assert( false );
					exit(0);
				}
			}
		}
	}
	fprintf(stderr,"\n");
	pthread_mutex_unlock(&__box_work_set_mutex);
}
void Scheduler::testCandidates()
{
	set<int>::iterator iter;

	set<int> candidates_s;
	for ( int i = 0; i < _candidates_v.size(); i++ )
		candidates_s.insert(_candidates_v[i]._box_id);

	set<int> missed_s;
	fprintf(stderr,"boxes from shm: ");
	for ( int i = 0; i < _actual_box_ids.size(); i++ )
	{
		int box_id = _actual_box_ids[i];
		for ( int j = 0; j < _bqm[box_id]._num_input_queues; j++ )
		{
			if ( shm_get_num_records_in_queue(_bqm[box_id]._input_queue_id[j]) > 0 )
			{
				fprintf(stderr,"%i, ", box_id);
				if ( candidates_s.find(box_id) != candidates_s.end() )
				{
					break;
				}
				else
				{
					Boxes *b = _catalog->getBox(box_id);
					if ( b->lockBoxTry() == 0 )
					{
						//fprintf(stderr,"ERROR :: candidates_v is wrong could not find box id: %i\n",box_id);
						missed_s.insert(box_id);
						b->unlockBox();
						//exit(0);
					}
				}
			}
		}
	}
	fprintf(stderr,"\n");
	fprintf(stderr,"missed_s:");
	for ( iter = missed_s.begin(); iter != missed_s.end(); iter++ )
		fprintf(stderr,"%i, ",*iter);
	fprintf(stderr,"\n");

}
double Scheduler::findMaxSlack(int box_id, qos_struct *qos_graph)
{
	double largest_slack = 0.0;
	for (int i = 0; i < qos_graph->_points.size()-1; i++ )
	{
		double slack = qos_graph->_points[i+1]->_x - qos_graph->_points[i]->_x;
		//fprintf(stderr,"box: %i point[%i]:(%f,%f)  point[%i]:(%f,%f)  slack: %f\n",box_id,i,qos_graph->_points[i]->_x,qos_graph->_points[i]->_y,i+1,qos_graph->_points[i+1]->_x,qos_graph->_points[i+1]->_y,slack);
		assert ( slack > 0.0 ); 
		
		if ( slack > largest_slack )
		{
			largest_slack = slack;
		}
	}
	return largest_slack;
}
double Scheduler::findMaxSlope(int box_id, qos_struct *qos_graph)
{
	double largest_slope = 0.0;
	for (int i = 0; i < qos_graph->_points.size()-1; i++ )
	{
		double slope = - (qos_graph->_points[i+1]->_y - qos_graph->_points[i]->_y)/
						(qos_graph->_points[i+1]->_x - qos_graph->_points[i]->_x);
		slope = fabs(slope); 
		//printf("box_id: %i   slope: %f\n",box_id,slope);
		
		if ( slope > largest_slope )
		{
			largest_slope = slope;
		}
	}
	return largest_slope;
}
double Scheduler::findMaxLatency(int box_id, qos_struct *qos_graph)
{
	double max_latency = 0.0;
	for (int i = 0; i < qos_graph->_points.size(); i++ )
	{
		if ( qos_graph->_points[i]->_x > max_latency )
		{
			max_latency = qos_graph->_points[i]->_x;
		}
	}
	return max_latency;
}
void Scheduler::initPriorityGrid()
{
	double largest_slack = 0.0;
	double largest_slope = 0.0;
	double max_latency = 0.0;

	//fprintf(stderr,"begin initPriorityGrid()\n");
	for ( int j = 0; j < _actual_box_ids.size(); j++ )
	{
		int box_id = _actual_box_ids[j];
		//printf("GOT TO box_id: %i\n",box_id);
		//fprintf(stderr,"GOT TO (*_box_qos_graphs)[%i]->_points.size(): %i\n",box_id,(*_box_qos_graphs)[box_id]->_points.size());
		for ( int x = 0; x < (*_box_qos_graphs)[box_id]->_points.size(); x++)
		{
			//fprintf(stderr,"[%i]:(%f,%f)\n",x,(*_box_qos_graphs)[box_id]->_points[x]->_x,(*_box_qos_graphs)[box_id]->_points[x]->_y);
		}
		double slack = findMaxSlack(box_id,(*_box_qos_graphs)[box_id]);
		if ( slack > largest_slack )
		{
			largest_slack = slack;
		}
		double slope = findMaxSlope(box_id,(*_box_qos_graphs)[box_id]);
		if ( slope > largest_slope )
		{
			largest_slope = slope;
		}
		double latency = findMaxLatency(box_id,(*_box_qos_graphs)[box_id]);
		if ( latency > max_latency )
		{
			max_latency = latency;
		}
	}
	//printf("largest_slack = %f\n",largest_slack);
	//printf("largest_slope = %f\n",largest_slope);
	//printf("max_latency = %f\n",max_latency);

	//_num_buckets = 50;
	_priority_grid = new PriorityGrid(_num_buckets);
	_grid_pointers = new GridPointers(_priority_grid,_num_buckets, largest_slack, largest_slope, max_latency);
	for ( int j = 0; j < _actual_box_ids.size(); j++ )
	{
		int box_id = _actual_box_ids[j];
		_grid_pointers->loadGridPointers(_actual_box_ids[j],(*_box_qos_graphs)[box_id]);
	}
	
	_priority_grid->createPriorityVector();
	_priority_grid->createUrgencyList();
	_priority_grid_bucket_vector = _priority_grid->getPriorityVector();
	//fprintf(stderr,"_priority_grid_bucket_vector->size(): %i\n",_priority_grid_bucket_vector->size());

	//_priority_grid->printGridPointers();
	//for ( int j = 0; j < _actual_box_ids.size(); j++ )
	//{
	//	int box_id = _actual_box_ids[j];
		//_grid_pointers->printBoxGridPointers(box_id);
	//}
}
