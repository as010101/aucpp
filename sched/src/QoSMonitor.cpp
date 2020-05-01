#include "QoSMonitor.H"
//nclude "global.H"
#include "Catalog.H"
#include <QoS.H>
#include "Arc.H"
#include <math.h>
#include <errno.h>
#include <RuntimeGlobals.H>
#include <DelayedDataMgr.H>

#define SAMPLE_SIZE 500

int add_link( void *sample,int appl_id, int diff )//link_head_t *lh, int diff )
{
  int shift = sizeof( double ) * (SAMPLE_SIZE+1) * appl_id;
  //double *blah = (double*)malloc ( 2*sizeof( double ) );

  //*(blah) = 15.0;
  //*(blah+sizeof( double ) ) = 10.0;
   
  *((double*)sample + shift + sizeof( double ) * ((int)(*((double *)sample + shift)) %SAMPLE_SIZE ) + sizeof( double )) = 1.0 * diff;
  ((int)(*((double *)sample + shift))++);

  //  printf(" qos new INDEX %d diff %f, recorded %f dff %d\n", ((int)(*(((double *)sample + shift ))-1) % SAMPLE_SIZE ), 1.0*diff, *((double*)sample + shift + sizeof( double ) * ((int)(*((double *)sample + shift)-1)) % SAMPLE_SIZE + sizeof( double )), diff);

  return diff;
}

float compute_average( void *sample, int appl_id, /*link_head_t *lh, */float curr, double *util )
{
  int i, size = 0,start, index;
  //link_t *l;
  int shift = sizeof( double ) * (SAMPLE_SIZE+1) * appl_id;
  double avg = 0, delta = 0;

  //  printf(" I ammmm here qos new INDEX %d riddo %f dff \n", ((int)(*(((double *)sample + shift ))) -1  % SAMPLE_SIZE ) , *((double*)sample + shift + sizeof( double ) * (((int)(*((double *)sample + shift)))) % SAMPLE_SIZE));

  if ( ((int)(*(((double *)sample + shift ))) < SAMPLE_SIZE ) )
    {
    start = 0;
    size = ((int)(*(((double *)sample + shift )))) - 1;
    }
  else
    { 
    start = (int)(*(((double *)sample + shift )));
    size = SAMPLE_SIZE;
    }
  if ( size <= 0 ) return 0;

  //printf("qos start %d size %d (sift %d)\n", start, size, shift );
  for (i = start; i < start + size; i ++ )
    {
      index = i % SAMPLE_SIZE;
delta = *((double*)sample + shift + (sizeof( double ) * index) + sizeof( double ));
//printf("qos (i=%d)NOW %d, delt %f, tot %d with size %d\n", i, (index), delta, ((( size + 1)*size)/2), size);

	  if ( delta <= 3 ) //delay_qos->getGraph()[1]->x_point)
	    *util += 100; //delay_qos->getGraph()[1]->utility;
	  else
	    if ( delta > 3 && delta < 11 )
	      *util += (11.0-delta)*12.5;
	  else
	    *util += 0;


      avg += ( delta * 1.0 * (index+1)) / ((( size + 1)*size)/2 );
      //printf("QoSAVG %f, last delta %d\n", avg, l->delta);
      //l = l->next;
    }

  *util /= size;

  return avg;
}


QoSMonitor::QoSMonitor( int sample_frequency, QueryNetwork *q_net )
{ 		     
  _sample_frequency = sample_frequency;
  _q_net = q_net;
  int max_arc_id = q_net->getMaxArcId()+1;

  pthread_mutex_init( &_mutex, NULL );
  pthread_cond_init( &_cond, NULL );

  delay_sample = (double **)malloc( max_arc_id*sizeof( int* ) );
  delay_sample_index = (int *)malloc( max_arc_id*sizeof( int ) );
  utility_sample = (double **)malloc( max_arc_id*sizeof( int* ) );
  utility_sample_index = (int *)malloc( max_arc_id*sizeof( int ) );
  bzero( delay_sample_index, max_arc_id*sizeof( int ) );
  bzero( utility_sample_index, max_arc_id*sizeof( int ) );
  map_to_arc_id = (int*) malloc( q_net->getNumberOfArcs()*sizeof(int) );

  setMonitorFlags( 1, 1 );
}

QoSMonitor::~QoSMonitor()
{
}

