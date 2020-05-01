#ifndef QUEUE_ELEMENT_H
#define QUEUE_ELEMENT_H


#include <list>
#include <QBox.H>

using namespace std;

typedef list<QBox*> BoxList_T;
typedef list<QBox*>::iterator BoxListIter_T;

struct queue_allocation
{
	int _arc_id;
	int _num_tuples; // total allocation for tuples
};

class QueueElement
{
public:
  QueueElement()  { boxList = new BoxList_T(); 
	  				_app_mutex = NULL;  }
	~QueueElement()  { 
		//printf("deleting QueueElement\n");
		BoxListIter_T iter;  
		
		
		for ( iter = boxList->begin();
				iter != boxList->end();
				iter++ )
		  {
		    // Please be careful! these were previously only delete, when delete[] was needed
			delete [] (*iter)->_inputArcId;
			delete [] (*iter)->_outputArcId;
			delete [] (*iter)->_output_arcs_byport;
			for ( int zz = 0; zz < (*iter)->_numInputArcs; zz++ )
				delete (*iter)->_tuple_descr_array[ zz ];
			delete [] (*iter)->_tuple_descr_array;
			delete (*iter)->_tuple_descr;
			delete *iter;
		}
		
		delete boxList;
	}

  BoxList_T   *getBoxList() { return boxList; }
  BoxList_T   *boxList;

  vector<queue_allocation> _enqueuePin_list;
  vector<queue_allocation> _dequeuePin_list;
  vector<queue_allocation> _dequeueEnqueuePin_list;
  pthread_mutex_t *_app_mutex;
  int	_app_num;

	void QueueElement::print()
	{
		printf("QueueElement\n");
		printf("============\n");
		BoxListIter_T iter;
		printf("\tBoxes: ");
		for ( iter = boxList->begin();
				iter != boxList->end();
				iter++ )
		{
			printf(" %ld",(*iter)->_boxId);
		}
		printf("\n");
		printf("\t_enqueuePin_list: ");
		for ( int i = 0; i < _enqueuePin_list.size(); i++ )
			printf(" (%i,%i)",_enqueuePin_list[i]._arc_id,_enqueuePin_list[i]._num_tuples);
		printf("\n");
		printf("\t_dequeuePin_list: ");
		for ( int i = 0; i < _dequeuePin_list.size(); i++ )
			printf(" (%i,%i)",_dequeuePin_list[i]._arc_id,_dequeuePin_list[i]._num_tuples);
		printf("\n");
		printf("\t_dequeueEnqueuePin_list: ");
		for ( int i = 0; i < _dequeueEnqueuePin_list.size(); i++ )
			printf(" (%i,%i)",_dequeueEnqueuePin_list[i]._arc_id,_dequeueEnqueuePin_list[i]._num_tuples);
		printf("\n");
	}
  
};


#endif
