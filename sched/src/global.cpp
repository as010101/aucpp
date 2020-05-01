#include "global.H"
#include "Measurement.H"
GlobalStop::GlobalStop() { 
	_stop = false; 
	_mutex = new pthread_mutex_t;
	pthread_mutex_init(_mutex,0);
	_sched_finished_mutex = new pthread_mutex_t;
	pthread_mutex_init(_sched_finished_mutex,0);
	_sched_finished_wait_cond = new pthread_cond_t;
	pthread_cond_init(_sched_finished_wait_cond,0);
}
void GlobalStop::setStop() {
	pthread_mutex_lock(_mutex);
	_stop = true;
	pthread_mutex_unlock(_mutex);
}
bool GlobalStop::getStop() {
	_measure->testStopCond();
	bool ret_val;
	pthread_mutex_lock(_mutex);
	ret_val = _stop;
	pthread_mutex_unlock(_mutex);
	return ret_val;
}
