#include <SMInterface.H>
#include <Measurement.H>
#include <Shared.H>
#include <UnionBox.H>

#include <malloc.h> // for memalign
#include <sys/mman.h> // for mprotect
#include <limits.h> // for PAGESIZE

//#define TOTAL_MEMORY 1000  //available memory
#define USE_MEMORY 1
int TOTAL_MEMORY = 0;


// These are defines for now, since it is easier to do than mess with static objects and initialization from non-static methods - eddie
// The default is also now 128MB
#define PAGEPOOL_PAGE_SIZE 4096 // bytes
#define PAGEPOOL_NUM_PAGES 65536 // PAGEPOOL_PAGE_SIZE * PAGEPOOL_NUM_PAGES = Bytes allocated

extern QueueMon* _queue_monitor;

bool SMInterface::_initialized = false;
PagePool SMInterface::_pool(PAGEPOOL_PAGE_SIZE,PAGEPOOL_NUM_PAGES,false);
map<int,TupleQueue*> SMInterface::_queues;


SMInterface::SMInterface(){

  if (!_initialized) {
    unsigned int n = 0;
    //_catalog->printArcs();
    vector<Arcs*> *arcs = _catalog->getArcs();
    for (int i = 0; i < arcs->size(); ++i) {
      // Notes: here we, for each good arc id we have, insert it into the map along with a TupleQueue for that arc.
      if ((*arcs)[i] != NULL) {
	++n;
	//cout << " the tuple size for this arc (id "<<(*arcs)[i]->getId()<<") is "<< TUPLE_DATA_OFFSET + ((*arcs)[i]->getTupleDescr())->getSize()<<endl;
	_queues.insert(make_pair((*arcs)[i]->getId(),new TupleQueue(_pool, TUPLE_DATA_OFFSET + ((*arcs)[i]->getTupleDescr())->getSize())));
      }
    }
    _initialized = true;
    //cout << "[SMInterface] Initialized "<<PAGEPOOL_PAGE_SIZE * PAGEPOOL_NUM_PAGES / 1048576 <<"MB Page Pool for "<<n<<" TupleQueues" << endl;
  }
  
  TOTAL_MEMORY = _measure->getMemorySize();
  _timed_chunk_1 = 0.0;
  _timed_chunk_2 = 0.0;
  _timed_chunk_3 = 0.0;
  _timed_chunk_4 = 0.0;
  _itimer_start = NULL;
  //printf(" TOTAL MEMORY %d\n", TOTAL_MEMORY );
  /**
     _queue_mutex = new pthread_mutex_t*[_catalog->getMaxArcId()+1];
     for (int i = 0; i < _catalog->getMaxArcId()+1; i++) {
     _queue_mutex[i] = new pthread_mutex_t;
     pthread_mutex_init(_queue_mutex[i],NULL);
     }
  */
}

SMInterface::~SMInterface()
{
};

void* SMInterface::SM_malloc(int bytes) {

	 if (bytes <= 0) 
	 {
		 //printf("[SMInterface] No bytes to allocated?! (%d) Please fix caller!\n", bytes);
		 return NULL;
	 }



	 /** New! We will provide memory protection right here... */
	 // step 1 - make bytes a multiple of PAGESIZE
	 int PAGESIZE = getpagesize();
	 if ((bytes % PAGESIZE) != 0) bytes += PAGESIZE - (bytes % PAGESIZE);
	 // step 2 - allocate 1 extra page beyond the bytes asked for, aligned
	 void* ptr = memalign(PAGESIZE, bytes+PAGESIZE);
	 if (ptr == NULL) {
	   printf("[SMInterface] SM_malloc: failed for %d bytes.\n",bytes);
	   perror("[SMInterface] SM_malloc: Aborting due to memalign() failure:\n");
	   abort();
	 }
	 // step 3 - mprotect that last page
	 //cout << "calling mprotect("<<ptr<<"+"<<bytes<<", "<<PAGESIZE<<",PROT_NONE"<<endl;
	 
	 // WARNING! This code allowed me to find out that someone WRITES PAST the end of the sm_malloc buffer
	 //           but apparentely does not READ from it! Leaving this code here, protecting it against reads
	 //           may push this bug away for now...

	 if (mprotect((char*)(ptr) + bytes,PAGESIZE,PROT_WRITE) != 0) { // why I have to do char*, I dunno
	   perror("[SMInterface] SM_malloc: failed mprotect():");
	   abort();
	 }

	 // OLD CODE WAS JUST THIS
	 /**
	 void* ptr = malloc(bytes);
	 if (ptr == NULL) {
	   printf("[SMInterface] SM_malloc: failed for %d bytes.\n",bytes);
	   perror("[SMInterface] SM_malloc: Aborting due to malloc() failure:\n");
	   abort();
	 }
	 */
	 /**
	 {
	   // test protection
	   char a = 'a';
	   char x = 'x';
	   // this should succeed
	   memcpy(ptr,&a,1);
	   // this should fail
	   memcpy((char*)(ptr)+bytes+1,&x,1);
	 }
	 */
	 _measure->incrementNumMallocs();
	 //memset(ptr,0,bytes);
	 //printf("[SMInterface] SM_malloc: %d bytes at %p\n", bytes, ptr);
	 return ptr;
}

void SMInterface::SM_free(void* p) 
{
	//printf("[SMInterface] SM_free: Freeing %p\n", p);
	 free(p);
}

char* SMInterface::enqueuePin(int arc_id, int num_tuples)
{ 
	 // New Code by Eddie Galvez

	 /**
	  * This function just allocates memory for num_tuples tuples
	  * Note: we must be careful to allocate the extra size needed for any fields
	  *  other than the user specified fields (currently timestamp (int) and streamid (int))
	  */
	 TupleDescription *tuple_descr;
	 int tuple_size=0;

	 Arcs *arc = _catalog->getArc(arc_id);
	 tuple_descr=arc->getTupleDescr();
	 tuple_size=tuple_descr->getSize();
  
	 int bytesNeeded = (tuple_size + TUPLE_TIMESTAMP_SIZE + TUPLE_STREAMID_SIZE) * num_tuples;
	 //printf("[SMInterface] enqueuePin: %d bytes for %d tuple(s) : arc_id: %i [%d free pages]\n", bytesNeeded, num_tuples,arc_id, _pool.free_pages());
	 char *ptr;
	 if (num_tuples == 0 || tuple_size == 0)
		  ptr = NULL;
	 else
	 {
		  ptr = (char*) SM_malloc(bytesNeeded);
	 }
  
	 return ptr;
};

// The arc_id is useful so I can writeout the tuple descriptor information
// to a file
// THIS FUNCTION HAS BEEN DISCONTINUED. We do not deal with disk, and when we
// DO, we will use SleepyCat rather than simple files 
/****************************************************************************
FILE* SMInterface::openQueueFileAndLock(char* fname, char* mode) {
	 printf("Opening queue file (%s) mode (%s)\n", fname,mode);
	 FILE* fd;
 openit:
	 errno = 0;
	 fd = fopen(fname, mode);
	 if (fd == NULL) 
	 {
		  if ((errno == ENOENT) && (strcmp(mode,"a+b") != 0)) 
		  { 
			   // This just means the file doesn't exist, so we should create it (see enqueueUnpin)
			   //fd = fopen(fname, "a+b"); 
			   mode = const_cast<char *>("a+b");
			   // Call to write out the tuple description for this queue
			   // NOTE: this implies the files have to be NONEXISTANT when we run
			   //       the system, unless I use a global flag to find out if
			   //       in this current run we have written the descriptors yet to file or not
			   // NOTE2: Actually, any "queue viewing" program should really just access the catalog to get the descriptions...
			   // NOTE3: Nevermind, see lex_stuff.C for a call to writeOutQueueDesc()

			   goto openit;
		  }
		  else if ( errno == EINTR )
		  {
			   cerr << "EINTR\n";
			   goto openit;
		  }

		  printf("fd: %p\n",fd);
		  perror("Failed opening file");
		  exit(1);
	 }
	 // try a fstat
	 //  stat b = (struct stat) malloc(sizeof(stat));
	 //struct stat st;
	 //  fstat(fileno(fd), &st);

	 // Now lock. We think that locking of file is no longer neccesary
	 // since all operations rely on locking the arc before initiating.
	 //lockQueue(fd);
	 return fd;
}

void SMInterface::closeQueueFileAndUnlock(FILE* fd) 
{
	 // Unlock first lockingQueue disabled. Relying on Arc locking now
	 // unlockQueue(fd);
	 // Now close file
	 //printf("Closing queue file\n");
	 if (fclose(fd) != 0) {
		  perror("Failed closing file");
		  exit(1);
	 }
}
****************************************************************************/


