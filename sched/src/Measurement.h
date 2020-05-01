#ifndef MEASUREMENT_H
#define MEASUREMENT_H


#include <StreamThread.H>
#include <stdio.h>
#include <pthread.h>
#include <set>
#include <vector>
#include <AppTupleMonitor.H>

using namespace std;

// Stop Conditions
#define TUPLES_WRITTEN 	1
#define TIME_STOP		2
#define TUPLES_WRITTEN_TO_MEMORY 	3
#define TUPLES_GENERATED_STOP		4
#define STOP_IGNORE 5

// Measurements
#define TIME_SPENT_SCHEDULING 				1
#define NUM_SCHEDULING_DECISIONS			2
#define NUM_BOX_CALLS						3
#define AVERAGE_LATENCY						4
#define AVERAGE_QOS							5
#define TIME_SPENT_IN_WORKER_THREADS 		6
#define TIME_SCHEDULING_VS_WORKER_THREADS 	7
#define TIME_TOTAL_SCHED_WORKER_THREADS 	8
#define SM_READS							9
#define SM_WRITES							10
#define AVERAGE_TUPLE_TRAIN_SIZE			11
#define DISK_IO                             12
#define BOX_COST                            13
#define DISK_READS                          14
#define DISK_WRITES                         15
#define TOTAL_RUN_TIME                      16
#define QOS_BOUND							17
#define NUM_MALLOCS							18
#define TIME_LOADING_QUEUES					19
#define TIME_UNLOADING_QUEUES				20
#define MEMORY_REMAINING					21
#define TIME_SPENT_IN_STREAM_THREAD			22
#define TIME_SPENT_IN_DO_BOX				23
#define TIME_SPENT_EXECUTING_BOXES			24
#define GENERAL_PROF_STATS					25
#define SCHEDULER_PROF_STATS				26
#define WORKERTHREAD_PROF_STATS				27
#define BOX_OVERHEAD						28
#define AVG_NUM_SCHEDULABLE_BOXES			29

// X Variables
#define INPUT_RATE 			1
#define APP_DEPTH 			2
#define APP_WIDTH 			3
#define MEMORY_SIZE 	   	4
#define BOX_SELECTIVITY 	5
#define APP_COUNT           6
#define K_SPANNER_VAL       7
#define BURST_SIZE          8
#define TRAIN_SIZE          9
#define BOX_COUNT           10
#define NUM_BUCKETS			11
#define BEQ_SIZE			12

class MemRemaining
{
public:
	double 	_elapsed_time;
	int		_mem_remaining;
};
struct stop_cond		// conditions for stopping a run
{
	int		_num_tuples_written;
	int		_num_tuples_written_to_memory;
	int		_experiment_time;
	int		_num_tuples_generated;
};

class Measurement
{
public:
	// 'atmon' is to be used by this Measurement when the stop condition is
	// deemed to be true.
	Measurement(AppTupleMonitor * atmon);
	~Measurement();

	void 		setStopType(char* stop_type);
	int 		getStopType();
	stop_cond* 	getStopCond();
	void		setStopCondTuplesWritten(int tuples_written);
	void		setStopCondTuplesWrittenToMemory(int tuples_written);
	void		setStopCondTimeStop(int seconds);
	void		setStopCondTuplesGenerated(int num_generated);
	inline bool testStreamThreadStopped() { 
				return (( _stream_thread->getTuplesGenerated() >= _sc._num_tuples_generated ) ? true : false);}
	void		testStopCond();
	void		incrementTuplesWritten(int incr);
	void		incrementSMReads();
    void		incrementSMWrites();
    int         getSMWrites();
	int         getSMReads();
	int         getIOOperations();
	long		getTuplesWritten();
	void		addMeasurementType(int mt);
	void		addXVariable(int x);
	void		outputMeasurements();
	void		print(const char *x_string,double x_val);
	void        printPoint(const char *x_string, double x_val, double y_val );
	void		setOutputFilename(char *filename);
	void 		setTimeSpentScheduling(double tss) 	{ _time_spent_scheduling = tss; }
	double 		getTimeSpentScheduling() 	{ return _time_spent_scheduling; }
	void		setNumSchedulingDecisions(int nsd) 	{ _num_scheduling_decisions = nsd; }
	void		incrementNumBoxCalls(int incr);
	void		incrementTimeSpentInWorkerThread(double incr);
	void		incrementToMemoryLatency(double incr);
	void		incrementToMemoryTuplesWritten(int incr);
	double		getTimeSpentInWorkerThread() {return _time_spent_in_worker_threads;}
	double 		getToMemoryLatency();
	void		setAverageLatency(vector<int> app_v);
	void		setAverageQoS(vector<int> app_v);
	void		setStartTime();
	void		addMemRemainingVector(vector<MemRemaining> *mr);
	void		incrementTotalBoxTuplesConsumed(int incr);
	void		incrementQosPerTuple(int app,int num_tuples_passed,double avg_qos);
	void		incrementNumMallocs();
	int			getNumMallocs();
	void		incrementTimeLoadingQueuesWT(double incr);
	double		getTimeLoadingQueuesWT();
	void		incrementTimeUnloadingQueuesWT(double incr);
	double		getTimeUnloadingQueuesWT();
	bool		testRunnableBoxes();
	void		setStreamThread(StreamThread* st) {_stream_thread = st;}
	StreamThread*		getStreamThread() {return _stream_thread;}
	void		incrementBoxOverhead(double incr);
	void		incrementNumSchedulableBoxes(int incr);

