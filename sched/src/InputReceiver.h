#ifndef _INPUTRECEIVER_H_
#define _INPUTRECEIVER_H_

#include <time.h>                    // for timeval
#include <string.h>                  // for memcpy
#include <vector.h>                  // for vector
#include <stdlib.h>                  // for drand48
#include <iostream.h>                // for {cout,cerr}
#include <iomanip.h>                 // for {setw}
#include <pthread.h>                 // for pthread
#include <unistd.h>                  // for sleep

#include "SMInterface.H"             // for SMInterface class

/////////////////////////////////////////////////////////////////
// Class SourceStreamTuple
//

class Timeval:public timeval 
{
public:
   Timeval()
   {
      tv_sec = 0;
      tv_usec = 0;
   };
   Timeval(long sec, long usec)
   {
      tv_sec = sec;
      tv_usec = usec;
   }
   Timeval(const timeval &t) 
   {
      tv_sec = t.tv_sec;
      tv_usec = t.tv_usec;
   }
   ~Timeval(){};

   void operator=(const Timeval &t) 
   {
      tv_sec = t.tv_sec;
      tv_usec = t.tv_usec;
   }

   void operator+=(const timeval &t)
   {
      tv_sec += t.tv_sec;
      tv_usec += t.tv_usec;
      if (tv_usec >= 1000000) {
         tv_sec += 1;
         tv_usec -= 1000000;
      }
   }

   void operator-=(const timeval &t)
   {
      tv_sec -= t.tv_sec;
      tv_usec -= t.tv_usec;
      if (tv_usec < 0) {
         tv_sec -= 1;
         tv_usec += 1000000;
      }
   }

   bool operator==(const timeval &t)
   {
      return ((tv_sec == t.tv_sec) &&
              (tv_usec == t.tv_usec));
   }

   bool operator<(const timeval &t) 
   {
      return ((tv_sec < t.tv_sec) ||
             ((tv_sec == t.tv_sec) &&
             (tv_usec < t.tv_usec)));
   }
};

/////////////////////////////////////////////////////////////////
// Class TimeStampeGenerator
//

//base class for use in StreamSrouce
class TimeStampGenerator
{
public:
   TimeStampGenerator()
   {
      gettimeofday(&_timestamp,0);
   }
   TimeStampGenerator(const timeval &t): 
      _timestamp(t),
      _next_timestamp(t)
   {};
   ~TimeStampGenerator(){};

   virtual void   getNextTimeStamp(Timeval *timestamp) = 0;
   void           getTimestamp(Timeval *timestamp) 
      {*timestamp = _timestamp;} 
   void           setTimeStamp(const timeval &t) { _next_timestamp = t; }

protected:
   Timeval   _timestamp;
   Timeval   _next_timestamp;
};

class PeriodicalTimestampGenerator: public TimeStampGenerator
{
public:
   PeriodicalTimestampGenerator(): _period(0,0) {};
   PeriodicalTimestampGenerator(long sec, long usec): _period(sec,usec) {};
   PeriodicalTimestampGenerator(
         long sec, 
         long usec, 
         const Timeval &init_t): 
      _period(sec,usec)
   {
      _timestamp = init_t;
      _next_timestamp = init_t;
   };
   PeriodicalTimestampGenerator(const Timeval &p): _period(p) {};
   PeriodicalTimestampGenerator(const Timeval &p, const Timeval init_t): 
      _period(p)
   {
      _timestamp = init_t;
      _next_timestamp = init_t;
   };
   ~PeriodicalTimestampGenerator(){};

   void getNextTimeStamp(Timeval *timestamp)
   {
      (*timestamp) = _next_timestamp;
      _timestamp = _next_timestamp;
      _next_timestamp += _period;
   }

private:
   Timeval  _period;
};

/////////////////////////////////////////////////////////////////
// Class SourceStreamTuple
//
typedef long source_stream_id_t;

class SourceStreamTuple
{
public:
   SourceStreamTuple(source_stream_id_t id):
      _source_stream_id(id),
      _data(0),
      _data_len(0)
   {}
   ~SourceStreamTuple() { delete _data; };

   source_stream_id_t getSourceStreamID()       { return _source_stream_id; }

   void  getTimestamp(Timeval *timestamp)       { *timestamp = _timestamp; }
   void  setTimestamp(const timeval &timestamp) { _timestamp = timestamp; } 
   int   getDataSize()                          { return _data_len; }
   char  *getDataHandle()                       { return _data; }

