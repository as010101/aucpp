#ifndef SHARED_H
#define SHARED_H



// Factor used to reduce the size requirement of _sum_timestamps_factored
// (to prevent overflow of the double we'll use for storage)
//#define _TS_SUM_FACTOR 3600
#define _TS_SUM_FACTOR 1.0 // plenty of room, nothing to worry!


#ifndef DEF_GLOBALS
#define DEF_GLOBALS
#include "global.H"
#undef DEF_GLOBALS
#endif

//#include "global.H"
#include "shm_util.H"
#include "sem_util.H"
#include "Catalog.H"
#include "QueueMon.H"

class QueueMon;
class Shared
{
public:
       
	int _queue_id;
	int _queue_priority;
	int _num_records_in_queue;
        double _sum_timestamps_factored;
	int _num_records_in_memory;
	int _num_records_desired;
	double _average_timestamp;
	int _state; // 0 == not scheduled; 1 == scheduled; 2 == in worker thread
};

int initialize_shared_mem(key_t k);
int initialize_semaphore(key_t k);
void cleanup(int shmid,int semid);

//int shmid;
//Shared *shm_ptr;
//int semid;;

int initialize_shared_mem(key_t k);
int initialize_semaphore(key_t k);
void cleanup(int shmid, int semid);
void shm_set_num_records(int queue_id, int in_queue, int in_memory);
void shm_set_num_records_in_queue(int queue_id, int num_records);
int shm_get_num_records_in_queue(int queue_id);
void shm_set_num_records_in_memory(int queue_id, int num_records);
int shm_get_num_records_in_memory(int queue_id);
void shm_set_average_timestamp(int queue_id, double average_timestamp);
double shm_get_average_timestamp(int queue_id);
void shm_set_sum_timestamp(int queue_id, double newsum);
double shm_get_sum_timestamp(int queue_id);
void shm_set_state(int queue_id, int state);
int shm_get_state(int queue_id);
void shm_print();


#endif
