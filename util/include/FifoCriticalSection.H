#ifndef FIFOCRITICALSECTION_H
#define FIFOCRITICALSECTION_H

#include <ILockable.H>
#include <PtMutex.H>
#include <PtCondition.H>
#include <exception>
#include <list>

using namespace std;

/*
  Guarantees first-come, first-serve access to a critical section.
 */
class FifoCriticalSection : public ILockable
{
public:
	FifoCriticalSection()
		throw (exception);

	// Behavior is undefined If you delete this object while at least one thread
	//  is awaiting or holding the lock.
	virtual ~FifoCriticalSection();
	
	// If an exception is thrown, the state of this object can no longer be
	// trusted.
	virtual void lock()
		throw (exception);
  
	// If an exception is thrown, the state of this object can no longer be
	// trusted.
	virtual void unlock()
		throw (exception);

private:
	PtMutex _mtx;
	list<PtCondition *> _waiters;
	bool _held;
};

#endif
