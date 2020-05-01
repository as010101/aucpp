#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "WorkerThread.H"
#include <unistd.h>
#include "ResampleQBox.H"
#include "JoinQBox.H"
#include "FilterQBox.H"
#include "MapQBox.H"    
#include "RestreamQBox.H"    
#include "DropQBox.H"
#include "UnionQBox.H"  
#include "BSortQBox.H"
#include "AggregateQBox.H"
#include "ExperimentQBox.H"
#include "HelloWorldQBox.H"
#include "UpdateRelationQBox.H"
#include "ReadRelationQBox.H"
#include "LRUpdateRelationQBox.H"
#include "LRReadRelationQBox.H"
#include "PriorityGrid.H"
#include "BitShit.H"
#include <vector>
#include <map>
#include <sys/times.h>
#include <time.h>
#include <sys/time.h>
#include <PropsFile.H>


#define ROOT_NODE 1
#define BOX_NODE 2
#define LEAF_NODE 3

#define SLOPE_SLACK_TYPE 1
#define BOX_RANDOM_TYPE 2
#define BOX_RR_TYPE 3
#define BOX_LQF_TYPE 4
#define BOX_BUCKETING_TYPE 5

#define APP_RANDOM_TYPE 1
#define APP_RR_TYPE 2
#define APP_SLOPE_SLACK_TYPE 3
#define APP_LQF_TYPE 4
#define APP_BUCKETING_TYPE 5

#define SCHED_BY_APP 1
#define SCHED_BY_BOX 2

//box traversal types
#define MIN_COST 		1
#define MIN_LATENCY 	2
#define MIN_MEMORY 		3

#define FULL_COST 		1
#define PARTIAL_COST	2

class SlopeSlack{
public:
	double _slope;
	double _slack;
};
struct sortable_slope_struct {
	int box_id;
	double slope;
	int eligible;
};
class BoxMemory {
public:
	int _box_id;
	double _memory_release;
	int operator <(const BoxMemory & bq) const;
};
class BoxQueue {
public:
	int _box_id;
	int _largest_input_queue_length;
	int operator <(const BoxQueue & bq) const;
};
class BoxLatency {
public:
	int _box_id;
	SlopeSlack _ss;
	int operator <(const BoxLatency & bl) const;

};

//static  int slopecompare(const sortable_slope_struct *i, const sortable_slope_struct *j);

class AppNode {
public:
	AppNode(int box_id, int node_type,vector<AppNode*> *input_boxes, int out_arc_id) {
		_box_id = box_id;
		_node_type = node_type;
		_input_boxes = input_boxes;
		_out_arc_id = out_arc_id;
		_parent = NULL;
	}
	int 		_box_id;
	int			_node_type;
	vector<AppNode*> *_input_boxes;
	AppNode		*_parent; // this should be changed to a vector to handle splits within apps
	int			_out_arc_id;
};
class AppToBoxMapping {
public:
	AppToBoxMapping(int i) { _app_arc = i; }
	int _app_arc;
	vector < int > _boxes;
	vector < int > _arcs;
};

/*
class Point {
public:
	Point(double x, double y) {_x = x; _y = y;}
	double _x;
	double _y;
	int operator <(const Point & p) const;
};
class qos_struct {
public:
	void insertPoint(double x, double y) {_points.push_back(new Point(x,y));}
	vector<Point*> _points;
};
*/

struct BoxQueueMapping
{
	int _box_id;
	int _num_input_queues;
	vector<int> _input_queue_id;
    	
    int _num_output_queues;
	vector<int> _output_queue_id;

	int      _num_out_ports;
	// this is replicated in QBox.H because I don't know of a better way
	// to do this right now.
    vector<long> *_output_arcs_byport;
};

class BoxListObject
{
public:
	int _box_id;
	AppNode* _node;
};
class BoxTraversal
{
public:
	double 		_traversal_cost;
	vector<int>	_traversal;
	int operator <(const BoxTraversal & bq) const;
};
class Scheduler
{
public:
	Scheduler(int max_queue_size,
			  int max_num_worker_threads,
			  int num_buckets,
			  AppTupleMonitor * pTupleMon
			  );
	
	virtual ~Scheduler();

	// This allows required and optional parameters to be set in one single method
	// call.
	//
	// For optionally set properties, an omission of that property from the props
	// file will simple prevent the corresponding set... method on this class from
	// being invoked.
	//
	// This Scheduler may retain a reference to the specified PropsFile until this
	// Scheduler object is destroyed.
	//
	// Mandatory properties:
	//    Scheduler.max_queue_size (int)
	//    Scheduler.max_num_worker_threads (int)
	//
	// Optional properties:
	//    Scheduler.AppScheduleType (string). Value values:
	//       APP_RANDOM_TYPE
	//       APP_RR_TYPE
	//       APP_SLOPE_SLACK_TYPE
	//       APP_LQF_TYPE
	//       
	//    Scheduler.BoxScheduleType (string). Value values:
	//       SLOPE_SLACK_TYPE
	//       BOX_RANDOM_TYPE
	//       BOX_RR_TYPE
	//       BOX_LQF_TYPE
	//
	//    Scheduler.BoxTraversalType (string). Value values:
	//       MIN_COST
	//       MIN_LATENCY
	//       MIN_MEMORY
	//
	//    Scheduler.SchedBy (string). Value values:
	//       SCHED_BY_APP
	//       SCHED_BY_BOX
	//
	//    Scheduler.TopKSpanner (int)
	//    Scheduler.FixedTrainSize (long)
	//
	// HACK ALERT: This class has setFixedTrainSize as a double, not a long. 
	// However, the XML/PropsFile code we've developed has no support currently
	// for doubles. I checked the Scheduler code for where _fixed_train_size is
	// used, and it always seems to be used as though the programmers wished it
	// was a integer, so this hack seems reasonable. -cjc, 7 Feb 2003.
	void initPropsFromFile(const PropsFile & props)
		throw (exception);

