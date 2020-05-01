//static char *cvs_id="@(#) $Id: NetThread.C,v 1.5 2003/03/26 19:06:25 cjc Exp $";


#include "NetThread.H"

#include "NetThread.H"
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAXMESG 3000



u_short portbase = 0;


extern QueueMon* _queue_monitor;

int NetThread::_tuples_generated = 0;
double NetThread::_rate_multiplier = 1.0;


NetThread::NetThread(int arc_id, 
		     double interarrival, 
		     int num_messages, 
		     int train_size, 
		     pthread_mutex_t *lock, 
		     pthread_cond_t *cond, 
		     pthread_mutex_t *tuple_count_mutex,
		     TraceLogger * pLogger)
{ 		     
  assert(pLogger != NULL);
  _pLogger = pLogger;
  _arc_id = arc_id;
  _interarrival = interarrival;
  _num_messages = num_messages;
  _train_size = train_size;
  printf("WTF2 %i\n", _train_size );
  _start_lock = lock;
  _start_cond = cond;
  _tuple_count_mutex = tuple_count_mutex;
  setStatus( 1, 0, 0 );
};

// This determines the current mode of work.
// send_to_out -- send the data to the input ports
// write_to_file -- write the data to file
// read_from_file -- read data from a data file.
void NetThread::setStatus( int send_to_out, int write_to_file, 
			      int read_from_file )
{
  status = 0;
  if ( send_to_out ) status = status | SEND_TO_OUT;
  if ( write_to_file ) status = status | WRITE_TO_FILE;
  if ( read_from_file ) status = status | READ_FROM_FILE;
}

NetThread::~NetThread()
{
};

void NetThread::setRate(double r) {
  _interarrival = r;
}
double NetThread::getRate(double r) {
  return _interarrival;
}
double NetThread::getRateMultiplier()
{
	double rm;
	pthread_mutex_lock(_tuple_count_mutex);
	rm =  _rate_multiplier;
	pthread_mutex_unlock(_tuple_count_mutex);
	return rm;
}
void NetThread::setRateMultiplier(double rm)
{
	pthread_mutex_lock(_tuple_count_mutex);
	_rate_multiplier = rm;
	pthread_mutex_unlock(_tuple_count_mutex);
}

int NetThread::getTuplesGenerated()
{
	int tg;
	pthread_mutex_lock(_tuple_count_mutex);
	tg =  _tuples_generated;
	pthread_mutex_unlock(_tuple_count_mutex);
	return tg;
}
void NetThread::incrementTuplesGenerated(int incr)
{
	pthread_mutex_lock(_tuple_count_mutex);
	_tuples_generated += incr;
	pthread_mutex_unlock(_tuple_count_mutex);
}

