//static char *cvs_id="@(#) $Id: SysMon.C,v 1.14 2003/03/26 19:06:25 cjc Exp $";
#include "SysMon.H"
#include <unistd.h>
#include <Shared.H>
#include <sys/times.h>

// I needed to declare this to get CLK_TCK defined on Mandrake 9.0/GCC 3.2. 
// time.h has the following content:
//# if !defined __STRICT_ANSI__ && !defined __USE_XOPEN2K
//#  ifndef CLK_TCK
//#   define CLK_TCK	CLOCKS_PER_SEC
//#  endif
//# endif
//
// This is maybe a side effect of my alteration to the Makefile to #define
// the symbol "SVR4", so that we don't get complaints when using forms.h.
// If it's a big deal to clean this up, we need some way of getting a clean
// inclusion of forms.h, without preventing the definition of CLK_TCK.
//
// Alternatively, we could just start using CLOCKS_PER_SEC instead of
// CLK_TCK, and maybe that would most cleanly do the trick.
//
// -cjc January 25
#  ifndef CLK_TCK
#   define CLK_TCK	CLOCKS_PER_SEC
#  endif

extern Catalog *_catalog;
extern Shared *shm_ptr;

/*
void *TestThread::run()
{
  printf("GOT TO  thread: %i\n", _input_parameter);

  return (void *)0;

}
*/

void *SysMon::entryPoint(void *pthis)
{
  printf("GOT TO entryPoint()\n");
  SysMon *pt = (SysMon*)pthis;
  pt->run();

  return (void *)0;
}

void SysMon::start(int input_parameter)
{
printf("GOT TO A\n");
    _input_parameter = input_parameter;
    pthread_create( &_thread, 0, SysMon::entryPoint, this);

}



