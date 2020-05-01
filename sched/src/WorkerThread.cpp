#include "WorkerThread.H"
#include <assert.h>
#include <map>
#include <time.h>
#include <LockHolder.H>
#include <iostream>
#include <typeinfo>
#include <RuntimeGlobals.H>
#include "Measurement.H"

extern QueueMon* _queue_monitor;

WorkerThread::WorkerThread(int threadNum,
						   pthread_cond_t * schedulerWakeCond,
						   pthread_mutex_t * schedulerWakeMutex,
						   AppTupleMonitor * pTupleMon)
{
  _number = threadNum;
  _sched_wake_cond = schedulerWakeCond;
  _sched_wake_mutex = schedulerWakeMutex;
  _tupleMon = pTupleMon;
}
void *WorkerThread::run()
{
	QueueElement *qe = NULL;
	SMInterface SM;

	Box_Out_T out_count;
	_num_box_calls = 0;

	struct itimerval *first;
	first = &_itimer_start;
	init_etime(first);
	SM.setITimerStart(&_itimer_start);
	double secs;
	secs = get_etime(first);
	timeval start_time, next_time;
	gettimeofday(&start_time,NULL);
	
	int total_superbox_tuples;
	int total_resident_tuples;
	int total_tuples_processed = 0;
	//int total_output_tuples = 0;

	double total_time_in_doBox = 0.0;
	double total_time_in_pre_doBox = 0.0;
	double total_time_in_post_doBox = 0.0;
	double total_time_waking_and_dequeueing = 0.0;
	double total_time_cleaning_up_WT = 0.0;
	double total_time_unloading_queues = 0.0;
	double total_time_loading_queues = 0.0;
	double total_time_signaling_scheduler = 0.0;
	double total_time_handling_memremaining = 0.0;
	double total_time_announcing_new_tuples = 0.0;
	double total_time_unlocking_boxes = 0.0;
	double total_time_deleting_in_and_out =0.0;
	double total_time_unlocking_app =0.0;
	double total_time_gathering_stats = 0.0;
	double total_time_computing_stats = 0.0;
	_total_time_dequeueUnpinning = 0.0;
	_total_time_enqueueUnpinning = 0.0;
	_total_time_dequeueEnqueueUnpinning = 0.0;



	// This tracks the number of tuples enqueued into output arcs by each 
	// iteration of this loop. Key = arcId, Value = # tuples enqueued in this
	// iteration.
	map<int, int> newAppTuples;

	bool tuples_written_to_app = false;
	
	pthread_cond_t *st_wake_cond;
	pthread_mutex_t *st_wake_mutex;
	st_wake_mutex = (_measure->getStreamThread())->getWakeMutex();
	st_wake_cond = (_measure->getStreamThread())->getWakeCondition();
	while (__global_stop.getStop() != true)
	{
		double t_a_start = get_etime(first);
		tuples_written_to_app = false;
		newAppTuples.clear();

		map<int, queue_info, less<int> > queue_info_map;

		if ( _catalog->inTPBMode() && ( qe != NULL ) )
		{
				int input_tuples = 0;
			do
			{
				_measure->testStopCond();
				//cout << " Worker thread -- " << (*((qe->getBoxList())->begin()))->_boxId << " is going to wait \n";
				//cout << " Queue Entry " << qe << endl;
				//cout << " QeueuBoxList " << qe->getBoxList() << endl;
				//cout << " SIZE " << (qe->getBoxList())->size();
				_catalog->TPBWaitForSignal( (*((qe->getBoxList())->begin()))->_boxId );
				//cout << " Worker thread -- " << (*((qe->getBoxList())->begin()))->_boxId << " is waking up and going to work \n";
				
				input_tuples = 0;
				qe->_dequeuePin_list.clear();
				qe->_enqueuePin_list.clear();
				// ALL OF THIS CODE ASSUMES ONE INPUT, ONE OUTPUT PORT!!
				//cout << " SIZE 2 " << ((qe->getBoxList())->size());
				//cout << " Input queue ids empty?? " << (*((qe->getBoxList())->begin()))->_numInputArcs << endl;
				
				for ( int i = 0; i < (*((qe->getBoxList())->begin()))->_numInputArcs; i++ )
				{				
					queue_allocation qa;
					qa._arc_id =(*((qe->getBoxList())->begin()))->_inputArcId[ i ];
					qa._num_tuples = shm_get_num_records_in_queue(qa._arc_id);
					//cout << " Pushed (dequeue) " << qa._num_tuples << " On to " << qa._arc_id << endl;

					input_tuples += qa._num_tuples;
					qe->_dequeuePin_list.push_back(qa);
				}
				QBox *q_box = (*((qe->getBoxList())->begin())) ;
				for (vector<long>::iterator iter = (q_box->_output_arcs_byport[ 0 ]).begin(); iter != (q_box->_output_arcs_byport[ 0 ]).end(); iter++ )
				{
					queue_allocation qa;
					qa._arc_id = *iter;
					//cout << " The select?? " << _catalog->getBox ((*((qe->getBoxList())->begin()))->_boxId )->getSelectivity() << endl;
					qa._num_tuples = ((int)(input_tuples * (_catalog->getBox( (*((qe->getBoxList())->begin()))->_boxId))->getSelectivity() * 1.1));
					//cout << " Pushed (enqueue) " << qa._num_tuples << " On to " << qa._arc_id << " total arcs " << (q_box->_output_arcs_byport[ 0 ]).size() << endl;
					if ( qa._num_tuples == 0 )
						;//cout << " NOT SURE WHY THIS IS HAPPENING " << " (perhaps exiting on timeout? ) "<< endl;
					qe->_enqueuePin_list.push_back(qa);
				}
			}
			while( input_tuples <= 0 && __global_stop.getStop() != true);
		}
		else
		{
			_execution_queue->lock();
			
			_measure->testStopCond();
			/*		timeval now;
	gettimeofday(&now,NULL);
	printf("      EXECUTION QUEUE wait::  TIME: %d %d\n", now.tv_sec, now.tv_usec);*/
			while ( _execution_queue->size() <= 0 && __global_stop.getStop() != true)
			{
				_execution_queue->signalIfEmpty( _sched_wake_cond,
				  _sched_wake_mutex);
				_execution_queue->wait();
			}
			/*gettimeofday(&now,NULL);
			  printf("      EXECUTION QUEUE Start working!::  TIME: %d %d\n", now.tv_sec, now.tv_usec);*/
			
			if (__global_stop.getStop() != true)
			{
				
				gettimeofday(&next_time,NULL);
				
				/*if ( start_time.tv_sec == next_time.tv_sec )
					cout << " Difference " << next_time.tv_usec - start_time.tv_usec <<endl;
				else
					cout << " Difference " << (next_time.tv_usec - start_time.tv_usec)+1000000<<endl;
				*/

				gettimeofday(&start_time, NULL );
				qe = _execution_queue->pop(_sched_wake_cond,
										   _sched_wake_mutex);
				assert ( qe != NULL );
			}
			_execution_queue->unlock();
		}

		if ( __global_stop.getStop() == true )
		{
			pthread_mutex_lock(_sched_wake_mutex);
			pthread_cond_broadcast(_sched_wake_cond);
			pthread_mutex_unlock(_sched_wake_mutex);
			break;
		}


		double t_a1_start = get_etime(first);
		total_resident_tuples = loadQueues(qe,&SM,&queue_info_map);
		//cout << " Having loaded the queues I say there are " << total_resident_tuples << endl;
		double t_a1_stop = get_etime(first);
		_measure->incrementTimeLoadingQueuesWT(t_a1_stop-t_a1_start);
		total_time_loading_queues += t_a1_stop-t_a1_start;

		BoxListIter_T box_iter;
		for (box_iter = (qe->getBoxList())->begin(); 
			box_iter != (qe->getBoxList())->end(); 
			box_iter++)
		{
		  // Note: this is mallocing the pointers for each (input|output) buffer
		  // so that _outStream[i] is a char* pointer (pointing to your buffer)
			//(*box_iter)->_outStream = (char**) malloc((*box_iter)->_numOutputArcs * sizeof(char*));
			(*box_iter)->_outStream = (char**) malloc((*box_iter)->_num_output_ports * sizeof(char*));
			(*box_iter)->_inStream = (char**) malloc((*box_iter)->_numInputArcs * sizeof(char*));
		}

		vector<MemRemaining> *remaining_v = new vector<MemRemaining>;
		timeval start_time;
		gettimeofday(&start_time,NULL);
		MemRemaining mr;
		mr._elapsed_time = 0.0;
		total_superbox_tuples = total_resident_tuples;
		mr._mem_remaining = 0;//total_resident_tuples;
		remaining_v->push_back(mr);

		int num_box_calls_per_superbox = 0;

		double t_a_stop = get_etime(first);
		total_time_waking_and_dequeueing += t_a_stop-t_a_start;

		//printf("WorkerThread: before executing BoxList\n");

		for (BoxListIter_T box_iter = (qe->getBoxList())->begin(); 
			box_iter != (qe->getBoxList())->end(); 
			box_iter++)
		{
			//timeval tv;
			//gettimeofday(&tv,NULL);
			//printf("WorkerThread executing box: %i at %i %i\n",(*box_iter)->_boxId,tv.tv_sec,tv.tv_usec);

			double t_b_start = get_etime(first);
			//_measure->testStopCond();
			//if (__global_stop.getStop() == true)
				//break;
					//queue_info qi1;
				//qi1 = queue_info_map[(*box_iter)->_inputArcId[0]];
				//(*box_iter)->_inStream[0] = qi1.front;
				//printf("READING FROM: %i\n",qi1.front);
			// Replaced by qis array for multiple ports.
			//queue_info qi3;
			//qi3 = queue_info_map[(*box_iter)->_outputArcId[0]];

			queue_info qis[ (*box_iter)->_num_output_ports ];
			// This is going to fail if there are no arcs.
			for ( int zz = 0; zz < (*box_iter)->_num_output_ports; zz++ )
			{
				if ( ((*box_iter)->_output_arcs_byport[zz]).size() == 0 )
					printf("I should throw exception here -- port %d has no arcs \n", zz );
				qis[ zz ] = queue_info_map[ (*((*box_iter)->_output_arcs_byport[zz]).begin()) ];
			} 

			(*box_iter)->_train_size.clear();
			int total_input_tuples = 0;
			for (int ii = 0; ii < (*box_iter)->_numInputArcs; ii++)
			{
				queue_info qi;
				qi = queue_info_map[(*box_iter)->_inputArcId[ii]];
			    (*box_iter)->_train_size.push_back(qi.num_tuples_resident);
			    (*box_iter)->_inStream[ii] = qi.front;
				if ( qi.front == NULL && (*box_iter)->_train_size[ii] > 1 )
				{
					printf("got to bad point qi.num_tuples_resident: %i\n",qi.num_tuples_resident);
				}
				//printf("WorkerThread: arc_id: %i box_id: %i ii: %i qi.front: %i qi.num_tuples_resident: %i _train_size[%i]: %i\n",(*box_iter)->_inputArcId[ii],(*box_iter)->_boxId,ii,qi.front,qi.num_tuples_resident,ii,(*box_iter)->_train_size[ii]);
				total_input_tuples += qi.num_tuples_resident;
				total_tuples_processed += qi.num_tuples_resident;
			}
					
			//(*box_iter)->_outStream[0] = qi3.rear;
			// replaced by a qis array for multiple ports
			for ( int zz = 0; zz < (*box_iter)->_num_output_ports; zz++ )
			{
				(*box_iter)->_outStream[ zz ] = qis[ zz ].rear;
				if ( (*box_iter)->_outStream[ zz ] == NULL )
				{
					//cout << " I am box " << (*box_iter)->_boxId << " and on port " << zz << " I messed up " << endl;
					//assert( false );
					//abort();
					//exit( 0 );
					continue;
				}
			}


			double t_b_stop = get_etime(first);
			total_time_in_pre_doBox += t_b_stop-t_b_start;
			double t_c_start1 =0.0;
			double t_c_stop1 =0.0;
			static int tmp_counter = 0;
			tmp_counter++;
			if ( total_input_tuples > 0 )
			{
				(*box_iter)->return_val.kept_input_count_array = NULL;

				((*box_iter)->_itimer_start) = first;
				double t1 = get_etime(first);
				timeval now;
				gettimeofday(&now,NULL);
				//printf("      ----->DO BOX run TIME: %d %d\n", now.tv_sec, now.tv_usec);

				out_count = (*box_iter)->doBox();

				double t2 = get_etime(first);
				total_time_in_doBox += t2-t1;
				t_c_start1 = get_etime(first);

				//---------------------------------------------------------------
				// Maintain Boxes' object's stats...
				//---------------------------------------------------------------

				// Track the number of tuples enqueued on each output port...
				/* 
				   QBox * pQBox = (*box_iter);
				int numOutputPorts = pQBox->_num_output_ports;
				
				Boxes * pBoxes = _catalog->getBox(pQBox->_boxId);
				
				assert(numOutputPorts == pBoxes->_total_enqueued_tuples_byport.size());

				for (int i = 0; i < numOutputPorts; ++i)
					{
						pBoxes->_total_enqueued_tuples_byport[i] += 
							out_count.output_tuples_array[i];
					}

				// Track the number of tuples dequeued, by any input port...
				int tuplesConsumed = 0;
				
				if (out_count.kept_input_count_array == NULL)
					{
						tuplesConsumed = 
							(pQBox->_train_size[0]) - out_count.kept_input_count;
					}
				else
					{
						size_t numInputs = pQBox->_train_size.size();
						for (size_t i = 0; i < numInputs; ++i)
							{
								tuplesConsumed += 
									(pQBox->_train_size[i]) - out_count.kept_input_count_array[i];
							}
					}

				pBoxes->_total_dequeued_tuples += tuplesConsumed;

				double t_stats_stop = get_etime(first);
				total_time_gathering_stats += t_stats_stop - t_c_start1;
				//---------------------------------------------------------------
				*/

				_num_box_calls++;
				num_box_calls_per_superbox++;

				// This duplicates the actions for every output port that
				// the box has
				for ( int zz = 0; zz < (*box_iter)->_num_output_ports; zz++ )
				{
					//printf(" Duplicate actions for port %d\n", zz );

					vector<long> * pArcsVect = & ((*box_iter)->_output_arcs_byport)[zz];
					for (int portArcIdx = 0; portArcIdx < pArcsVect->size(); ++portArcIdx)
						{
							int arcId = (*pArcsVect)[portArcIdx];

					//int arcId = (*box_iter)->_outputArcId[ zz ];
					Arcs *a = _catalog->getArc(arcId);
					
					if ( a->getIsApp() == true )
					{
						//total_output_tuples += out_count.output_tuples;
						//dpc printf("total_output_tuples: %i\n", total_output_tuples);

						tuples_written_to_app = true;

						if (out_count.output_tuples > 0)
						{
							// ??? This assumes only one arc leaving a port -cjc
							//total_output_tuples += out_count.output_tuples_array[ zz ];
							
							// if other arcs from splits go to applications
							_measure->incrementToMemoryLatency(out_count.total_latency);
							_measure->incrementToMemoryTuplesWritten(out_count.output_tuples_array[ zz ]);
							//cout << " Incrementing ? " << out_count.output_tuples_array[ zz ] << " box? " << int((*box_iter)->_boxId) << endl;
							
							if (out_count.output_tuples_array[ zz ] > 0)
							{
								map<int, int>::iterator pos = newAppTuples.find(arcId);
								if (pos == newAppTuples.end())
								{
									newAppTuples.insert(make_pair(arcId, out_count.output_tuples_array[ zz ]));
								}
								else
								{
									pos->second += out_count.output_tuples_array[ zz ];
								}
							}
						}
					}
						}
				}

				for (int ii = 0; ii < (*box_iter)->_numInputArcs; ii++)
				{
					if ( out_count.kept_input_count_array != NULL )
					{
						if ( out_count.kept_input_count_array[ii] >= 0 )
						{
							queue_info_map[(*box_iter)->_inputArcId[ii]].num_tuples_resident = 
							  out_count.kept_input_count_array[ii];

							queue_info_map[(*box_iter)->_inputArcId[ii]].front += 
							  ((*box_iter)->_train_size[ii] - out_count.kept_input_count_array[ii]) * 
							  out_count.output_tuple_size;
						}
					}
					else
					{
						//printf("Should never get here anymore tmp_counter: %i\n",tmp_counter);
						//abort();
						queue_info_map[(*box_iter)->_inputArcId[ii]].num_tuples_resident = out_count.kept_input_count;
						queue_info_map[(*box_iter)->_inputArcId[ii]].front +=
						  ((*box_iter)->_train_size[ii] - out_count.kept_input_count) * out_count.output_tuple_size;
						
					}
				}
				for (int ii = 0; ii < (*box_iter)->_numInputArcs; ii++)
				{
					if ( out_count.kept_input_count_array != NULL )
					{
						if ( out_count.kept_input_count_array[ii] >= 0 )
						{
							queue_info_map[(*box_iter)->_inputArcId[ii]].num_tuples_resident = 
							  out_count.kept_input_count_array[ii];

							queue_info_map[(*box_iter)->_inputArcId[ii]].front += 
							  ((*box_iter)->_train_size[ii] - out_count.kept_input_count_array[ii]) * 
							  out_count.output_tuple_size;
						}
					}
					else
					{
						//printf("Should never get here anymore tmp_counter: %i\n",tmp_counter);
						//abort();
						queue_info_map[(*box_iter)->_inputArcId[ii]].num_tuples_resident = out_count.kept_input_count;
						queue_info_map[(*box_iter)->_inputArcId[ii]].front +=
						  ((*box_iter)->_train_size[ii] - out_count.kept_input_count) * out_count.output_tuple_size;
						
					}
				}
				t_c_stop1 = get_etime(first);
			}
			else
			{
				assert(false);
				out_count.output_tuples = 0;
				out_count.output_tuple_size = 
				  (*box_iter)->_tuple_descr->getSize() + 
				  (*box_iter)->getTsSize() + 
				  (*box_iter)->getSidSize();
				out_count.output_tuples_array = new int[(*box_iter)->_num_output_ports];
				for ( int zz = 0; zz < (*box_iter)->_num_output_ports; zz++ )
					out_count.output_tuples_array[ zz ] = 0;

				out_count.kept_input_count = 0;
				out_count.kept_input_count_array = NULL;

				if ( tmp_counter % 1000 == 0 )
					printf("Why am I here? tmp_counter: %i\n",tmp_counter);
				//abort();
			}
			double t_c_start2 = get_etime(first);


			// This duplicates the actions for every output port that
			// the box has
			for ( int zz = 0; zz < (*box_iter)->_num_output_ports; zz++ )
			{
				//queue_info_map[(*box_iter)->_outputArcId[0]].num_tuples_resident = 
				//queue_info_map[(*box_iter)->_outputArcId[0]].num_tuples_resident + 
				//out_count.output_tuples;
				
				//printf("RUN for port %d and record for ARC %d exactly %d tuples\n", zz, *(((*box_iter)->_output_arcs_byport[zz]).begin()), out_count.output_tuples_array[ zz ] );
				if (out_count.output_tuples_array != NULL )
				{
					queue_info_map[*(((*box_iter)->_output_arcs_byport[zz]).begin())].num_tuples_resident = 
						queue_info_map[*(((*box_iter)->_output_arcs_byport[zz]).begin())].num_tuples_resident + 
						out_count.output_tuples_array[ zz ];
					//printf ("A queue_info_map[%i].num_tuples_resident: %i\n",*(((*box_iter)->_output_arcs_byport[zz]).begin()),queue_info_map[*(((*box_iter)->_output_arcs_byport[zz]).begin())].num_tuples_resident);
				}
				
				MemRemaining mr;
				timeval now;
				gettimeofday(&now,NULL);

				mr._elapsed_time = 
					(now.tv_sec + (now.tv_usec*1e-6)) -
					(start_time.tv_sec + (start_time.tv_usec*1e-6));
				
				if ( 0 )// if ( _catalog->getArc((*box_iter)->_outputArcId[0])->getIsApp() == true )
					total_resident_tuples = total_resident_tuples - total_input_tuples;
				else
					total_resident_tuples = total_resident_tuples - ( total_input_tuples - out_count.output_tuples_array[ zz ]);
				
				//cout << " Total resident tuples Having " << total_resident_tuples << endl;
				mr._mem_remaining = ( 100 - (int) ( (100.0 * total_resident_tuples)/(1.0* total_superbox_tuples) ));
				
				//cout << " Recording " << mr._mem_remaining << " AT " << mr._elapsed_time << "  Total tuples " << total_resident_tuples << " total superbox tuples " << total_superbox_tuples << endl;
				remaining_v->push_back(mr);
				
				(*box_iter)->_out_count.output_tuples = out_count.output_tuples;
				(*box_iter)->_out_count.output_tuples_array = out_count.output_tuples_array;
				(*box_iter)->_out_count.output_tuple_size = out_count.output_tuple_size;
				(*box_iter)->_out_count.kept_input_count = out_count.kept_input_count;
				
				//queue_info_map[(*box_iter)->_outputArcId[0]].rear += out_count.output_tuples * out_count.output_tuple_size;
				if (out_count.output_tuples_array != NULL )
					queue_info_map[ *((*box_iter)->_output_arcs_byport[zz]).begin() ].rear += out_count.output_tuples_array[ zz ] * out_count.output_tuple_size;
				
				//printf(" all assignments are done --- ready for replication\n");
				// now memcpy _outStream[ firstArcInPort ] to all the other _outStream[ samePort ]
				//cerr << "WorkerThread::run distributing output from box Id "
				//     << (*box_iter)->_boxId << " port " << zz << endl;
				//for (int g = 1; g < (*box_iter)->_numOutputArcs; g++)
				if (out_count.output_tuples_array != NULL )
				{
					for ( vector<long>::iterator iter = (((*box_iter)->_output_arcs_byport[zz]).begin()+1); iter != ((*box_iter)->_output_arcs_byport[zz]).end(); iter ++ )
					{
						//printf("Replicate the tuple to other arcs\n");
						memcpy(queue_info_map[ (*iter) ].rear, (*box_iter)->_outStream[ zz ],
							   (*box_iter)->_out_count.output_tuples_array[ zz ] * (*box_iter)->_out_count.output_tuple_size);
						
						queue_info_map[ (*iter) ].num_tuples_resident = 
							queue_info_map[ (*iter) ].num_tuples_resident + 
							out_count.output_tuples_array[ zz ];
						//printf("B queue_info_map[%i].num_tuples_resident: %i\n",*iter,queue_info_map[ (*iter) ].num_tuples_resident);
						
						queue_info_map[ (*iter) ].rear += out_count.output_tuples_array[ zz ] * out_count.output_tuple_size;
					}
				}
			}


			double t_c_stop2 = get_etime(first);
			total_time_in_post_doBox += (t_c_stop2-t_c_start2)+(t_c_stop1-t_c_start1);
		}  // loop through all the boxes in the boxlist


		double t_d_start = get_etime(first);

		double t_d1_start = get_etime(first);

		if ( total_superbox_tuples > 10 )  // filter out the degenerate cases
			_measure->addMemRemainingVector(remaining_v);
		double t_d1_stop = get_etime(first);
		total_time_handling_memremaining += t_d1_stop - t_d1_start;

		double t_d2_start = get_etime(first);

		unloadQueues(qe,&SM,&queue_info_map);

		double t_d2_stop = get_etime(first);
		_measure->incrementTimeUnloadingQueuesWT(t_d2_stop - t_d2_start);
		total_time_unloading_queues += t_d2_stop - t_d2_start;
		
		double t_d3_start = get_etime(first);
		announceNewAppTuples(newAppTuples);
		double t_d3_stop = get_etime(first);
		total_time_announcing_new_tuples += t_d3_stop - t_d3_start;

		// Once the queues have been UNLOADED, we may call the downstream
		// box
		// NOTE: this will only work on thread per box mode --
		// so in general the loop over all boxes is just one box.
		if ( _catalog->inTPBMode() )
			for (BoxListIter_T box_iter = (qe->getBoxList())->begin(); 
				 box_iter != (qe->getBoxList())->end(); 
				 box_iter++)
				for ( int zz = 0; zz < (*box_iter)->_num_output_ports; zz++ )
				{
					int arcId = (*box_iter)->_outputArcId[ zz ];
					Arcs *a = _catalog->getArc(arcId);
					if ( a->getIsApp() != true )
					{
						if (out_count.output_tuples_array[ zz ] > 0 &&
							_catalog->inTPBMode() )
						{
							//cout << "            SIGNALING to " << a->getDestId() << " on output port " << zz << "  that produced " << out_count.output_tuples_array[ zz ] << " tuples. " << endl;
							_catalog->TPBSignal( a->getDestId() );
						}
					}
				}

		double t_d8_start = get_etime(first);

		// Update the DelayedDataMgr's stats for the boxes / arcs we messed 
		// with...
		/*
		  DelayedDataMgr * pDdm = RuntimeGlobals::getDelayedDataMgr();
		StatsImage & si = pDdm->getWritableImage();

		// Do the per-arc stats reporting...
		for (map<int, queue_info, less<int> >::iterator pos = queue_info_map.begin();
			 pos != queue_info_map.end(); 
			 ++ pos)
			{				
				si._numTuplesOnArcs[pos->first] = pos->second.num_tuples_resident;
			}

		//-----------------------------------------------------------------------
		// Do the per-box stats reporting...
		//-----------------------------------------------------------------------
		for (BoxListIter_T box_iter = (qe->getBoxList())->begin(); 
			box_iter != (qe->getBoxList())->end(); 
			box_iter++)
		{
			int boxId = int((*box_iter)->_boxId);
			Boxes * pBox = _catalog->getBox(boxId);

			BoxStats & bstats = si._boxesStats[boxId];
			bstats._numConsumedTuples = pBox->_total_dequeued_tuples;

			int numPorts = bstats._numProducedTuples.size();
			assert(pBox->_total_enqueued_tuples_byport.size() == numPorts);

			// This code assumes that both vectors involved are vector<int>.
			// It's the fastest way I know of for copying the data, but it relies
			// on the vector's being the same size and of the same intrinsic 
			// type. -cjc
			memcpy(& bstats._numProducedTuples[0], 
				   & pBox->_total_enqueued_tuples_byport[0], 
				   sizeof(int) * numPorts);
		}		
		pDdm->releaseWritableImage();

		*/
		double t_d8_stop = get_etime(first);
		total_time_computing_stats += t_d8_stop - t_d8_start;

		//-----------------------------------------------------------------------
		// Release the locks we held for processing the current QueueElement...
		//-----------------------------------------------------------------------
		double t_d4_start = get_etime(first);

		for (BoxListIter_T box_iter = (qe->getBoxList())->begin(); 
			box_iter != (qe->getBoxList())->end(); 
			box_iter++)
		{
			if ( !_catalog->inTPBMode() )
				(_catalog->getBox((int)((*box_iter)->_boxId)))->unlockBox();
		}		
		double t_d4_stop = get_etime(first);
		total_time_unlocking_boxes += t_d4_stop - t_d4_start;
			
		double t_d5_start = get_etime(first);
		for (box_iter = (qe->getBoxList())->begin(); 
			box_iter != (qe->getBoxList())->end(); 
			box_iter++)
		{
		  // Euh... I'm not sure you can use delete[] if you used malloc before,
		  // in fact, my book here says IT IS UNDEFINED WHAT HAPPENS.
		  //delete[] ((*box_iter)->_outStream);
		  //delete[] ((*box_iter)->_inStream);
		  // NOTE: the actual buffers are freed in unloadQueues
		  free((*box_iter)->_outStream);
		  free((*box_iter)->_inStream);
		}
		double t_d5_stop = get_etime(first);
		total_time_deleting_in_and_out += t_d5_stop - t_d5_start;
			
		double t_d6_start = get_etime(first);
		if ( qe->_app_mutex != NULL  && !_catalog->inTPBMode() )
		{
			pthread_mutex_unlock(qe->_app_mutex);
		}
		double t_d6_stop = get_etime(first);
		total_time_unlocking_app += t_d6_stop - t_d6_start;

		// Deallocate the queue box (it was new'ed in a diff thread)
		if ( !_catalog->inTPBMode() )
			delete qe;

		double t_d7_start = get_etime(first);
		
		//if ( tuples_written_to_app == false  ||
			//_measure->testStreamThreadStopped() )		// if the StreamThread stopped producing tuples
														// then the Scheduler does not need to sleep anymore
		if ( _execution_queue->size() <= 0 )
		{
			//printf("WorkerThread: Signaling Scheduler\n");
			pthread_mutex_lock(_sched_wake_mutex);
			pthread_cond_broadcast(_sched_wake_cond);
			pthread_mutex_unlock(_sched_wake_mutex);
			//timeval tv;
			//gettimeofday(&tv,NULL);
			//printf("WorkerThread just signaled Scheduler at %i %i\n",tv.tv_sec,tv.tv_usec);
		}
		//signal StreamThread
		//pthread_mutex_lock(st_wake_mutex);
		//pthread_cond_broadcast(st_wake_cond);
		//pthread_mutex_unlock(st_wake_mutex);

		double t_d7_stop = get_etime(first);
		total_time_signaling_scheduler += t_d7_stop - t_d7_start;

		double t_d_stop = get_etime(first);
		total_time_cleaning_up_WT += t_d_stop-t_d_start;
	} // loop while not force_exit

	_measure->incrementTimeSpentInWorkerThread(get_etime(first)-secs);
	_measure->incrementTimeSpentInDoBox(total_time_in_doBox);
	_measure->incrementNumBoxCalls(_num_box_calls);

	// dpc printf("total_time_in_doBox: %f\n", total_time_in_doBox);
	// dpc printf("total_time_in_pre_doBox: %f\n", total_time_in_pre_doBox);
	// dpc printf("total_time_in_post_doBox: %f\n", total_time_in_post_doBox);
	// dpc printf("total_time_waking_and_dequeueing: %f\n", total_time_waking_and_dequeueing);
	// dpc printf("total_time_cleaning_up_WT: %f\n", total_time_cleaning_up_WT);
	// dpc printf("       total_time_unloading_queues: %f\n", total_time_unloading_queues);
	// dpc printf("			_total_time_dequeueUnpinning: %f\n",_total_time_dequeueUnpinning);
	// dpc printf("			_total_time_enqueueUnpinning: %f\n",_total_time_enqueueUnpinning);
	// dpc printf("			_total_time_dequeueEnqueueUnpinning: %f\n",_total_time_dequeueEnqueueUnpinning);
	// dpc printf("       total_time_signaling_scheduler: %f\n", total_time_signaling_scheduler);
	// dpc printf("       total_time_handling_memremaining: %f\n", total_time_handling_memremaining);
	// dpc printf("       total_time_announcing_new_tuples: %f\n", total_time_announcing_new_tuples);
	// dpc printf("       total_time_unlocking_boxes: %f\n", total_time_unlocking_boxes);
	// dpc printf("       total_time_deleting_in_and_out: %f\n", total_time_deleting_in_and_out);
	// dpc printf("       total_time_unlocking_app: %f\n", total_time_unlocking_app);
	// dpc printf("cleaning cumulative: %f\n",total_time_unloading_queues +
    // dpc total_time_signaling_scheduler +
    // dpc total_time_handling_memremaining +
    // dpc total_time_announcing_new_tuples +
    // dpc total_time_unlocking_boxes +
    // dpc total_time_deleting_in_and_out  +
    // dpc total_time_unlocking_app);
	// dpc printf("WT cumulative: %f\n", total_time_in_doBox +
	// dpc total_time_in_pre_doBox +
	// dpc total_time_in_post_doBox +
	// dpc total_time_waking_and_dequeueing +
	// dpc total_time_cleaning_up_WT);
	// dpc double total_time_in_worker_thread = get_etime(first)-secs;
	// dpc SM.printTimings();
	// dpc printf("WT total_time: %f\n",total_time_in_worker_thread);
	// dpc printf("WT			loading_queues  : %lf\n",total_time_loading_queues);
	// dpc printf("WT			Box Overhead    : %lf\n",total_time_in_doBox-_measure->getTimeSpentExecutingBoxes());
	// dpc printf("WT			Box Execution   : %lf\n",_measure->getTimeSpentExecutingBoxes());
	// dpc printf("WT			unloading_queues: %lf\n",total_time_unloading_queues);
	// dpc printf("WT			overhead        : %lf\n",total_time_in_worker_thread -
	// dpc											( 	total_time_loading_queues +
	// dpc										  	total_time_in_doBox +
	// dpc											total_time_unloading_queues )
	// dpc										  	);                                                                        
	// dpc printf("Stats object: total_time_gathering_stats: %lf\n",total_time_gathering_stats);
	// dpc printf("Stats object: total_time_computing_stats: %lf\n",total_time_computing_stats);
	// dpc printf("WTSTATS loading_queues box_overhead box_execution unloading_queues WT_overhead\n");
	// dpc printf("WTSTATS: %lf %lf %lf %lf %lf\n",total_time_loading_queues,
    // dpc                                              total_time_in_doBox-_measure->getTimeSpentExecutingBoxes(),
    // dpc                                              _measure->getTimeSpentExecutingBoxes(),
    // dpc                                              total_time_unloading_queues,
    // dpc                                              total_time_in_worker_thread -
    // dpc                                              ( 	total_time_loading_queues +
    // dpc                                              total_time_in_doBox +
    // dpc                                              total_time_unloading_queues )
    // dpc                                              );                                                            
	_measure->incrementBoxOverhead(total_time_in_doBox-_measure->getTimeSpentExecutingBoxes());
	pthread_exit(NULL);
  
	return (void *)0;
}