void *NetThread::run()
{
  //int n;
	int temp;
	//int _queue;
	float r;
	timeval *t = new timeval;
	time_t start_t;
	//char buff[MAXBUFF + 1];
	TupleDescription *tuple_descr;
	int num_fields;
	int field_offset;
	//int c=0;

	unsigned int seed = _arc_id;

	if ( (status & ( WRITE_TO_FILE | READ_FROM_FILE )) > 0 )
	  sprintf( queueFilename, "input.%05d", _arc_id );

	if ( (status & WRITE_TO_FILE) > 0 )
	    fd = fopen( queueFilename, "w+b" );

	if ( (status & READ_FROM_FILE) > 0 )
	    fd = fopen( queueFilename, "rb" );

	if ( fd == NULL && ( (status&( WRITE_TO_FILE | READ_FROM_FILE)) > 0 ) )
	  {
	    printf("NetThread: Arc %d input I/O Failure\n", _arc_id );
	    exit( 1 );
	  }

	pthread_mutex_lock(_start_lock);
	pthread_cond_wait(_start_cond, _start_lock);
	pthread_mutex_unlock(_start_lock);
	printf("NetThread for arc: %i    @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ %d\n",_arc_id,_train_size);

	SMInterface SM(_pLogger);

	char* ptr;

	(void) time (&start_t);
	Arcs *arc = _catalog->getArc(_arc_id);

	tuple_descr=arc->getTupleDescr();

	num_fields = tuple_descr->getNumOfFields();

	// tell the queue monitor (if in use) how many messages there will be
	if (_queue_monitor != NULL) {
		_queue_monitor->setNumMessages(_num_messages);
	}

	printf("_interarrival: %f\n",_interarrival);
	char service[10];
	sprintf(service,"%i",8000+_arc_id);
	int msock = passiveTCP(service, 200);
	int ssock;
	char rmesg[MAXMESG];
	struct sockaddr_in fsin;
	int clilen;
	char tmpToken[1000];
	
	for(int i = 0;;i++)
	{    
		// If the queue monitor is in use, check if we are paused
		if (_queue_monitor != NULL) {
			while (!_queue_monitor->isRunning()) sleep(1);
		}
		for ( int tuple = 0; tuple < _train_size; tuple++ )
		{
			printf("[NetThread] Tuple %i/%i, train sz=%d tuple is %d\n",i+tuple,_num_messages, _train_size, tuple );
			// Remember: tuples now implicitly include a special timestamp and streamid field
			if ( (status & SEND_TO_OUT) > 0 )
				ptr = SM.enqueuePin( _arc_id, _train_size );
			else
				// the sizeof(int) comes from stream id (hard coded now!)
				ptr = ( char * ) malloc( _train_size * (tuple_descr->getSize() + 
														TUPLE_DATA_OFFSET + sizeof(int)));
			
			// get the current time stamp
			//(void) time (&t);
			
			// this is where we would read input from file!.
			// else do it the usual, oldfashioned way :)
			// New! More precision, seconds and microseconds
			if ( (status & READ_FROM_FILE) > 0 )
			{
				temp = fread( t, TUPLE_STREAMID_OFFSET, 1, fd );
				if ( temp <= 0 )
				{
					printf( "ST: Read failed miserably %d\n", _arc_id );
					exit( -1 );
				}
				printf(" ST: READ: %ld\n", t->tv_sec );
			}
			else
				if ( (status & SEND_TO_OUT) > 0 )
					if (gettimeofday(t, NULL) != 0) 
					{
						perror("NetThread: gettimeofday failed, setting timestamp for new tuple");
						exit(-1);
					}
			printf("YYY:NetThreadArc%d: t1: %ld  t2: %ld\n", _arc_id, t->tv_sec,t->tv_usec);
			
			if ( (status & WRITE_TO_FILE) > 0 )
			{
				if ( fwrite( t, TUPLE_STREAMID_OFFSET, 1, fd ) > 0 )
				printf(" SMI: WROTE: %ld\n", t->tv_sec );
				else
				{
					printf("SMI: I/O write failure\n");
					exit( 1 );
				}
			}
			// and now place it in the tuple. NOTE the hard coded 
			// assumption here that the tuple has enough space
			// in its timestamp field for whatever space this variable 't' takes up
			memcpy(ptr, t, sizeof(timeval));
			
			// Now, the streamid. For now, totally hard coded to be "1"
			int stream_id = 1;
			*((int*)(ptr+TUPLE_STREAMID_OFFSET)) = stream_id;

			ssock = accept(msock, (struct sockaddr *)&fsin,(socklen_t *) &clilen);
			if (ssock < 0)
			{
				printf("accept failed: %s\n",strerror(errno));
				exit(0);
			}
			bzero ((char *) rmesg, sizeof (rmesg));
			tcp_read(ssock,rmesg,MAXMESG);

			(void) close(ssock);

			int len = strlen(rmesg);
			istrstream ssLine(rmesg);

			
			printf("%dSMI: Generating (or reading) a tuple [{ts, streamid}, %d fields] : {%p, %d} \n",
			       _arc_id, num_fields, t, stream_id);
			
			for (int f=0; f<num_fields && ssLine.good() != 0; f++)
			{	  
				field_offset = TUPLE_DATA_OFFSET + tuple_descr->getFieldOffset(f);
				// here we read the file... and go to next field.
				if ( (status & READ_FROM_FILE) > 0 )
				{
					fread( ptr+field_offset, tuple_descr->getFieldSize( f ), 1, fd );
					if ( temp <= 0 ) {
						printf("ST: Read failed miserably %d\n",_arc_id);
						exit( -1 );
					}
					printf( "SMI: READ from file %d\n", *(ptr+field_offset ) );
					continue; // skip to end of filed generation
				}
				r = rand_r(&seed) % 19 + 1 + 0.01; // generate random tuple from 1 to 9
				memset((char *) tmpToken, '.', sizeof (tmpToken));
				ssLine.getline(tmpToken,len,' ');

				switch(tuple_descr->getFieldType(f))
				{
				case INT_TYPE:
					r = atof(tmpToken);
					*((int*)(ptr+field_offset)) = atoi(tmpToken);
					printf(", %d",  (int)r);	   
					break;
				case FLOAT_TYPE:
					r = atof(tmpToken);
					*((float*)(ptr+field_offset)) = atof(tmpToken); 
					printf(", %f",  (float)r);	  
					break;
				case DOUBLE_TYPE:
					r = atof(tmpToken);
					*((double*)(ptr+field_offset)) = atof(tmpToken);        
					printf(", %f",  (double)r);	  
					break;
				case STRING_TYPE:
					int chars = tuple_descr->getFieldSize(f);
					int char_offset = 0;
					printf(", ");
					  for (int si = 0; si < chars; si++) {
					    // This generates a char between a and d (d = ascii code 97)
                        char c = tmpToken[si];
                        if (c == '\0')
                            c = '.';
					    *(ptr + field_offset + char_offset) = c;
					    printf("%c",c);
					    char_offset++;
					  }
					break;
			}
			if ( (status & WRITE_TO_FILE) > 0 )
			  {
			    fwrite( ptr+field_offset, tuple_descr->getFieldSize( f ), 1, fd );
			    printf( "SMI: WROTE to file %d\n", *(ptr+field_offset ) );
			  }
		}
		printf("\n");
		printf("[NetThread]:ptr: %i\n",(int)*((int*)ptr));
		printTuple(ptr, "iii");
		    incrementTuplesGenerated(1);

		if ( (status & SEND_TO_OUT) > 0 )
		  {
		    //printf(" -------------------------------------->ENQUEUE, ST, into queue %d ( III%d )\n", _arc_id, i );
		    SM.enqueueUnpin(_arc_id, ptr, 1); 
		    //printf(" -------------------------------------->ENQUEUE2, ST, into queue %d ( III%d )\n", _arc_id, i );
		    // report one message generated
		  }
		if (_queue_monitor != NULL) {
			_queue_monitor->tickMessage();
		}
		free(ptr);
		pthread_mutex_lock(_sched_wake_mutex);
		pthread_cond_broadcast(_sched_wake_cond);
		pthread_mutex_unlock(_sched_wake_mutex);
		}
		
		// this is the sleeping part...
		double rm;
		if ( i % 5 == 0 )
		{
			rm = getRateMultiplier();
			_interarrival = (_interarrival/rm)*_train_size;
		}
		struct timeval timeout;
		timeout.tv_sec = (int)_interarrival; 
		timeout.tv_usec = (int)((_interarrival - (int)_interarrival) * 1000000); 
		//printf("_interarrival: %f\n",_interarrival);
		select(0,0,0,0,&timeout);
	}

	if ( (status & ( WRITE_TO_FILE )) > 0 ) fclose( fd );
	return (void *)0;
}

