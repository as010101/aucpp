static char *cvs_id="@(#) $Id: PriorityGrid.C,v 1.6 2003/07/04 02:40:02 alexr Exp $";
#include "PriorityGrid.H"

// GridElement methods
GRID_ELEMENT_BOX_ITER GridElement::push_back(int box_id)
{
	return(_box_list.insert(_box_list.end(), box_id));
}
void GridElement::erase(GRID_ELEMENT_BOX_ITER iter)
{
	_box_list.erase(iter);
}

// PriorityGrid methods
PriorityGrid::PriorityGrid(int num_buckets)
{
	_num_buckets = num_buckets;
	_grid = new GridElement*[_num_buckets];
	for ( int i = 0; i < _num_buckets; i++ )
	{
		_grid[i] = new GridElement[_num_buckets];
	}
	_priority_bucket_v = new vector<GridElement*>;
	_urgency_list_v = new vector<UrgencyStruct>;

}
GridElement* PriorityGrid::getGridElement(int slack_bucket, int slope_bucket)
{
	return &(_grid[slack_bucket][slope_bucket]);
}
void PriorityGrid::printGrid()
{
	printf("Priority Grid\n");
	for ( int slope = _num_buckets-1; slope >= 0; slope-- )
	{
		for ( int slack = 0; slack < _num_buckets; slack++ )
		{
			printf("(");
			GRID_ELEMENT_BOX_ITER iter;
			for ( iter = _grid[slack][slope]._box_list.begin();
				iter != _grid[slack][slope]._box_list.end();
				iter++ )
			{
				if ( iter != _grid[slack][slope]._box_list.begin())
					printf(",");
				printf("%i",*iter);
			}
			printf(")");
		}
		printf("\n");
	}
}
void PriorityGrid::printGridPointers()
{
	printf("Priority Grid\n");
	for ( int slope = _num_buckets-1; slope >= 0; slope-- )
	{
		for ( int slack = 0; slack < _num_buckets; slack++ )
		{
			printf("(%p)",&_grid[slack][slope]);
		}
		printf("\n");
	}
}
void PriorityGrid::createPriorityVector()
{
	for ( int slope_bucket = _num_buckets-1 ; slope_bucket >= 0; slope_bucket-- )
	{
		for ( int slack_bucket = 0; slack_bucket < _num_buckets; slack_bucket++ )
		{
			if ( _grid[slack_bucket][slope_bucket]._candidate_boxes.size() > 0 )
			{
				_priority_bucket_v->push_back(getGridElement(slack_bucket,slope_bucket));
			}
		}
	}
}
void PriorityGrid::createUrgencyList()
{
	for ( int slope_bucket = _num_buckets-1 ; slope_bucket >= 0; slope_bucket-- )
	{
		for ( int slack_bucket = 0; slack_bucket < _num_buckets; slack_bucket++ )
		{
			if ( _grid[slack_bucket][slope_bucket]._candidate_boxes.size() > 0 )
			{
				for ( int i = 0; i <  _grid[slack_bucket][slope_bucket]._candidate_boxes.size(); i++ )
				{
					UrgencyStruct us;
					us._ge = &(_grid[slack_bucket][slope_bucket]);
					us._box_id = _grid[slack_bucket][slope_bucket]._candidate_boxes[i];
					_urgency_list_v->push_back(us);
				}
			}
		}
	}
}