	void 	start();

	void	setBEQSize(int beq_size) {_beq_size = beq_size;}
	void	setAppScheduleType(int type);
	int		getAppScheduleType();
	void	setBoxScheduleType(int type);
	int		getBoxScheduleType();
	void	setBoxTraversalType(int type);
	int		getBoxTraversalType();
	void	setSchedBy(int type);
	int		getSchedBy();
	long	getSchedulingTicks();
	int 	vectorFind(vector<BoxListObject> *v,int box_id);
	pthread_cond_t  *getWakeCondition() {return _wait_cond;}
	pthread_mutex_t *getWakeMutex() {return _wait_mutex;}
	double	getNetworkCost() {return _total_network_cost;}
	void	setTopKSpanner(int k_val) { _top_k_spanner_flag = true; _K = k_val; }

	void	setFixedTrainSize(double size) {_fixed_train_size = size; }
	int 	refillQueue(int num_to_schedule);

	int		_max_queue_size;	// max allowable size for box execution queue
	int		_max_num_worker_threads;
	BoxExecutionQueue 	*_execution_queue;
	WorkerThread		**_thread_pool;
	BoxQueueMapping		*_bqm;
	vector<AppNode*>	_application_trees;
	multimap<int, int> _bam; // box to application mapping
	

	// this is going to be used by worker thread in case of
	// thread per box mode... perhaps there is a cleaner way?
	QBox*	loadBox(int box_id, int train_size);
private:
	Scheduler() {}

	int findEligibleBox_SlopeSlack();
	int findEligibleBox_SlopeSlack_nolock();
	int findEligibleBox_Random();
	int findEligibleBox_Random_nolock();
	int findEligibleBox_RR();
	int findEligibleBox_RR_nolock();
	int findEligibleBox_LQF();
	int findEligibleBox_LQF_nolock();
	int findEligibleBox_Bucketing(int num_to_schedule);
	int findEligibleBox_Bucketing1(int num_to_schedule);
	int findEligibleApplication_RR();
	int findEligibleApplication_Random();
	int findEligibleApplication_BoxLatency();
	int findEligibleApplication_Bucketing();
	int findEligibleApplication_LQF();

	int 	waitForEvent();
	void 	setPriorities();
	void	loadApplicationTree(int curr_app, int arc_id);
	AppNode *traverseApplicationTree(int curr_app, int box_id, int from_arc, vector<BoxListObject> *box_list, vector<int> **parent_upstream_queues);
	void 	printApplicationTrees();
	void 	traverseAppTreePrint(AppNode *node);
	void	getPreOrderTraversals();
	void	traversePreOrder(AppNode *node, int app_num, vector<int> *po_traversal);
	void	getPostOrderTraversals();
	void	traversePostOrder(AppNode *node, int app_num, vector<int> *po_traversal);
	//void 	loadQueueElement(QueueElement *qe);
	int 	traverse_LoadQueue(AppNode *node, int out_arc, QueueElement *qe, BoxList_T *bl, AppNode *parent, map<int,int> *queue_tuples_map, set<int> *locked_boxes_set);
	void	initQosGraphs();
	void	printQosGraphs();
	SlopeSlack	findSlopeAt(double current_time,int box_id);
	void 	loadBoxAppMapping();
	void 	traverseAppTreeBAM(AppNode *node,int app_num);
	double	findYGivenLineAndX(double x1, double y1, double x2, double y2, double x3);
	void 	loadBoxQosGraphs();
	double 	getToRootCost(int box_id);
	double 	findYGivenXAndQos(double x,int qos_graph_num);
	void 	loadSingleBox(int box_id,QueueElement *qe);
	int		MITRE_initOutputConnections();
	u_long 	resolve_name(char* namep, char canonp[], int canonl);
	void 	init_etime(struct itimerval *first);
	double 	get_etime(struct itimerval *first);
	void	generateMinLatencyTraversals(int application);
	void	generateMinMemoryTraversals(int application);
	void	traverseAppMinLatency(AppNode *node,int app_num,vector<BoxTraversal> *to_root_traversals);
	double	toRootTraversal(AppNode *node, vector<int> *traversal, int cost_type);
	vector<int>* createSingleTraversal(vector< BoxTraversal > *to_root_traversals);
	void 	printMLTraversals();
	void 	printMMTraversals();
	void	loadMinLatencyTraversal(int app_num,QueueElement *qe, map<int,int> *queue_tuples_map, set<int> *locked_boxes_set);
	void	createMinLatencyBoxList( vector<int> *traversal_v, BoxList_T *bl, set<int> *locked_boxes_set);
	void	createMinMemoryBoxList( vector<int> *traversal_v, BoxList_T *bl, set<int> *locked_boxes_set);
	void	createMinCostBoxList( vector<int> *traversal_v, BoxList_T *bl, set<int> *locked_boxes_set);
	void 	generateAppBoxNodeMap();
	void 	traverse_loadAppBoxNodeMap(AppNode *node, map<int,AppNode*> *app_node_map);
	void 	getAppCost(int application);
	void 	traverseAppCost(AppNode *node,int app_num, double *app_cost);
	bool	isAppRunnable(int app_num);
	vector<int>* getTopKBoxes(int app,int k);
	void 	createTopKSpannerBoxList( int app, int k, vector<int> *top_k_boxes, set<int> *locked_boxes_set,QueueElement *qe);
	void 	createPinLists(QueueElement *qe, set<int> *dequeuePinList, set<int> *dequeueEnqueuePinList, set<int> *enqueuePinList, map<int,int> *queue_sizes);
	bool 	checkPathToRootUnlocked(int app, int box_id);
	int		checkAppForTuples(int app);
	void	testBoxWorkSet();
	void	testCandidates();
	void	printBoxList(BoxList_T* bl);
	double findMaxSlack(int box_id, qos_struct *qos_graph);
	double findMaxSlope(int box_id, qos_struct *qos_graph);
	double findMaxLatency(int box_id, qos_struct *qos_graph);
	void    initPriorityGrid();