/******************
void SMInterface::readTopTuples(char* ptr, int sizePerTuple, int num_tuples, FILE* fd) {
	 //rewind(fd); // Seek to start of queue
	 // Use fseek instead, rewind returns no value
	 if (fseek(fd, 0L, SEEK_SET) != 0) {
		  perror("readTopTuples: failed to seek (to offset 0)");
		  exit(1);
	 }
	 //printf(" DEBUG readTopTuples in readTopTuples %d out of? (mem )\n", num_tuples);
	 readTuples(ptr, sizePerTuple, num_tuples, fd);
}
******************/

void SMInterface::readTopTuples2(char* ptr, int sizePerTuple, int num_tuples, 
								 int queue_id ) 
{
	 //  printf(" THIS IS GOING TROUGH read Top Tuples2 YYY\n");

	 readTuples2(ptr, sizePerTuple, num_tuples, queue_id );
}

void SMInterface::readTuples2(char* ptr, int sizePerTuple, int num_tuples, int queue_id ) {
	 char* lptr = ptr;
	 //printf("Reading %d tuples into memory\n", num_tuples);
	 for (int i = 0; i < num_tuples; i++) {
		  readTuple2(lptr, sizePerTuple, queue_id, i );
		  //printf(" debug: %i(out of %i)YYY readTuples2-----> T1%i, T2%i, into queue %d sz %d STREAM?%i\n",i,num_tuples,(int)*((int*)lptr),(int)*((int*)(lptr+4)), queue_id, m_queue[ queue_id ].size(), (int)*((int*)(lptr+8)));
		  lptr += sizePerTuple;
	 }  
}

/*************
void SMInterface::readTuples(char* ptr, int sizePerTuple, int num_tuples, FILE* fd) {
	 char* lptr = ptr;
	 _measure->incrementSMReads();
	 //printf("Reading %d tuples into memory\n", num_tuples);
	 for (int i = 0; i < num_tuples; i++) {
		  readTuple(lptr, sizePerTuple, fd);
		  //printf("%i readTuples -----> %i, %i, %i\n",i,(int)*((int*)lptr),(int)*((int*)(lptr+4)),(int)*((int*)(lptr+8)));
		  lptr += sizePerTuple;
	 }  
}
**************/

void SMInterface::readTuple2(char* ptr, int len, int queue_id, int which ) 
{
  /** NEWSM HACK */
  /**
	 //printf(" SHM DEBUG %d\n", shm_get_num_records_in_queue(queue_id) );
	 if ( m_queue[ queue_id ].size() <= 0 )
	 {
		  printf("DEBUG ERROR: SMInterface: Attempted to read from an empty queue %d?\n",
				 queue_id );
		  exit( 1 );
	 } 

	 char *tup = m_queue[ queue_id ][ which ];
	 memcpy( ptr, tup, len );
	 //m_queue[ queue_id ].pop();
	 //free( tup );  pop performs free? that free crashes at any rate...
	 */
	 /** NEWSM **/
	 // Here, we just overwrite ptr with OUR stuff
	 TupleQueue::DeqIterator deq = _queues[queue_id]->deq_iterator();
	 assert(deq.avail());
	 //cout << " NEW SM CALL: memcpy(ptr, deq.tuple(), "<<len<<")"<<endl;
	 memcpy(ptr,(char*)deq.tuple(),len);
	 ++deq;
}

/************
void SMInterface::readTuple(char* ptr, int len, FILE* fd) {
  cout << " readTuple() DO NOT CALL ME " << endl;
  exit(1);
	 int x = 0;
	 if ((x=fread(ptr, len, 1, fd)) != 1) {
		  if (feof(fd))
			   fprintf(stderr, "[SMInterface] Failed reading one tuple due to EOF");
		  else
			   perror("[SMInterface] Failed reading one tuple");
		  
		  assert(false);
		  abort();
	 }
}
**************/

// Dump the queues to disk and free up sufficient space
// to record the new tuples in memory.
// Function no longer exists -- erase when everything compiles and works fine
/***********************************************************************
int SMInterface::dumpQueuesToDisk( int _currqueue, int need_free )
{


  // NEWSM HACK - ALWAYS "HAVE MEMORY", DISREGARD CHECK
  return 1;


	 //printf("[SMInterface] DEBUG dump todisk: purg Freeing %d space(s) currently available %d for queue %d\n", need_free , TOTAL_MEMORY-memory_in_use, _currqueue );
	 // we loop until enough space is freed up. We assume that we
	 // never need to free more memory than there is available by dumping
	 // queues to disk.
	 FILE *fd;
	 char queueFilename[FILENAME_MAX]; // The file
	 int purge_queue = -1;

	 vector<int>::iterator iter;
	 vector<int> *v = new vector<int>();
	double t_a_start = get_etime(_itimer_start);
	 for ( int purge_queue = 0; purge_queue < _catalog->getMaxArcId()+1; purge_queue++)
		  v->push_back( purge_queue );
  
	 random_shuffle( v->begin(), v->end() );
	double t_a_stop = get_etime(_itimer_start);
	_timed_chunk_1 += t_a_stop-t_a_start;
	 for ( iter = v->begin(); iter != v->end(); iter++ )
	 {
		  purge_queue = (*iter);
		  pthread_mutex_lock( &memory_mutex );
		  if ( memory_in_use + need_free < TOTAL_MEMORY * 100 ) // NEWSM hack * 100
		  {
			   memory_in_use+=need_free;
			   //printf("DEBUG MEMORY now using %d, just reserved %d FOR %d\n", memory_in_use, need_free, _currqueue );

			   pthread_mutex_unlock( &memory_mutex );
			   delete v;
			   return 1; // success, free memory found and reserved.
		  }
		  else
			   pthread_mutex_unlock( &memory_mutex ); // keep trying...
	
		  if ( purge_queue == _currqueue ) continue;
		  //printf("DEBUG: MEMORY let's purge queue %d (for %d) mem %d\n", purge_queue, _currqueue, memory_in_use  );

		  TupleDescription *tuple_descr;
		  int sizePerTuple;
      
		  // Lock the arc
		  Arcs *arc = _catalog->getArc( purge_queue );  
		  if ( arc->tryLockArc() == EBUSY ) 
		  {
			   //printf(" DEBUG: tried and failed to lock %d\n", purge_queue );
			   continue; // couldn't get that arc. oh well..
		  }
		  else
			   ;//printf("DEBUG: SUCCESS ... locked arc %d with %d in mem\n", purge_queue, m_queue[ purge_queue ].size() );

		  if ( m_queue[ purge_queue ].empty() ) 
		  {
			   arc->unlockArc();
			   continue; 
		  }
      
		  // Retrieve metadata needed
		  tuple_descr=arc->getTupleDescr();  
		  sizePerTuple = tuple_descr->getSize() + TUPLE_STREAMID_SIZE + TUPLE_TIMESTAMP_SIZE;
      
		  printf("Opening queue file (queue. line 264\n");
		  sprintf(queueFilename,"queue.%05d", purge_queue );
		  fd = openQueueFileAndLock(queueFilename, const_cast<char*>("r+b"));

		  // Write out the tuples AHA!
		  int sz = m_queue[ purge_queue ].size();

		  if ( sz * sizePerTuple <= 0 )
		  {
			   printf(" DEBUG WTF2? sz=%d, queue %d\n",sz,m_queue[ purge_queue ].size() );
			   exit( 1 );
		  }
		  //char *front = (char *) malloc( sz * sizePerTuple );

		  for ( int i = 0; i < sz; i ++ )
		  {
			   if ( m_queue[ purge_queue ].size() <= 0 )
			   {printf(" ERROR: DEBUG: queue %d has no tuples %d yet is read from!\n", purge_queue, m_queue[ purge_queue ].size() );
			   exit( 1 );
			   }

			   writeTuple( m_queue[ purge_queue ][i], sizePerTuple, fd );
			   //.back()
			   tuplesOnDisk[ purge_queue ] ++;
			   //m_queue[ purge_queue ].pop();
		  } 
		  //int psz = m_queue[ purge_queue ].size();
		  Vector::iterator iter;
		  for ( iter = m_queue[ purge_queue ].begin(); 
			iter != ( m_queue[ purge_queue ].end() ); iter++ ) {
		    //free( (*iter) ); // NEWSM HACK CUZ I DONT PUSH MEMORY IN THERE ANYMORE
		  }
		  m_queue[ purge_queue ].clear();
    
		  pthread_mutex_lock( &memory_mutex );
		  memory_in_use -= sizePerTuple * sz;
		  //printf("DEBUG MEMORY now purge %d, just lost %d from QUEUE %d tups %d\n", memory_in_use, sizePerTuple * sz, purge_queue, shm_get_num_records_in_queue( purge_queue ) );
      
		  if ( memory_in_use < 0 || memory_in_use > TOTAL_MEMORY ) 
		  { printf("SMInterface: something wrong with mem usage!\n");
		  exit( 1 );}

		  pthread_mutex_unlock( &memory_mutex );
		  closeQueueFileAndUnlock(fd);
      
		  // Unlock arc
		  arc->unlockArc();
		  printf("DEBUG: DONE  purgeing queue %d (for %d) mem %d\n", purge_queue, _currqueue, memory_in_use );
      
	 }
  
	 delete v;
	 printf(" DEBUG: lock? failed to secure memory for %d. oh well...\n", _currqueue);
	 return 0;  // we have failed to free the memory for a queue.
	 // perhaps if we had been smarter, we'd succseeded... but alas.
	 // just write the queue to disk then.
}
***********************************************************************************/