void *NetThread::entryPoint(void *pthis)
{
  NetThread *pt = (NetThread*)pthis;
  pt->run();
  return (void *)0;
}

void NetThread::start()
{
  pthread_create(&_thread, 0, entryPoint, this);
}
int NetThread::passiveTCP(char *service, int qlen)
{
	return passivesock(service, "tcp", qlen);
}/* end passiveTCP */

/***************************************************************************
**	passivesock - allocate & bind a server socket using TCP or UDP
**		the socket descriptor is returned
***************************************************************************/
int NetThread::passivesock(const char *service, const char *protocol, int qlen)
{
	struct servent *pse;
	struct protoent *ppe;
	struct sockaddr_in sin;
	int s, type;

	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	if ((pse = getservbyname(service, protocol)))
		sin.sin_port = htons(ntohs((u_short)pse->s_port) + portbase);
	else if ( (sin.sin_port = htons((u_short)atoi(service))) == 0)
	{
		printf("can't get \"%s\" sevice entry\n",service);
		exit(1);
	}

	if((ppe = getprotobyname(protocol)) == 0)
	{
		printf("can't get \"%s\" protocol entry\n", protocol);
		exit(1);
	}

	if (strcmp(protocol, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	s = socket(PF_INET, type, ppe->p_proto);
	if(s < 0)
	{
		printf("can't create socket: %s\n", strerror(errno));
		exit(1);
	}

	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf ("can't bind to %s port: %s\n",service,strerror(errno)); 
		exit(1);
	}
	if (type == SOCK_STREAM && listen(s,qlen) < 0)
	{
		printf("can't listen on %s port : %s\n",service,strerror(errno));
		exit(1);
	}

	return s;
}/* end passivesock */
void NetThread::tcp_read(int s, char *buf, int maxlinelen)
{
	int n;
	int inchars;

	inchars = 0;
	while(inchars < maxlinelen)
	{
		n = read(s,&buf[inchars],maxlinelen-inchars);
		//if ((n==0) && (buf[inchars] == NULL))
		if (n==0)
			break;
		inchars = inchars+n;
		if (buf[inchars-1] == '\n')
			break;
	}
}/* end tcp_read */
void NetThread::tcp_read_tuple(int s, char *buf, int num_fields, TupleDescription *tuple_descr, int maxlinelen)
{
	int n;
	int inchars;
	int num_words;

	num_words = 0;
	inchars = 0;
	int num_string_chars_read = 0;
	while(inchars < maxlinelen && num_words < num_fields)
	{
		n = read(s,&buf[inchars],1);
		if ( tuple_descr->getFieldType(num_words) != STRING_TYPE )
		{
			if (buf[inchars] == ' ')
				num_words++;
			if (n==0)
				break;
			if (buf[inchars] == '\n')
			{
				inchars = inchars+n;
				break;
			}
		}
		else
		{
			num_string_chars_read++;
			if ( num_string_chars_read == tuple_descr->getFieldSize(num_words) )
			{
				num_words++;
				num_string_chars_read=0;
			}

		}
		inchars = inchars+n;
	}
}/* end tcp_read */
