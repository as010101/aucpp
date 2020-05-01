#ifndef STREAMTHREADTHROTTLE_H
#define STREAMTHREADTHROTTLE_H

#include <Monitor.H>

typedef enum 
  {
    THROTTLE_STOPPED,
    THROTTLE_RUNNING,
    THROTTLE_DONE
  } StreamThreadThrottleState;

typedef Monitor<StreamThreadThrottleState> StreamThreadThrottle;

#endif
