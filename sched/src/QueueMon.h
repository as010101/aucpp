#ifndef QUEUE_MON_H
#define QUEUE_MON_H

#define FORM_WIDTH 800
#define FORM_HEIGHT 600

#define QBUT_WIDTH 100
#define QBUT_HEIGHT 30

#define START_QUEUE_WIDTH 300
#define START_QUEUE_HEIGHT 300

#define QUEUE_MON_GRAY_ON_REMOVE 1

#define QUEUE_MON_DEFAULT_TUPLE_COLOR "@C2@f" // @f requests a fixed font
#define QUEUE_MON_GRAY_TUPLE_COLOR "@C3@f" // C14 is RED, C3 is YELLOW

#define VERIFY_DUPLICATES 0 // set to 1 for queuemon to verify that no duplicate tuples come in
                            // useful, unless "tuples with same values" is a possible normal thing

#include "global.H"
#include <stdlib.h>
#include <unistd.h>

// Modified by cjc on January 24 to enable compilation
//#include "forms.h"
#include <forms.h>

#include <pthread.h>
#include <StreamThread.H>

class StreamThread;
class QueueMon {

public:
  QueueMon(int num_queues);
  QueueMon(QueryNetwork *q_net,
	   StreamThread *stream_thread,
	   int catalog_max_arc_id);
  QueueMon(QueryNetwork *q_net,
	   StreamThread *stream_thread,
	   Catalog *catalog);
  ~QueueMon();
  void initialize_form();
  void *run();
  void start(int input_parameter);
  static void *entryPoint(void *pthis);
  static const char *intFilter(FL_OBJECT *ob, double value, int prec);
  static void slider_cb(FL_OBJECT *ob, long data);
  static void thumbwheel_cb(FL_OBJECT *ob, long data);
  static int closecpanel_cb(FL_FORM *f, void* v);
  static void toggleRunState_cb(FL_OBJECT *ob, long data);
  static void toggleQueueState_cb(FL_OBJECT *ob, long queue_id);
  static void toggleQueueDir_cb(FL_OBJECT *ob, long data);
  static void toggleQueueFontSize_cb(FL_OBJECT *ob, long data);
  static void toggleRate_cb(FL_OBJECT *ob, long data);
  static void swapQueueDirections(QueueMon* q);
  bool isRunning();
  void setRunning(bool b);
  void addTuple(char* t, TupleDescription* td, int queue_id);
  void removeTuple(int queue_id);
  void setNumMessages(int n);
  void tickMessage();
  void ping();
  int getusecRate();
private:
  StreamThread *_st; // Access to the stream thread generator
  QueryNetwork *_qnet; // Access to the Query Netowrk

  FL_FORM *_form; // Main form - DEPRECATED. There is no MAIN FORM anymore.
  FL_OBJECT *_formbrowser; // to hold all the other forms
  FL_FORM *_cpanel; // Control Panel (light switches)
  FL_FORM *_runcpanel; // Control Panel for running, quitting, etc

  FL_FORM *_folderform; // Holds the tab folder
  FL_OBJECT *_tabfolder; // A new control panel using tab folders to hold queue controls and runtime stuff
  FL_FORM *_cpanel_inputs; // three tabs to hold the queue switches for input/output/intermediate queues
  FL_FORM *_cpanel_outputs;
  FL_FORM *_cpanel_intermediate;
  FL_FORM *_cpanel_options;

  FL_OBJECT *_queuefontchoice; // Queue font size choice
  char _queue_font_choice;
  FL_OBJECT *_queuedirchoice; // Queue direction choice
  FL_OBJECT *_quitbut; // Quit button
  FL_OBJECT *_runbut; // Start/Pause button
  bool _running; // Whether the user is requesting the system to be running or not
  /** Note: currently we are busy-waiting and checking this variable (well, we sleep in between)
      and currently only StreamThread (the input generator) is paused - other processing is not paused
      We should also pause Scheduler, obviously.
  */
  FL_OBJECT *_topnslider; // The slider/wheel to choose how many tuples per queue
  FL_FORM **_queueforms; // Each queue will be in a form, so you can place wherever you want on screen
  FL_OBJECT **_queues; // Array of textviewers (browsers, 1 per queue)
  char** qlabels;
  bool* qvisible; // Whether they are visible or not
  FL_OBJECT **_queuesOnOffbut;
  char** qbutlabels;
  char** qbutshortcuts;
  FL_OBJECT *_messagedial;
  FL_OBJECT *_ratedial;

  bool _tuples_from_above; // Whether tuples enter the queues visually from above or from below
  int* _currline; // How many lines are currently displayed per queue
  void add_n_setobjects_main();
  void add_n_set_queue(int queue_index, bool moreThanTen);
  void add_n_setobjects_cpanel();
  void add_n_setobjects_cpanel_type(char type);
  void add_n_setobjects_runcpanel();
  void redrawQueues(); // Check which are hidden/visible, and redraw - DEPRECATED
  void repositionQueues(); // Check which queue windows are visible, and reposition them
  int	_stop;
  int _num_queues;
  //int *_arc_ids; // Queue index (0,1,2...) to Arc Id (0,4,64...)
  //int *_arc_ids_rev; // Arc id to queue index
  // BUG BUG BUG HACK HACK HACK this should be dynamic. why when i do it i get segfault? Dunno
  int _arc_ids[128];
  int _arc_ids_rev[128];
  char _queue_type[128]; // Tell if an arc is an [i]nput, [m]iddle (intermediate), [o]utput
  pthread_t _thread;
  pthread_mutex_t 	*_qmon_mutex;
  // If we choose to control StreamThread using thread control. For now, busy-waiting.
  //pthread_mutex_t *_run_mutex; 
  //pthread_cond_t *_run_cond;

  int _input_parameter;
  int _num_messages;
  int _showntuples; // This many tuples will actually be shown (top tuples only)

  int _usecRate; // Temporary rate control
};



#endif