SysMon::SysMon(int num_chart_vals, StreamThread *st, Scheduler *sched)
{
	_stop = 0;
	_num_chart_vals = num_chart_vals;
	_st = st;
	_sched = sched;
	
}
void *SysMon::run()
{
  printf("GOT TO SysMon::run()\n");
  for ( int j = 0; j < _num_chart_vals; j++ )
    {
      printf("GOT TO SysMon A\n");
      if ( _catalog->getArc(j) != NULL )
	_queue_toggle_v.push_back(1);
      else
	_queue_toggle_v.push_back(0);
    }
  printf("GOT TO SysMon B\n");
  initialize_form();
  printf("GOT TO SysMon B1\n");
  init_chart();
  FL_OBJECT *obj;
  int c;
  char item_string[25];
  int counter = 0;

  int _app_slope_button_set = 0;
  int _app_random_button_set =0;
  int _app_rr_button_set = 0;
  int _box_slope_button_set = 0;
  int _box_random_button_set =0;
  int _box_rr_button_set =0;
  int _sched_by_app_button_set = 0;
  int _sched_by_box_button_set = 0;

  printf("GOT TO SysMon C\n");
  switch(_sched->getAppScheduleType())
    {
    case APP_RANDOM_TYPE:
      _app_random_button_set = 1;
      fl_set_button(_app_random_button,1);
      break;
    case APP_RR_TYPE:
      _app_rr_button_set = 1;
      fl_set_button(_app_rr_button,1);
      break;
    case APP_SLOPE_SLACK_TYPE:
      _app_slope_button_set = 1;
      fl_set_button(_app_slope_button,1);
      break;
    default:
      printf("[SYSMON]: Bad AppScheduleType\n");
      exit(0);
      break;
    }
  printf("GOT TO SysMon D\n");
  switch(_sched->getBoxScheduleType())
    {
    case BOX_RANDOM_TYPE:
      _box_random_button_set = 1;
      fl_set_button(_box_random_button,1);
      break;
    case SLOPE_SLACK_TYPE:
      _box_slope_button_set = 1;
      fl_set_button(_box_slope_button,1);
      break;
    case BOX_RR_TYPE:
      _box_rr_button_set = 1;
      fl_set_button(_box_rr_button,1);
      break;
    default:
      printf("[SYSMON]: Bad BoxScheduleType\n");
      exit(0);
      break;
    }
  printf("GOT TO SysMon::run() C\n");
  switch(_sched->getSchedBy())
    {
    case SCHED_BY_APP:
      _sched_by_app_button_set = 1;
      fl_set_button(_sched_by_app_button,1);
      break;
    case SCHED_BY_BOX:
      _sched_by_box_button_set = 1;
      fl_set_button(_sched_by_box_button,1);
      break;
    default:
      printf("[SYSMON]: Bad SchedByType\n");
      exit(0);
      break;
    }

  int start_time = time(NULL);
  int now;
  int time_diff;
  double dial_val;
  int q_state;
  long ticks;
  double tpt; // time per tuple (seconds);
  do
    {
      printf("GOT TO SysMon::B()\n");
      if ( counter % 5 == 0 )
	{
	  double *chart_vals;
	  double max_value;
			
	  chart_vals = get_chart_vals(&max_value);
			
	  double min,max;
	  fl_get_chart_bounds(_barchart,&min,&max);
	  printf("GOT TO max: %lf\n",max);
	  int bar_count = 0;
	  for ( int i = 0; i < _num_chart_vals; i++ )
	    {
	      printf("%lf ",chart_vals[i]);
	      if ( _queue_toggle_v[i] != 0 )
		{
		  q_state = shm_get_state(i);
		  if ( q_state == 0 )
		    c = FL_RED; // not scheduled
		  else if ( q_state == 1 )
		    c = FL_BLUE; // scheduled
		  else if ( q_state == 2 )
		    c = FL_YELLOW; // in worker thread
		  /*
		    if ( chart_vals[i] > (0.66*max_value) )
		    c = FL_RED;
		    else if ( chart_vals[i] > (0.33*max_value) )
		    c = FL_YELLOW;
		    else
		    c = FL_BLUE;
		  */
		  if ( _num_chart_vals <= 30 )
		    sprintf(item_string,"%i (%i)\n",i,(int)chart_vals[i]);
		  else
		    {
		      if ( chart_vals[i] > (0.33*max_value)  )
			sprintf(item_string,"%i (%i)",i,(int)chart_vals[i]);
		      //else if ( i % 10 == 0 )
		      //sprintf(item_string,"%i (%i)\n",i,(int)chart_vals[i]);
		      else
			// Corrected this because of format string / parms mismatch. -cjc 17 Feb 2003
			//sprintf(item_string,"",(int)chart_vals[i]);
			strcpy(item_string, "");

		      //sprintf(item_string,"(%i)",(int)chart_vals[i]);
		    }
		  fl_replace_chart_value(_barchart,bar_count+1,chart_vals[i],item_string,c);
		  bar_count++;
		}
	    }
	  now = time(NULL);
	  time_diff = now - start_time;
	  int tg = _st->getTuplesGenerated();
	  double rate = 0.0;
	  if (time_diff > 0)
	    rate = (double)tg/time_diff;
	  sprintf(item_string,"%.1f",rate);
	  fl_set_object_label(_rate_box,item_string);
	  sprintf(item_string,"%i",time_diff);
	  fl_set_object_label(_elapsed_time_box,item_string);
	  sprintf(item_string,"%i",tg);
	  fl_set_object_label(_tuples_generated_box,item_string);
	  dial_val = fl_get_dial_value(_ratedial);
	  sprintf(item_string,"%.1f",dial_val);
	  fl_set_object_label(_rate_multiplier_box,item_string);
	  _st->setRateMultiplier(dial_val);

	  if ( fl_get_button(_app_slope_button) != _app_slope_button_set ||
	       fl_get_button(_app_random_button) != _app_random_button_set ||
	       fl_get_button(_app_rr_button) != _app_rr_button_set )
	    {
	      _app_slope_button_set = fl_get_button(_app_slope_button);
	      _app_random_button_set = fl_get_button(_app_random_button);
	      _app_rr_button_set = fl_get_button(_app_rr_button);
	      if (_app_slope_button_set)
		{
		  _sched->setAppScheduleType(APP_SLOPE_SLACK_TYPE);
		}
	      else if (_app_random_button_set)
		{
		  _sched->setAppScheduleType(APP_RANDOM_TYPE);
		}
	      else if (_app_rr_button_set)
		{
		  _sched->setAppScheduleType(APP_RR_TYPE);
		}
	    }

	  if ( fl_get_button(_box_slope_button) != _box_slope_button_set ||
	       fl_get_button(_box_random_button) != _box_random_button_set ||
	       fl_get_button(_box_rr_button) != _box_rr_button_set )
	    {
	      _box_slope_button_set = fl_get_button(_box_slope_button);
	      _box_random_button_set = fl_get_button(_box_random_button);
	      _box_rr_button_set = fl_get_button(_box_rr_button);
	      if (_box_slope_button_set)
		{
		  _sched->setBoxScheduleType(SLOPE_SLACK_TYPE);
		}
	      else if (_box_random_button_set)
		{
		  _sched->setBoxScheduleType(BOX_RANDOM_TYPE);
		}
	      else if (_box_rr_button_set)
		{
		  _sched->setBoxScheduleType(BOX_RR_TYPE);
		}
	    }

	  if ( fl_get_button(_sched_by_app_button) != _sched_by_app_button_set ||
	       fl_get_button(_sched_by_box_button) != _sched_by_box_button_set )
	    {
	      _sched_by_app_button_set = fl_get_button(_sched_by_app_button);
	      _sched_by_box_button_set = fl_get_button(_sched_by_box_button);
	      if ( _sched_by_app_button_set)
		{
		  _sched->setSchedBy(SCHED_BY_APP);
		}
	      else if ( _sched_by_box_button_set)
		{
		  _sched->setSchedBy(SCHED_BY_BOX);
		}
	    }

	  ticks = _sched->getSchedulingTicks();
	  //tpt = (ticks/(double)CLK_TCK)/tg;
	  tpt = (ticks/(double)CLK_TCK);
	  fl_insert_chart_value(_stpt_chart,1,tpt,"",1);
	  sprintf(item_string,"%f",tpt);
	  fl_set_object_label(_stpt_box,item_string);
	  /*
	    int static test_counter = 0;
	    if ( test_counter % 20 == 0 )
	    {
	    sprintf(item_string,"%f",tpt);
	    fl_replace_chart_value(_stpt_chart,1,tpt,item_string,c);
	    test_counter++;
	    }
	  */
	  // double max,min;
	  //fl_get_chart_bounds(_stpt_chart,&max,&min);
	  //fl_insert_chart_value(_qos_chart,1,0.5,"",1);


	  printf("\n");
	  printf("GOT TO tuples generated: %i\n",_st->getTuplesGenerated());
	}
      obj = fl_check_forms();
      //usleep(100000);
      usleep(50000);
      counter++;
    } while (obj != _quitbut && _stop != 1);
  cleanup(shmid,semid);
  exit(0);
  return (void *)0;
}


