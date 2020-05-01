#ifndef _QOSMONITOR_H_
#define _QOSMONITOR_H_

// this is defined twice, but not sure of a better way
// to define a constant...
#define MICRO 1000000.0

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
//#include "global.H"
//#include <list.h>
#include <QueryNetwork.H>

typedef struct link_head
{
  struct link *first;
  struct link *last;
} link_head_t;

typedef struct link 
{
  int delta;
  struct link *next;
} link_t;

class QoSMonitor 
{  
  
public:
  
  QoSMonitor( int sample_frequency, QueryNetwork *q_net ); 
  ~QoSMonitor();
  
  void start( );
  void run( );  
  
  int getSampleFrequency() { return _sample_frequency; } 
  // return the frequency at which the outputs are sampled
  
  void QoSMonitor::wakeUp();
  // wake up and recompute the priorities

  int setSampleFrequency() { return _sample_frequency; } 
  // set the frequency at which the outputs are sampled
  
  static void *entryPoint( void *pthis );
  void tuples_written( int tuple_num, double *t_stamps, int arc_id );
  double get_average_delay( int arc_id );
  double get_average_utility( int arc_id );

  void QoSMonitor::setMonitorFlags( int delay, int util );

private:
	// Like their namesakes, but these assume the caller already holds the
	// needed mutex.
	double int_get_average_delay( int arc_id );
	double int_get_average_utility( int arc_id );
 
  int _sample_frequency;
  QueryNetwork *_q_net;
  time_t _start_time;
  int utilityOn, delayOn;
  
  pthread_t _thread;
  pthread_mutex_t _mutex;
  pthread_mutex_t *_arc_mutexes;
  //  pthread_mutex_t 
  pthread_cond_t _cond;

  char *_arc_sample_bitmap;
  double *sample;
  double **delay_sample;
  int *delay_sample_index;
  double **utility_sample;
  int *utility_sample_index;
  int *map_to_arc_id;
  QoS **_qos_graphs;
};

#endif