   int   getData(char *data, int len)
   {
            if (len < _data_len) {
               return 0;
            }
            memcpy(data, _data, _data_len);
            return _data_len;
   }
   char  *newData(int len)
   {
            _data = new char[len];
            _data_len = len;
            return _data;
   }

private:
   source_stream_id_t   _source_stream_id;
   Timeval              _timestamp;
   char                 *_data;
   int                  _data_len;
};

////////////////////////////////////////////////////////////////
// Class SingleDataSource 
//

class SingleDataSource
{
public:
   SingleDataSource():_data_size(0){};
   ~SingleDataSource(){};
   int          getDataSize() { return _data_size; }
   virtual int  getNextData(char* data) = 0;
protected:
   int          _data_size;
};

////////////////////////////////////////////////////////////////////

template <class T> class TimestampDataSource : public SingleDataSource
{
public:
   TimestampDataSource(TimeStampGenerator *timestamp_generater): 
      _timestamp_generator(timestamp_generater)
   {
      _data_size = sizeof(T);
   };
   ~TimestampDataSource(){};

   int      getNextData(char* data);
private:
   TimeStampGenerator *_timestamp_generator;
};

template <class T> int TimestampDataSource<T>::getNextData(char* data)
{
   Timeval t;
   _timestamp_generator->getTimestamp(&t);

   T timestamp = static_cast<T>(t.tv_sec);
   memcpy(data, reinterpret_cast<char*>(&timestamp), _data_size);
   return _data_size; 
}


////////////////////////////////////////////////////////////////////

template <class T> class RandomWalkDataSource : public SingleDataSource
{
public:
   RandomWalkDataSource( T      init_value, 
                         T      increment,
                         double p = 0.5,
                         double bound = 1000000):
      _last_value(init_value),
      _increment(increment),
      _p(p),
      _bound(bound)
   {
      _data_size = sizeof(T);
   };
   ~RandomWalkDataSource(){};

   int      getNextData(char* data);
private:
   T        _last_value;
   T        _increment;
   double   _p;
   double   _bound;
};

template <class T> int RandomWalkDataSource<T>::getNextData(char* data)
{
   memcpy(data, reinterpret_cast<char*>(&_last_value), _data_size);
   if ( drand48() < _p )
      _last_value += _increment;
   else
      _last_value -= _increment;

   if (_last_value > _bound)
      _last_value = _bound;
   if (_last_value < -_bound)
      _last_value = -_bound;

   return _data_size; 
}
////////////////////////////////////////////////////////////////////

template <class T> class ConstantDataSource : public SingleDataSource
{
public:
   ConstantDataSource(T  value): _value(value)
   {
      _data_size = sizeof(T);
   };
   ~ConstantDataSource(){};

   int      getNextData(char* data);
private:
   T        _value;
};

template <class T> int ConstantDataSource<T>::getNextData(char* data)
{
   memcpy(data, reinterpret_cast<char*>(&_value), _data_size);
   return _data_size; 
}


////////////////////////////////////////////////////////////////////

template <class T> class LinearDataSource : public SingleDataSource
{
public:
   LinearDataSource( T      init_value, 
                         T      increment):
      _last_value(init_value),
      _increment(increment)
   {
      _data_size = sizeof(T);
   };
   ~LinearDataSource(){};

   int      getNextData(char* data);
private:
   T        _last_value;
   T        _increment;
};

template <class T> int LinearDataSource<T>::getNextData(char* data)
{
   memcpy(data, reinterpret_cast<char*>(&_last_value), _data_size);
   _last_value += _increment;

   if (_last_value > 1000000)
      _last_value = 1000000;
   if (_last_value < -1000000)
      _last_value = -1000000;

   return _data_size; 
}

/////////////////////////////////////////////////////////////////
// Class StreamSource
//
class StreamSource
{
public:
   StreamSource(
         source_stream_id_t id, 
         TimeStampGenerator *timstamp_generator):
      _stream_id(id),
      _timestamp_generator(timstamp_generator),
      _tuple_data_size(0)
   {};
   ~StreamSource(){};

   SourceStreamTuple  *getNextTuple();
   int                getTupleDataSize() { return _tuple_data_size;}

   void addDataSources(SingleDataSource *single_data_source)
   {
      _single_data_sources.push_back(single_data_source);
      _tuple_data_size += single_data_source->getDataSize();
   }
      
protected:
   source_stream_id_t          _stream_id;
   TimeStampGenerator          *_timestamp_generator;
   vector<SingleDataSource*>   _single_data_sources;
   int                         _tuple_data_size;
};