int WorkerThread::loadQueues(QueueElement *qe,SMInterface *SM, map<int, queue_info, less<int> > *queue_info_map)
{
			int total_resident_tuples = 0;
			char *ptr;
			queue_info qi;
			for ( int i = 0; i < qe->_dequeuePin_list.size(); i++ )
			{
				ptr = SM->dequeuePin( qe->_dequeuePin_list[i]._arc_id,
									qe->_dequeuePin_list[i]._num_tuples);
				shm_set_state(qe->_dequeuePin_list[i]._arc_id,2);
				
				
				qi.front = ptr;
				qi.rear = NULL;
				qi.malloc_ptr = ptr;
				qi.num_tuples_alloc = qe->_dequeuePin_list[i]._num_tuples;
				qi.num_tuples_resident = qe->_dequeuePin_list[i]._num_tuples;
				//printf("dequeuePin: arc_id: %i  num_tuples_resident: %i\n",qe->_dequeuePin_list[i]._arc_id,qi.num_tuples_resident);
				total_resident_tuples += qi.num_tuples_resident;
				(*queue_info_map)[qe->_dequeuePin_list[i]._arc_id] = qi;
			}
			for ( int i = 0; i < qe->_enqueuePin_list.size(); i++ )
			{
				//cout << "[WorkerThread] Load queues for arc " << qe->_enqueuePin_list[i]._arc_id << " and tuples there are " << qe->_enqueuePin_list[i]._num_tuples <<endl; 
				ptr = SM->enqueuePin( qe->_enqueuePin_list[i]._arc_id,
									qe->_enqueuePin_list[i]._num_tuples);
				shm_set_state(qe->_enqueuePin_list[i]._arc_id,2);
				
				
				qi.front = NULL;
				qi.rear = ptr;
				qi.malloc_ptr = ptr;
				qi.num_tuples_alloc = qe->_enqueuePin_list[i]._num_tuples;
				qi.num_tuples_resident = 0;
				//printf("enqueuePin: arc_id: %i  num_tuples_resident: %i\n",qe->_enqueuePin_list[i]._arc_id,qi.num_tuples_resident);
				(*queue_info_map)[qe->_enqueuePin_list[i]._arc_id] = qi;

			}
			for ( int i = 0; i < qe->_dequeueEnqueuePin_list.size(); i++ )
			{
				qi = SM->dequeueEnqueuePin( qe->_dequeueEnqueuePin_list[i]._arc_id,
									qe->_dequeueEnqueuePin_list[i]._num_tuples);

				if ( qi.front == NULL )
				{
					printf("------GOT NULL qi.front for arc: arc_id: %i  num_tuples: %i\n",qe->_dequeueEnqueuePin_list[i]._arc_id,qe->_dequeueEnqueuePin_list[i]._num_tuples);
				}

				qi.malloc_ptr = qi.front;
				total_resident_tuples += qi.num_tuples_resident;
				shm_set_state(qe->_dequeueEnqueuePin_list[i]._arc_id,2);

				//printf("dequeueEnqueuePin: arc_id: %i  num_tuples_resident: %i\n",qe->_dequeueEnqueuePin_list[i]._arc_id,qi.num_tuples_resident);
				
				
				(*queue_info_map)[qe->_dequeueEnqueuePin_list[i]._arc_id] = qi;
			}
			return total_resident_tuples;
}
void WorkerThread::unloadQueues(QueueElement *qe,SMInterface *SM, map<int, queue_info, less<int> > *queue_info_map)
{
	struct itimerval *first;
	first = &_itimer_start;
		double t_q_start = get_etime(first);
			for ( int i = 0; i < qe->_dequeuePin_list.size(); i++ )
			{
				int arc_id = qe->_dequeuePin_list[i]._arc_id;
				int num_to_remove = ((*queue_info_map)[arc_id].num_tuples_alloc)-((*queue_info_map)[arc_id].num_tuples_resident);
				//printf(" DequeueUnpin arc == %d num of tup ==  %d  NUM ALLOC %d, RESIDENT %d\n", arc_id, num_to_remove,((*queue_info_map)[arc_id].num_tuples_alloc),  ((*queue_info_map)[arc_id].num_tuples_resident) );
				SM->dequeueUnpin( arc_id, num_to_remove);
				shm_set_state(arc_id,0);
				//free((*queue_info_map)[arc_id].front);
				free((*queue_info_map)[arc_id].malloc_ptr);
				
			}
		double t_q_stop = get_etime(first);
		_total_time_dequeueUnpinning += t_q_stop-t_q_start;


		double t_r_start = get_etime(first);
				for ( int i = 0; i < qe->_enqueuePin_list.size(); i++ )
				{
					int arc_id = qe->_enqueuePin_list[i]._arc_id;
					//SM->enqueueUnpin( arc_id, (*queue_info_map)[arc_id].rear, (*queue_info_map)[arc_id].num_tuples_resident);

					int numTuplesInQueue = (*queue_info_map)[arc_id].num_tuples_resident;
					//					printf("[WorkerThread] EnqueueUnpin on arc %d tuples %d\n", arc_id, numTuplesInQueue );
					if ( numTuplesInQueue != 0 )
						SM->enqueueUnpin( arc_id, (*queue_info_map)[arc_id].malloc_ptr, numTuplesInQueue);
					shm_set_state(arc_id,0);
					//free((*queue_info_map)[arc_id].rear);
					free((*queue_info_map)[arc_id].malloc_ptr);
				}
		double t_r_stop = get_etime(first);
		_total_time_enqueueUnpinning += t_r_stop-t_r_start;
			
		double t_s_start = get_etime(first);
			for ( int i = 0; i < qe->_dequeueEnqueuePin_list.size(); i++ )
			{
				int arc_id = qe->_dequeueEnqueuePin_list[i]._arc_id;
				if (  (*queue_info_map)[arc_id].num_tuples_resident > 0 )
					SM->dequeueEnqueueUnpin( arc_id, (*queue_info_map)[arc_id] );
				shm_set_state(arc_id,0);
				//free((*queue_info_map)[arc_id].front);
				free((*queue_info_map)[arc_id].malloc_ptr);
			}
		double t_s_stop = get_etime(first);
		_total_time_dequeueEnqueueUnpinning += t_s_stop-t_s_start;
}

