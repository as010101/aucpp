#ifndef PRIORITY_GRID_H
#define PRIORITY_GRID_H
#include "global.H"
#include <stdio.h>
#include <map>
#include <list>
#include <vector>

typedef list<int>::iterator GRID_ELEMENT_BOX_ITER;
class GridElement
{
public:
	GridElement() {_current_assigned_count = 0;}
	GRID_ELEMENT_BOX_ITER push_back(int box_id);
	void erase(GRID_ELEMENT_BOX_ITER iter);
	list<int> _box_list;
	vector<int> _candidate_boxes;
	int _current_assigned_count;
};

class UrgencyStruct
{
public:
	int _box_id;
	GridElement* _ge;
};

class PriorityGrid
{
public:
	PriorityGrid(int num_buckets);
	~PriorityGrid(){};
	GridElement* getGridElement(int slack_bucket, int slope_bucket);
	void printGrid();
	void printGridPointers();
	void createPriorityVector();
	vector<GridElement*> *getPriorityVector() {return _priority_bucket_v;}
	vector<UrgencyStruct> *getUrgencyList() {return _urgency_list_v;}
	void createUrgencyList();
private:
	vector<GridElement*> *_priority_bucket_v;
	vector<UrgencyStruct> *_urgency_list_v;
	GridElement **_grid;
	int _num_buckets;
};


class BoxLatencyGridPointers
{
public:
	BoxLatencyGridPointers(){
		_current_grid_location = NULL;
		_current_list_iter = NULL;
	}

	vector<GridElement*> _grid_pointers;
	GridElement *_current_grid_location;
	GRID_ELEMENT_BOX_ITER _current_list_iter;
	int _max_grid_slot;
private:
};


class GridPointers
{
public:
	GridPointers(PriorityGrid *pg, int num_buckets, double max_slack, double max_slope, double max_latency) {
		_num_buckets = num_buckets;
		_max_slack = max_slack;
		_max_slope = max_slope;
		_max_latency = max_latency;
		_priority_grid = pg;
	}
	void setPriorityGrid(PriorityGrid *pg) { _priority_grid = pg; }
	void loadGridPointers(int box_id, qos_struct *qos_graph);
	int  calcSlackBucket(double latency_start, double latency_stop, qos_struct *qos_graph);
	int  calcSlopeBucket(double latency_start, double latency_stop, qos_struct *qos_graph);
	GridElement* loadPriorityAssignment(int box_id, double latency);
	void printBoxGridPointers(int box_id);
	void removeFromGrid(int box_id);
private:
	PriorityGrid* _priority_grid;
	map<int, BoxLatencyGridPointers> _box_latency_grid_map;
	int _num_buckets;
	double _max_slack;
	double _max_slope;
	double _max_latency;
};
#endif