SourceStreamTuple *StreamSource::getNextTuple()
{
   //create a new tuple
   SourceStreamTuple *next_tuple = new SourceStreamTuple(_stream_id);

   //allocate memery
   char *data = next_tuple->newData(_tuple_data_size); 

   //set timestamp
   Timeval timestamp;
   _timestamp_generator->getNextTimeStamp(&timestamp);
   next_tuple->setTimestamp(timestamp);

   //set data
   for (vector<SingleDataSource*>::iterator it = _single_data_sources.begin(); 
        it != _single_data_sources.end();
        ++it)
   {
      int len = (*it)->getNextData(data);
      data += len;
   }
// cerr << endl;
   return next_tuple;
}

/////////////////////////////////////////////////////////////////
// Class StreamBuffer
//
// FIFO buffer
class StreamBuffer
{
public:
protected:
 
};

/////////////////////////////////////////////////////////////////
//  Just for demo
//

/////////////////////////////////////////////////////////////////
// Class StreamListenerThread
//
class ConnectionThread 
{
public:
   ConnectionThread(StreamSource *stream_source)
   {
      pthread_create(&_thread,0,(void*(*)(void*))ConnectionHandler,
                     (void*)stream_source);
   }

   pthread_t _thread;
private:
   static void ConnectionHandler(StreamSource*);
};

void ConnectionThread::ConnectionHandler(StreamSource* stream_source)
{
   while(1){
      SourceStreamTuple tuple = *(stream_source->getNextTuple());
      Timeval t;
      tuple.getTimestamp(&t);

      Timeval now;
      gettimeofday(&now,0);

      cerr << setw(15) << "timestamp:  " << setw(15) << t.tv_sec
           << setw(15) << t.tv_usec << endl;
      cerr << setw(15) << "time now:  " << setw(15) << now.tv_sec
           << setw(15) << t.tv_usec << endl;

      now -= t; 
      if (Timeval(0,0) < t) {
         timespec interval;
         timespec rtpt;
         interval.tv_sec = now.tv_sec;
         interval.tv_nsec = now.tv_usec * 1000;
         //nanosleep(&interval, &rtpt);

         cerr << "sleeping ... " << endl;
      }

      gettimeofday(&now,0);
      cerr << setw(15) << "time now:  " << setw(15) << now.tv_sec
           << setw(15) << t.tv_usec << endl;
      cerr << endl; 

      // give data to Storage manager
      char* data = tuple.getDataHandle(); 
      //
   }
}


/////////////////////////////////////////////////////////////////
// Class StreamListenerThread
//
class InputRecieverThread
{
public:
   InputRecieverThread(SMInterface* SM, int par): _SM(SM), _par(par) {};
   ~InputRecieverThread(){};

   void          Start();
   int           getPar() { return _par; }
   SMInterface*  getSM() { return _SM; }
   pthread_t*    getThread() { return  &_thread; }

private:
   static void InputRecieverHandler(InputRecieverThread*);

   pthread_t     _thread;
   int           _par;
   SMInterface   *_SM;
};

void InputRecieverThread::Start() 
{
   pthread_create(&_thread,0,(void*(*)(void*))InputRecieverHandler,
                     (void*)this);
}

