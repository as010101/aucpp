#ifndef APPTUPLEMONITOR_H
#define APPTUPLEMONITOR_H

#include <Monitor.H>
#include <map>

using namespace std;

// The map<int, int> maps arcId's to the number of tuples currently in that
// arc (aka queue).
// Only arcs that are directly connected to Applications will appear in this
// map.
//
// '_shutdown' is true whenn the user has expressed a desire for the Aurora
// library to wrap things up.
//
// '_schedWantsShutdown' is true whenn the scheduler has expressed a desire for
//  the Aurora runtime and its application library to wrap things up.
struct AppTupleInfo
{
	AppTupleInfo()
	{
		_shutdown = false;
		_schedWantsShutdown = false;
	}
	
	map<int, int> _appArcCounts;
	bool _shutdown;
	bool _schedWantsShutdown;
};

typedef Monitor<AppTupleInfo> AppTupleMonitor;

#endif
