#ifndef CATALOG_H
#define CATALOG_H

#include "TupleDescription.H"
#include <Predicate.H>

#include "Box.H"
#include "BoxRecord.H"
#include <QueryNetwork.H>
#include "AggregateState.H"
#include <logutil.H>

#include <Predicate.H>
#include <pthread.h>
#include <vector>
#include <assert.h>


#include <iostream>
#include <errno.h>
#include <string>
#include <unistd.h>

using namespace std;

// Can someone explain this? Is there some system in numbering these guys?
// Do these have to match anything? hope not! - eddie
// See other box defines used in catalogmgr/BoxRecord.H but the numbers dont match
//  we should consolidate
//#define SELECT_BOX 		0  bad name, its filter!
#define FILTER_BOX 0
#define MAP_BOX 		1
#define WMAP_BOX 		2
#define UNION_BOX 		3
#define RESAMPLE_BOX	4
#define JOIN_BOX 		5
#define DROP_BOX 		6
#define RESTREAM_BOX	7
//#define TUMBLE_BOX		8 replaced by AGGREGATE_BOX
#define AGGREGATE_BOX           8
//#define SLIDE_BOX		9
//#define XSECTION_BOX	10
// Bye bye WSORT! - eddie
//#define WSORT_BOX		11
#define INPUTPORT_BOX	13
#define EXPERIMENT_BOX	99	
// New operator work (starting at 20, cuz why not)
#define BSORT_BOX               11
// Linear Road operators, tibbetts@mit.edu Starting at 101.
#define HELLO_WORLD_BOX		101
#define UPDATE_RELATION_BOX	102
#define READ_RELATION_BOX	103
#define LR_UPDATE_RELATION_BOX	104
#define LR_READ_RELATION_BOX	105

class Network
{
public:
	Network() {};
	Network(int num_boxes, int num_inputs, int num_outputs) 
	{
		_num_boxes = num_boxes;
		_num_inputs = num_inputs;
		_num_outputs = num_outputs;
	}
	~Network() {}
	int 	getNumBoxes() 			{return _num_boxes;}
	int 	getNumInputs() 			{return _num_inputs;}
	int 	getNumOutputs() 		{return _num_outputs;}
	void 	setNumBoxes(int val) 	{_num_boxes = val;}
	void 	setNumInputs(int val) 	{_num_inputs = val;}
	void 	setNumOutputs(int val) 	{_num_outputs = val;}

private:
	int		_num_boxes;	
	int		_num_inputs;
	int		_num_outputs;
};

class Boxes
{
public:
  Box *_db_box; // YES, PUBLIC
	Boxes(int id, int type, double selectivity, double cost, int win_size, Predicate *pred, vector<Expression*> *expr, AggregateState* agg_state, const char *modifier,
			float drop_rate, Box* db_box)
	{
		_id = id;	
		_type = type;
		_selectivity = selectivity;
		_cost = cost;
		_window_size = win_size;
		_predicate = pred;
		_expression = expr;
		_agg_state = agg_state;
		_mutex = new pthread_mutex_t;	
		pthread_mutex_init(_mutex,0);
		_modifier = modifier;
		_drop_rate = drop_rate;
		_num_input_queues = 0;
		_db_box = db_box;
	}
	void 	setId(int id) { _id = id; }
	void 	setType(int type) { _type = type; }
	void 	setSelectivity(double selectivity) { _selectivity = selectivity; }
	void 	setWinSize(int winsize) { _window_size = winsize; }
	void 	setPredicate(Predicate *pred) { _predicate = pred; }
	void 	setCost(double cost) { _cost = cost; }

	int 	getId() { return(_id); }
	int 	getType() { return(_type); }
	double 	getSelectivity() { return(_selectivity); }
	double 	getCost() { return(_cost); }
	int 	getWinSize() { return(_window_size); }
	const char      *getModifier() { return(_modifier); }
	Predicate		*getPredicate() { return(_predicate); }
	vector<Expression*>      *getExpression()  { return(_expression); }
	AggregateState  *getState()  { return (_agg_state); }
	float	getDropRate() { return(_drop_rate); }
	void	setNumInputQueues(int n) { _num_input_queues = n; }
	int		getNumInputQueues() { return(_num_input_queues); }
	void	addInputQueueId(int arc_id)	{ _input_queue_ids.push_back(arc_id); }
	vector<int>*	getInputQueueIds()	{ return &_input_queue_ids; }