void *WorkerThread::entryPoint(void *pthis)
{
	WorkerThread *pt = (WorkerThread*)pthis;
	pt->run();

	return (void *)0;
}

void WorkerThread::start(BoxExecutionQueue *beq)
{
	_execution_queue = beq;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_create( &_thread, &attr, WorkerThread::entryPoint, this);
	//pthread_create( &_thread, 0, WorkerThread::entryPoint, this);
  
}

void WorkerThread::init_etime(struct itimerval *first)
{
	first->it_value.tv_sec = 1000000;
	first->it_value.tv_usec = 0;
	//setitimer(ITIMER_PROF,first,NULL);
	setitimer(ITIMER_VIRTUAL,first,NULL);
}
double WorkerThread::get_etime(struct itimerval *first)
{
	struct itimerval curr;
	//getitimer(ITIMER_PROF,&curr);
	getitimer(ITIMER_VIRTUAL,&curr);
	return (double)(
		(first->it_value.tv_sec + (first->it_value.tv_usec*1e-6)) -
		(curr.it_value.tv_sec + (curr.it_value.tv_usec*1e-6)));


	//return (double)(
		//(first->it_value.tv_sec - curr.it_value.tv_sec) +
		//(first->it_value.tv_usec - curr.it_value.tv_usec)*1e-6);
}
void WorkerThread::announceNewAppTuples(const map<int, int> & localMap)
{
    if (! localMap.empty())
	{
	    LockHolder lh(_tupleMon->_mtx);
	  
	    map<int, int> & sharedMap = _tupleMon->_value._appArcCounts;

	    for (map<int, int>::const_iterator localPos = localMap.begin();
		     localPos != localMap.end(); ++localPos)
		{
		    int arcId = localPos->first;
		    int numNewTuples = localPos->second;
			sharedMap[arcId] += numNewTuples;

			/*
			map<int, int>::iterator sharedPos = sharedMap.find(arcId);

			if (sharedPos == sharedMap.end())
			{
				sharedMap.insert(make_pair(arcId, numNewTuples));
			}
			else
			{
			    sharedPos->second += numNewTuples;
			}
			*/
		}


		_tupleMon->_cond.broadcast();
	}
}
