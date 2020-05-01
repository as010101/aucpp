#ifndef SM_INTERFACE_H
#define SM_INTERFACE_H

#define MAXBUFF 100
#define MICRO   1000000.0

#include <time.h>
#include <map>
#include <queue>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/errno.h>

#include <sys/file.h>


#include <ctype.h>
#include "TupleDescription.H"
#include "global.H"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <map>
#include <PagePool.H>
#include <TupleQueue.H>

using namespace std;

extern int errno;

struct queue_info 
{
	char*	front;
	char*	rear;
	char*	malloc_ptr;
	int		num_tuples_resident; // number of tuples actually in queue
	int		num_tuples_alloc;	// space allocated for tuples
};

//using std::string;

class SMInterface 
{
  
public:
  SMInterface();
  static PagePool _pool;
  static map<int,TupleQueue*> _queues; // Note, in some sense, it's really "ARCS" in the current implementation
  static bool _initialized;
  static pthread_mutex_t *_queues_mutex;

  virtual ~SMInterface();  

	//void SMInterface::readTopTuples(char* ptr, int sizePerTuple, int num_tuples, FILE* fd);
  void SMInterface::readTopTuples2(char* ptr, int sizePerTuple, int num_tuples, int queue_id );
	//void SMInterface::readTuples(char* ptr, int sizePerTuple, int num_tuples, FILE* fd);
  void SMInterface::readTuples2(char* ptr, int sizePerTuple, int num_tuples, int queue_id );
	//void SMInterface::readTuple(char* ptr, int len, FILE* fd);
	void SMInterface::readTuple2(char* ptr, int len, int queue_id, int which );
	void SMInterface::writeTuples(char* ptr, int sizePerTuple, int num_tuples, int _currqueue );
		//void SMInterface::writeTuples(char* ptr, int sizePerTuple, int num_tuples, 
		//FILE* fd, int arc_id, int seekFD);
	//void SMInterface::writeTuple(char* ptr, int len, FILE* fd);
  void SMInterface::writeTuple2(char* ptr, int len, int queue_id );

	// this returns 1 on success, otherwise just write the queue to disk
	// as before. 
	// In fact this is a discontinued function!
	//int SMInterface::dumpQueuesToDisk( int _currqueue, int need_free );

	//void SMInterface::checkMemory();

  void* SM_malloc(int bytes);
  void SM_free(void* p);

	//FILE* openQueueFileAndLock(char* fname, char* mode);
	//void closeQueueFileAndUnlock(FILE* fd);

  char* enqueuePin(int arc_id, int num_tuples);
  char* dequeuePin(int arc_id, int num_desired);
  queue_info dequeueEnqueuePin(int arc_id, int num_tuples);
  void dequeueEnqueueUnpin(int arc_id, queue_info qi);
  void dequeueUnpin(int arc_id, int num_desired);
  void enqueueUnpin(int arc_id, char* ptr, int num_tuples);
  
  void lockQueue(FILE* fd);
  void unlockQueue(FILE* fd);  

  void SMInterface::MITRE_sendData(void* p, int sockfd, int size);
  void setITimerStart(struct itimerval *first) {_itimer_start = first;}

  void printTimings(){printf("SMInterface:_timed_chunk_1: %f\n",_timed_chunk_1);
  						printf("SMInterface:_timed_chunk_2: %f\n",_timed_chunk_2);
  						printf("SMInterface:_timed_chunk_3: %f\n",_timed_chunk_3);
  						printf("SMInterface:_timed_chunk_4: %f\n",_timed_chunk_4);}
	void init_etime(struct itimerval *first);
	double get_etime(struct itimerval *first);

  /** ALL THE BELOW ARE UNUSED:
  int getBoxID() {return _box_id;}  
  int getBoxPriority() {return _box_priority;}  
  int getNumRecordsDesired() {return _num_records_desired;}  
  int getNumRecordsInQueue() {return _num_records_in_queue;}  
  int getNumRecordsInMemory() {return _num_records_in_memory;}  
  
  void setBoxID(int box_id) 
    {
      _box_id = box_id;
    }  
  
  void setBoxPriority(int box_priority) 
    {
      _box_priority = box_priority;
    }  
  
  void setNumRecordsDesired(int num_records_desired) 
    {
      _num_records_desired = num_records_desired;
    }  
  
  void setNumRecordsInQueue(int num_records_in_queue) 
    {
      _num_records_in_queue = num_records_in_queue;
    }  
  
  void setNumRecordsInMemory(int num_records_in_memory) 
    {
      _num_records_in_memory = num_records_in_memory;
    }
  */
  
private:
  
  // All these guys are actually unused now:
  //int _box_id;
  //int _box_priority;
  //int _num_records_in_queue;
  //int _num_records_in_memory;
  //int _num_records_desired;
  //int _currqueue; // Current queue with activity
  //pthread_mutex_t **_queue_mutex; 
  
  int **_timestamp_sums; // Used to compute a running average of timestamps in the queue
                         // along with shared memory statistics (num records in queue)

  struct itimerval *_itimer_start;
  double _timed_chunk_1;
  double _timed_chunk_2;
  double _timed_chunk_3;
  double _timed_chunk_4;

};

#endif
