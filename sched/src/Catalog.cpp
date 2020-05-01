#include "Catalog.H"
#include "stdio.h"
//#include "global.H"
//#include <RuntimeGlobals.H>
//#include "lex_stuff.H"
#include <RuntimeGlobals.H> //not ok here

#include <CompositeType.H>
#include <TypeManager.H>
#include <FilterBox.H> // Why am I here?
#include <logutil.H>

Catalog::Catalog()
{
	_q_net = NULL;
	_network = new Network;
	_boxes   = new vector<Boxes *>;
	_arcs    = new vector<Arcs*>;
	_inputs    = new vector<Inputs*>;
	_num_arcs = 0;
	_num_boxes = 0;
	_num_inputs = 0;
	_max_arc_id = 0;
	_max_box_id = 0;
	_experiment_flag = false;

	_thread_per_box_mode = false;
}
void Catalog::insertBox(Boxes *b)
{
	_num_boxes++;
	int incr = 10;
	if ( _boxes->size() <= b->getId() )
	{
		incr = (b->getId()-_boxes->size()) + 10;
		for ( int i=0; i < incr; i++)
			_boxes->push_back(NULL);
	}
	(*_boxes)[b->getId()] = b;

	//	cout << "Catalog::insertBox(" << b << "): (*_boxes)[" << b->getId() << "] = " << (*_boxes)[b->getId()] << endl;


	if ( b->getId() > _max_box_id )
		_max_box_id = b->getId();

	//	cout << "                                 (*_boxes)[" << b->getId() << "] = " << (*_boxes)[b->getId()] << endl
	//		 << endl;
	
}
void Catalog::insertArc(Arcs *a)
{
	_num_arcs++;
	int incr = 10;
	if ( _arcs->size() <= a->getId() )
	{
		incr = (a->getId()-_arcs->size()) + 10;
		for ( int i=0; i < incr; i++)
			_arcs->push_back(NULL);
	}
	(*_arcs)[a->getId()] = a;

	if ( a->getId() > _max_arc_id )
		_max_arc_id = a->getId();
}
void Catalog::printArcs()
{
	printf("ARCS\n");
	printf("====\n");
	printf("id src dest Tuple fields\n");
	for ( int i = 0; i < _arcs->size(); i++ )
	{
		if ( (*_arcs)[i] != NULL )
		{
			printf("%i ",(*_arcs)[i]->getId());

			printf("%i ",(*_arcs)[i]->getSrcId());
			printf("%i ",(*_arcs)[i]->getDestId());
			printf("\n");
		}
	}
	printf("\n----- NETWORK FILE SUCCESSFULLY READ ----- \n\n");
}
void Catalog::printBoxes()
{
}
void Catalog::insertInput(Inputs *inputs)
{
	_num_inputs++;
	_inputs->push_back(inputs);
}
void Catalog::loadFromDB(QueryNetwork *q_net)
{
	_q_net = q_net;
	loadNetwork();
	loadArcs();
	loadBoxes();

	Boxes *b = _catalog->getBox(66);

	//	cout << "Catalog::loadFromDB(...) 1:  b->getType on box id " << 66 << " says " << b->getType() << endl
	//		 <<  "   b = " << b << endl << endl;

	loadInputs();

	//	cout << "Catalog::loadFromDB(...) 2:  b->getType on box id " << 66 << " says " << b->getType() << endl
	//		 <<  "   b = " << b << endl << endl;
}
void Catalog::loadNetwork()
{
	Network *new_net = new Network(_q_net->getNumberOfBoxes(),
						_q_net->getNumberOfInputStreams(),
						_q_net->getNumberOfApplications());
	//	printf("GOT TO NETWORK: numBoxes: %i  NumInputs: %i  NumOutputs: %i\n",
	//				new_net->getNumBoxes(),
	//				new_net->getNumInputs(),
	//				new_net->getNumOutputs());
	this->setNetwork(new_net);
}
void Catalog::loadInputs()
{
  //int num_inputs = _q_net->getNumberOfInputStreams();
	int max_input_id = _q_net->getMaxStreamId();
	//printf("GOT TO max_input_id: %i\n",max_input_id);
	
	Inputs *new_inputs;
	for ( int i = 0; i <= max_input_id; i++ )
	{
		SourceStream *db_input;
		db_input = _q_net->getInputStream(i);
		if ( db_input != 0 )
		{
//printf("GOT INPUT: id: %i  rate: %f  dest: %i\n",db_input->getId(),db_input->getRate(),db_input->getDestId());
			int arc_id = findArcIdBySource(db_input->getId()); // the id of a source stream
                                                       // is a box id .. therefore,
                                                       // we have to find the arc with
                                                       // this source stream's id as
                                                       // its source
			//new_inputs = new Inputs(db_input->getId(),db_input->getRate(),10);
			//new_inputs = new Inputs(arc_id,db_input->getRate(),10);
			new_inputs = new Inputs(arc_id,db_input->getRate(),db_input->getNumberOfTuples());
			_catalog->insertInput(new_inputs);
		}
	}
	
}
int Catalog::findArcIdBySource(int source_id)
{
	int max_arc_id = _q_net->getMaxArcId();
	for ( int i = 0; i <= max_arc_id; i++)
	{
		Arc *db_arc;
		db_arc = _q_net->getArc(i);
		if ( db_arc != 0 )
		{
			if ( source_id == db_arc->getSourceId() )
				return(db_arc->getId());
		}
	}
	printf("AURORA :: ERROR :: Could not findArcIdBySource(%i)\n",source_id);
	abort();
}

