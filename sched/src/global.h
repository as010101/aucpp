#ifndef GLOBAL_H
#define GLOBAL_H

#include "Catalog.H"
#include "QoSMonitor.H"
#include "Shared.H"
//#include "Measurement.H"
class Measurement;
#include <set>
#include <queue>
#define TUPLE_TIMESTAMP_OFFSET 0
#define TUPLE_TIMESTAMP_SIZE 8 // two longs, (its the timeval struct)
#define TUPLE_STREAMID_OFFSET 8
#define TUPLE_STREAMID_SIZE 4
#define TUPLE_DATA_OFFSET 12 // Where the actual tuple data begins
// Eventually replace this with SIZEOF_INT set by the compiler defines


#ifndef DEF_GLOBALS
#define EX extern
#else
#define EX
#endif


//typedef queue<char*> Queue;
typedef vector<char*> Vector;


EX Catalog *_catalog;
EX int shmid;
EX int semid;
EX QoSMonitor *_qos;
//EX Scheduler *_sched;
EX Measurement *_measure;
EX int *queueAccessTm;
EX int *tuplesOnDisk;
EX int memory_in_use;
EX pthread_mutex_t memory_mutex;
//class qos_struct;
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

EX qos_struct          *__qos_graphs;
EX set<int>             __box_work_set;
EX pthread_mutex_t      __box_work_set_mutex;

class GlobalStop
{
	public:
		GlobalStop();
		~GlobalStop() {}
		void setStop();
		bool getStop();
		pthread_mutex_t*	getSchedFinishedMutex() {return _sched_finished_mutex;}
		pthread_cond_t*		getSchedFinishedWaitCond() {return _sched_finished_wait_cond;}

	private:
		pthread_mutex_t     *_mutex;
		bool				_stop;
		pthread_mutex_t		*_sched_finished_mutex;
		pthread_cond_t		*_sched_finished_wait_cond;

};

EX GlobalStop __global_stop;



void shm_set_num_records(int queue_id, int in_queue, int in_memory);
void shm_set_num_records_in_queue(int queue_id, int num_records);
int shm_get_num_records_in_queue(int queue_id);
void shm_set_num_records_in_memory(int queue_id, int num_records);
int shm_get_num_records_in_memory(int queue_id);
void shm_print(const char *request_source);

// endif GLOBAL_H
#endif
