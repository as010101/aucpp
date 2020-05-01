#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <sys/time.h>
#include <sstream>

using namespace std;

class Timestamp : public timeval 
{
public:
  Timestamp()
  {
    tv_sec = 0;
    tv_usec = 0;
  };
  
  Timestamp(long sec, long usec)
   {
     tv_sec = sec;
      tv_usec = usec;
   }

   Timestamp(const timeval &t) 
   {
      tv_sec = t.tv_sec;
      tv_usec = t.tv_usec;
   }
  
  ~Timestamp(){};
  
  void operator=(const Timestamp &t) 
  {
    tv_sec = t.tv_sec;
    tv_usec = t.tv_usec;
  }
  
   void operator+=(const Timestamp &t)
   {
      tv_sec += t.tv_sec;
      tv_usec += t.tv_usec;
      if (tv_usec >= 1000000) {
         tv_sec += 1;
         tv_usec -= 1000000;
      }
   }

  Timestamp operator-(const Timestamp &t)
   {
     timeval temp;
     temp.tv_sec = tv_sec - t.tv_sec;
     temp.tv_usec = tv_usec - t.tv_usec;
     if (temp.tv_sec < 0)
       {
	 temp.tv_sec -= 1;
	 temp.tv_usec += 1000000;
       }
   }

   void operator-=(const Timestamp &t)
   {
      tv_sec -= t.tv_sec;
      tv_usec -= t.tv_usec;
      if (tv_usec < 0) {
	tv_sec -= 1;
	tv_usec += 1000000;
      }
   }
  
  bool operator==(const Timestamp &t)
  {
    return ((tv_sec == t.tv_sec) &&
	    (tv_usec == t.tv_usec));
  }
  
  bool operator<(const Timestamp &t) 
  {
    return ((tv_sec < t.tv_sec) ||
	    ((tv_sec == t.tv_sec) &&
             (tv_usec < t.tv_usec)));
  }

  bool operator>(const Timestamp &t) 
  {
    return ((tv_sec > t.tv_sec) ||
	    ((tv_sec == t.tv_sec) &&
             (tv_usec > t.tv_usec)));
  }

  char* toString() { //(const Timestamp &t) {
    ostringstream os;
    os << "(" << tv_sec << ", " << tv_usec << ")";

    string s = os.str();
    size_t numOutputBytes = s.length() + 1;
    
    char * pszTemp = new char[numOutputBytes];
    strncpy(pszTemp, s.c_str(), numOutputBytes);

    return pszTemp;
  }

};


#endif