void QoSMonitor::tuples_written( int tuple_num, double* t_stamps, int arc_id )
{
  // NOTE: this does not *really* have to be here. the mutex should
  // be locked later (where commented). Unfortunately the initial couple
  // of seconds before the sample_bitmap is initialized present a problem.
  // so need to find a way to wait for init here, when lock not needed
  // (mutex shared with the run() method where sample_bitmap is set.
  // FIXED? pthread_mutex_lock(&_mutex);

  if ( !_arc_sample_bitmap[ arc_id ] )
    {
      //printf(" SKPPED qos %d\n", arc_id );
      //pthread_mutex_unlock(&_mutex);
      return;
    }
  //printf(" GOT TO A\n");
  //printf(" HERE FOR %d\n", arc_id );
  //int curr_time = time( NULL );
  struct timeval t;
  gettimeofday( &t, NULL); 
  // curr time in microseconds
  double curr_time = t.tv_sec + ( 1.0 * t.tv_usec / MICRO );   
  //printf(" CURR COMPUTATION %d and %d results in %f second part ? %f fst part %f\n", t.tv_sec, t.tv_usec,
  //curr_time, ( 1.0 * t.tv_usec / MICRO ), ((double)t.tv_sec) );

  pthread_mutex_lock(&_mutex);

  //printf(" QOS qos got %d at arc %d fst st %d diff %d\n", tuple_num, arc_id, *t_stamps, time( NULL ) - *t_stamps );

  //printf("QoS: tup num %d, util? %d delay? %d\n", tuple_num, utilityOn, delayOn );
  for ( int i = 0; i < tuple_num && ( utilityOn || delayOn ); i ++ )
    {
      if ( delayOn )
	{
	  if ( delay_sample_index[ arc_id ] == SAMPLE_SIZE )
	    delay_sample_index[ arc_id ] = 0;
	  delay_sample[ arc_id ][ delay_sample_index[ arc_id ]++ ] = curr_time - (t_stamps[ i ]);
	}
      //printf("QoS: reported t_stamp %f, vs curr time %f, so recorded delay of %f\n", t_stamps[i], curr_time, delay_sample[ arc_id ][ delay_sample_index[ arc_id ]-1 ]);

      if ( utilityOn && _qos_graphs[ arc_id ] != NULL )
	{
	  if ( utility_sample_index[ arc_id ] == SAMPLE_SIZE )
	    utility_sample_index[ arc_id ] = 0; 
	  utility_sample[ arc_id ][ utility_sample_index[ arc_id ]++ ] =
	    _qos_graphs[ arc_id ]->getUtility
	    ( delay_sample[ arc_id ][ delay_sample_index[ arc_id ] -1] ); 
	  //cout << " Delay Sample " << delay_sample[ arc_id ][ delay_sample_index[ arc_id ] -1] << "  AVG Utility " << _qos_graphs[ arc_id ]->getUtility( delay_sample[ arc_id ][ delay_sample_index[ arc_id ] -1] ) << endl;
	  //get_average_utility( arc_id );
	}

      //printf(" QOS **witheld :) ** at() and delay %f and curr %f \n",  delay_sample[ arc_id ][ delay_sample_index[ arc_id ] -1], curr_time);
      } 

  // Maintain the global stats...
  /*DelayedDataMgr * pDdm = RuntimeGlobals::getDelayedDataMgr();
  StatsImage & si = pDdm->getWritableImage();
  AppArcStats & aaStats = si._appArcsStats[ arc_id ];
  aaStats._avgLatency = int_get_average_delay(arc_id);
  aaStats._avgLatencyUtility = int_get_average_utility(arc_id);
  pDdm->releaseWritableImage();*/

  pthread_mutex_unlock(&_mutex);
  //get_average_delay( arc_id );
}

double QoSMonitor::int_get_average_utility( int arc_id )
{
  double total_util = 0;
  int i;

  for ( i = 0; i < SAMPLE_SIZE && utility_sample[ arc_id ][ i ] != -1; i++ )
    {
      total_util += utility_sample[ arc_id ][ i ];
      //printf(" QOS added %f\n", utility_sample[ arc_id ][ i ]);
    }

  //printf("DEBUG: QoS total util arc %d got %f\n", arc_id, 1.0 * total_util/i );
  if ( i == 0 )
	  return 0.0;
  else
	  return ( 1.0 * total_util / i );
}

double QoSMonitor::get_average_utility( int arc_id )
{
  pthread_mutex_lock(&_mutex);
  double returnVal = int_get_average_utility( arc_id );
  pthread_mutex_unlock(&_mutex);
  return returnVal;
}

