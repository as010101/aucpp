/********************************************************************************
 * NOTE: I NEVER USED THIS TEMPLATE.
 *
 *       I RELEAZED I DIDN'T NEED IT RIGHT AFTER CODING IT.
 * 
 *       IT'S NOT DEBUGGED AT ALL, AND MIGHT EVEN NOT COMPILE.
 *
 * ALSO, IF YOU LOOK IN FifoCriticalSection.C, YOU'LL SEE A MORE EFFICIENT WAY
 * TO IMPLEMENT THIS USING PtCondition OBJECTS AND JUST ONE PtMutex.
 * 
 *
 * -CJC
 *
 *******************************************************************************/

#ifndef ROOMSYNCHRONIZER_H
#define ROOMSYNCHRONIZER_H

#include <exception>
#include <vector>

#include <PtMutex.H>
#include <BinarySem.H>
#include <LockHolder.H>

using namespace std;

// TRoomEnum is a enum giving all of the possible room names.
//
// We ensure the following with this class:
// - Only one room at a time contains running threads.
// - Room entrance is granted in first-come-first-serve order.
//
// Since this synchronizer uses locking, a wayward thread in a room can starve 
// all other threads trying to enter the synchronizer.

template 
class RoomSynchronizer<typename TRoomEnum>
{
public:
	RoomSynchronizer()
		throw (exception);

	// There must not be any threads currently waiting for rooms when this is 
	// called.
	virtual ~RoomSynchronizer();

	// Blocks until the room can be entered.
	//
	// You must always call exitRoom(...) after one call of this method, before
	// making another call to this method.
	void enterRoom(TRoomEnum desiredRoom)
		throw (exception);

	void exitRoom()
		throw (exception);

private:
	// Shortly held mutex just to let this class be threadsafe.
	PtMutex _mtx;

	// Iff _threadsInCurrentRoom > 0, _currentRoom says which room those threads
	// are in.
	size_t _threadsInCurrentRoom;
	TRoomEnum _currentRoom;


	struct WaiterInfo
	{
		WaiterInfo(TRoomEnum desiredRoom, BinarySem * pEnterRoomSignal) :
			_desiredRoom(desiredRoom),
			_pEnterRoomSignal(pEnterRoomSignal)
		{
		}

		TRoomEnum   _desiredRoom;
		BinarySem * _pEnterRoomSignal;
	};


	// This is for threads that want to enter rooms, but couldn't yet because of
	// current room occupancy. _waiters[0] is the next thread to enter a room, 
	// _waiters[1] is the next, etc.
	vector<WaiterInfo> _waiters;
};

//===============================================================================

template 
RoomSynchronizer<typename TRoomEnum>::RoomSynchronizer()
		throw (exception)
{
	_threadsInCurrentRoom = 0;
}

//===============================================================================

template 
RoomSynchronizer<typename TRoomEnum>::~RoomSynchronizer()
{
	assert(_waiters.size() == 0);
}

//===============================================================================

template 
void RoomSynchronizer<typename TRoomEnum>::enterRoom(TRoomEnum desiredRoom)
		throw (exception)
{
	BinarySem * pWaitSem = NULL;

	LockHolder lh(_mtx);

	if (_threadsInCurrentRoom == 0)
		{
			_threadsInCurrentRoom = 1;
			_currentRoom = desiredRoom;
		}
	else if (_waiters.size() == 0)
		{
			// There's at least the possibility that we can enter the room...
			if (_currentRoom == desiredRoom)
				{
					++ _threadsInCurrentRoom;
				}
			else
				{
					pWaitSem = new BinarySem;
					_waiters.push_back(WaiterInfo(desiredRoom, pWaitSem));
				}
		}
		
	// Do we have to wait to enter the room?
	if (pWaitSem != NULL)
		{
			lh.release();
			pWaitSem->awaitPost();
			delete pWaitSem;
			lh.reacquire();


			TODO!!! Need to remove myself from _waiters, because I'm no longe a waiter!
            This problably means we need to use a list, not a vector, for _waiters. -cjc

			// Go ahead and enter the room...
			assert((_currentRoom == desiredRoom) || (_threadsInCurrentRoom == 0));

			++ _threadsInCurrentRoom;

			// The assignment is a no-op in case where 
			// _currentRoom == desiredRoom,
			//
			_currentRoom = desiredRoom; 
		}
}


//===============================================================================

template 
void RoomSynchronizer<typename TRoomEnum>::exitRoom()
		throw (exception)
{
	LockHolder lh(_mtx);

	-- _threadsInCurrentRoom;

	// If we're vacating a room, let one or more waiters into the house...
	if ((_threadsInCurrentRoom == 0) && (! _waiters.empty()))
		{
			_currentRoom = _waiters[0]._desiredRoom;

			size_t numWaiters = _waiters.size();
			for (size_t i = 0; 
				 (i < numWaiters) && (_waiters[i]._desiredRoom == _currentRoom);
				 ++ i)
				{
					++ _threadsInCurrentRoom;
					_waiters[i]._pEnterRoomSignal->post();
				}
		}
}

//===============================================================================

#endif