//this function is sort of discountinued.
/******************************************************************************
void SMInterface::checkMemory()
{
	 int arcs = _catalog->getMaxArcId()+1;
	 int tups =0;

	 for (int i = 0; i < arcs; i++ )
		  tups += m_queue[ i ].size();

	 if ( memory_in_use / tups != 12 )
	 { printf(" mem!=tuples DEBUG\n" );
	 exit( 1 ); }
}
******************************************************************************/

// old interface.
//void SMInterface::writeTuples(char* ptr, int sizePerTuple, int num_tuples, 
//							  FILE* fd, int _currqueue, int seekFD ) 
void SMInterface::writeTuples(char* ptr, int sizePerTuple, int num_tuples, int _currqueue )
{
	//char queueFilename[FILENAME_MAX]; // The file
	 //int openedFD = 0;

	 if (num_tuples <= 0) 
	 {
	   // DO NOT COMMENT THIS. It should be a BUG to call writeTuples when there actually is nothing to write
	   //printf("[SMInterface] writeTuples: No tuples to write?! (%d) Please fix caller!\n", num_tuples);
	   return;
	 }

	 //  closeQueueFileAndUnlock( fd );
	 char* lptr = ptr;
	 //printf("[SMInterface] Writing %d tuples into queue %i (curr queue size %d)\n", num_tuples,_currqueue, m_queue[ _currqueue ].size());

	 //int size_modifier = 0; //this is a special modifier to account for
	 // bringing the tuples that were dumped to disk.
	 Arcs *a = _catalog->getArc(_currqueue);

	 if (_itimer_start == NULL )
		_itimer_start = new struct itimerval;
	 /****

	 int totalTuples = tuplesOnDisk[ _currqueue ] + m_queue[ _currqueue ].size();
	 if ( USE_MEMORY && m_queue[ _currqueue ].size() < totalTuples )
	 {	 
	   cout << " writeTuples() DON'T BE HERE PLEASE! " << endl;
	   exit(1);
		  if ( !openedFD )
		  {
			   printf("Opening queue file (queue. line 358\n");
			   sprintf(queueFilename,"queue.%05d", _currqueue);
			   fd = openQueueFileAndLock(queueFilename, const_cast<char*>("r+b"));
			   openedFD = 1;
		  }
		
		
		  // Use fseek, rewind returns no value
		  if (fseek(fd, 0L, SEEK_SET) != 0) {
			   perror("writeTuples: failed to seek (to offset 0)");
			   exit(1);
		  }

		  //printf("       DEBUG: YYY TUPS MEMORY SMInterface, non zero queue %d total(?) %d, in queue %d on disk %d\n", _currqueue, totalTuples, m_queue[ _currqueue ].size(), tuplesOnDisk[ _currqueue ] );

		  int size = tuplesOnDisk[ _currqueue ] - m_queue[ _currqueue ].size();
		  char *t;
		  _measure->incrementSMReads();
		  for ( int i = 0; i < size; i++ )
		  {
			   t = ( char * )malloc( sizePerTuple );
			   fread( t, sizePerTuple, 1, fd );
			   m_queue[ _currqueue ].push_back( t );
			   tuplesOnDisk[ _currqueue ]--;
			   size_modifier += sizePerTuple;
		  }
	 }
	 *************/

	 /*******************
	 int use_memory = USE_MEMORY;

	 if ( USE_MEMORY )
	 {
		 if  ( dumpQueuesToDisk( _currqueue, num_tuples * sizePerTuple + size_modifier ) )
		 {
		  //printf(" RESERVED YYY for %d, memory = %d\n",  _currqueue, num_tuples * sizePerTuple + size_modifier );
			   if ( tuplesOnDisk[ _currqueue ] > 0 )
					exit( 1 );
		  }
		  else
		  {
			   //printf("DEBUG: lock there was no memory to be found... had to improvize (for %d sz %d) needed TUPS %d\n", _currqueue, m_queue[ _currqueue ].size(), num_tuples);
			   use_memory = 0;
	   
			   if ( !m_queue[ _currqueue ].empty() ) 
			   {
			     cout << " writeTuple() 2 - I DONT want to be here !!! " << endl;
			     exit(1);

					TupleDescription *tuple_descr;
					int sizePerTuple;
					// Retrieve metadata needed
					tuple_descr=a->getTupleDescr();  
					sizePerTuple = tuple_descr->getSize() + TUPLE_STREAMID_SIZE + TUPLE_TIMESTAMP_SIZE;
		
					// Write out the tuples AHA!
					int sz = m_queue[ _currqueue ].size();
					//printf(" DEBUG PUURGING SELF? sz %d, szperT %d, damn arc %d\n", sz, sizePerTuple, _currqueue );
		
					if ( !openedFD )
					{
						 printf("Opening queue file (queue. line 414\n");
						 sprintf(queueFilename,"queue.%05d", _currqueue);
						 fd = openQueueFileAndLock(queueFilename, const_cast<char*>("r+b"));
						 openedFD = 1;
					}

					// Use fseek instead, rewind returns no value
					if (fseek(fd, 0L, SEEK_SET) != 0) {
						 perror("writeTuples: failed to seek (to offset 0)");
						 exit(1);
					}

	double t_b_start = get_etime(_itimer_start);
					for ( int i = 0; i < sz; i ++ )
					{
						 writeTuple( m_queue[ _currqueue ][ i ], sizePerTuple, fd );
						 //.back()
						 tuplesOnDisk[ _currqueue ] ++;
						 //m_queue[ purge_queue ].pop();
					}

					Vector::iterator iter;
					for ( iter = m_queue[ _currqueue ].begin(); 
					      iter != ( m_queue[ _currqueue ].end() ); iter++ ) {
					  //free( (*iter) ); // NEWSM HACK CUZ I DONT PUSH MEM ANYMORE IN THERE
					}
					m_queue[ _currqueue ].clear();
					pthread_mutex_lock( &memory_mutex );
					memory_in_use -= ( sizePerTuple * sz - size_modifier );
					//printf("DEBUG MEMORY SELF is NOW now using %d, just lost %d from %d (have %d in queue, correction %d )\n", memory_in_use, sizePerTuple * sz - size_modifier, _currqueue, m_queue[ _currqueue ].size(), totalTuples );
					pthread_mutex_unlock( &memory_mutex );
	double t_b_stop = get_etime(_itimer_start);
	_timed_chunk_2 += t_b_stop-t_b_start;
			   }
		  }
	 }
	   
	 //printf("YYY queue %d, writing tuples %d, use memory NOW %d, curr queue size %d and on disk? %d\n", _currqueue, num_tuples, use_memory, m_queue[_currqueue].size(), tuplesOnDisk[ _currqueue ] );
	 *********************/

	 double t_c_start = get_etime(_itimer_start);
	 for (int i = 0; i < num_tuples; i++) 
	 {
		  if (_queue_monitor != NULL) 
		  {
			  //cout << " Adding tuple queue " << _currqueue << endl;
			   _queue_monitor->addTuple(lptr, a->getTupleDescr(), _currqueue);
		  }
		  //if ( use_memory )
		  //cout << " Write tuple second " << " hm, num tuples is ? " << num_tuples <<  endl;
		  writeTuple2(lptr, sizePerTuple, _currqueue);
		  /******
				 else
				 {
		    cout << "writeTuple() 3 -- euh, i dont really want this either yo" << endl;
			   _measure->incrementSMWrites();
			   if ( !openedFD )
			   {
					printf("Opening queue file (queue. line 457\n");
					sprintf(queueFilename,"queue.%05d", _currqueue);
					fd = openQueueFileAndLock(queueFilename, const_cast<char*>("r+b"));
					openedFD = 1;

					// Use fseek instead, rewind returns no value
					if (fseek(fd, seekFD, SEEK_SET) != 0) {
						 perror("writeTuples: failed to seek (to offset 0)");
						 exit(1);
					}
			   }

			   writeTuple(lptr, sizePerTuple, fd );
			   if ( USE_MEMORY )  //this means we failed to secure memory
					// and though we use memory, we dump to disk.
					tuplesOnDisk[ _currqueue ]++;
		  }
		  *******/
		  lptr += sizePerTuple;
	 }
	double t_c_stop = get_etime(_itimer_start);
	_timed_chunk_3 += t_c_stop-t_c_start;
	double t_d_start = get_etime(_itimer_start);
	 if ( a->getIsApp() == true )
	 {
		  if ( _measure->getStopType() == TUPLES_WRITTEN )
		  {
			   _measure->incrementTuplesWritten(num_tuples);
			   _measure->testStopCond();
		  }
	 }
	 lptr=ptr;
	 if ((num_tuples > 0) && (a->getSockFD() > 0))
	 {
		  //change to however you 
		  // want to detect it's the last
		  // queue. Make sure you check
		  // for num_tuples > 0
		  // Send the data the application wants
		  // [id] [x] [y] [color]

		  void* p = malloc((sizeof(int) * 3) + sizeof(char)); // alloc the packet 
		  memcpy(p, ptr + TUPLE_DATA_OFFSET, sizeof(int)); // copy the data (for now only 1 int)
		  for (int i = 0; i < num_tuples; i++) 
		  {
			   MITRE_sendData(lptr, a->getSockFD(),sizePerTuple); //!!!
			   lptr += sizePerTuple;
		  }
		
	 }

	 // NO MORE FILE ACCESS (EVER again)
	 //if ( openedFD )  // should be safe, all synchronized on lockArc()
	 //closeQueueFileAndUnlock( fd );

	double t_d_stop = get_etime(_itimer_start);
	_timed_chunk_4 += t_d_stop-t_d_start;
	 //printf("DEBUG: finshed write tuples for %d\n", _currqueue );
}