/* Thread Per Box mode, init the conditions
   for waking up the box's threads as tuples
   com int
*/
void Catalog::initTPBSignalConditions()
{
	_thread_per_box_mode = true;

	// I know this is wasteful, but conditions are small
	// and are only inited during special mode.
	_tpb_signal_cond = new pthread_cond_t[ _max_box_id + 1 ];
	_tpb_signal_mutex = new pthread_mutex_t[ _max_box_id + 1 ];

	for ( int i = 0; i <= _max_box_id; i++ )
	{
		/* pthread_cond_t pct = PTHREAD_COND_INITIALIZER;
		   _tpb_signal_cond[ i ] = pct; */
		pthread_cond_init( &(_tpb_signal_cond[ i ]), NULL );
		pthread_mutex_init( &(_tpb_signal_mutex[ i ]), NULL );
	}

	//pthread_mutex_init( &_tpb_signal_mutex, NULL );
}

/*
  We are using box id since there is one thread per box
*/

void Catalog::TPBWaitForSignal( int id )
{
	struct timeval now;
	struct timespec timeout;
	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + 5;
	timeout.tv_nsec = now.tv_usec * 1000;

	//cout << "  ****************************WAIT on " << id << endl;
	pthread_mutex_lock( &(_tpb_signal_mutex[ id ]) );
	
	//printf(" Waiting for signal box %d\n", id );
	pthread_cond_timedwait( &(_tpb_signal_cond[ id ]), &(_tpb_signal_mutex[id]), &timeout);
	pthread_mutex_unlock( &(_tpb_signal_mutex[ id ]) );
}

void Catalog::TPBSignal( int id )
{
	//cout << "  ****************************CALL SIGNAL on " << id << endl;
	pthread_mutex_lock( &(_tpb_signal_mutex[ id ]) );

	//printf(" Wake up %d box? \n", id);
	pthread_cond_signal( &(_tpb_signal_cond[ id ]) );
	pthread_mutex_unlock( &(_tpb_signal_mutex[ id ]) );
}