const int NUM_TUPLES = 20;
const unsigned int RATE = 1;
void InputRecieverThread::InputRecieverHandler(InputRecieverThread* me)
{
   switch ( me->getPar() )
   {
      case 1:
      {

         const int NUM_SOLDERS = 4;
         PeriodicalTimestampGenerator* tm_gen[NUM_SOLDERS];

         TimestampDataSource<int>*     ts[NUM_SOLDERS];
         ConstantDataSource<int>*      sid[NUM_SOLDERS];     
         LinearDataSource<int>*        pos[NUM_SOLDERS];
         RandomWalkDataSource<float>*  hr[NUM_SOLDERS];     

         StreamSource*                 sr[NUM_SOLDERS];

         for (int i = 0; i < NUM_SOLDERS; ++i)
         {
            tm_gen[i] = new PeriodicalTimestampGenerator(RATE, 0, Timeval(0,0));

            ts[i]  = new TimestampDataSource<int>(tm_gen[i]);
            sid[i] = new ConstantDataSource<int>(i+1);
            pos[i] = new LinearDataSource<int>(10*i + 1, i);
            hr[i]  = new RandomWalkDataSource<float>(72.1 + 0.1 * i , i * 4);

            sr[i]  = new StreamSource(0, tm_gen[i]);      

            sr[i]->addDataSources(ts[i]);
            sr[i]->addDataSources(sid[i]);
            sr[i]->addDataSources(pos[i]);
            sr[i]->addDataSources(hr[i]);
         }
/*
         cout << setw(15) << "Stream ID"
              << setw(15) << "Timestamp"
              << setw(15) << "sid" 
              << setw(15) << "pos" 
              << setw(15) << "hr" 
              << endl;
*/

         for (int j = 0; j < NUM_TUPLES/NUM_SOLDERS; ++j)
         {
            for (int i = 0; i < NUM_SOLDERS; ++i)
            {
               int  data_len = sr[i]->getTupleDataSize();
               char *data = new char[data_len];
               SourceStreamTuple *tuple = sr[i]->getNextTuple();
               tuple->getData(data, data_len);

                  char*  buffer = me->getSM()->enqueuePin(
                        tuple->getSourceStreamID(),
                        1);
                  memcpy(buffer, data, data_len);
                  me->getSM()->enqueueUnpin(
                        tuple->getSourceStreamID(),
                        buffer, 
                        1);
/*
               cout << setw(15) << tuple->getSourceStreamID();
               cout << setw(15) << *reinterpret_cast<int*>(data);
               data += sizeof(int);
               cout << setw(15) << *reinterpret_cast<int*>(data);
               data += sizeof(int);
               cout << setw(15) << *reinterpret_cast<int*>(data);
               data += sizeof(int);
               cout << setw(15) << *reinterpret_cast<float*>(data);
               cout << endl;
*/
            }
            sleep(RATE);
         }

         break;
      }
      case 2:
      {
         const int NUM_STREAMS = 2;
         const int NUM_SOLDERS = 2;

   
         PeriodicalTimestampGenerator* tm_gen[NUM_STREAMS][NUM_SOLDERS];

         TimestampDataSource<int>*     ts[NUM_STREAMS][NUM_SOLDERS];
         ConstantDataSource<int>*      sid[NUM_STREAMS][NUM_SOLDERS];     
         LinearDataSource<int>*        pos[NUM_STREAMS][NUM_SOLDERS];
         RandomWalkDataSource<float>*  hr[NUM_STREAMS][NUM_SOLDERS];     

         StreamSource*                 sr[NUM_STREAMS][NUM_SOLDERS];
         for( int k = 0; k < NUM_STREAMS; ++k){
            for (int i = 0; i < NUM_SOLDERS; ++i)
            {
               tm_gen[k][i] = 
                  new PeriodicalTimestampGenerator(RATE, 0, Timeval(0,0));

               ts[k][i]  = new TimestampDataSource<int>(tm_gen[k][i]);
               sid[k][i] = new ConstantDataSource<int>(k * 10 + i+1);
               pos[k][i] = new LinearDataSource<int>
                                     (10 - i * 2 - 8 * k, k + 1);
               hr[k][i]  = new RandomWalkDataSource<float>
                                     (72.1 + 0.1 * i + 0.2 * k, 0.3);

               sr[k][i]  = new StreamSource(k, tm_gen[k][i]);      

               sr[k][i]->addDataSources(ts[k][i]);
               sr[k][i]->addDataSources(sid[k][i]);
               sr[k][i]->addDataSources(pos[k][i]);
               sr[k][i]->addDataSources(hr[k][i]);
            }
         }
/*
         cout << setw(15) << "Stream ID"
              << setw(15) << "Timestamp"
              << setw(15) << "sid"
              << setw(15) << "pos" 
              << setw(15) << "hr" 
              << endl;
*/

         for (int j = 0; j < NUM_TUPLES/NUM_SOLDERS; ++j)
         {
            for( int k = 0; k < NUM_STREAMS; ++k)
            {
               for (int i = 0; i < NUM_SOLDERS; ++i)
               {
                  int  data_len = sr[k][i]->getTupleDataSize();
                  char *data = new char[data_len];
                  SourceStreamTuple *tuple = sr[k][i]->getNextTuple();
                  tuple->getData(data, data_len);

                  char*  buffer = me->getSM()->enqueuePin(
                        tuple->getSourceStreamID(),
                        1);
                  memcpy(buffer, data, data_len);
                  me->getSM()->enqueueUnpin(
                        tuple->getSourceStreamID(),
                        buffer, 
                        1);
/*
                  cout << setw(15) << tuple->getSourceStreamID();
                  cout << setw(15) << *reinterpret_cast<int*>(data);
                  data += sizeof(int);
                  cout << setw(15) << *reinterpret_cast<int*>(data);
                  data += sizeof(int);
                  cout << setw(15) << *reinterpret_cast<int*>(data);
                  data += sizeof(int);
                  cout << setw(15) << *reinterpret_cast<float*>(data);
                  cout << endl;
*/
               }
               sleep(RATE);
            } 
         }
         break;
      }
      default:
         break;
   }
}


#endif
