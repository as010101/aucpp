#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include <stdio.h>
#include <pthread.h>
#include "BoxExecutionQueue.H"
#include "SMInterface.H"
#include <AppTupleMonitor.H>

//#include "SelectBox.H"  // ADDED FOR TESTING PURPOSES

class WorkerThread
{
public:
	WorkerThread(int threadNum,
				 pthread_cond_t * schedulerWakeCond,
				 pthread_mutex_t * schedulerWakeMutex,
				 AppTupleMonitor * pTupleMon);
	
	~WorkerThread() {}
	BoxExecutionQueue* getBoxExecutionQueue() {return _execution_queue;}

/*
  void setThreadNumber(int num) {_number = num;}
  void setSchedulerWakeCondition(pthread_cond_t *cond) {_sched_wake_cond = cond; }
  void setSchedulerWakeMutex(pthread_mutex_t *mutex) {_sched_wake_mutex = mutex; }

  // When this is called, pTupleMon should already have one entry for each of
  // set of the Application arc ids, and no other entries.
  void setAppTupleMonitor(AppTupleMonitor * pTupleMon) { _tupleMon = pTupleMon; }
*/

  void start(BoxExecutionQueue *beq);
  static void *entryPoint(void *pthis);
  int loadQueues(QueueElement *qe,SMInterface *SM, map<int, queue_info, less<int> > *queue_info_map);
  void unloadQueues(QueueElement *qe,SMInterface *SM, map<int, queue_info, less<int> > *queue_info_map);
  pthread_t getThread() {return _thread;}
  void *run();
	void 				init_etime(struct itimerval *first);
	double 				get_etime(struct itimerval *first);
private:
  // aMap gives the (arc id, # new tuples) pairing for some set of arcs that
  // go into Application boxes. This method will update _tupleMon appropriately,
  // and broadcast _tupleMon's condition.
  void announceNewAppTuples(const map<int, int> & localMap);

  int			 _number;
  pthread_t 		 _thread;
  BoxExecutionQueue 	*_execution_queue;
  pthread_cond_t	*_sched_wake_cond;
  pthread_mutex_t	*_sched_wake_mutex;
  int			_num_box_calls;
  AppTupleMonitor       *_tupleMon;
  struct itimerval 		_itimer_start;
  double _total_time_dequeueUnpinning;
  double _total_time_enqueueUnpinning;
  double _total_time_dequeueEnqueueUnpinning;
};

#endif