// this is temporary new version of writeTuple
// maybe not so temporary
void SMInterface::writeTuple2( char *ptr, int len, int queue_id )
{
	 /*if ( m_queue[ queue_id ] == NULL )
	   {
	   Queue* q = new Queue();
	   //queue<char *> *q = new (queue<char *>)();
      
	   m_queue[ queue_id ] = q;
	   }
	   else
	   printf(" DEBUG: guess what, it's already there! %d\n", queue_id );
	 */

  /** NEWSM HACK (no more hacks :)*/
  //m_queue[queue_id].push_back("");
  /**
	 char *tup = ( char * )malloc( len );
	 memcpy( tup, ptr, len );
	 m_queue[ queue_id ].push_back( tup );
  */
	 //printf("---> YYY push into queue %d, tup %i, %i, total resulting queue size %d (tuple size? %d) on disk? %d\n", queue_id, (int)*((int*)tup), (int)*((int*)(tup+4)), m_queue[ queue_id ].size(), len, tuplesOnDisk[ queue_id ] );
	 //printf("DEBUG: TUPS: MEMORY push tup INTO queue %d for total %d out of ? %d\n", queue_id,
	 //	  m_queue[ queue_id ].size(), shm_get_num_records_in_queue(queue_id));
	 //if ( m_queue[ queue_id ].size() < shm_get_num_records_in_queue(queue_id))
	 //  exit( 1 );

	 /** NEWSM */
	 TupleQueue::EnqIterator enq = _queues[queue_id]->enq_iterator();
	 //cout << " NEW SM CALL: memcpy(enq.tuple(), ptr, "<<len<<")"<<endl;
	 memcpy((char*)enq.tuple(), ptr, len);
	 ++enq;
}

/****
	 void SMInterface::writeTuple(char* ptr, int len, FILE* fd) {
  cout << " EEEK dont call me please!" << endl;

	 if (fwrite(ptr, len, 1, fd) != 1) {
		  perror("Failed writing one tuple");
		  exit(1);
	 }
}
******/