double QoSMonitor::int_get_average_delay( int arc_id )
{
  double total_delay = 0;
  int i;

  for ( i = 0; i < SAMPLE_SIZE && delay_sample[ arc_id ][ i ] != -1; i++ )
    {
      total_delay += delay_sample[ arc_id ][ i ];
      //printf(" QOS added %f\n", delay_sample[ arc_id ][ i ]);
    }

  //printf("DEBUG: QoS arc %d, avg delay %f\n", arc_id, ( 1.0 * total_delay/i ));
  return ( 1.0 * total_delay/i );
}



double QoSMonitor::get_average_delay( int arc_id )
{
  pthread_mutex_lock(&_mutex);
  double returnVal = int_get_average_delay(arc_id);
  pthread_mutex_unlock(&_mutex);
  return returnVal;
}

void QoSMonitor::setMonitorFlags( int delay, int util )
{
  pthread_mutex_lock(&_mutex);

  delayOn = delay;
  utilityOn = util;

  pthread_mutex_unlock(&_mutex);

}

void QoSMonitor::run( )
{ 
  pthread_mutex_lock(&_mutex);
  //printf("lockED QoS mutex \n");

  //timespec_t wait_time;
  link_head_t *lh;
  ApplicationMap m_applications;
  //QoSGraphs *qos;// = _catalog->getQoSGraph( 0 );
  //int total, n;
  int fd, rd, counter = 0, self, arc_id, 
    appl_num = _q_net->getNumberOfApplications(),
    max_arc_id = _q_net->getMaxArcId() + 1; // compensate for 0<-->id range.

  _arc_sample_bitmap = ( char* ) malloc( max_arc_id );
  bzero( _arc_sample_bitmap, max_arc_id );
  //_catalog->getNumApplications() ;
  //QoS *qosGraphs[ appl_num ];
  Application *appls[ appl_num ]; //, *application;
  Arc *arc;   //hack. need SM support...
  char buff[10];
  char filename[FILENAME_MAX];
  //const char *QoSOutput = "QoSOutput";
  const char *format = "Output at Arc#%d has average delay of %f with utility %f\n";
  char * temp;
  time_t t;
  double avg, util, last_delay = 0;
  
  _qos_graphs = ( QoS ** ) malloc( sizeof( QoS *) * max_arc_id );

  temp = reinterpret_cast<char * > (malloc( strlen( format ) + 10 ));
  lh = ( link_head_t* ) malloc( sizeof( struct link_head ));
  lh->first = lh->last = NULL;

  time( &_start_time );
  //printf("really START qos time %ld with %d appls\n", _start_time, appl_num );
  //wait_time.tv_nsec = 0;

  // _q_net->printQueryNetwork();

  self = open("QoSOutput", O_CREAT | O_WRONLY | O_TRUNC, 0644 );

  if (self < 0)
    {
      printf("can qos not open output file err=%d\n", errno);
      exit( 1 );
    }
  close( self );
  counter = 0;
  m_applications = _q_net->getApplications();

  for ( int i = 0; i <= _q_net->getMaxArcId(); i++ )
    {
      delay_sample[ i ] = ( double * ) malloc( SAMPLE_SIZE * sizeof( double ) );
      
      for ( int j = 0; j < SAMPLE_SIZE; j ++ )
	delay_sample[ i ][j] = -1; 
    }

  bzero( _qos_graphs, sizeof( QoS *) * max_arc_id );

  for (ApplicationMapIter piter = m_applications.begin(); piter != m_applications.end(); piter++)
    {
      appls[ counter++ ] = (*piter).second;
      //printf(" QOS SET HERE %d\n", appls[ counter - 1 ]->getIncomingArcId() );
      _qos_graphs[ appls[ counter - 1 ]->getIncomingArcId() ] = 
	appls[ counter - 1 ]->getQoS();
      _arc_sample_bitmap [ appls[ counter - 1 ]->getIncomingArcId() ] = 1;

      utility_sample[ appls[ counter - 1 ]->getIncomingArcId() ] = 
	( double * ) malloc( SAMPLE_SIZE * sizeof( double ) );
      for ( int j = 0; j < SAMPLE_SIZE; j ++ )
      utility_sample[ appls[ counter - 1 ]->getIncomingArcId() ][j] = -1; 
    }
  
  sample = ( double * )malloc( (1+SAMPLE_SIZE) * sizeof( double ) * appl_num +500);
  bzero( sample, (1+SAMPLE_SIZE) * sizeof( double ) * appl_num );

  pthread_mutex_unlock(&_mutex);
  //printf("QoS mutex released\n");
  return;

  while ( 0 )
    {
      self = open("QoSOutput", O_APPEND | O_WRONLY);
      if (self < 0)
	{
		printf("can not open output file err=%d\n", errno);
	  exit( 1 );
	}
      for (int i = 0; i < appl_num; i++)
	{
		//printf("Working out of %d at i=%d\n", appl_num, i );
	  //arc_id = _q_net->getApplication( i )->getIncomingArcId();
	  //printf("QOS from %d at arc %d\n", i, arc_id );
	  // delay_qos = _q_net->getApplication( i )->getQos2();

	  arc_id = _q_net->getApplication( i )->getIncomingArcId();//3;
	  //printf(" ABRA %d\n", _catalog->getApplication( i )->getIncomingArcId() );
	  arc = _q_net->getArc( arc_id );
	  //_catalog->getApplication( i )->getIncomingArcId() );
	  arc->lockArc();
	  //printf("ARC locked %d\n", arc->getId() );
	  sprintf(filename,"queue.%03d", arc_id );
	  //sprintf(filename,"queue.%03d", 1 );
	  //  lockQueue
	  (fd=open(filename, O_RDONLY));
	  
	  if (fd<0)
	    {
			//printf("QoS:can't open %s\n", filename);
	    break;//continue;
	    }
	  //else
	  //  printf("QoS:opened %s\n", filename);
	  
	  lseek(fd,0L,0);
	  
	  do 
	    {
	      //do
	      rd = read(fd,buff,10);
	      //} while (buff[counter]!=' '&& rd > 0);
	      //if ((n=sscanf(buff, "%d", &t))!=1) printf("sscanf error %d %s\n",n, buff);
	      time( &t );
	      //printf("qos %d t, and got adding%d sam buf sz\n", t, atoi(buff));
	      //avg = (time_t)abs((avg * .5 + ( atoi( buff ) - _start_time - _start_time )) * .5 ));
	      time( &_start_time );

	      if ( rd != 0 )
			  /*printf("rec DELTA %d\n",*/ add_link( sample, /*lh*/i , _start_time - atoi( buff )); //);
	      //else
	      //	printf("QOS not adding deltn=-=\n");
	      //avg = (avg *.9 + atoi( buff ) * .1 );
	      counter = 0;
	      do
		{
		  rd = read(fd,buff,1);
		} while (buff[0]!='\n' && rd > 0);
	    } while ( rd != 0);
	
	  util = 0;

	  avg = compute_average( sample, i, avg, &util);
	  //printf("COMPUTED qos tm %f\n", avg);
	  //for (int j = 0; j < 3; j++)
	  //  printf(" %f\n", delay_qos->getGraph()[j]->x_point);

	  //printf( "qos util %f\n", util );
	  
	  if ( avg < 0 )
	    util = 0;
	  //rintf(temp, format, arc_id, avg, util );
	  sprintf(temp, format, arc_id, avg, util );

	  /*if ( floor( 100*last_delay) == (floor( 100 * avg)))
	    {
	      if ( t % 7 == 0 )
		write( self, temp, strlen( temp ) );
	    }
	    else */
		write( self, temp, strlen( temp ) );
		if ( i == appl_num -1 )
		  write( self, "\n", strlen("\n"));
	  last_delay = avg;
	  //printf("abra%s, %d\n", temp, strlen(temp));
	  close( fd );
	  //ifexit( 1 );
	  fd = open(filename, O_WRONLY | O_TRUNC );
	   close( fd );
	  //exit( 1 );
	  
	   /*self = open(filename, O_APPEND | O_WRONLY);
	     close( self ); */

	  arc->unlockArc();  
	}

      //wait_time.tv_sec = (time_t)(time( NULL )) + _sample_frequency;
      //pthread_mutex_lock( &_mutex );
      //pthread_cond_timedwait( &_cond, &_mutex, &wait_time ); 
      //pthread_mutex_unlock( &_mutex );
      close( self );
      sleep( 4 );
      //printf("QoSMonitor Waking up\n");
    }
}

void *QoSMonitor::entryPoint(void *pthis)
{
  QoSMonitor *pt = (QoSMonitor*)pthis;
  pt->run();
  return (void *)0;
}

//signal the monitor to wake up now and start recomputing the priorities.
void QoSMonitor::wakeUp()
{
  pthread_cond_signal( &_cond );
}

void QoSMonitor::start()
{
  pthread_create(&_thread, 0, entryPoint, this);
}