	// These perhaps warrant accessor methods, but we can't be bothered
	// right now...

	// Total number of tuples that have been dequeued by this box since the 
	// beginning of this program run, regardless of which box input port they 
	// were dequeued from.
	long _total_dequeued_tuples;

	// Total number of tuples enqueued by this box onto each box output port
	// since the beginning of this program run.
	vector<long> _total_enqueued_tuples_byport;



  void lockBox()
  { 

    int rc = pthread_mutex_lock(_mutex); 
    
/*
    string s;
    
    switch (rc)
      {
      case 0:
	s = "OK";
	break;
      case EINVAL:
	s = "EINVAL";
	break;
      case EDEADLK:
	s = "EDEADLK";
	break;
      case EBUSY:
	s = "EBUSY";
	break;
      case EPERM:
	s = "EPERM";
	break;
      default:
	s = "(bogus rc)";
      }

	if (rc != 0)
		{
			cerr << "Boxes::lockBox() - EXITING. pthread_mutex_trylock rc=" << s << endl;
			abort();
		}
*/
  }


  int lockBoxTry()
  { 

    int rc = pthread_mutex_trylock(_mutex); 
/*
    string s;
    switch (rc)
      {
      case 0:
	s = "OK";
	break;
      case EINVAL:
	s = "EINVAL";
	break;
      case EDEADLK:
	s = "EDEADLK";
	break;
      case EBUSY:
	s = "EBUSY";
	break;
      case EPERM:
	s = "EPERM";
	break;
      default:
	s = "(bogus rc)";
      }

	if (rc != 0)
		{
			cerr << "Boxes::lockBoxTry() - pthread_mutex_trylock rc=" << s << endl;
		}

    _pLogger->log("locking", makeDebugLogLines("Boxes::lockBoxTry() - EXITING", 
					       "pthread_mutex_trylock rc", s));
*/
    return rc;
  }

  void unlockBox()
  { 

      int rc = pthread_mutex_unlock(_mutex); 

/*
    string s;
    switch (rc)
      {
      case 0:
	s = "OK";
	break;
      case EINVAL:
	s = "EINVAL";
	break;
      case EDEADLK:
	s = "EDEADLK";
	break;
      case EBUSY:
	s = "EBUSY";
	break;
      case EPERM:
	s = "EPERM";
	break;
      default:
	s = "(bogus rc)";
      }

	if (rc != 0)
		{
			cerr << "Boxes::unlockBox() - EXITING. pthread_mutex_trylock rc=" << s << endl;
			abort();
		}
*/
  }

private:
	int			_id;
	int			_type;
	double		_selectivity;
	double		_cost;
	int			_window_size;
	Predicate 	*_predicate;
	float		_drop_rate;
	vector<Expression*>      *_expression;
	pthread_mutex_t		 *_mutex; 	
        const char               *_modifier;
        AggregateState           *_agg_state;
	int			_num_input_queues;
	vector<int> _input_queue_ids;
};

class Arcs
{
public:
	Arcs() {}
	Arcs(int id, int src_id, int src_port_id, int dest_id, int dest_port_id,
		 TupleDescription *td) 
	{
		_id = id;
		_src_id = src_id;
		_src_port_id = src_port_id;
		_dest_id = dest_id;
		_dest_port_id = dest_port_id;
		_tuple_descr = td;
		_mutex = new pthread_mutex_t;	
		pthread_mutex_init(_mutex,0);
		_sock_fd = -1;
		_is_app = false;
		_is_input = false;
		_app_num = -1;
	}
	void 	setId(int id) { _id = id; }
	void 	setSrcId(int id) { _src_id = id; }
	void 	setDestId(int id) { _dest_id = id; }
	void 	setTupleDescr(TupleDescription *td) { _tuple_descr = td; }
	void	setSockFD(int s) { _sock_fd = s; }
	void	setIsApp(bool is_app, int app_num) {_is_app = is_app; _app_num = app_num;}
	void	setIsInput(bool is_input) {_is_input = is_input;}
	int 	getId() { return(_id); }
	int 	getSrcId() { return(_src_id); }
	int 	getSrcPortId() { return(_src_port_id); }
	int 	getDestId() { return(_dest_id); }
	int 	getDestPortId() { return(_dest_port_id); }
	int		getSockFD() {return (_sock_fd);}
	bool	getIsApp() {return (_is_app);}
	bool	getIsInput() {return (_is_input);}
	TupleDescription	*getTupleDescr() { return(_tuple_descr); }
	int		getAppNum() {return (_app_num);}
  