char* SMInterface::dequeuePin(int arc_id, int num_tuples)
{
	 // METHOD LOCK!
	 //pthread_mutex_lock(_queue_mutex[arc_id]);

	 // Code by Eddie Galvez 
         //printf("[SMInterface] dequeuePin: Read %d tuple(s) from arc %d\n", num_tuples, arc_id);

	 /**
	  * This function should:
	  * Read tuples from the queue files to memory and return pointer to start of that region
	  */
	 //FILE* fd = NULL;
	 //char queueFilename[FILENAME_MAX]; // The file
	 TupleDescription *tuple_descr;
	 int sizePerTuple;
    
	 // Lock the arc
	 Arcs *arc = _catalog->getArc(arc_id);  
	 //printf("DEBUG: SMInterface line 527: locking arc %d\n", arc_id );
	 arc->lockArc();

	 //_currqueue = arc_id;

	 // Retrieve metadata needed
	 tuple_descr=arc->getTupleDescr();  
	 sizePerTuple = tuple_descr->getSize() + TUPLE_TIMESTAMP_SIZE + TUPLE_STREAMID_SIZE;
	 // Allocate memory for tuples to be read into
	 char *ptr = enqueuePin( arc_id, num_tuples );
  
	 // Read the top num_tuples tuples into memory
	 // arc is locked, if queue empty, contents are on disk...
	 //if ( USE_MEMORY && !(m_queue[ arc_id ].empty() ))
	 //if ( true )
	 
	 // we do not want to read in dequeuePin (i.e. we want to read
	 // but we don't want to consume the tuples)
	 //readTopTuples2(ptr, sizePerTuple, num_tuples, arc_id );
	 
	 TupleQueue::SeekIterator seek = _queues[ arc_id ]->seek_iterator();
	 for ( int i = 0; i < num_tuples; i++ )
	 {
		 memcpy(ptr + (i * sizePerTuple), (char*)seek.tuple(), sizePerTuple);
		 ++seek;
	 }

	 /*****************************  NEVER READ FROM DISK ****************
                     
		  else
	 { // read from disk :(

	   cout << "dequeuePin() HELL NO get me outta here please" << endl;
	   exit(1);

		  if ( num_tuples > 0 )  // unless there is really nothing to read...
		  {
			   // Open the storage file for reading
			   printf(" Opening qu? %d (line 577 new)\n", num_tuples );
			   printf("Opening queue file (queue. line 577\n");
			   sprintf(queueFilename,"queue.%05d", arc_id);
			   fd = openQueueFileAndLock(queueFilename, const_cast<char*>("rb"));
		
			   //printf("DEBUG about to readTopTuples, arc %d\n", arc_id );
			   readTopTuples(ptr, sizePerTuple, num_tuples, fd);
		
			   // Done!
			   closeQueueFileAndUnlock(fd);
		  }
		  else
			   ;//cerr << "dequeuePin called with num_tuples==0. Why? You tell me..." << endl;
	 }
		  ***************************************/

	 // Unlock arc
	 arc->unlockArc();
	 // Return

	 // METHOD LOCK!
	 //pthread_mutex_unlock(_queue_mutex[arc_id]);

	 //printf("ARC_ID: %i  address: %i\n",arc_id,ptr);
	 return ptr;
};

queue_info SMInterface::dequeueEnqueuePin(int arc_id, int num_tuples)
{
	 // this function allocates enough memory for the 
	 // the current contents of a queue plus the number of 
	 // tuples expected to be written to the rear of the queue.
	 // assume that this function is only used for train scheduling
	 // situations so, the whole queue is dequeuePin'd
	
	//FILE* fd = NULL;
	//char queueFilename[FILENAME_MAX]; // The file
	 TupleDescription *tuple_descr;
	 int sizePerTuple;
    
	 // Lock the arc
	 Arcs *arc = _catalog->getArc(arc_id);  
	 //printf("DEBUG: SMInterface line 581: locking arc %d\n", arc_id );
	 arc->lockArc();


	 // Retrieve metadata needed
	 tuple_descr=arc->getTupleDescr();  
	 sizePerTuple = tuple_descr->getSize() + TUPLE_STREAMID_SIZE + TUPLE_TIMESTAMP_SIZE;
	 // Allocate memory for tuples to be read into
	 queue_info qi;
	 //cout << " EnqueuePin " << arc_id << " Num tuples " << num_tuples << " in queue? " << shm_get_num_records_in_queue(arc_id) << endl;
	 qi.front = enqueuePin(arc_id, (num_tuples+shm_get_num_records_in_queue(arc_id)));
	 qi.rear = qi.front + (shm_get_num_records_in_queue(arc_id) * sizePerTuple);
	 qi.num_tuples_resident = shm_get_num_records_in_queue(arc_id);
	 qi.num_tuples_alloc = num_tuples+shm_get_num_records_in_queue(arc_id);
	 //char *ptr = enqueuePin(arc_id, (num_tuples+shm_get_num_records_in_queue(arc_id)));
	 //char *ptr = enqueuePin(arc_id, num_tuples);

	 //assert ( shm_get_num_records_in_queue(arc_id) < num_tuples);
	 // Read the top num_tuples tuples into memory

	 TupleQueue::DeqIterator deq = _queues[ arc_id ]->deq_iterator();
	 
	 // in dequeueEnqueuePin tuples ARE CONSUMED BY DEFAULT. which is
	 // different from enqueuePin where tuples are NOT consumed by default.
	 // unless dequeueEnqueueUnPin returns some tuples, the queue will be empty.
	 for ( int i = 0; i < shm_get_num_records_in_queue(arc_id); i++ )
	 {
		 memcpy( qi.front, (char*)deq.tuple(), sizePerTuple);
		 ++deq;
	 }
	 /*******************************  unneccessary..
	 if ( num_tuples > 0 ) // just skip the reading part...
	 {
		  if ( USE_MEMORY && !(m_queue[ arc_id ].empty() ))
			   readTopTuples2(qi.front, sizePerTuple, shm_get_num_records_in_queue(arc_id), arc_id );
		  else 
			   if ( shm_get_num_records_in_queue(arc_id) > 0 )
			   {
			     
			     cout << " HELLLLLL NOOO IM OUTTA" << endl;
			     exit(1);
		
					printf("Opening quWTF? empty? %d, shm??? %d\n", m_queue[ arc_id ].empty(), shm_get_num_records_in_queue(arc_id) );
					// Open the storage file for reading
					printf("Opening queue file (queue. line 638\n");
					sprintf(queueFilename,"queue.%05d", arc_id);
					fd = openQueueFileAndLock(queueFilename, const_cast<char*>("rb"));
					//printf("DEBUG about to readTopTuples, arc %d\n", arc_id );
		
					readTopTuples(qi.front, sizePerTuple, 
								  shm_get_num_records_in_queue(arc_id), fd);
					// Done!
					closeQueueFileAndUnlock(fd);
			   }
	 }
	 else
		  ;//cerr << "dequeueEnqueuePin called with num_tuples==0. Why? You tell me..." << endl;

	 //printf("[SMInterface] dequeueEnqueuePin: Read %d tuple(s) from queue.%05d\n", shm_get_num_records_in_queue(arc_id), arc_id);
	 *******************************/

	 // Unlock arc
	 arc->unlockArc();
	 // Return

	 // METHOD LOCK!
	 //pthread_mutex_unlock(_queue_mutex[arc_id]);

	 return qi;
};

