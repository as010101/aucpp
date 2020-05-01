#ifndef _STREAMTHREAD_H_
#define _STREAMTHREAD_H_

#define MAXBUFF 100

#define WRITE_TO_FILE 1
#define READ_FROM_FILE 2
#define SEND_TO_OUT 4

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <list>
#include "SMInterface.H"
#include <set>
#include <StreamThreadThrottle.H>
#include <PtMutex.H>
#include <InputPortBoxInfo.H>

class NextArrival
{
public:
    NextArrival() {}
    int operator<(const NextArrival &na) const;
    int operator==(const NextArrival &na) const;

	double   _interarrival;
	double	 _next_t;
	int      _arc_id;
	int      _num_messages_remain;
	int      _seq_id;
};

class StreamThread 
{  
  
public:
  
	StreamThread(int arc_id, double interarrival, int num_messages, 
				 int train_size, 
				 pthread_mutex_t *tuple_count_mutex,
				 StreamThreadThrottle & throttle,
				 PtMutex & mtxInputPortInfoMap,
				 map<int, InputPortBoxInfo> & inputPortInfoMap );

	// not sure whether tuple_count_mutex is needed in single thread mode.
	StreamThread( int input_count,  
				  int max_arc_id,
				  pthread_mutex_t *tuple_count_mutex,
				  StreamThreadThrottle & throttle,
				  PtMutex & mtxInputPortInfoMap,	
				  map<int, InputPortBoxInfo> & inputPortInfoMap );

	virtual ~StreamThread();
   
  
	void start();

	// Blocks until the thread created by 'start()' terminates. Must only be
	// called after a successful call to 'start()'.
	void join();

	static void *entryPoint(void *pthis);
	void *run();  
	
	int getID() {return _id;} 
	// return stream ID
	
	// stream ID does not seem to be used by anyone!
	// so I am not supporting single thread load generator for it.
	void setID (int id) {_id = id;} 
	// set stream ID

	// add a input source
	void addInput( int arc_id, double interarrival, int num_messages,
				   int train_size );
	
	void setRate(double r);
	double getRate(double r);
	
	double getRateMultiplier();
	void setRateMultiplier(double rm);
	
	int getDestination() {return _destination;} 
	// return destination (starting box) ID
	
	void setDestination (int destination) {_destination = destination;} 
	// set destination (starting box) ID
	
	void incrementTuplesGenerated(int incr);
	int getTuplesGenerated();
	
	void setSchedulerWakeCondition(pthread_cond_t *cond) 
	{ _sched_wake_cond = cond; }
	void setSchedulerWakeMutex(pthread_mutex_t *mutex) 
	{ _sched_wake_mutex = mutex; }
	void setSchedulerWakeInterval(int num_tuples_generated)
	{ _sched_wake_interval = num_tuples_generated; }
	void setNumTuplesToGenerate(int n) {_num_tuples_to_generate = n;}
	
	// this is not rewritten to single thread load generator yet.
	void setStatus( int send_to_out, int write_to_file, int read_from_file );
	void 	init_etime(struct itimerval *first);
	double 	get_etime(struct itimerval *first);
	pthread_cond_t  *getWakeCondition() {return _wait_cond;}
	pthread_mutex_t *getWakeMutex() {return _wait_mutex;}
	pthread_cond_t 		*_wait_cond;
	pthread_mutex_t 	*_wait_mutex;
	
private:
	
	// this does not seem to be used, so I ignore it for the time being.
	int _id; // stream ID
	int *_destinations; // box ids array for each input generator.
	int _destination; // starting box ID
	int *_arc_ids; // arc ids array for input generator
	//int _arc_id;
	int *_train_sizes; // train sizes array for each input generator.
	int _train_size;
	double *_interarrivals; // inter-arrival rates for each input generator.
	double _interarrival;
	int *_num_messages_arr; // number of messages for each input generator
	int _num_messages;
	int *_accumulator; // accumulate the sleep times until some
	// resonable unit is reached. (sleeping for < 10ms has strange effects)
	static int _tuples_generated; // does this have to be static?
	static double _rate_multiplier;
	static int _input_counter; // internal counter, for input arrays.
	int _num_tuples_to_generate;

	int status; // write to disk/from disk/to output
	int _input_count; // number of inputs this thread is serving.

    multiset<NextArrival> allInputs; // this keeps track of multiple inputs

	// hopefully I will get away and not have to declare an array of these.
	char queueFilename[FILENAME_MAX]; // The file
	FILE *fd;
  
	pthread_t _thread;
	pthread_mutex_t *_mutex;

	StreamThreadThrottle & _throttle;
	// this was added to support input rate measurement for Stats object.
	PtMutex & _mtxInputPortInfoMap;	
	map<int, InputPortBoxInfo> & _inputPortInfoMap;
	map< int, int > _queueIdToInputPortMap;

	pthread_mutex_t *_tuple_count_mutex;
	pthread_mutex_t *_rate_multiplier_mutex;
	
	static pthread_cond_t    *_sched_wake_cond;
	static pthread_mutex_t   *_sched_wake_mutex;
	int _sched_wake_interval;
};

#endif
