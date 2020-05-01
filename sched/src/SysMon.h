#ifndef SYS_MON_H
#define SYS_MON_H


#include "global.H"
#include <stdlib.h>
#include <unistd.h>

// Corrected by cjc on January 24 to enable compilation.
//#include "forms.h"
#include <forms.h>

#include <pthread.h>
#include <StreamThread.H>
#include "Scheduler.H"


class SysMon
{
public:
	SysMon(int num_chart_vals, StreamThread *st, Scheduler *sched);
	~SysMon();
	void initialize_form();
	void init_chart();
	double *get_chart_vals(double *max_value);
	void *run();
    void start(int input_parameter);
    static void *entryPoint(void *pthis);
private:
	FL_FORM *_form;
	FL_OBJECT *_barchart;
	FL_OBJECT *_quitbut;
	FL_OBJECT *_ratedial;
	FL_OBJECT *_rate_box;
	FL_OBJECT *_tuples_generated_box;
	FL_OBJECT *_elapsed_time_box;
	FL_OBJECT *_rate_multiplier_box;
	FL_OBJECT *_qos_chart;
	FL_OBJECT *_stpt_chart; // scheduling time per tuple chart
	FL_OBJECT *_stpt_box; // scheduling time per tuple chart
	FL_OBJECT *_app_slope_button;
	FL_OBJECT *_app_rr_button;
	FL_OBJECT *_app_random_button;
	FL_OBJECT *_box_slope_button;
	FL_OBJECT *_box_random_button;
	FL_OBJECT *_box_rr_button;
	FL_OBJECT *_sched_by_app_button;
	FL_OBJECT *_sched_by_box_button;
	int	_stop;
	int _num_chart_vals;
    pthread_t        _thread;
    int         _input_parameter;
	StreamThread *_st;
	Scheduler *_sched;
	vector<int> _queue_toggle_v;
};

#endif