void SMInterface::dequeueEnqueueUnpin(int arc_id, queue_info qi)
{
	 // assume that the entire queue is in memory
	 // this function truncates the file and writes new
	 // tuples from the queue ..  qi.num_tuples from qi.front
  
	 //printf("[SMInterface] dequeueEnqueueUnpin: Write %d tuple(s) to queue.%05d\n", qi.num_tuples_resident, arc_id);
	//FILE* fd = NULL;
	 //char queueFilename[FILENAME_MAX]; // The file
	 TupleDescription *tuple_descr;
	 int sizePerTuple;
   
	 // Lock the arc
	 Arcs *arc = _catalog->getArc(arc_id);  
	 //printf("DEBUG: SMInterface line 645: locking arc %d\n", arc_id );
	 arc->lockArc();

	 // Retrieve metadata needed
	 tuple_descr=arc->getTupleDescr();  
	 sizePerTuple = tuple_descr->getSize() + TUPLE_STREAMID_SIZE + TUPLE_TIMESTAMP_SIZE;
  
	 //sprintf(queueFilename,"queue.%05d", arc_id);
	 //fd = openQueueFileAndLock(queueFilename, "r+b");

	 /*if ( USE_MEMORY ) // we assume that whole queue has been loaded.
	 {
		  // thus all elements in queue have to go before we write
		  Vector::iterator iter;
		  for ( iter = m_queue[ arc_id ].begin(); 
			iter != ( m_queue[ arc_id ].end() ); iter++ ) {
		    //free( (*iter) ); // NEWSM HACK CUZ I DONT PUSH MEM THERE ANYMORE
		  }
		  m_queue[ arc_id ].clear();
		  }*/
  
	 // Write out the tuples
	 // Tuples need to be written out again.  dequeueEnqueueUnpin retuns
	 // tuples in qi datastructure. Only write tuples when queue non-empty
	 if ( qi.num_tuples_resident > 0 )
		 writeTuples(qi.front, sizePerTuple, qi.num_tuples_resident, arc_id );
  
	 // Inform our metadata stuff we have more tuples now
	 shm_set_num_records_in_queue(arc_id, qi.num_tuples_resident);
	 // Update average-timestamp info
	 // "sum up" the timestamps of all the new tuples now
	 
	 double *t_stamps;
	 if ( qi.num_tuples_resident > 0 )
		 {
			 t_stamps = ( double* )malloc( sizeof(double) * qi.num_tuples_resident );
		 }

	 double new_tuple_timestamp_total = 0, curr_timestamp = 0;

	 for (int t = 0, tp = 0; t < qi.num_tuples_resident; t++) {
		  // As alwadys, we assume timestamp here is the first field...
		  // and add the second field as microseconds.
		  curr_timestamp = (( (*(int*) (qi.front + tp)) + ((*(int*) (qi.front + tp + sizeof(int) ))/MICRO)) / _TS_SUM_FACTOR);
		  printf("SMI: intermediate stamps %f\n", curr_timestamp);
		  new_tuple_timestamp_total += curr_timestamp;
		  t_stamps[t] = curr_timestamp;  // record the time stamp.

		  tp += sizePerTuple;
	 } 
	 shm_set_sum_timestamp(arc_id, shm_get_sum_timestamp(arc_id) + new_tuple_timestamp_total);

	 if ( (qi.num_tuples_resident > 0) && (arc->getIsApp() != true) )
	 {
		  pthread_mutex_lock(&__box_work_set_mutex);
		  printf("Inserted box %i by arc id %d\n",arc->getDestId(), arc->getId());
		  __box_work_set.insert( arc->getDestId() );
		  pthread_mutex_unlock(&__box_work_set_mutex);
	 }

	 if ( _qos != NULL &&  qi.num_tuples_resident > 0 )
		 {
			 _qos->tuples_written( qi.num_tuples_resident, t_stamps, arc_id ); // report timestamps
		 }

	 if ( qi.num_tuples_resident > 0 )
		 {
			 free( t_stamps );
		 }

	 // Done!
	 //closeQueueFileAndUnlock(fd);

	 // Unlock arc
	 arc->unlockArc();

	 // METHOD LOCK!
	 //pthread_mutex_unlock(_queue_mutex[arc_id]);

	 // Done!
	 return;
}