void Catalog::loadArcs()
{
  //int num_arcs = _q_net->getNumberOfArcs();
	int max_arc_id = _q_net->getMaxArcId();

	//	printf("GOT TO num arcs: %i\n",_q_net->getNumberOfArcs());
	//	printf("GOT TO max arc id: %i\n",_q_net->getMaxArcId());
	for ( int i = 0; i <= max_arc_id; i++)
	{
		Arc *db_arc;
		db_arc = _q_net->getArc(i);

		if ( db_arc != 0 )
		{
			int num_tuple_fields;
			int *field_types;
			int *field_sizes;
			CompositeType *schema;
			schema = db_arc->getSchema();
			//			printf("schema: %p\n", schema);
			//			printf("db_arc: %p\n", db_arc);
			//			printf("i: %i\n",i);
			if ( schema == NULL )
			{
			  printf("ERROR:[CATALOG]:FOUND BAD SCHEMA\n");
				abort();
			}
			num_tuple_fields = schema->getNumberOfAttributes();
			field_types = new int[num_tuple_fields];
			field_sizes = new int[num_tuple_fields];
			//			printf("GOT TO ARC id: %i  src: %i  dest: %i\n",i,db_arc->getSourceId(),db_arc->getDestinationId());
			//printf("FIELD TYPES: ");
			for ( int j = 0; j < num_tuple_fields; j++ )
			{
				field_sizes[j] = (schema->getAttribute(j)).m_size;
				TypeManager tm;
				int type;
				type=(schema->getAttribute(j)).m_fieldType;

				if ( type ==  tm.INTEGER )
				{
					field_types[j] = INT_TYPE;
				} 
				else if ( type == tm.FLOAT )
				{
					field_types[j] = FLOAT_TYPE;
				}
				else if ( type == tm.DOUBLE )
				{
					field_types[j] = DOUBLE_TYPE;
				}
				else if ( type == tm.STRING )
				{
					field_types[j] = STRING_TYPE;
				}
				else if ( type == tm.TIMESTAMP )
				  {
				    field_types[j] = TIMESTAMP_TYPE;
				  }
				else
				{
					printf("Aurora ERROR: Bad type from getAttribute\n");
					exit(0);
				}				
			}

			TupleDescription *new_td = new TupleDescription(num_tuple_fields, field_types, field_sizes);
			for ( int j = 0; j < num_tuple_fields; j++ )
				new_td->setFieldName(j,(schema->getAttribute(j)).m_fieldName);
			Arcs *new_arc = new Arcs(i,db_arc->getSourceId(), db_arc->getSourcePortId(), db_arc->getDestinationId(), db_arc->getDestinationPortId(), new_td);

			// Is this paragraph superfluous?  We're only doing it to let the 
			// queue_mon work, but perhaps this is unnecessary. -cjc
			// This isn't for queuemon, it was useful when the data was written to files,
			//  but now this is obsolete... commented out now - eddie
			//if (RuntimeGlobals::getPropsFile()->getBool("Scheduler.queue_mon_flag"))
			//{
			//writeOutQueueDesc(i, new_td);
			//}


			//			printf("GOT TO arc: %p\n",new_arc);
			this->insertArc(new_arc);
		}
	}
}
// This function will write out to queue.arc_id.desc
// a description of the queue schema
void Catalog::writeOutQueueDesc(int arc_id, TupleDescription* t) {
  FILE* fd;
  char queueFilename[FILENAME_MAX]; // The file
  sprintf(queueFilename,"queue.%03d.desc", arc_id);

  fd = fopen(queueFilename, "w+");
  if (fd == NULL) {
    perror("writeOutQueueDesc: WARNING: Failed opening queue description file");
    return;
  }
  // Now write the tuple description stuff in there
  // Format of that file for now
  // line1:
  // x,y
  //   where x and y are ints. x: num fields in tuple. y: bytes per tuple
  // lines after that:
  // type,len
  //  where type is the type, as defined in TupleDescription.H
  //  and len is the length in bytes of that field

  // NEEDS CHECKS FOR GOOD WRITES
  fprintf(fd, "# num of fields, bytes per tuple\n");
  fprintf(fd, "%d,%d\n", t->getNumOfFields(), t->getSize());
  fprintf(fd, "# field type, bytes\n");
  for (int i = 0; i < t->getNumOfFields(); i++) {
    fprintf(fd, "%d,%d\n", t->getFieldType(i), t->getFieldSize(i));
  }
  if (fclose(fd) != 0) {
    perror("writeOutQueueDesc: WARNING: Failed closing queue description file");
  }
}