void GridPointers::removeFromGrid(int box_id)
{
	BoxLatencyGridPointers *blgp = &_box_latency_grid_map[box_id];
	assert ( blgp->_current_grid_location != NULL );
	(blgp->_current_grid_location)->erase(blgp->_current_list_iter);
	blgp->_current_grid_location = NULL;
	blgp->_current_list_iter = NULL;

}
GridElement* GridPointers::loadPriorityAssignment(int box_id, double latency)
{
	// assumption .. latency bucket 0 is always valid
	int latency_bucket;

	BoxLatencyGridPointers *blgp = &_box_latency_grid_map[box_id];
	//cout << " priority assignment  blgm for " << box_id << endl;


	if (latency > _max_latency)
	{
		//fprintf(stderr,"loadPriorityAssignment():1: This tuple should be ejected, it has no value!!\n");
		latency_bucket = 0; // tuples should really be ejected
		//latency_bucket = _num_buckets-1; // tuples should really be ejected
		//latency_bucket = blgp->_max_grid_slot; // tuples should really be ejected
	}
	else
	{
		latency_bucket = (int)((latency/_max_latency)*_num_buckets);
		if (latency_bucket == _num_buckets)
		{
			//latency_bucket = _num_buckets-1;
			latency_bucket = blgp->_max_grid_slot;
		}
	}
	//printf("latency: %f _max_latency: %f   latency_bucket: %i\n",latency,_max_latency, latency_bucket);
	assert (latency_bucket < _num_buckets);

	if ( blgp->_current_grid_location != NULL && blgp->_current_grid_location == blgp->_grid_pointers[latency_bucket] )
	{
		return (blgp->_current_grid_location);
	}
	if ( blgp->_current_grid_location != NULL )
	{
		(blgp->_current_grid_location)->erase(blgp->_current_list_iter);
	}
	if ( blgp->_grid_pointers[latency_bucket] == NULL )
	{
		//fprintf(stderr,"loadPriorityAssignment():2: This tuple should be ejected, it has no value!!\n");
		latency_bucket = 0; // tuples should really be ejected
		//latency_bucket = _num_buckets-1; // tuples should really be ejected
		//latency_bucket = blgp->_max_grid_slot; // tuples should really be ejected
	}
	blgp->_current_grid_location = blgp->_grid_pointers[latency_bucket];
	//fprintf(stderr,"latency_bucket: %i\n",latency_bucket);
	assert(blgp->_current_grid_location != NULL );
	blgp->_current_list_iter = (blgp->_current_grid_location)->push_back(box_id);
	assert(blgp->_current_grid_location!=NULL);
	return (blgp->_current_grid_location);

}
void GridPointers::loadGridPointers(int box_id, qos_struct *qos_graph)
{
	int slack_bucket,slope_bucket;
	double latency_step = _max_latency/(double)_num_buckets;
	BoxLatencyGridPointers blgp;
	for ( int i = 0; i < _num_buckets; i++ )
	{
		double latency_start = (double)i * latency_step;
		double latency_stop = (double)(i+1) * latency_step;
		slack_bucket = calcSlackBucket(latency_start, latency_stop, qos_graph);
		assert (slack_bucket < _num_buckets);
		//printf("box_id: %i  (%f->%f): slack_bucket: %i\n",box_id,latency_start,latency_stop,slack_bucket);
		slope_bucket = calcSlopeBucket(latency_start, latency_stop, qos_graph);
		assert (slope_bucket < _num_buckets);
		//printf("box_id: %i  (%f->%f): slope_bucket: %i\n",box_id,latency_start,latency_stop,slope_bucket);
		GridElement* ge;
		if ( slope_bucket >= 0 && slack_bucket >= 0 )
		{
			ge = _priority_grid->getGridElement(slack_bucket, slope_bucket);
			//printf("ge[%i][%i]: %i  ge[0][3]: %i ge[1][3]: %i\n",slack_bucket,slope_bucket,ge,_priority_grid->getGridElement(0,3),_priority_grid->getGridElement(1,3));
			ge->_candidate_boxes.push_back(box_id);
			blgp._max_grid_slot = i;
		}
		else
			ge = NULL;
		blgp._grid_pointers.push_back(ge);
	}
	_box_latency_grid_map[box_id]=blgp;
}
void GridPointers::printBoxGridPointers(int box_id)
{
	BoxLatencyGridPointers *blgp = &_box_latency_grid_map[box_id];
	printf("BoxGridPointers:box_id(%i)\n",box_id);
	for ( int i = 0; i < _num_buckets; i++ )
	{
		printf("(%p)",blgp->_grid_pointers[i]);
	}
	printf("\n");
}
int GridPointers::calcSlackBucket(double latency_start, double latency_stop, qos_struct *qos_graph)
{
	// make sure latency start is not beyond the last point in qos_graph
	if ( qos_graph->_points[qos_graph->_points.size()-1]->_x <= latency_start )
			return -1;

	double largest_slack = 0.0;
	for (int i = 0; i < qos_graph->_points.size(); i++ )
	{
		//cout << " Latency Start " << latency_start  << " i " << i << " Qos graph x " << qos_graph->_points[i]->_x << endl;
		if ( qos_graph->_points[i]->_x > latency_start )
		{
			assert(i > 0);
			double slack;
			if ( qos_graph->_points[i-1]->_x >= latency_start )
				slack = qos_graph->_points[i]->_x - qos_graph->_points[i-1]->_x;
			else
				slack = qos_graph->_points[i]->_x - latency_start;

			if ( slack > largest_slack )
				largest_slack = slack;
			if ( qos_graph->_points[i]->_x >= latency_stop )
				break;
		}
	}

	int slack_bucket = (int)((largest_slack/_max_slack)*(_num_buckets));
	if (slack_bucket == _num_buckets)
			slack_bucket = _num_buckets - 1;
	return slack_bucket;
}
int GridPointers::calcSlopeBucket(double latency_start, double latency_stop, qos_struct *qos_graph)
{
	// make sure latency start is not beyond the last point in qos_graph
	if ( qos_graph->_points[qos_graph->_points.size()-1]->_x <= latency_start )
			return -1;
	double largest_slope = 0.0;
	for (int i = 0; i < qos_graph->_points.size(); i++ )
	{
		if ( qos_graph->_points[i]->_x > latency_start )
		{
			assert(i > 0);
			double slope = - (qos_graph->_points[i]->_y - qos_graph->_points[i-1]->_y)/
						(qos_graph->_points[i]->_x - qos_graph->_points[i-1]->_x);

			if ( slope > largest_slope )
				largest_slope = slope;
			if ( qos_graph->_points[i]->_x >= latency_stop )
				break;
		}
	}
	
	largest_slope = fabs(largest_slope);
	int slope_bucket = (int)((largest_slope/_max_slope)*(_num_buckets-1));
	if (slope_bucket == _num_buckets)
			slope_bucket = _num_buckets - 1;
	return slope_bucket;
}
