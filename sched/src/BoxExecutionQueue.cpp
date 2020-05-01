//static char *cvs_id="@(#) $Id: BoxExecutionQueue.C,v 1.4 2003/07/04 02:40:01 alexr Exp $";
#include "BoxExecutionQueue.H"


BoxExecutionQueue::BoxExecutionQueue()  
{
	_mutex = new pthread_mutex_t;
	_cond = new pthread_cond_t;
	pthread_mutex_init(_mutex,NULL);
	pthread_cond_init(_cond,NULL);
	_executionQueue = new queue<QueueElement *>;
}
int BoxExecutionQueue::push(QueueElement *q_element)
{
	// this method is used by the scheduler
	// to place a box on the queue

	/*timeval now;
	gettimeofday(&now,NULL);
	printf("      PUSH NOW:  TIME: %d %d\n", now.tv_sec, now.tv_usec);*/


  //  fprintf(stdout, "cme: Calling PUSH \n");

	int status = 0;

	//pthread_mutex_lock(_mutex);


	_executionQueue->push(q_element);
	//  fprintf(stdout, "_executionQueue->size() = %i \n", _executionQueue->size() );
  
	
	//pthread_mutex_unlock(_mutex);

	return status;
}

void BoxExecutionQueue::signalIfEmpty(pthread_cond_t * schedulerWakeCond,
									 pthread_mutex_t * schedulerWakeMutex )
{
	if ( _executionQueue->size() == 0 )
	{
		pthread_mutex_lock(schedulerWakeMutex);
		//cout << "                     Wakeup Up scheduler " << endl;
		pthread_cond_broadcast(schedulerWakeCond);
		pthread_mutex_unlock(schedulerWakeMutex);
	}
}


QueueElement* BoxExecutionQueue::pop(pthread_cond_t * schedulerWakeCond,
									 pthread_mutex_t * schedulerWakeMutex )
{
	// this method is used by the scheduler
	// to place a box on the queue

	
	QueueElement *q_element = NULL;

	//pthread_mutex_lock(_mutex);

	if ( _executionQueue->size() > 0 )
	{
		q_element = _executionQueue->front();
		_executionQueue->pop();
	}


	if ( _executionQueue->size() == 0 )
	{
		pthread_mutex_lock(schedulerWakeMutex);
		//cout << "                     Wakeup Up scheduler " << endl;
		pthread_cond_broadcast(schedulerWakeCond);
		pthread_mutex_unlock(schedulerWakeMutex);
	}
	//pthread_mutex_unlock(_mutex);
	

	return q_element;
}