	void		setTotalRunTime( double tot_time )		{ _total_running_time = tot_time; }
	void		setQoSBound( double qos_bnd )		{ _qos_bound = qos_bnd; }
	void		setInputRate(double input_rate)				{ _input_rate = input_rate; }
	void		setNumBuckets(int num_buckets)				{ _num_buckets = num_buckets; }
	void		setBEQSize(int beq_size)				{ _beq_size = beq_size; }
	void        setMemorySize( int mem )                { _memory_size = mem; }
	// this is read only (by SMInterface) after it is set. no mutex needed.
	int         getMemorySize( )                { return _memory_size; }
	void		setAppDepth(double depth)				{ _app_depth = depth; }
	void		setAppWidth(double width)				{ _app_width = width; }
	int		    setAppCount( int count )				{ _app_count = count; }
	int		    setBoxCount( int count )				{ _box_count = count; }
	void		setBoxCost(double cost)				{ _box_cost = cost; }
	void		setKSpannerVal(int k)				{ _k_spanner_val = k; }
	void		setNumApps(int n);
	void		setBurstSize(int s)					{ _burst_size = s; }
	void		setTrainSize( double s )			{  _train_size = s; cout << " TRAIN size " << s << endl; }
	double      getTrainSize() { return _train_size; }

	void getMemRemainingGraph(); // this used to be private, but I really
	// really need to call it from outside... so it moved here.
	void		incrementTimeSpentInStreamThread(double incr);
	void 		setTimeSpentInStreamThread(double ts) 	{ _time_spent_in_stream_thread = ts; }
	double 		getTimeSpentInStreamThread() 	{ return _time_spent_in_stream_thread; }
	void 		incrementTimeSpentInDoBox(double incr);
	void 		incrementTimeSpentExecutingBoxes(double incr);
	double		getTimeSpentExecutingBoxes();
	double 		getSecondsPerRand() {return _seconds_per_rand;}
	void 		setSecondsPerRand(double secs_per_rand) {_seconds_per_rand = secs_per_rand;}

private:
	AppTupleMonitor * _atmon;
	StreamThread*		_stream_thread;

	char *_out_filename;
	int _stop_type;		// condition on which experiment should stop (TUPLES_WRITTEN, TIME_STOP)
	stop_cond _sc;
	long _total_tuples_written;
	int _start_time;
	int _num_apps;


	pthread_mutex_t		*_mutex;
	
	set<int, less<int> >  _measurement_types_set;
	set<int, less<int> >  _x_variable_set;
	vector<vector<MemRemaining>*> _mem_remaining_v;
	vector<MemRemaining> _avg_mem_remaining_v;

	double _seconds_per_rand;

	// measures
	double 	_time_spent_scheduling;
	double	_time_spent_in_worker_threads;
	int		_num_scheduling_decisions;
	int		_num_box_calls;
	double	_average_latency;
	double	_average_qos;
	double	*_app_qos; // total qos .. must be divided by num tuples passed to find average per tuple
	int		*_app_qos_tuples_passed;
	double	_to_memory_latency;
	int		_to_memory_tuples_written;
	int		_out_step;
	int		_total_box_tuples_consumed;
	int     _sm_reads;
	int     _sm_writes;
	int     _disk_io;
	double  _total_running_time;
	double  _qos_bound;
	int		_num_mallocs;
	double	_time_loading_queues;
	double	_time_unloading_queues;
	double	_time_spent_in_stream_thread;
	double	_time_spent_in_doBox;
	double	_time_spent_executing_boxes;
	double  _box_overhead;
	int		_num_schedulable_boxes_total;
	int		_num_schedulable_boxes_counter;

	// x variables
	double _input_rate;
	double	_app_depth;
	double	_app_width;
	double	_box_cost;
	int     _app_count, _box_count;
	double	_box_selectivity;
	int     _memory_size;
	double  _k_spanner_val;
	int		_burst_size;
	double	_train_size;
	int	 	_num_buckets;
	int	 	_beq_size;
};

#endif