void SMInterface::dequeueUnpin(int arc_id, int num_to_remove)
{

	 // METHOD LOCK!
	 //pthread_mutex_lock(_queue_mutex[arc_id]);

	 // Code by Eddie Galvez
	 //if ( num_to_remove != 0 )
	 //printf("[SMInterface] dequeueUnpin: Remove %d (into queue %d) tuple(s) from queue.%05d\n", num_to_remove, arc_id, arc_id);

	 /**
	  * This function should:
	  * Remove (num_to_remove) tuples from the queue
	  * Reset the "average timestamp" stat from the remaining tuples
	  *
	  * This will be achieved by reading the tuples past num_to_remove,
	  * and writing them out at the start of the file.
	  * IOTW, the file will mantain its maximum size ever reached,
	  * unless something "cleaned" up the file (truncating the file
	  * to the size of the active tuples as indicated by num_records_in_queue
	  * metadata in shared memory)
	  */

	 // FILE* fd = NULL;  NO FILE ACCESS
	//char queueFilename[FILENAME_MAX]; // The file
	 TupleDescription *tuple_descr;
	 int sizePerTuple;  //, openedFD = 0;
	 int totalTuples;
	 //char* ptr = NULL;
	 
	 //int shared_struct_updated = 0; // this is used to avoid double updating
	 // the shared structure
    
	 // Lock the arc
	 Arcs *arc = _catalog->getArc(arc_id);  
	 //printf("DEBUG: SMInterface line 746: locking arc %d\n", arc_id );
	 arc->lockArc();

	 // Retrieve metadata needed
	 tuple_descr=arc->getTupleDescr();  
	 sizePerTuple = tuple_descr->getSize() + TUPLE_STREAMID_SIZE + TUPLE_TIMESTAMP_SIZE;
	 totalTuples = shm_get_num_records_in_queue(arc_id);
      
	 // Read the remaining tuples into memory 
	 // bytesNeeded is basically the size of the remaining in queue tuples
	 int bytesNeeded = (totalTuples - num_to_remove) * sizePerTuple;
      
	 //printf(" bts? %d,...dequeueUnpin totalTuples: %i   num_to_remove: %i queue size IN MEM %d\n", bytesNeeded, totalTuples, num_to_remove, m_queue[ arc_id ].size());
	 //printf("bytesNeeded: %i\n",bytesNeeded);
      
	 int new_tuple_count = 0;
	 double new_tuple_timestamp_total = 0;
	 // If you are removing all the tuples off the queue, no file operation needed
	 //  (could optimize by checking this before you even open/lock the file)										  
	 new_tuple_count = totalTuples - num_to_remove;

	 /*****************************************
        I don't think we need any of this :)
	 if (bytesNeeded > 0 || ( USE_MEMORY && !(m_queue[ arc_id ].empty() ))) 
	 {
	
		  //printf("dequeueUnpin: Requesting %d bytes of memory\n", bytesNeeded);
		  if ( bytesNeeded > 0 )
			   ptr = (char*) SM_malloc(bytesNeeded);  
		  int sz = 0;
		  // Arc is locked here.


		  if ( USE_MEMORY && !(m_queue[ arc_id ].empty() ))
		  {
			   //printf("DEBUG: TUPS size %d, QUEUE %d, numto_rem %d, total tup %d, hm..\n", m_queue[ arc_id ].size(), arc_id, num_to_remove, totalTuples );
 			   sz = m_queue[ arc_id ].size();
			   Vector::iterator iter;
			   for ( iter = m_queue[ arc_id ].begin(); 
				 iter != ( m_queue[ arc_id ].begin()+num_to_remove ); iter++ ) {
			     //free( (*iter) ); // NEWSM HACK CUZ I DONT PUSH MEM THERE
			   }
			   m_queue[ arc_id ].erase( m_queue[ arc_id ].begin(),
										m_queue[ arc_id ].begin()+num_to_remove );
			   //printf(" YYYYY ARE YOU READING ME? arc %d, want %d, have %d, think have? %d\n", arc_id, new_tuple_count, m_queue[arc_id].size(), tuplesOnDisk[arc_id]);
			   readTuples2(ptr, sizePerTuple, new_tuple_count, arc_id );
			   //if ( m_queue[ arc_id ].size() != new_tuple_count )
			   //  {
			   //printf("SMInterface: HUSTON TUPSwe got a problem DEBUG queue %d, sz %d, total %d, to remove %d\n", arc_id, m_queue[arc_id].size(), totalTuples, num_to_remove );
			   //exit( 1 );
			   //  }

			   for ( iter = m_queue[ arc_id ].begin(); 
					 iter != ( m_queue[ arc_id ].end() ); iter++ ) {
			     //free( (*iter) ); // NEWSM HACK CUZ I DONT PUSH MEM THERE
			   }

			   m_queue[ arc_id ].clear();

			   // clear memory 
			   pthread_mutex_lock( &memory_mutex );
			   memory_in_use -= sz * sizePerTuple;
			   //printf("SMInterface: currently use %d, just lost %d from queue %d\n", memory_in_use, sz * sizePerTuple, arc_id );
			   // NEWSM SHORT CIRCUIT THIS JUNK
			   
			   if ( memory_in_use < 0 | memory_in_use > TOTAL_MEMORY) 
			   { printf("DEBUG failed dequeueUnPin! mem not proper\n");
			   exit( 1 );}
			   
			   pthread_mutex_unlock( &memory_mutex );
		  }
		  else
		  {
		    cout << " EUH DO I REALLY WANT TO BE HERE??? " << endl;
		    exit(1);

			   //printf(" DEBUG readTuples in dequeueUnpin %d, arc %d\n", new_tuple_count , arc_id );
			   // Open the storage file for reading and writing
			   if ( !openedFD )
			   {
					printf("Opening queue file (queue. line 843\n");
					sprintf(queueFilename,"queue.%05d", arc_id);
					fd = openQueueFileAndLock(queueFilename, const_cast<char*>("r+b"));
					openedFD = 1;
			   }
		
			   // Seek past the num_to_remove tuples
			   if (fseek(fd, (num_to_remove * sizePerTuple) ,SEEK_SET) != 0) {
					perror("dequeueUnpin: failed to seek");
					exit(1);
			   }

			   readTuples(ptr, sizePerTuple, new_tuple_count, fd);
		  }
		  if ( bytesNeeded > 0 )
		  {
			   // For the average-timestamp stats
			   // "sum up" the timestamps of all the tuples now
			   for (int t = 0, tp = 0; t < new_tuple_count; t++) {
					// As always, we assume timestamp here is the first field...
					new_tuple_timestamp_total += ((*(int*) (ptr + tp)) / _TS_SUM_FACTOR);      
					tp += sizePerTuple;
			   }
	
			   // Now write out the tuples to the start of the file
			   //rewind(fd);
			   if ( !openedFD )
				 {
				 sprintf(queueFilename,"queue.%05d", arc_id);
				 fd = openQueueFileAndLock(queueFilename, "r+b");
				 openedFD = 1;
				 }	
				 // Use fseek instead, rewind returns no value
				 if (fseek(fd, 0L, SEEK_SET) != 0) {
				 perror("dequeueUnpin: failed to seek (to offset 0)");
				 exit(1);
				 } 
	
			   //printf("                CALL to dequeu unpin into queue %d, new tuple count %d meaining bytes needed? %d and num to remove is %d, queue size %d\n", arc_id, new_tuple_count, bytesNeeded, num_to_remove, m_queue[ arc_id ].size() );
			   writeTuples(ptr, sizePerTuple, new_tuple_count, fd, arc_id, 0);
	
		  }
}
	 ***************************************/
	 
	 // this is going to consume tuples 
	 TupleQueue::DeqIterator deq = _queues[ arc_id ]->deq_iterator();
	 for ( int i = 0; i < num_to_remove; i++ )
		 ++deq;

	 // Inform our metadata stuff we have less tuples now
	 shm_set_num_records_in_queue(arc_id, new_tuple_count);

	 // MUST set new_tuple_timestamp_total!!!
	 double curr_timestamp;
	 new_tuple_timestamp_total = 0.0;
	 TupleQueue::SeekIterator seek = _queues[ arc_id ]->seek_iterator();
		 
	 for (int t = 0; t < new_tuple_count; t++) 
	 {
		 timeval tt;
		 memcpy(&tt, seek.tuple(), sizeof( timeval ));

		 // As always, we assume timestamp here is the first field...
		 // and add the second field as microseconds.
		 
		 curr_timestamp = ( tt.tv_sec + (tt.tv_usec*1e-06) );
			 //(( (*(int*) ((char*)seek.tuple())) + ((*(int*) ((char*) seek.tuple()) + sizeof(int) ))/MICRO)) / _TS_SUM_FACTOR;
		 //cout << " curr - Other way  " << curr_timestamp - ( tt.tv_sec + (tt.tv_usec*1e-06) ) << endl << endl << endl;

		  new_tuple_timestamp_total += curr_timestamp;
		  ++seek;
	 }

	 //fprintf(stderr,"shm_set_num_records_in_queue(%i,%i)\n",arc_id, new_tuple_count);
	 // And the _queue_monitor too
	 if (_queue_monitor != NULL)  
		 for (int n = 0; n < num_to_remove; n++) _queue_monitor->removeTuple(arc_id);

	 // Update average-timestamp tracking
	 if (new_tuple_count == 0) {
		  int in_tuples = 0;
		  Boxes *b;

		  int destId = arc->getDestId();
		  b = _catalog->getBox(destId);

		  if (b != NULL)
		  {
			  for ( int x = 0; x < b->getNumInputQueues(); x++ )
			  {
				  Arcs *a = _catalog->getArc(  (*(b->getInputQueueIds()))[x]     );
				  in_tuples = shm_get_num_records_in_queue( a->getId() );
				  if ( in_tuples > 0 )
					  break;
			  }
	
			  if ( in_tuples == 0 )
			  {
				  pthread_mutex_lock(&__box_work_set_mutex);
				  int all_input_tuples = 0;
				  for ( vector<int>::iterator it= (_catalog->getBox( arc->getDestId() ))->getInputQueueIds()->begin();
						it != (_catalog->getBox( arc->getDestId() ))->getInputQueueIds()->end(); it++ )
				  {
					  if ( (*it) != arc->getId() )
						  all_input_tuples+=shm_get_num_records_in_queue(*it );
				  }
				  if ( all_input_tuples == 0 )
					  __box_work_set.erase( arc->getDestId() );

				  pthread_mutex_unlock(&__box_work_set_mutex);
			  }
		  }

		  shm_set_sum_timestamp(arc_id, 0);
		  //shm_set_average_timestamp(arc_id, 0);
	 } else {
		 shm_set_sum_timestamp(arc_id, new_tuple_timestamp_total);
		  //shm_set_average_timestamp(arc_id, new_tuple_timestamp_total / new_tuple_count);
	 }


	 // Done with the queue
	 // if ( openedFD )
	 // closeQueueFileAndUnlock(fd);
	 // Cleanup our temporary memory
	 //cout << "           this free call really can't be a problem, right? RIGHT?!" << endl;
	 // this is a temporary allocation that is NO LONGER NECCESARY
	 //	 if (bytesNeeded > 0) SM_free(ptr);
	 // Unlock arc
	 arc->unlockArc();
	 // Done!

	 // METHOD LOCK!
	 //pthread_mutex_unlock(_queue_mutex[arc_id]);
}
  