void SysMon::init_chart()
{
  int c = FL_BLUE;
  char item_string[25];
  for ( int i = 0; i < _num_chart_vals; i++ )
    {
      //double val = drand48();
      if (_queue_toggle_v[i] != 0)
	{
	  double val = shm_ptr[i]._num_records_in_queue;
	  sprintf(item_string,"S%i (%i)\n",i,(int)val);
	  fl_add_chart_value(_barchart,drand48(),item_string,c);
	}
    }
}
double *SysMon::get_chart_vals(double *max_value)
{
  double *vals = new double[_num_chart_vals];
  *max_value = 0.0;
  for ( int i = 0; i < _num_chart_vals; i++ )
    {
      //vals[i] = drand48();
      vals[i] = shm_ptr[i]._num_records_in_queue;
      if ( vals[i] > *max_value)
	*max_value = vals[i];
    }
  return vals;
}
void SysMon::initialize_form()
{

	printf("GOT TO SysMon::initialize_form A\n");
	const char ** argv;
	argv = new const char *[1];
	argv[0] = "test";
	int one = 1;
	printf("GOT TO SysMon::initialize_form B\n");
	fl_initialize(&one, const_cast<char **>(argv), "FormDemo", 0, 0);
	printf("GOT TO SysMon::initialize_form C\n");
	//fl_init();
	FL_OBJECT *tmp_object1;
	FL_OBJECT *tmp_object2;
	FL_OBJECT *tmp_object3;
	printf("GOT TO SysMon::initialize_form D\n");
	_form = fl_bgn_form(FL_UP_BOX, 900, 500);
	{
		_quitbut = fl_add_button(FL_NORMAL_BUTTON, 400, 450, 50, 30,"Quit");
		_barchart =  fl_add_chart(FL_BAR_CHART,20,20,860,240,"QUEUES");
		_ratedial = fl_add_dial(FL_LINE_DIAL,20,400,60,60,"Rate Multiplier");
		tmp_object1 =			fl_add_box(FL_FLAT_BOX,10,280,110,30,"Rate(tps):");
		tmp_object2 =			fl_add_box(FL_FLAT_BOX,10,315,110,30,"Tuples Generated:");
		tmp_object3 = 			fl_add_box(FL_FLAT_BOX,10,350,110,30,"Elapsed Time:");
		_rate_box = 			fl_add_box(FL_DOWN_BOX,120,280,100,30,"");
		_tuples_generated_box =	fl_add_box(FL_DOWN_BOX,120,315,100,30,"");
		_elapsed_time_box = 	fl_add_box(FL_DOWN_BOX,120,350,100,30,"");
		_stpt_box = 	fl_add_box(FL_DOWN_BOX,620,400,100,30,"stpt");
		_stpt_chart = fl_add_chart(FL_LINE_CHART,620,280,220,100,"STPT");
		//_qos_chart = fl_add_chart(FL_LINE_CHART,700,400,200,120,"QoS");
		_rate_multiplier_box = 	fl_add_box(FL_DOWN_BOX,90,410,100,30,"1.0");

		fl_bgn_group();
		FL_OBJECT *obj;
		int box_x = 300;
		int box_y = 280;
		obj = fl_add_box(FL_BORDER_BOX,box_x,box_y,300,55,"App Schedule Type");
		fl_set_object_lalign(obj,FL_ALIGN_TOP | FL_ALIGN_INSIDE);
		_app_slope_button = 	fl_add_checkbutton(FL_RADIO_BUTTON,box_x+10 ,box_y+20,50,40,"Slope");
		_app_random_button = 	fl_add_checkbutton(FL_RADIO_BUTTON,box_x+90,box_y+20,50,40,"Random");
		_app_rr_button = 		fl_add_checkbutton(FL_RADIO_BUTTON,box_x+180,box_y+20,50,40,"Round Robin");
		//fl_set_button(_app_slope_button,1);
		fl_end_group();

		fl_bgn_group();
		box_x = 300;
		box_y = 336;
		obj = fl_add_box(FL_BORDER_BOX,box_x,box_y,300,55,"Box Schedule Type");
		fl_set_object_lalign(obj,FL_ALIGN_TOP | FL_ALIGN_INSIDE);
		_box_slope_button = 	fl_add_checkbutton(FL_RADIO_BUTTON,box_x+10 ,box_y+20,50,40,"Slope");
		_box_random_button = 	fl_add_checkbutton(FL_RADIO_BUTTON,box_x+90,box_y+20,50,40,"Random");
		_box_rr_button = 	    fl_add_checkbutton(FL_RADIO_BUTTON,box_x+180,box_y+20,50,40,"RR");
		//fl_set_button(_box_slope_button,1);
		fl_end_group();

		fl_bgn_group();
		box_x = 240;
		box_y = 280;
		obj = fl_add_box(FL_BORDER_BOX,box_x,box_y,59,111,"");
		fl_set_object_lalign(obj,FL_ALIGN_TOP | FL_ALIGN_INSIDE);
		_sched_by_app_button = 	fl_add_checkbutton(FL_RADIO_BUTTON,box_x+10 ,box_y+20,50,40,"App");
		fl_set_object_lalign(_sched_by_app_button,FL_ALIGN_TOP | FL_ALIGN_INSIDE);
		//fl_set_button(_sched_by_app_button,0);
		_sched_by_box_button = 	fl_add_checkbutton(FL_RADIO_BUTTON,box_x+10,box_y+70,50,40,"Box");
		//fl_set_button(_sched_by_box_button,1);
		fl_set_object_lalign(_sched_by_box_button,FL_ALIGN_TOP | FL_ALIGN_INSIDE);
		fl_end_group();

		 fl_set_object_lsize(_rate_box,FL_LARGE_SIZE);
		 fl_set_object_lsize(_tuples_generated_box,FL_LARGE_SIZE);
		 fl_set_object_lsize(_elapsed_time_box,FL_LARGE_SIZE);

		fl_set_object_lalign(_rate_box,  FL_ALIGN_INSIDE | FL_ALIGN_RIGHT );
		fl_set_object_lalign(_tuples_generated_box,  FL_ALIGN_INSIDE | FL_ALIGN_RIGHT );
		fl_set_object_lalign(_elapsed_time_box,  FL_ALIGN_INSIDE | FL_ALIGN_RIGHT );
		fl_set_object_lalign(tmp_object1,  FL_ALIGN_INSIDE | FL_ALIGN_RIGHT );
		fl_set_object_lalign(tmp_object2,  FL_ALIGN_INSIDE | FL_ALIGN_RIGHT );
		fl_set_object_lalign(tmp_object3,  FL_ALIGN_INSIDE | FL_ALIGN_RIGHT );

		fl_set_dial_value(_ratedial,(double)1.0);
		fl_set_dial_bounds(_ratedial,(double)1.0,(double)1000.0);
		fl_set_object_color(_ratedial,FL_RED,FL_DIAL_COL2);

		//fl_set_chart_bounds(_stpt_chart,0.0,1.0);
		fl_set_chart_maxnumb(_stpt_chart,80);
		fl_set_chart_autosize(_stpt_chart,0);

		//fl_set_chart_bounds(_qos_chart,0.0,1.0);
		//fl_set_chart_maxnumb(_qos_chart,80);
		//fl_set_chart_autosize(_qos_chart,0);
	}
	fl_end_form();
	fl_show_form(_form,FL_PLACE_MOUSE,FL_TRANSIENT,"Question");

}

/*
int main(int argc, char **argv)
{
	SysMon *sm1 = new SysMon(10); 
	sm1->start(5);


	for ( int i = 0; i < 10; i++ )
	{
		printf("GOT TO MAIN >>>>>>>>>>>>> sleeping .........\n");
    	sleep(1);
	}

}
*/

