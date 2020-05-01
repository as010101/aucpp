#include <FifoCriticalSection.H>
#include <LockHolder.H>

//===============================================================================

FifoCriticalSection::FifoCriticalSection()
	throw (exception)
{
	_held = false;
}

//===============================================================================

FifoCriticalSection::~FifoCriticalSection()
{
	assert(! _held);
	assert(_waiters.empty());
}

//===============================================================================
	
void FifoCriticalSection::lock()
	throw (exception)
{
	_mtx.lock();

	if (! _held)
		{
			_held = true;
		}
	else
		{
			PtCondition maybeNotHeldCond;

			_waiters.push_back(& maybeNotHeldCond);

			// We need a loop, because pthreads allows conditions to experience
			// spurious wakeups...
			while (_held)
				{
					_mtx.waitCond(maybeNotHeldCond);
				}

			assert(! _waiters.empty());
			assert(_waiters.front() == & maybeNotHeldCond);

			_held = true;
			_waiters.pop_front();
		}

	_mtx.unlock();
}

//===============================================================================
  
void FifoCriticalSection::unlock()
	throw (exception)
{
	LockHolder lh(_mtx);

	assert(_held);
	_held = false;

	if (! _waiters.empty())
		{
			_waiters.front()->signal();
		}
}

//===============================================================================
