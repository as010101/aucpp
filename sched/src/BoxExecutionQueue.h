#ifndef BOX_EXECUTION_QUEUE_H
#define BOX_EXECUTION_QUEUE_H


#include <pthread.h>
//#include <stack.h>
//#include <list.h>
#include <queue>
#include <list>
#include <QueueElement.H>


using namespace std;
class BoxExecutionQueue
{
public:
	BoxExecutionQueue();
	~BoxExecutionQueue() {}

	int size()	{	return _executionQueue->size(); }

	int push(QueueElement *q_element);
	QueueElement* pop(pthread_cond_t * schedulerWakeCond,
					  pthread_mutex_t * schedulerWakeMutex );
	void signalIfEmpty(pthread_cond_t * schedulerWakeCond,
					   pthread_mutex_t * schedulerWakeMutex );

	void lock() 	{ pthread_mutex_lock(_mutex); }
	void unlock() 	{ pthread_mutex_unlock(_mutex); }
	void wait()		{ pthread_cond_wait(_cond,_mutex); }
	void signal()	{ pthread_cond_broadcast(_cond); }

private:
	queue<QueueElement *>	*_executionQueue;
  //	int						_size;
	pthread_mutex_t			*_mutex; 	// lock_variable .. only one thread at a time 
										// will be allowed to access the queue
	pthread_cond_t			*_cond;		// condition variable
	
};


#endif
