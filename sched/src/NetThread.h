#ifndef _NETTHREAD_H_
#define _NETTHREAD_H_

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
#include <TraceLogger.H>
#include "SMInterface.H"

using namespace std;

class NetThread 
{  
  
public:
  
  NetThread(int arc_id, 
	    double interarrival, 
	    int num_messages, 
	    int train_size, 
	    pthread_mutex_t *lock, 
	    pthread_cond_t *cond, 
	    pthread_mutex_t *tuple_count_mutex,
	    TraceLogger * pLogger);

  virtual ~NetThread();
   
  
  void start();
  static void *entryPoint(void *pthis);
  void *run();  
  
  int getID() {return _id;} 
  // return stream ID
  
  void setID (int id) {_id = id;} 
  // set stream ID
  
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
  
  void setSchedulerWakeCondition(pthread_cond_t *cond) {_sched_wake_cond = cond; }
  void setSchedulerWakeMutex(pthread_mutex_t *mutex) {_sched_wake_mutex = mutex; }
  
  void setStatus( int send_to_out, int write_to_file, int read_from_file );
  
private:
  
	int _id; // stream ID
	int _destination; // starting box ID
	int _arc_id;
	int _train_size;
	double _interarrival;
	int _num_messages;
	static int _tuples_generated;
	static double _rate_multiplier;
	
	int status;
  char queueFilename[FILENAME_MAX]; // The file
  FILE *fd;
  
  pthread_t _thread;
  pthread_mutex_t *_mutex;
  pthread_mutex_t *_start_lock;
  pthread_cond_t *_start_cond;
  
  pthread_mutex_t *_tuple_count_mutex;
  pthread_mutex_t *_rate_multiplier_mutex;

  pthread_cond_t    *_sched_wake_cond;
  pthread_mutex_t   *_sched_wake_mutex;

  TraceLogger * _pLogger;

	int passiveTCP(char *service, int qlen);
	int passivesock(const char *service, const char *protocol, int qlen);
	void NetThread::tcp_read(int s, char *buf, int maxlinelen);
	void NetThread::tcp_read_tuple(int s, char *buf, int num_fields, TupleDescription *tuple_descr, int maxlinelen);

};




#endif