  void lockArc(){ /*printf("DEBUG: lock arc %d\n", _id);*/ pthread_mutex_lock(_mutex); }
  int tryLockArc(){ /*printf("DEBUG: try lock arc %d\n", _id);*/ return pthread_mutex_trylock(_mutex); }
  void unlockArc(){ /*printf("DEBUG: UNlock arc %d\n", _id);*/ pthread_mutex_unlock(_mutex); }
  

private:
	int					_id;
	int					_src_id, _src_port_id;
	int					_dest_id, _dest_port_id;
	TupleDescription 	*_tuple_descr;
	pthread_mutex_t			*_mutex; 	
	int					_sock_fd;
	bool				_is_app;
	bool				_is_input;
	int					_app_num; // if this arc is an app, then this is the app number


 
};

class Inputs
{
public:
	Inputs() {}
	Inputs(int a, double r, int n) 
	{
		_arc_id = a;
		_rate = r;
		_num_tuples = n;
	}
	void 	setArcId(int id) { _arc_id = id; }
	void 	setRate(double rate) { _rate = rate; }
	void 	setNumTuples(int num) { _num_tuples = num; }
	int 	getArcId() { return(_arc_id); }
	double 	getRate() { return(_rate); }
	int 	getNumTuples() { return(_num_tuples); }

private:
	int					_arc_id;
	double				_rate;
	int					_num_tuples;
 
};

class Catalog
{
public:
	Catalog();
	~Catalog() {}
  void writeOutQueueDesc(int arc_id, TupleDescription*); // Eddie added this
	void	loadFromDB(QueryNetwork *q_net);
	QueryNetwork* getQNet() {return _q_net;};
	void 	loadNetwork();
	void 	loadArcs();
	void 	loadBoxes();
	void 	loadInputs();
	int		convertBoxType(int type);
	Predicate* convertBoxPredicate(Predicate* pred);
	void 	setNetwork(Network *n) {_network = n;}
	void 	insertBox(Boxes *b);
	void 	insertArc(Arcs *a);
	void 	insertInput(Inputs *inputs);
	void 	printBoxes();
	void 	printArcs();
	Arcs	*getArc(int arc_id)      {return (*_arcs)[arc_id];}
  vector<Arcs*> *getArcs() { return _arcs; }
	Boxes	*getBox(int box_id)      
  {
    /*
    cout << "$$$$$ Catalog::getBox(" << box_id << "): " << endl
	 << "$$$$$    _max_box_id = " << _max_box_id << endl
	 << "$$$$$    _boxes->size() = " << _boxes->size() << endl;
    */
    if(box_id > _max_box_id) return NULL;

    //    cout << "$$$$$    (*_boxes)[box_id] = " << _boxes->at(box_id) << endl;
	  /*else*/ return (*_boxes)[box_id];}
	int		getMaxArcId()					{return _max_arc_id; }
	int		getMaxBoxId()					{return _max_box_id; }
	int		getNumInputs()			{return _num_inputs;}
	Inputs	*getInput(int index)	{return (*_inputs)[index];}
	int		findArcIdBySource(int source_id);
	int		getNumBoxes()		{assert (_num_boxes == _network->getNumBoxes());
									return (_num_boxes); }
	void 	setExperimentFlag(bool flag)	{_experiment_flag = flag;}
	void    initTPBSignalConditions();   //Thread Per Box mode.
	void Catalog::TPBWaitForSignal( int id );
	void Catalog::TPBSignal( int id );
	bool Catalog::inTPBMode() { return _thread_per_box_mode; }

private:
	QueryNetwork		*_q_net;
	Network 			*_network;
	vector<Boxes*>		*_boxes;
	vector<Arcs*>		*_arcs;
	vector<Inputs*>		*_inputs;
	int					_num_boxes;
	int					_num_arcs;
	int					_num_inputs;
	int					_max_arc_id;
	int					_max_box_id;
	bool				_experiment_flag;
	bool                _thread_per_box_mode;
	pthread_cond_t      *_tpb_signal_cond;

	pthread_mutex_t     *_tpb_signal_mutex;
};

#endif