void Catalog::loadBoxes()
{
  //int num_boxes = _q_net->getNumberOfBoxes();
  int max_box_id = _q_net->getMaxBoxId();

  //cout << "************* Catalog::loadBoxes() **************" << endl;

  for ( int i = 0; i <= max_box_id; i++)
    {
      Box *db_box;
      db_box = _q_net->getBox(i);
      

	  //cout << "************* _q_net->getBox(" << i << ") returned " << db_box << endl;

      if ( db_box != NULL )
	{
	  //printf("GOT BOX id: %i\n",i);
	  int box_type;
	  box_type = convertBoxType(db_box->getBoxType());
			
	  //Predicate *new_pred = NULL;				
		  				
	  int win_size=1;
	  /*
	    switch(box_type)
	    {
	    case WMAP_BOX:
	    win_size = ((WMapBox*)db_box)->getWindowSize();
	    break;
	    case RESAMPLE_BOX:
	    win_size = ((ResampleBox*)db_box)->getWindowSize();
	    break;
	    case JOIN_BOX:
	    win_size = ((JoinBox*)db_box)->getWindowSize();
	    break;
	    }
	  */


	  //vector<Expression*> *e = db_box->getExpression();
	  //printf("GOT TO Expression: %x\n",e);
	  //double selectivity = 0.9; 		// db_box->getSelectivity()
	  //double cost = drand48()/100.0; 	//db_box->getCost()
	  //double cost = 0.01;
	  //double cost = 0.001;
	  
	  //cout << "Catalog::loadBoxes: box_type = " << box_type << ", id = " << i << " BOX COST " << ((double)db_box->getCost()) << endl;
  
	  Boxes *new_box = new_box = new Boxes(i,
					       box_type,
					       (double)db_box->getSelectivity(),
					       (double)db_box->getCost(),
					       win_size,
					       db_box->getPredicate(),
					       db_box->getExpression(), 
					       db_box->getState(), 
					       db_box->getModifier(), 
					       db_box->getDropRate(),
					       db_box); // This allows the QBox to talk to its own Box ! - eddie
	  //			printf("ZZZ: new_box->getCost(): %f\n",new_box->getCost());



	  //cout << "     new_box:       getType = " << new_box->getType() << ", getId = " << new_box->getId() << endl;

	  // Setup members used for tracking selectivity info...
	  new_box->_total_dequeued_tuples = 0;
	  //cout << "     new_box:       getType = " << new_box->getType() << ", getId = " << new_box->getId() << endl;
	  new_box->_total_enqueued_tuples_byport.resize(_q_net->getBoxOutputPortCount(i));

	  //cout << "     new_box:       getType = " << new_box->getType() << ", getId = " << new_box->getId() << endl 
	  //	 << "     new_box = " << reinterpret_cast<void *>(new_box) << endl
	  //	 << endl;

	  int max_arc_id = _q_net->getMaxArcId();
	  for ( int i = 0; i <= max_arc_id; i++)
	  {
		  Arc *db_arc;
		  db_arc = _q_net->getArc(i);
		  
		  if ( db_arc != 0 )
		  {
			  if( db_arc->getDestinationId() == new_box->getId() )
				  new_box->addInputQueueId( i );
		  }
	  }
	  this->insertBox(new_box);
	  
	}
    }

  //cout << "************* Catalog::loadBoxes() DONE **************" << endl;

}
int Catalog::convertBoxType(int type)
{
	int box_type;

	if ( _experiment_flag == true )
	{
		box_type = EXPERIMENT_BOX;
	}
	else
	{
		switch(type)
		{
		case FILTER:
		  box_type = FILTER_BOX;
		  break;
		case MAP:
		  box_type = MAP_BOX;
		  break;
		case RESTREAM:
		  box_type = RESTREAM_BOX;
		  break;
		case UNION:
		  box_type = UNION_BOX;
		  break;
		  //case TUMBLE:
		  //box_type = TUMBLE_BOX;
		  //break;
		case AGGREGATE: // Replaces tumble in terms of id-ing
		  box_type = AGGREGATE_BOX;
		  break;
		  //		case SLIDE:
		  //box_type = SLIDE_BOX;
		  //break;
		  //case XSECTION:
		  //box_type = XSECTION_BOX;
		  //break;
		case BSORT:
		  box_type = BSORT_BOX;
		  break;
				// Bye bye WSORT! - eddie
				//case WSORT:
				//box_type = WSORT_BOX;
				//break;
			case RESAMPLE:
				box_type = RESAMPLE_BOX;
				break;
			case JOIN:
				box_type = JOIN_BOX;
				break;
			case DROP:
				box_type = DROP_BOX;
				break;
			case INPUTPORT:
				box_type = INPUTPORT_BOX;
				break;
		case HELLO_WORLD:
		  box_type = HELLO_WORLD_BOX;
		  break;
		case UPDATE_RELATION:
		  box_type = UPDATE_RELATION_BOX;
		  break;
		case READ_RELATION:
		  box_type = READ_RELATION_BOX;
		  break;
		case LR_UPDATE_RELATION:
		  box_type = LR_UPDATE_RELATION_BOX;
		  break;
		case LR_READ_RELATION:
		  box_type = LR_READ_RELATION_BOX;
		  break;

			default:
			  printf("Aurora ERROR: Bad Box type \n");
				abort();
				break;
		}
	}

	return box_type;
}
/*
Predicate* Catalog::convertBoxPredicate(Predicate* pred)
{
	int op_type;
	switch(pred->getOperator())
	{
		case GT:
			op_type = GREATER_THAN;
			break;
		case LT:
			op_type = LESS_THAN;
			break;
		case EQ:
			op_type = EQUAL;
			break;
		case GEQ:
			op_type = GREATER_OR_EQUAL;
			break;
		case LEQ:
			op_type = LESS_OR_EQUAL;
			break;
		case NEQ:
		default:
		//			printf("Aurora ERROR: Bad Operator type \n");
			abort();
			break;
	}
	int rs_type;
	old_u_type rs_value;
	int rs_value_type;
	int rs_index;
	switch ( pred->getPredicateType() )
	{
		case CONST_PREDICATE:
			rs_type = 0;
			//rs_value = ((ConstPredicate*)pred)->getRSValue();
			switch(((ConstPredicate*)pred)->getRSValueType())
			{
				case INTEGER: 
					rs_value._ival = (((ConstPredicate*)pred)->getRSValue())._ival;
					rs_value_type = INT_TOK;
					break;
				case FLOAT: 
					rs_value._fval = (((ConstPredicate*)pred)->getRSValue())._fval;
					rs_value_type = FLOAT_TOK;
					break;
				case DOUBLE: 
					break;
				case STRING:
				default:
				printf("Aurora ERROR: Bad RS value type \n");
					abort();
					break;
			}
			break;
		case JOIN_PREDICATE:
			rs_index = ((JoinPredicate*)pred)->getLSFieldIndex();
			rs_type = 1;
			break;
		default:
		//	printf("Aurora ERROR: Bad RS type \n");
			abort();
			break;
	}
	
	Predicate *new_pred = new Predicate(pred->getLSFieldIndex(),op_type,rs_type,rs_value,
								rs_value_type,rs_index);
	return new_pred;
							
}
*/