	pthread_mutex_t 	*_wait_mutex;
	pthread_mutex_t 	*_box_schedule_mutex;
	pthread_mutex_t 	*_app_schedule_mutex;
	pthread_mutex_t 	*_sched_by_mutex;
	pthread_mutex_t 	*_scheduling_ticks_mutex;
	pthread_mutex_t 	*_box_traversal_type_mutex;
	pthread_cond_t 		*_wait_cond;
	qos_struct			*_qos_graphs;
	int					*_box_applications;
	vector < AppToBoxMapping* > _app_to_box_mappings;
	sortable_slope_struct *_sort_list;
	map < int, qos_struct* >	*_box_qos_graphs;
	vector<BoxLatency> _candidates_v;
	vector<BoxQueue> 	_candidates_q_v;
	int 				_app_schedule_type;
	int 				_box_schedule_type;
	int					_sched_by;
	long				_total_ticks;
	int					_num_scheduling_events;
	vector<int> 		_app_v;
	vector< vector<int>* > _ML_traversals;  // min-latency application traversals
	vector< vector<int>* > _MM_traversals;  // min-memory application traversals
	vector< vector<int>* > _post_order_traversals;
	vector< vector<int>* > _pre_order_traversals;
	vector< map<int, AppNode*>* > _app_box_node_v;
	int					_traversal_type;
	vector<double>			_app_costs;
	double					_total_network_cost;
	int					_num_beq_elements_pushed;
	int					_K;
	bool				_top_k_spanner_flag;
	bool				_use_buckets_flag;
	int					_bucket_size;
	vector<pthread_mutex_t*>	_app_mutexes;
	double					_fixed_train_size;
	vector< map<int, vector<int> *> *>	_queue_to_upstream_queues_v;  // this is a vector of maps for each application
																	  // (one map per application)
																	  // each map stores a vector of upstream queues
																	  // for each queue of that application
																	  // the purpose of this data structure is to 
																	  // make it easy to calculate the memory required
																	  // for a downstream queue


	int _num_applications;

	vector<int> _actual_box_ids;
	PriorityGrid *_priority_grid;
	GridPointers *_grid_pointers;
	int _num_buckets;
	vector<GridElement*> *_priority_grid_bucket_vector;
	BitShit	*_runnable_boxes_bitarray;
	AppTupleMonitor * _tupleMon;
	double _time_spent_copying_runnable_boxes;
	double _time_spent_verifying_lockable_boxes;
	double _time_spent_loading_candidate_boxes;
	double _time_spent_loading_execution_queue;
	double _time_spent_traverse_loadqueue;
	double _time_spent_create_boxlist;
	double _time_spent_loading_execution_queue_app;
	double _time_spent_loading_dequeue_pin_lists;
	double _time_spent_loading_pin_lists;
	double _time_spent_loadBox;
	double _time_spent_insert_locked_box_set;
	double _time_spent_shm_get_num_records;
	double _time_spent_findEligibleBox_Bucketing;
	double _time_spent_Bucketing_removeFromGrid;
	double _time_spent_Bucketing_finding_candidates;
	double _time_spent_Bucketing_finding_schedulable;
	
	struct itimerval _first;
	int		_beq_size;
};



#endif //SCHEDULER_H

