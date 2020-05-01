#ifndef QBOX_H
#define QBOX_H

#include <stdio.h>
#include <list>
#include <iostream>
#include <string>
#include <map>
#include <Predicate.H>
#include "FieldExt.H"
#include "TupleDescription.H"
#include "Catalog.H"
#include "FunPred.H"

#include "AggregateState.H"

#include "global.H"

using namespace std;

// LoadShedder-related types BEGIN (tatbul@cs.brown.edu)
//
// if (drop_predicate) then value-based load shedding is to be done
// else if (drop_rate > 0) then random dropping to be done
// else no load shedding at this box
// Each vector entry corresponds to the _inStream of the same index.
typedef struct {
	double 		drop_rate;
	Predicate 	*drop_predicate;
} QBoxDropInfo;

typedef map<int, QBoxDropInfo*>			QBoxDropInfoMap;
typedef QBoxDropInfoMap::iterator 		QBoxDropInfoMapIter;
//
// LoadShedder-related types END (tatbul@cs.brown.edu)

//class Stream{};
class Cost
{
public:
	Cost() {};
	double	_cost_time;	// Cost to process one tuple (microseconds)
};

//enum boxType { SELECT_BOX, MAP_BOX, WMAP_BOX, MERGE_BOX, RESAMPLE_BOX, JOIN_BOX, DROP_BOX };

struct Box_Out_T
{
  int kept_input_count;  //number of tuples to keep on input queue
  int *kept_input_count_array;  //number of tuples to keep for each queue
  int output_tuples;     //number of tuples output by the box
  int *output_tuples_array;     //number of output tuples per PORT. should
                       	//  make the single output_tuples redundant eventually
  int output_tuple_size;  // size in sizeof(char)'s of the output tuples
  double total_latency;  // used if out arc is an output arc

  // I'm adding this because I keep on having Valgrind discover WorkerThread's 
  // use of uninitialzied fields in this structure, as returned by the various
  // QBox-derived-class' doBox() methods. -cjc, 3 Mar 2003
  Box_Out_T()
  {
    kept_input_count        = 0; 
    kept_input_count_array  = NULL;
    output_tuples           = 0;
	output_tuples_array     = NULL;
    output_tuple_size       = 0;
    total_latency           = 0.0; 
  }
};


class QBox
{
public:
  virtual ~QBox();

  virtual Box_Out_T doBox() = 0;

  FieldExt** parseAtts(const char *group_by_atts, int num_atts);
  int        getSidSize();
  int        getTsSize();
  char*      getSid(char *tuple);
  Timestamp  getTs(char *tuple);

  long	  _boxId;		// each box in the net has a unique boxId
  
  long*   _inputArcId;

  long    _numInputArcs;


  long*    _outputArcId;

	vector<long> *_output_arcs_byport;

	int _num_output_ports;

  long   _numOutputArcs;

  // This is clashing with the definitions laid out in Catalog.H
  // I am commenting out this version for now but we probably want to go back
  // to it... I think that Catlog.H is going to be removed anyway.
  //boxType _boxType;		// select, map, etc.
  int     _boxType;		        // select, map, etc.

  int     _window1;		// window definition for first input
    
  char    **_inStream;      // array of input streams

  char    **_outStream;		// array of output streams

  Box_Out_T	_out_count; // 
  
  double	  _selectivity;	// size of expected input/output
  
  Cost	  _unitCost;		// processing cost
  
  vector<long>      _train_size;  // the number of tuples in this queue

  // describes the tuple format  for the input streams
  TupleDescription *_tuple_descr;
  TupleDescription **_tuple_descr_array;
  
  Box_Out_T return_val;

	// LoadShedder-related members BEGIN (tatbul@cs.brown.edu)
	//
  	QBoxDropInfoMap			_ls_info;	
	bool					**_ls_flag;

	void applyLoadShedding();
	//
	// LoadShedder-related members END (tatbul@cs.brown.edu)


  struct itimerval *_itimer_start;
};

#endif
