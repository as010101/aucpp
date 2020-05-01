#include <FairMutexWithCancel.H>
#include <LockHolder.H>

//===============================================================================

FairMutexWithCancel::FairMutexWithCancel()
{
	_cancelled = false;
}

//===============================================================================

FairMutexWithCancel::~FairMutexWithCancel()
{
}

//===============================================================================

bool FairMutexWithCancel::lock()
	throw (exception)
{
	_mtx.lock();

	if (_cancelled)
		{
			_mtx.unlock();
			return false;
		}

	// If mutex is cancelled before we leave this method, then pAlarm is deleted
	// in this method. Otherwise, pAlarm remains on _alarmClocks until this 
	// thread calls the unlock() method at which point pAlarm will be deleted.
	PtCondition * pAlarm = new PtCondition();

	_alarmClocks.push_back(pAlarm);
			
	// pthreads allows spurious wakeups of conditions, so we need to
	// loop...
	while ((_alarmClocks.front() != pAlarm) && (! _cancelled))
		{
			_mtx.waitCond(* pAlarm);
		}

	bool wasCancelled = _cancelled;
	if (wasCancelled)
		{
			// Don't bother removing pAlarm from _alarmClocks. Since cancellation
			// has begun, no one is going to look at the content of _alarmClocks
			// ever again.
			delete pAlarm;
		}
	else
		{
			_currentHolder = pthread_self();
		}

	_mtx.unlock();

	return (! wasCancelled);
}

//===============================================================================

void FairMutexWithCancel::unlock()
	throw (exception)
{
	LockHolder lh(_mtx);
	assert(_currentHolder == pthread_self());

	delete _alarmClocks.front();
	_alarmClocks.pop_front();

	if (! _alarmClocks.empty())
		{
			_alarmClocks.front()->signal();
		}
}

//===============================================================================
	
void FairMutexWithCancel::cancel()
	throw (exception)
{
	LockHolder lh(_mtx);

	_cancelled = true;

	for (list<PtCondition *>::iterator pos = _alarmClocks.begin();
		 pos != _alarmClocks.end(); 
		 ++ pos)
		{
			_alarmClocks.front()->signal();			
		}
}

//===============================================================================
