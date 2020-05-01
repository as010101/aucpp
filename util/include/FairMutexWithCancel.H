#ifndef FAIRMUTEXWITHCANCEL_H
#define FAIRMUTEXWITHCANCEL_H

#include <PtMutex.H>
#include <PtCondition.H>
#include <list>
#include <exception>

using namespace std;

/*** NOTE: THIS CODE BUILDS, BUT HAS NOT BEEN UNIT TESTED!!! -CJC ***/

/*
  Like a regular mutex, but it can also be cancelled. Cancellation is a
  permanent state.

  A thread waiting on this mutex can be awakened because it was granted
  the mutex, or because the mutex was cancelled.

  Waiters are granted the mutex first-come-first-serve.

  This is actually a lot like a cancellable version of FifoCriticalSection,
  but it doesn't implement ILockable (since lock() can fail under normal
  circumstances).
 */

class FairMutexWithCancel
{
public:
	FairMutexWithCancel();
	virtual ~FairMutexWithCancel();

	// Acquires the mutex. Returns true if the mutex was granted, false if
	// the mutex has been cancelled.
	bool lock()
		throw (exception);

	// A thread holding the lock must call this exactly once to release it, 
	// if/when it's OK for other threads to acquire the lock.
	void unlock()
		throw (exception);
	
	// Forever marks the mutex as cancelled, and causes all pending and future
	// invocations of lock() to return 'false'.
	void cancel()
		throw (exception);

private:
	PtMutex _mtx;
	bool _cancelled;

	// Owner of the condition at the front of the list is the thread that 
	// currently holds this lock.
	list<PtCondition*> _alarmClocks;

	// Iff (! _alarmClocks.empty()), then either:
	// (A) this is the thread that's currently holding the lock, or
	// (B) this is the thread that previously held the lock, and the thread
	//     that is to next hold the lock is now at the front of _alarmClocks.
	//  We just use it for double-checking that the thread calling the "unlock()"
	//  method is the thread holding the lock.
	pthread_t _currentHolder;
};

#endif