void SMInterface::enqueueUnpin(int arc_id, char* ptr, int num_tuples)
{
	 // METHOD LOCK!
	 //pthread_mutex_lock(_queue_mutex[arc_id]);

	 // Code by Eddie Galvez
	 //printf("[SMInterface] enqueueUnpin: Write %d tuple(s) to queue.%05d\n", num_tuples, arc_id);
/**
 *  This function should:
 *   Write out tuples given to the queue (appended to the end)
 *   Update the average timestamp of all tuples in the queue (including the tuples just arrived)
 */
	//FILE* fd = NULL;
	 //char queueFilename[FILENAME_MAX]; // The file
	 TupleDescription *tuple_descr;
	 int sizePerTuple;
	 int totalTuples;
    
	 //printf("GOT T A enqueuePin arc %d\n", arc_id );

	 // Lock the arc
	 Arcs *arc = _catalog->getArc(arc_id);  
	 //printf("DEBUG: SMInterface line 921: locking arc %d\n", arc_id );
	 arc->lockArc();
	 //printf("StreamThread debug: locked arc %d\n", arc_id );

	 //_currqueue = arc_id;

	 // Retrieve metadata needed
	 tuple_descr=arc->getTupleDescr();  
	 sizePerTuple = tuple_descr->getSize() + TUPLE_STREAMID_SIZE + TUPLE_TIMESTAMP_SIZE;
	 totalTuples = shm_get_num_records_in_queue(arc_id);  // total tuples already in queue
  

	 // Open the storage file for writing (appending to end)
	 //  CAVEAT! do NOT assume simply fopen() with "append" works
	 //          since for us, the "end of the file" IS NOT THE SAME
	 //          as the end of the queue. You must seek manually to the end of the queue
	 //  CAVEAT2! If you use mode "a" (open for write and place at end) you CANNOT
	 //           seek backwards (or so it seems) -- any seek to "start + offset 0"
	 //           really doesn't move you. Perhaps because an open with "a"
	 //           really never points to the beginning of the file
	 //           --> this implies now we have to be careful if trying to
	 //               open a file just beacuse "file not found", we should create it
	 /* NO FILE ACCESS!
		sprintf(queueFilename,"queue.%05d", arc_id);
		fd = openQueueFileAndLock(queueFilename, "r+b");

		if (totalTuples > 0) { // If we must skip over existing tuples
		// Seek the end of queue (not the same as the end of the file!)
		if (fseek(fd, totalTuples * sizePerTuple, SEEK_SET) != 0) {
		perror("enqueueUnpin: failed to seek");
		exit(1);
		}
		} else { // 0 tuples? Sure, write on top of everyone, from the start
		//rewind(fsd);
		// Use fseek instead, rewind returns no value
		if (fseek(fd, 0L, SEEK_SET) != 0) {
		perror("enqueueUnpin: failed to seek (to offset 0)");
		exit(1);
		}
		}
	 */

	 // Write out the NEWLY arrived tuples
	 //printf("arc: %i   num_tuples: %i\n",arc_id,num_tuples);
	 //cout << " CALLING writeTuples(ptr="<<&ptr<<",sizePerTuple="<<sizePerTuple<<",num_tuples="<<num_tuples<<",fd="<<-1<<",arc_id="<<arc_id<<",totaltup * sizepertup="<<(totalTuples*sizePerTuple)<<")"<<endl;
	 writeTuples(ptr, sizePerTuple, num_tuples, arc_id );
	 //printf(" about to lock/write A\n");
  
	 if ( (totalTuples + num_tuples > 0) && ( arc->getIsApp() != true ) )
	 {
		 //cout << " Total tuples ? " << totalTuples << " NUM " << num_tuples << " id? " << arc_id << endl;
		  pthread_mutex_lock(&__box_work_set_mutex);
		  __box_work_set.insert( arc->getDestId() );
		  //printf("Inserted box %i by arc id %d\n",arc->getDestId(), arc->getId());
		  pthread_mutex_unlock(&__box_work_set_mutex);
	 }

	 // Inform our metadata stuff we have more tuples now
	 shm_set_num_records_in_queue(arc_id, totalTuples + num_tuples);
	 // Update average-timestamp info
	 // "sum up" the timestamps of all the new tuples now
	 //  -- WHAT IF THERE ARE NO NEW TUPLES? (num_tuples == 0)
	 //  -- I ASSUME IT SHOULD "NO-OP" BY NOT CHANGING ANY THING ABOUT avg-timestamp
	 // well, num_tuples shouldn't be a 0, but if it is, NO-OP sounds reasonable
	 if (num_tuples > 0) {
		  double new_tuple_timestamp_total = 0, curr_timestamp;
		  double* t_stamps = ( double* )malloc( sizeof(double) * num_tuples );
    
		  for (int t = 0, tp = 0; t < num_tuples; t++) {
			   // As always, we assume timestamp here is the first field...
			   curr_timestamp = (( (*(int*) (ptr + tp)) + ((*(int*) (ptr + tp + sizeof(int) ))/MICRO)) / _TS_SUM_FACTOR);
			   new_tuple_timestamp_total += curr_timestamp;//((*(int*) (ptr + tp)) / _TS_SUM_FACTOR);
			   t_stamps[t] = curr_timestamp;  // record the time stamp.
			   tp += sizePerTuple;
		  } 

		  if ( _qos != NULL )
			  {
				  _qos->tuples_written( num_tuples, t_stamps, arc_id ); // report timestamps
			  }

		  free( t_stamps );
    
		  shm_set_sum_timestamp(arc_id, shm_get_sum_timestamp( arc_id ) + new_tuple_timestamp_total);
	 }
 
	 // Done!
	 //  closeQueueFileAndUnlock(fd);

	 // Unlock arc
	 arc->unlockArc();

	 //printf("[SMInterface] FINISHED enqueueUnpin: Write %d tuple(s) to queue.%05d\n", num_tuples, arc_id);
	 // METHOD LOCK!
	 //pthread_mutex_unlock(_queue_mutex[arc_id]);
	 //printf("Sleeping 5\n");sleep(5);

	 // Done!
	 return;
};

void SMInterface::lockQueue(FILE* fd)
{
	 // Code by Eddie Galvez
	 int status = 0;
	 int fdint = fileno(fd);

	 errno = 0;

	 if (fdint == -1) {
		  perror("lockQueue: fileno() failed on file descriptor");
		  exit(1);
	 }

	 // STRANGELY, LOCKF() DID NOT WORK (kept giving EBADF)
	 //status = lockf(fdint, F_LOCK, 0L); // Blocking operation
	 status = flock(fdint, LOCK_EX);
	 if ( status  != 0) {
		  perror("Failed lock");
		  //printf("\tfd is %p and fdint is %d\n", fd, fdint);
		  exit(1);
	 }
};

void SMInterface::unlockQueue(FILE* fd)
{
	 // Code by Eddie Galvez
	 //printf("Unlocking queue file\n");
	 //lseek(fd, 0L, 0);
	 int status = 0;
	 int fdint = fileno(fd);
	 if (fdint == -1) {
		  perror("lockQueue: fileno() failed on file descriptor");
		  exit(1);
	 }
  
	 // STRANGELY, LOCKF() DID NOT WORK (kept giving EBADF)
	 //status = lockf(fdint, F_ULOCK, 0L);
	 status = flock(fdint, LOCK_UN);
	 if ( status  != 0) {
		  perror("Failed unlock");
		  //printf("\tfd is %p and fdint is %d\n", fd, fdint);
		  exit(1);
	 }
};

void SMInterface::MITRE_sendData(void* p, int sockfd, int size) 
{
	 // you need this cuz you only want to init the connection once

	 void *ptr = (int*)p + 2;
	 int out_size = size - (2*sizeof(int));
	
	 // now you can write stuff

	 if (write(sockfd, ptr, out_size) < 0)  // MUST BE adjusted for the right length of stuff to send
	 {
		  perror("[MITRE] write failed");
		  exit(1);
	 }
} 
void SMInterface::init_etime(struct itimerval *first)
{
	first->it_value.tv_sec = 1000000;
	first->it_value.tv_usec = 0;
	setitimer(ITIMER_VIRTUAL,first,NULL);
}
double SMInterface::get_etime(struct itimerval *first)
{
	struct itimerval curr;
	getitimer(ITIMER_VIRTUAL,&curr);
	return (double)(
		(first->it_value.tv_sec + (first->it_value.tv_usec*1e-6)) -
		(curr.it_value.tv_sec + (curr.it_value.tv_usec*1e-6)));


	//return (double)(
		//(first->it_value.tv_sec - curr.it_value.tv_sec) +
		//(first->it_value.tv_usec - curr.it_value.tv_usec)*1e-6);
}
