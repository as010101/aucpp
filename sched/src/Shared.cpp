static char *cvs_id="@(#) $Id: Shared.C,v 1.11 2003/07/04 02:40:04 alexr Exp $";
#include "Shared.H"

Shared *shm_ptr;
QueueMon* _queue_monitor = NULL;

int initialize_shared_mem(key_t k)
{

	//printf("GOT TO shm size: %i\n",sizeof(Shared)*(_catalog->getMaxArcId()+1));

	// cout << "######## initialize_shared_mem(" << k << "):  " << endl
	//      << "########     _catalog->getMaxArcId()+1 = " << _catalog->getMaxArcId()+1 << endl
	//      << "########     sizeof(Shared) = " << sizeof(Shared) << endl
	//      << "########     sizeof(Shared)*(_catalog->getMaxArcId()+1)) = " << sizeof(Shared)*(_catalog->getMaxArcId()+1) << endl
	//      << endl;

	if ( (shmid = shm_create(k,sizeof(Shared)*(_catalog->getMaxArcId()+1))) < 0)
	{
	  perror("error occured ");
	  printf("GOT TO errno: %i\n",errno);
	  fprintf(stderr,"can't get shared memory %s\n",check_ip(errno));
	}


	shm_ptr = (Shared *) shm_attach(shmid);

	for ( int i = 0; i < _catalog->getMaxArcId()+1; i++ )
	{
		shm_ptr[i]._queue_id = i;
		shm_ptr[i]._queue_priority = 0;
		shm_ptr[i]._num_records_in_queue = 0;
		shm_ptr[i]._num_records_in_memory = 0;
		shm_ptr[i]._num_records_desired = 0;
		shm_ptr[i]._sum_timestamps_factored = 0;
		shm_ptr[i]._state = 0;
	}
	return shmid;
}
int initialize_semaphore(key_t k)
{

	if ( (semid = sem_create(k)) < 0)
	{
		printf("GOT TO errno: %i\n",errno);
		fprintf(stderr,"can't create semaphore: %s\n",check_ip(errno));
	}
	return semid;
}
void cleanup(int shmid, int semid)
{
	if ( (shm_rm(shmid)) < 0)
	{
		printf("GOT TO errno: %i\n",errno);
		fprintf(stderr,"can't remove shared mem %s\n",check_ip(errno));
	}
	sem_rm(semid);
}

void shm_set_num_records(int queue_id, int in_queue, int in_memory)
{
	sem_lock(semid);
	shm_ptr[queue_id]._num_records_in_queue = in_queue;
	shm_ptr[queue_id]._num_records_in_memory = in_memory;
	sem_unlock(semid);
}
void shm_set_num_records_in_queue(int queue_id, int num_records)
{
	sem_lock(semid);
	shm_ptr[queue_id]._num_records_in_queue = num_records;
	sem_unlock(semid);
}
int shm_get_num_records_in_queue(int queue_id)
{
	int ret_val;
	sem_lock(semid);
	ret_val = shm_ptr[queue_id]._num_records_in_queue;
	sem_unlock(semid);
	return ret_val;
}
void shm_set_num_records_in_memory(int queue_id, int num_records)
{
	sem_lock(semid);
	shm_ptr[queue_id]._num_records_in_memory = num_records;
	sem_unlock(semid);
}
int shm_get_num_records_in_memory(int queue_id)
{
	int ret_val;
	sem_lock(semid);
	ret_val = shm_ptr[queue_id]._num_records_in_memory;
	sem_unlock(semid);
	return ret_val;
}
void shm_set_average_timestamp(int queue_id, double average_timestamp)
{
  printf("[Shared.C]: shm_set_average_timestamp: Warning! Function is deprecated\n");
  return;
  // Dont set anything - we compute when needed from timestamp sum and num of tuples
  //sem_lock(semid);
  //shm_ptr[queue_id]._average_timestamp = average_timestamp;
  //sem_unlock(semid);
}
double shm_get_average_timestamp(int queue_id)
{
	double ret_val;
	sem_lock(semid);
	//ret_val = shm_ptr[queue_id]._average_timestamp;
	if (shm_ptr[queue_id]._num_records_in_queue == 0)
	  ret_val = 0;
	else
	  ret_val = shm_ptr[queue_id]._sum_timestamps_factored / shm_ptr[queue_id]._num_records_in_queue * _TS_SUM_FACTOR;
	sem_unlock(semid);
	//printf("[Shared]: shm_get_average_timestamp(%d): returning %f\n", queue_id, ret_val);
	return ret_val;
}
void shm_set_sum_timestamp(int queue_id, double newsum)
{
  sem_lock(semid);
  shm_ptr[queue_id]._sum_timestamps_factored = newsum;

	{
	double time_now, avg_tm;
	timeval t_now;
	gettimeofday(&t_now,NULL);
	time_now = t_now.tv_sec + (t_now.tv_usec*1e-06);
	
	avg_tm = shm_ptr[queue_id]._sum_timestamps_factored / shm_ptr[queue_id]._num_records_in_queue * _TS_SUM_FACTOR;

	if( avg_tm > time_now )
		cout << " QUEUE ID " << queue_id << " WARNING, tm now " << time_now << " avg time " << avg_tm << " DIfference " << ( time_now - avg_tm ) << " total tuples " << shm_ptr[queue_id]._num_records_in_queue << endl; 

	}



  sem_unlock(semid);
}
double shm_get_sum_timestamp(int queue_id)
{
  double ret_val;
  sem_lock(semid);
  ret_val = shm_ptr[queue_id]._sum_timestamps_factored;
  sem_unlock(semid);
  return ret_val;
}
void shm_set_state(int queue_id, int state)
{
  sem_lock(semid);
  shm_ptr[queue_id]._state = state;
  sem_unlock(semid);
}
int shm_get_state(int queue_id)
{
  int ret_val;
  sem_lock(semid);
  ret_val = shm_ptr[queue_id]._state;
  sem_unlock(semid);
  return ret_val;
}
void shm_print(const char *request_source)
{
printf("SHARED MEM (%s)\n",request_source);
printf("==========\n");

	for ( int i = 0; i < _catalog->getMaxArcId()+1; i++ )
	{
		printf("%i %i %i %i %i\n",
			shm_ptr[i]._queue_id,
			shm_ptr[i]._queue_priority,
			shm_ptr[i]._num_records_in_queue,
			shm_ptr[i]._num_records_in_memory,
			shm_ptr[i]._num_records_desired);
	}
}

