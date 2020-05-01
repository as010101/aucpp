#include "QueueMon.H"
#include <unistd.h>
#include <Shared.H>
#include <sys/time.h>
#include <iostream>
#include <util.H>
extern Catalog *_catalog;
extern Shared *shm_ptr;


void *QueueMon::entryPoint(void *pthis)
{
  QueueMon *pt = (QueueMon*)pthis;
  pt->run();

  return (void *)0;
}

void QueueMon::start(int input_parameter)
{
  /** THREAD STUFF (to avoid Xlib errors) */
  //  #include <X11/Intrinsic.h>
  //  XtToolkitThreadInitialize();
  //  XInitThreads(); Doesnt really work, not sure why... hangs on any keyboard input event

  _input_parameter = input_parameter;
  pthread_create( &_thread, 0, QueueMon::entryPoint, this);

}

QueueMon::QueueMon(int num_queues) {
  
  printf("[QueueMon] Fatal! Calling deprecated constructor (does not count arcs correctly)\n");
  exit(-1);

  _qmon_mutex = new pthread_mutex_t;
  pthread_mutex_init(_qmon_mutex,NULL);
  //_run_mutex = new pthread_mutex_t;
  //_run_cond = new pthread_cond_t;
  //pthread_mutex_init(_run_mutex,NULL);
  //pthread_cond_init(_run_cond,NULL);
  _stop = 0;
  _showntuples = 5;
  _num_queues = num_queues;
  _tuples_from_above = true;
  _running = false;
}

/** NOTE: Somehow I cannot trust a call to _q_net->getMaxArcId()
    ... it will return 0, unless you call it again (or "later" due to threads?
    ... although the one obtained by calling _catalog seems to return
    the right thing?
    This needs investigation... but im trusting the second parameter
*/
QueueMon::QueueMon(QueryNetwork *_q_net, StreamThread *stream_thread, int catalog_max_arc_id) {

  printf("[QueueMon] Fatal! Calling deprecated constructor (cannot access Catalog object)\n");
  exit(-1);

  // Mark the stream thread
  _st = stream_thread;
  // Mark the QueryNetwork to get arc information whenever desired
  _qnet = _q_net;

  // Figure out how many arcs (queues) there are, and their ids
  //int num_arcs = _q_net->getNumberOfArcs();
  // This is the bad call ... trace and it will return 0 but if you call it again
  //  it usually gets the right value. MESSED UP I SAY!
  int max_arc_id = _q_net->getMaxArcId();

  max_arc_id = catalog_max_arc_id;
  _num_queues = 0;
  //printf("MAX ARC ID SAYS %d\n", max_arc_id);
  //_arc_ids = new int[max_arc_id + 1];
  //_arc_ids_rev = new int[max_arc_id + 1];
  int curr_id = 0;
  for ( int i = 0; i <= max_arc_id; i++)
    {
      Arc *db_arc;
      db_arc = _q_net->getArc(i);
      
      if ( db_arc != NULL ) {
	cout << db_arc->toString() << endl;
	_num_queues++;
	printf("[QueueMon] Found arc id %d for queue index %d\n",i, curr_id);
	
	fflush(stdout);
	_arc_ids[curr_id] = i;
	_arc_ids_rev[i] = curr_id;
	curr_id++;
      }
    }
  // And the usual stuff
    _qmon_mutex = new pthread_mutex_t;
  pthread_mutex_init(_qmon_mutex,NULL);
  //_run_mutex = new pthread_mutex_t;
  //_run_cond = new pthread_cond_t;
  //pthread_mutex_init(_run_mutex,NULL);
  //pthread_cond_init(_run_cond,NULL);
  _stop = 0;
  _showntuples = 5;
  _tuples_from_above = true;
  _running = false;

  // init some reasonable default
  _usecRate = 1000000; // 1 second

  printf("[QueueMon] Detected %d (actual) queues\n", _num_queues);
}

/** 
 * This version gets the catalog object. Why? Because I want to differentiate arcs
 * as inputs/outputs/other...
*/
QueueMon::QueueMon(QueryNetwork *_q_net, StreamThread *stream_thread, Catalog *catalog) {

  int catalog_max_arc_id = catalog->getMaxArcId();

  // Mark the stream thread
  _st = stream_thread;
  // Mark the QueryNetwork to get arc information whenever desired
  _qnet = _q_net;

  // Figure out how many arcs (queues) there are, and their ids
  //int num_arcs = _q_net->getNumberOfArcs();
  // This is the bad call ... trace and it will return 0 but if you call it again
  //  it usually gets the right value. MESSED UP I SAY!
  int max_arc_id = _q_net->getMaxArcId();

  max_arc_id = catalog_max_arc_id;
  _num_queues = 0;
  //printf("MAX ARC ID SAYS %d\n", max_arc_id);
  //_arc_ids = new int[max_arc_id + 1];
  //_arc_ids_rev = new int[max_arc_id + 1];
  int curr_id = 0;
  for ( int i = 0; i <= max_arc_id; i++)
    {
      Arc *db_arc;
      db_arc = _q_net->getArc(i);
      
      if ( db_arc != NULL ) {
	// Figure out if input, output, or other
	string t;
	if (catalog->getArc(i)->getIsInput()) { _queue_type[curr_id] = 'i'; t = "INPUT"; } 
	else if (catalog->getArc(i)->getIsApp()) { _queue_type[curr_id] = 'o'; t = "OUTPUT"; }
	else { _queue_type[curr_id] = 'm';  t = "INTERMEDIATE"; }

	//cout << db_arc->toString() << endl;
	_num_queues++;
	cout << "[QueueMon] Found " << t << " arc id " << i << " for queue index " << curr_id << endl;

	/**
	cout << "[QueueMon] |_ Schema (attrib names): ";
	int nattribs = db_arc->getSchema()->getNumberOfAttributes();
	for (int ii = 0; ii < nattribs; ++ii) cout << " " << db_arc->getSchema()->getAttribute(ii).m_fieldName << " ";
	cout << endl;
	*/
	_arc_ids[curr_id] = i;
	_arc_ids_rev[i] = curr_id;
	curr_id++;
      }
    }
  // And the usual stuff
    _qmon_mutex = new pthread_mutex_t;
  pthread_mutex_init(_qmon_mutex,NULL);
  //_run_mutex = new pthread_mutex_t;
  //_run_cond = new pthread_cond_t;
  //pthread_mutex_init(_run_mutex,NULL);
  //pthread_cond_init(_run_cond,NULL);
  _stop = 0;
  _showntuples = 5;
  _tuples_from_above = true;
  _running = false;

  // Some nulls...
  _queuesOnOffbut = NULL;
  qbutlabels = NULL;
  // init some reasonable default
  _usecRate = 1000000; // 1 second

  printf("[QueueMon] Ready with %d queues\n", _num_queues);
}

bool QueueMon::isRunning() {
  return _running;
}
void QueueMon::setRunning(bool b) {
  _running = b;
}
const char* QueueMon::intFilter(FL_OBJECT *ob, double value, int prec) {
  static char buf[32];
  sprintf(buf, "%f", value);
  return buf;
}

void QueueMon::slider_cb(FL_OBJECT *ob, long data) {
  
  char str[30];
  sprintf(str,"Show top %d tuples",(int) fl_get_slider_value(ob));
  fl_set_object_label((FL_OBJECT *)data,str);
}

void QueueMon::thumbwheel_cb(FL_OBJECT *ob, long data) {
  
  char str[30];
  sprintf(str,"Show top %d tuples",(int) fl_get_thumbwheel_value(ob));
  fl_set_object_label((FL_OBJECT *)data,str);
}

int QueueMon::closecpanel_cb(FL_FORM *f, void* v) {
  return FL_IGNORE; // do not close if you "close" the control panel
}
void QueueMon::toggleRunState_cb(FL_OBJECT *ob, long data) {
  
  QueueMon* q = (QueueMon*) ob->u_vdata;
  if (q->isRunning() == true) {
    fl_set_object_label(ob, "Paused");
    fl_set_object_color(ob, 1, FL_RED);
    q->setRunning(false);
  } else {
    fl_set_object_label(ob, "Running");
    fl_set_object_color(ob, 1, FL_GREEN);
    q->setRunning(true);
  }

}

void QueueMon::toggleQueueState_cb(FL_OBJECT *ob, long queue_id) {
  QueueMon* q = (QueueMon*) ob->u_vdata;
  //printf("Button for queue %d pushed : it is now [%d]\n", queue_id, fl_get_button(ob));
  if (fl_get_button(ob) == 0) { // This means remove the queue from display
    if (q->qvisible[queue_id]) {
      //fl_hide_object(q->_queues[queue_id]);
      fl_hide_form(q->_queueforms[queue_id]);
      q->qvisible[queue_id] = false;
      //q->redrawQueues();
      //q->repositionQueues();
    }
    
  } else { // Display the queue
    if (!q->qvisible[queue_id]) {
      //fl_show_object(q->_queues[queue_id]);
      fl_show_form(q->_queueforms[queue_id], FL_PLACE_MOUSE|FL_FREE_SIZE, FL_FULLBORDER, q->qlabels[queue_id]);
      q->qvisible[queue_id] = true;
      //q->redrawQueues();
      //q->repositionQueues();
    }
  }
}

void QueueMon::toggleRate_cb(FL_OBJECT *ob, long data) {
  QueueMon* q = (QueueMon*) ob->u_vdata;
  double dial_val = fl_get_dial_value(ob);
  int dial_val_i = (int) dial_val;
  if (dial_val_i == 0) dial_val_i = 1;
  q->_usecRate = 1000000 / dial_val_i;; // temporary rate control for StreamThread
  //q->_st->setRateMultiplier(dial_val);
}

int QueueMon::getusecRate() {
  cout << "[QueueMon] WARNING! getusecRate() is deprecated, and its use may cause runtime problems. Please verify caller!" << endl;
  return _usecRate;
}

void QueueMon::toggleQueueFontSize_cb(FL_OBJECT *ob, long data) {
  QueueMon* q = (QueueMon*) ob->u_vdata;
  const char* choice = fl_get_choice_item_text(ob, fl_get_choice(ob));
  if (strcmp(choice,"Large") == 0) { // large font requested
    q->_queue_font_choice = 'l';
    for (int i=0; i < q->_num_queues; ++i) fl_set_browser_fontsize(q->_queues[i],FL_LARGE_SIZE);
  } else if (strcmp(choice,"Medium") == 0) { // large font requested
    q->_queue_font_choice = 'm';
    for (int i=0; i < q->_num_queues; ++i) fl_set_browser_fontsize(q->_queues[i],FL_MEDIUM_SIZE);
  } else if (strcmp(choice,"Small") == 0) { // large font requested
    q->_queue_font_choice = 's';
    for (int i=0; i < q->_num_queues; ++i) fl_set_browser_fontsize(q->_queues[i],FL_SMALL_SIZE);
  } else {
    cout << "[QueueMon] Invalid font size request from font size menu (" << choice << ")" << endl;
  }
}

void QueueMon::toggleQueueDir_cb(FL_OBJECT *ob, long data) {
  QueueMon* q = (QueueMon*) ob->u_vdata;
  const char* choice = fl_get_choice_item_text(ob, fl_get_choice(ob));
  
  if (strcmp(choice,"below") == 0) { // User chose "BELOW"
    if (!q->_tuples_from_above) return; // Redundant choice

    swapQueueDirections(q);
    q->_tuples_from_above = false;

    // TODO : For each queue, make sure LAST line is visible

  } else { // User chose "ABOVE"
    if (q->_tuples_from_above) return; // Redundant choice

    swapQueueDirections(q);
    q->_tuples_from_above = true;

    // TODO : For each queue, make sure TOP line is visible
    
  }
}

void QueueMon::swapQueueDirections(QueueMon* q) {
  // Go through the queues, swap any existing lines. USE THE LOCKS
  pthread_mutex_lock(q->_qmon_mutex);
  // Also freeze the visual display of any visible queues: NOTE, do the swap even for invisible queues
  for (int i = 0; i < q->_num_queues; i++) {
    // If there are no lines for some queue, skip over
    //if (q->_currline[i] == 0) continue;
    if (fl_get_browser_maxline(q->_queues[i]) == 0) continue;

    //printf("\n\nQueue %d : _currline is [%d], first is [%s]\n", i, q->_currline[i], fl_get_browser_line(q->_queues[i], 1));

    // If its visible, freeze the display while we swap the lines
    if (q->qvisible[i]) fl_freeze_form(q->_queueforms[i]);
    
    // Now swap the lines
    int currline = 1;
    //int lastline = q->_currline[i] ;
    int lastline = fl_get_browser_maxline(q->_queues[i]);
    char * temp = (char*) malloc(2048); // buffer style
    while (currline < lastline) {
      strcpy(temp, fl_get_browser_line(q->_queues[i], lastline));
      //printf("-- ITER! Currline: %d | Lastline: %d --\n", currline, lastline);
      //printf("\tTemp is now [%s]\n", temp);
      //printf("\tReplacing line %d with [%s]\n", lastline, fl_get_browser_line(q->_queues[i], currline));
      fl_replace_browser_line(q->_queues[i], lastline, fl_get_browser_line(q->_queues[i], currline));
      //printf("\t->Confirm: [%s]\n", fl_get_browser_line(q->_queues[i], lastline));
      //printf("\tReplacing line  %d with [%s]\n", currline, temp);
      fl_replace_browser_line(q->_queues[i], currline, temp);
      //printf("\t->Confirm: [%s]\n", fl_get_browser_line(q->_queues[i], currline));
      currline++; lastline--;
    }
    free(temp);


    // Unfreeze if we need to
    if (q->qvisible[i]) fl_unfreeze_form(q->_queueforms[i]);
  }
  

  pthread_mutex_unlock(q->_qmon_mutex);
}

/** Message dial stuff removed --- see comment below (search April 9 2003) */
void QueueMon::setNumMessages(int n) {
  _num_messages = n;
  // set the dial bounds
  //fl_set_dial_bounds(_messagedial, 0, n);
}
void QueueMon::tickMessage() {
  //fl_set_dial_value(_messagedial, fl_get_dial_value(_messagedial) + 1.0);
}


// Jun 2003 - deprecated, I wont call this function anymore
void QueueMon::repositionQueues() {
  
  int x = 0;
  int y = 0;
  int px; int py; int pw; int ph;
  int ph_max = 0; // the maximum queue height seen in one row
  for (int i = 0; i < _num_queues; i++) {
    if (!qvisible[i]) continue;
    fl_show_form(_queueforms[i], FL_PLACE_FREE, FL_FULLBORDER, qlabels[i]);

    fl_set_form_position(_queueforms[i], x, y);
    // Make sure you respect the widths of the queues now appropriately
    fl_get_object_geometry(_queues[i], &px, &py, &pw, &ph);
    if (ph > ph_max) ph_max = ph;
    x += pw + 10;
    if ((i % 4) == 3) { y += ph_max + 10; x = 0; ph_max = 0;}

  }
  


}
// DEPRECATED - Each queue is now in its own WINDOW (see repositonQueues())
void QueueMon::redrawQueues() {
  cout << "[QueueMon] use of redrawQueues() is deprecated" << endl;
  int num_active = 0;

  for (int i = 0; i < _num_queues; i++) {
    if (qvisible[i]) num_active++;
  }

  int pos_x = 10;
  int pos_y = 10;
  int width = (int) ((FORM_WIDTH - (_num_queues * 10)) / num_active);
  int height = FORM_HEIGHT - 150;
  
  fl_freeze_form(_form);
  for (int i = 0; i < _num_queues; i++) {
  
    if (!qvisible[i]) continue;
    // Now replace/redraw
    fl_set_object_position(_queues[i], pos_x, pos_y);
    fl_set_object_size(_queues[i], width, height);
    pos_x += width + 10;
  }
  fl_unfreeze_form(_form);
}

void QueueMon::initialize_form() {
  const char ** argv;
  argv = new const char *[1];
  argv[0] = "QueueMon";
  int one = 1;

  fl_initialize(&one, const_cast<char **>(argv), "FormDemo", 0, 0);
  // --------- Main Window - DEPRECTATED. EACH QUEUE NOW HAS ITS OWN FORM
  // ITS BACK AS A FORM BROWSER! - Nope, formbrowser sucks
  //_form = fl_bgn_form(FL_UP_BOX, FORM_WIDTH, FORM_HEIGHT);
  //_formbrowser = fl_add_formbrowser(FL_NORMAL_FORMBROWSER, 0, 0, FORM_WIDTH, FORM_HEIGHT, "All Queues");
  //fl_end_form();

  _queueforms = new FL_FORM*[_num_queues];
  _queues = new FL_OBJECT*[_num_queues];
  //_currline = new int[_num_queues];
  qlabels = new char*[_num_queues];
  qvisible = new bool[_num_queues];

  for (int i = 0; i < _num_queues; i++) {
    _queueforms[i] = fl_bgn_form(FL_UP_BOX, START_QUEUE_WIDTH, START_QUEUE_HEIGHT + 30);
    add_n_set_queue(i,(_num_queues > 10));
    fl_end_form();
    // Prevent closing program upon closing a queue window
    fl_set_form_atclose(_queueforms[i], closecpanel_cb, NULL);
    //fl_addto_formbrowser(_formbrowser, _queueforms[i]);
  }
  //{
  //add_n_setobjects_main();
  //}
  //fl_end_form();
  // --------- Control Panel
  // May 03 : Make the control panel wider as needed
  // we will put 10 per column

  // Prepare the tab forms
  int DAWIDTH = (QBUT_WIDTH + 20) * 4;
  int DAHEIGHT = 10 + (QBUT_HEIGHT + 10) * (int) ceil((_num_queues / 4.0));
  // Tab with buttons only for input queues
  _cpanel_inputs = fl_bgn_form(FL_FRAME_BOX, DAWIDTH, DAHEIGHT);
  {
    add_n_setobjects_cpanel_type('i');
  }
  fl_end_form();
  // Tab with buttons only for output queues
  _cpanel_outputs = fl_bgn_form(FL_FRAME_BOX,  DAWIDTH, DAHEIGHT);
  {
    add_n_setobjects_cpanel_type('o');
  }
  fl_end_form();
  // Tab with buttons only for middle queues
  _cpanel_intermediate = fl_bgn_form(FL_FRAME_BOX,  DAWIDTH, DAHEIGHT);
  {
    add_n_setobjects_cpanel_type('m');
  }
  fl_end_form();
  // Tab with options
  _cpanel_options = fl_bgn_form(FL_FRAME_BOX, DAWIDTH, DAHEIGHT);
  {
    /** Queue direction chooser **/
    // Choosing "from above, from below" for tuples
    // The queue direction choice, with the explanation label above
    int pos_x = 10; int pos_y = 5;    
    fl_add_text(FL_NORMAL_TEXT, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT + 5, "New tuples\nfrom:");
    pos_y += QBUT_HEIGHT + 5;    
    _queuedirchoice = fl_add_choice(FL_NORMAL_CHOICE2, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT, "");
    fl_addto_choice(_queuedirchoice, "above");
    fl_addto_choice(_queuedirchoice, "below");
    _tuples_from_above = true;     // Since I added above first, that's the default
    _queuedirchoice->u_vdata = (void*) this; // callback
    fl_set_object_callback(_queuedirchoice, toggleQueueDir_cb, 0);
    /** Font size selector **/
    pos_x =  QBUT_WIDTH + 20; pos_y = 5;
    fl_add_text(FL_NORMAL_TEXT, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT + 5, "Queue font size:");
    pos_y += QBUT_HEIGHT+5;
    _queuefontchoice = fl_add_choice(FL_NORMAL_CHOICE2, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT,"");
    fl_addto_choice(_queuefontchoice, "Small");
    fl_addto_choice(_queuefontchoice, "Medium");
    fl_addto_choice(_queuefontchoice, "Large");
    _queue_font_choice = 's'; // default is small
    _queuefontchoice->u_vdata = (void*) this; // for callback    
    fl_set_object_callback(_queuefontchoice,toggleQueueFontSize_cb, 0);
  }
  fl_end_form();



  /**
      THIS STUFF IS DEPRECATED BY THE UBER TABBED CODE
  */
  /**
  int cols = 1;
  if (_num_queues > 10) cols = _num_queues / 10;
  _cpanel = fl_bgn_form(FL_UP_BOX, (QBUT_WIDTH + 20) * (cols+1),
			(_num_queues < 10) ? ((QBUT_HEIGHT + 10) * _num_queues) + 20 : ((QBUT_HEIGHT + 10) * (10)) + 20);
  {
    add_n_setobjects_cpanel();
  }
  fl_end_form();

  _runcpanel = fl_bgn_form(FL_UP_BOX, (QBUT_WIDTH+20) * 1, (QBUT_HEIGHT + 10) * 4);
  {
    add_n_setobjects_runcpanel();
  }
  fl_end_form();

  // Make sure closing both control panels... well, won't close anything
  fl_set_form_atclose(_cpanel, closecpanel_cb, NULL);
  fl_set_form_atclose(_runcpanel, closecpanel_cb, NULL);
  */


  // Now show all queue forms - try to place "clever" on the screen, top right, to bottom left  
  // June 2003, tab edition : never show queue forms upon startup
  /**
  int x = 0;
  int y = 0;
  if (!(_num_queues > 10)) {
    for (int i = 0; i < _num_queues; i++) {
      fl_show_form(_queueforms[i], FL_PLACE_FREE, FL_FULLBORDER, qlabels[i]);
      fl_set_form_position(_queueforms[i], x, y);
      x += START_QUEUE_WIDTH + 10;
      if ((i % 4) == 3) { y += START_QUEUE_WIDTH + 10; x = 0; }
    }
  }
  */

  // GONE SINCE OUR UBER TAB CODE
  //fl_show_form(_form,FL_PLACE_CENTERFREE,FL_FULLBORDER,"Queue Viewer");
  //  fl_show_form(_cpanel,FL_PLACE_MOUSE,FL_TRANSIENT,"Queue Control");
  //fl_show_form(_runcpanel,FL_PLACE_MOUSE,FL_TRANSIENT,"Runtime Control");


  {
    _folderform = fl_bgn_form(FL_FLAT_BOX,DAWIDTH + 5,DAHEIGHT + QBUT_HEIGHT + 20);
    _tabfolder = fl_add_tabfolder(FL_TOP_TABFOLDER,5,QBUT_HEIGHT+20,DAWIDTH - 5,DAHEIGHT - QBUT_HEIGHT - 20,"Tabs");
    
    // TOP LINE - START | QUIT
    {
      // Start button
      int pos_x = (DAWIDTH - (2 * (QBUT_WIDTH + 10))) / 2; int pos_y = 5;
      _runbut = fl_add_button(FL_PUSH_BUTTON, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT,"Start");
      fl_set_button_shortcut(_runbut, " ", 1);
      _running = false;
      _runbut->u_vdata = (void*)this;
      fl_set_object_callback(_runbut,toggleRunState_cb,0);
      
      // quit button
      pos_x += QBUT_WIDTH + 10;
      _quitbut = fl_add_button(FL_NORMAL_BUTTON, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT,"Quit");
      // make it have a shortcut button (q)
      fl_set_button_shortcut(_quitbut, "q", 1);
    }

    // Add the tab for the input queues
    fl_addto_tabfolder(_tabfolder,"Input Queues",_cpanel_inputs);
    fl_addto_tabfolder(_tabfolder,"Output Queues",_cpanel_outputs);
    fl_addto_tabfolder(_tabfolder,"Intermediate Queues",_cpanel_intermediate);
    fl_addto_tabfolder(_tabfolder,"Options",_cpanel_options);
    fl_end_form();
  }
  //fl_set_tabfolder_autofit(_tabfolder, FL_FIT);
  fl_show_form(_folderform,FL_PLACE_MOUSE,FL_TRANSIENT,"QueueMon - Tabbed Edition");



  

}


/** The actual code! */
void *QueueMon::run()
{
	initialize_form();

	FL_OBJECT *obj;
	//int c;
	//char item_string[25];
	//int counter = 0;
	do
	  {
	    obj = fl_check_forms();
	    usleep(80000);
	  } while (obj != _quitbut && _stop != 1);
	cleanup(shmid,semid);
	exit(0);
  return (void *)0;
}

// Adds a queue
void QueueMon::add_n_set_queue(int queue_index,bool moreThanTen) {

  int pos_x =0;
  int pos_y = 0;
  
  qlabels[queue_index] = new char[24];
  sprintf(qlabels[queue_index], "Queue %d (Arc id %d)", queue_index, _arc_ids[queue_index]);
  // add schema info line too
  string schemaline = "Schema ( TS ";

  int nattribs = _qnet->getArc(_arc_ids[queue_index])->getSchema()->getNumberOfAttributes();
  for (int ii = 0; ii < nattribs; ++ii) schemaline = schemaline + ", " +_qnet->getArc(_arc_ids[queue_index])->getSchema()->getAttribute(ii).m_fieldName + " ";
  schemaline += ")";
  fl_add_text(FL_NORMAL_TEXT,pos_x,pos_y,START_QUEUE_WIDTH, 20,schemaline.c_str());
  pos_y += 20;
  _queues[queue_index] = fl_add_browser(FL_NORMAL_BROWSER, pos_x, pos_y, START_QUEUE_WIDTH, START_QUEUE_HEIGHT - 20, qlabels[queue_index]);
  fl_set_object_color(_queues[queue_index], FL_BLACK, FL_RED);
  if (moreThanTen) qvisible[queue_index] = false;
  else qvisible[queue_index] = true;
  //_currline[queue_index] = 0;

}

// DEPRECATED
void QueueMon::add_n_setobjects_main() {
  // Use the slider for normal slider, else its a thumbwheel

  /** If you want a slider
  //_topnslider = fl_add_slider(FL_HOR_NICE_SLIDER, 350, 400, 200, 30, "Show top n tuples");
  //fl_set_slider_bounds(_topnslider, 1, 30);
  //fl_set_slider_value(_topnslider, 5);
  //fl_set_slider_step(_topnslider, 1);
  //fl_set_slider_return(_topnslider, FL_RETURN_END_CHANGED);
  //fl_set_slider_filter(_topnslider, QueueMon::intFilter);
  //fl_set_object_callback(_topnslider,slider_cb,(long) _topnslider);
  */
  
  /** if you want the thumbwheel
  //_topnslider = fl_add_thumbwheel(FL_HOR_THUMBWHEEL, 300, 500, 200, 30, "Show top 5 tuples");
  fl_set_thumbwheel_bounds(_topnslider, 1, 30);
  fl_set_thumbwheel_value(_topnslider, _showntuples);
  fl_set_thumbwheel_step(_topnslider, 0.25);
  fl_set_thumbwheel_return(_topnslider, FL_RETURN_END_CHANGED);
  _topnslider->u_vdata = (void*)this;
  fl_set_object_callback(_topnslider,thumbwheel_cb,(long) _topnslider);
  */

  // For each queue, prepare a Browser object
  _queues = new FL_OBJECT*[_num_queues];
  qlabels = new char*[_num_queues];
  qvisible = new bool[_num_queues];
  int pos_x = 10;
  int pos_y = 10;
  int width = (int) ((FORM_WIDTH - (_num_queues * 15)) / _num_queues);
  int height = FORM_HEIGHT - 150;
  
  //_currline = new int[_num_queues];
  for (int i = 0; i < _num_queues; i++) {
    qlabels[i] = new char[12];
    sprintf(qlabels[i], "Arc %d", _arc_ids[i]);
    _queues[i] = fl_add_browser(FL_NORMAL_BROWSER, pos_x, pos_y, width, height, qlabels[i]);
    fl_set_object_color(_queues[i], FL_BLACK, FL_RED);
    qvisible[i] = true;
    pos_x += width + 10;
    //_currline[i] = 0;
  }
  
}

// Shows runtime buttons (start, quit, tuple-from selection, dials...
void QueueMon::add_n_setobjects_runcpanel() {
  int pos_x = 10; int pos_y = 10;

  // Start button
  _runbut = fl_add_button(FL_PUSH_BUTTON, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT,"Start");
  fl_set_button_shortcut(_runbut, " ", 1);
  _running = false;
  _runbut->u_vdata = (void*)this;
  fl_set_object_callback(_runbut,toggleRunState_cb,0);


  // quit button
  pos_y += QBUT_HEIGHT+10;
  _quitbut = fl_add_button(FL_NORMAL_BUTTON, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT,"Quit");
  // make it have a shortcut button (q)
  fl_set_button_shortcut(_quitbut, "q", 1);


  // The queue direction choice, with the explanation label above
  pos_y += QBUT_HEIGHT + 5;
  
  fl_add_text(FL_NORMAL_TEXT, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT + 5, "New tuples\nfrom:");
  pos_y += QBUT_HEIGHT + 5;

  _queuedirchoice = fl_add_choice(FL_NORMAL_CHOICE2, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT, "");
  fl_addto_choice(_queuedirchoice, "above");
  fl_addto_choice(_queuedirchoice, "below");
  // Since I added above first, that's the default
  _tuples_from_above = true;
  // Callback
  _queuedirchoice->u_vdata = (void*) this;
  fl_set_object_callback(_queuedirchoice, toggleQueueDir_cb, 0);
  //pos_y += QBUT_HEIGHT + 20;
  //pos_x += QBUT_WIDTH + 20;

  // message dial
  // REMOVED April 9 2003 since it looks like StreamThread doesn't care about "message count" anymore
  //  and just keeps pumping tuples ...
  //_messagedial = fl_add_dial(FL_FILL_DIAL, pos_x, pos_y, QBUT_WIDTH, QBUT_WIDTH, "Message Count");

  //pos_y += QBUT_WIDTH + 20;

  // rate dial
  pos_x += QBUT_WIDTH + 20; pos_y = 10;
    // REMOVED April 9 2003 since it looks like StreamThread doesn't want it for the time being
  // PUT BACK May 2003 which affects the "timeout" for streamthread
  // naaa, out May 29
  /**
  _ratedial = fl_add_dial(FL_LINE_DIAL,pos_x,pos_y,QBUT_WIDTH,QBUT_WIDTH,"Rate Multiplier");
  fl_set_dial_value(_ratedial,(double)1.0);
  fl_set_dial_bounds(_ratedial,(double)1.0,(double)2000.0);
  // Callback for changing the rate
  _ratedial->u_vdata = (void*) this;
  fl_set_object_callback(_ratedial, toggleRate_cb, 0);
  */

  // No clicking on message dial!
  //fl_deactivate_object(_messagedial);
  //fl_set_dial_value(_messagedial, 0.0);
  
  //pos_y += QBUT_WIDTH + 20;
  //pos_x += QBUT_WIDTH+20;


}

// Adds only a certain type of queues:  [i]nput, [m]iddle, [o]utputs
// and goes left to right, 4 columns
void QueueMon::add_n_setobjects_cpanel_type(char type) {

  int pos_x = 10;
  int pos_y = 10;
  
  
  qbutshortcuts = new char*[_num_queues];
  
  // The "active/inactive" light switches
  if (_queuesOnOffbut == NULL) { // initialize only once
    _queuesOnOffbut = new FL_OBJECT*[_num_queues];
    qbutlabels = new char*[_num_queues];
  }

  int done = 0;
  for (int i = 0; i < _num_queues; i++) {

    // Determine if to do this one, or skip
    if (type != _queue_type[i]) continue;
    else ++done; // ok, proceed and mark one will be done
    
    if (done > 4 && done % 4 == 0) {pos_y += (QBUT_HEIGHT+10); pos_x = 10; }
    qbutlabels[i] = new char[12];
    sprintf(qbutlabels[i], "Queue %d", i);
    _queuesOnOffbut[i] = fl_add_lightbutton(FL_PUSH_BUTTON, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT, qbutlabels[i]);
    pos_x += (QBUT_WIDTH + 20);
    // Set callback
    _queuesOnOffbut[i]->u_vdata = (void*)this;
    fl_set_object_callback(_queuesOnOffbut[i],toggleQueueState_cb,i);
    // Set initially pushed UNLESS THERE ARE MORE THAN 10 QUEUES
    if (_num_queues > 10)
      fl_set_button(_queuesOnOffbut[i], 0);
    else
      fl_set_button(_queuesOnOffbut[i], 1);
    /**
    // Set CTRL-n shortcut from queues 0 to 9
    if (i < 10) {
      qbutshortcuts[i] = new char[2];
      sprintf(qbutshortcuts[i], "%d", i);
      fl_set_button_shortcut(_queuesOnOffbut[i], qbutshortcuts[i], 1);

    }
    // No more, since tabbed versions change focus
    */ 
  }


}
void QueueMon::add_n_setobjects_cpanel() {

  int pos_x = 10;
  int pos_y = 10;

  
  qbutshortcuts = new char*[_num_queues];
  
  // The "active/inactive" light switches
  _queuesOnOffbut = new FL_OBJECT*[_num_queues];
  qbutlabels = new char*[_num_queues];
  for (int i = 0; i < _num_queues; i++) {
    if (i > 9 && i % 10 == 0) {pos_x += (QBUT_WIDTH+20); pos_y = 10; }
    qbutlabels[i] = new char[12];
    sprintf(qbutlabels[i], "Arc %d", _arc_ids[i]);
    _queuesOnOffbut[i] = fl_add_lightbutton(FL_PUSH_BUTTON, pos_x, pos_y, QBUT_WIDTH, QBUT_HEIGHT, qbutlabels[i]);
    pos_y += 40;
    // Set callback
    _queuesOnOffbut[i]->u_vdata = (void*)this;
    fl_set_object_callback(_queuesOnOffbut[i],toggleQueueState_cb,i);
    // Set initially pushed UNLESS THERE ARE MORE THAN 10 QUEUES
    if (_num_queues > 10)
      fl_set_button(_queuesOnOffbut[i], 0);
    else
      fl_set_button(_queuesOnOffbut[i], 1);
    // Set CTRL-n shortcut from queues 0 to 9
    if (i < 10) {
      qbutshortcuts[i] = new char[2];
      sprintf(qbutshortcuts[i], "%d", i);
      fl_set_button_shortcut(_queuesOnOffbut[i], qbutshortcuts[i], 1);

    }
  }

  // reset back to left side of control panel, and below all light switches
  //pos_y = 10 + 11 * (QBUT_HEIGHT + 10);
  //pos_x = 10;

  
  

}

/**
   Function called to add a tuple to the display of queue number queue_id
   // Note the queue_id is the actualy ARC ID, not the "queue index" used
   //  in the viewer. Must translate (the param should be called arc_id really)
*/
void QueueMon::addTuple(char* ptr, TupleDescription *td, int queue_id) {
  // The arc_id to queue index translation
  int queue_index = _arc_ids_rev[queue_id];
  
  // Check if the queue is inactive for viewing, if so return
  if (fl_get_button(_queuesOnOffbut[queue_index]) == 0) return;

  // Must use thread locks here
  pthread_mutex_lock(_qmon_mutex);

  // ??? What's this 100 hard coded here? 
  //  oohhh its cuz to represent some of the tuple data, it may take more than what it takes in memory
  //printf("[QueueMon] addTuple: using TupleDescription %p\n", td);
  char* str = (char*) malloc(100 + td->getSize() + TUPLE_TIMESTAMP_SIZE + TUPLE_STREAMID_SIZE + 1 + td->getNumOfFields());
  char* buf = (char*) malloc(100 + td->getSize() + TUPLE_TIMESTAMP_SIZE + TUPLE_STREAMID_SIZE + 1);
  int str_offset = 0;
  int buf_offset = 0;
  //char tempbuf[td->getSize()];
  if (buf == NULL || str == NULL) {
    perror("[QueueMon] addTuple: malloc for tuple data buffers failed!");
    exit(1);
  }
  
  // Green lines
  str_offset += sprintf(str+str_offset,"%s ", QUEUE_MON_DEFAULT_TUPLE_COLOR);

  //bool ts_skipped = false;

  
  // FOR NOW: PRINT THE TIMESTAMP AND STREAM ID (if you dont want it printed, take this out)
  // first the timestamp
  memset(buf, 0, 100 + td->getSize() + TUPLE_TIMESTAMP_SIZE + TUPLE_STREAMID_SIZE + 1);
  memcpy(buf, ptr + TUPLE_TIMESTAMP_OFFSET, TUPLE_TIMESTAMP_SIZE);
  /**
  str_offset += sprintf(str+str_offset,"(%ld , %ld) ",
			((timeval*) buf)->tv_sec, 
			((timeval*) buf)->tv_usec);
  */
  // Pretty print
  struct tm* now = localtime(&((timeval*) buf)->tv_sec);
  char* format_str = (char*) malloc(32);
  sprintf(format_str,"%%T (%6.2fms) ",( ((timeval*) buf)->tv_usec) / 1000.0 );
  str_offset += strftime(str+str_offset,
			 100 + td->getSize() + TUPLE_TIMESTAMP_SIZE + TUPLE_STREAMID_SIZE + 1 + td->getNumOfFields() - str_offset,
			 format_str,
			 now);
  free(format_str);
  // the stream id
  // APRIL 28 2003 - NO MORE SHOWING STREAM ID!
  //memset(buf, 0, 100 + td->getSize() + TUPLE_TIMESTAMP_SIZE + TUPLE_STREAMID_SIZE + 1);
  //memcpy(buf, ptr + TUPLE_STREAMID_OFFSET, TUPLE_STREAMID_SIZE);
  //str_offset += sprintf(str+str_offset,"{%d}  ", *((int*) buf));

  /**
  cout << "TD REPORTING IN: NUM FIELDS ["<<td->getNumOfFields()<<"]" << endl;
  for (int i = 0; i < td->getNumOfFields(); i++) {
    cout << " FIELD " << i << ":" << endl;
    cout << "  type ";
    
    switch (td->getFieldType(i)) {
    case INT_TYPE:
      cout << "(int)" << endl;
      break;
    case FLOAT_TYPE:
      cout << "(float)" << endl;
      break;
    case DOUBLE_TYPE:
      cout << "(double)" << endl;
      break;
    case STRING_TYPE:
      cout << "(string)" << endl;
      break;
    default:
      cout << "(HUH!?)" << endl;
      break;
    }
   
    cout << "  size " << td->getFieldSize(i) << endl;
  }

  */

  // The fields
  for (int i = 0; i < td->getNumOfFields(); i++) {
    // Clear out the variable buffer
    memset(buf, 0, 100 + td->getSize() + TUPLE_TIMESTAMP_SIZE + TUPLE_STREAMID_SIZE + 1);

    // Copy the contents to a buffer
    memcpy(buf, ptr + TUPLE_DATA_OFFSET + td->getFieldOffset(i), td->getFieldSize(i));
    buf_offset = 0;

    // Note: since we skip over (TUPLE_DATA_OFFSET), we are actually skipping over metadata (currently timestamp, and streamid)
    //       iotw: they won't get displayed
    // NOTE: DONT USE THIS TS_SKIPPED THING ANYMORE
    switch (td->getFieldType(i)) {
    case TIMESTAMP_TYPE:
      {
	struct tm* now2 = localtime(&((timeval*) buf)->tv_sec);
	char* format_str2 = (char*) malloc(32);
	sprintf(format_str2,"[%%T (%6.2fns)]\t",( ((timeval*) buf)->tv_usec) / 1000.0 );

	str_offset += strftime(str+str_offset,
			       100 + td->getSize() + TUPLE_TIMESTAMP_SIZE + TUPLE_STREAMID_SIZE + 1 + td->getNumOfFields() - str_offset,
			       format_str2,
			       now2);
	free(format_str2);
      }
      break;
    case INT_TYPE:
    // Note 2: While in compatibility mode (code not totally supporting streamid/ts) I have this
      //if (TUPLE_DATA_OFFSET == 0) {
      //if (!ts_skipped) { ts_skipped = true; break; }
      //}
      str_offset += sprintf(str+str_offset,"[%4d]\t", *((int*) buf));
      break;
    case FLOAT_TYPE:
      str_offset += sprintf(str+str_offset,"[%10f]\t",*((float*) buf));
      break;
    case DOUBLE_TYPE:
      str_offset += sprintf(str+str_offset,"[%10e]\t",*((double*) buf));
      break;
    case STRING_TYPE:
      str_offset += sprintf(str+str_offset,"[");
      for (int l = 0; l < td->getFieldSize(i); l++) {
	str_offset += sprintf(str+str_offset,"%c",*((char*) (buf + buf_offset)));
	buf_offset++;
      }
      str_offset += sprintf(str+str_offset,"]");
      str_offset += sprintf(str+str_offset,"\t");
      break;
    }
  }
  str[str_offset] = '\0';
  //printf("[QueueMon] addTuple: Adding [%s] to queue %d (arc id %d)\n", str, queue_index, queue_id);
  // This will verify that the tuple isn't already in the display, which signifies a bug in the SMInterface code calling
  //   us here [not always possible, cuz some boxes will send timestamps that may reappear later...]
  if (VERIFY_DUPLICATES) {
    for (int id = 1; id <= fl_get_browser_maxline(_queues[queue_index]); ++id) {
      if (strcmp(str,fl_get_browser_line(_queues[queue_index],id)) == 0) {
	printf("[QueueMon] addTuple: WARNING! IGNORING ALREADY SHOWN TUPLE. PLEASE FIX CALLER!\n");
	free(buf);
	pthread_mutex_unlock(_qmon_mutex);
	return;
      }
    }
  }
  
  // From above, means add at the top of the window
  if (_tuples_from_above) fl_insert_browser_line(_queues[queue_index], 1, str);
  else  fl_addto_browser(_queues[queue_index], str); // below, you add it at the end
  //_currline[queue_index]++;

  // If we have more tuples than set to display, "hide" the last one at the bottom
  /**  if (_currline[queue_id] > _showntuples) {
    fl_freeze_form(_form);
    for (int i = _showntuples+1; i < _currline[queue_id] + 1; i++) { // from first line to hide til end

      //str_offset += sprintf(str+str_offset,"%s ", "@C2");
      const char* oldstr =  fl_get_browser_line(_queues[queue_id], i);
      char* strn = (char*) malloc(strlen(oldstr) + 5);
      sprintf(strn, "@C2 %s", oldstr);
      fl_replace_browser_line(_queues[queue_id], i, strn);
    }
    fl_unfreeze_form(_form);
  } 
  */

  free(buf);
  pthread_mutex_unlock(_qmon_mutex);
}
/**
   Function called to remove a tuple to the display of queue number queue_id
   April 2003: "gray out" removed tuples, don't remove them from display
                could be an option too... I'll leave it here as a define up top, for now
*/
void QueueMon::removeTuple(int queue_id) {
  // The arc_id to queue index translation (see addTuple)
  int queue_index = _arc_ids_rev[queue_id];

  // Must use thread locks here
  pthread_mutex_lock(_qmon_mutex);

  // ---------- GRAY OUT CODE BEGIN
  // Strategy - search for the first tuple that needs "greying out" to indicate it removed.
  //  tuples_from_above tells you the direction to look for the tuple
  if (QUEUE_MON_GRAY_ON_REMOVE) {

    // TODO, perhaps...
    // When the rate gets very high, stop trying to change colors
    

    // Search from 1 to maxline
    if (_tuples_from_above) {
      for (int i = 1; i <= fl_get_browser_maxline(_queues[queue_index]); i++) {
	const char* theline = fl_get_browser_line(_queues[queue_index],i);
	char* p = strstr(theline,QUEUE_MON_DEFAULT_TUPLE_COLOR);
	if (p == NULL) continue; // QUEUE_MON_DEFAULT_TUPLE_COLOR not found in this tuple, move on
	char* thenewline = (char*) malloc(strlen(theline) + 1);
	strcpy(thenewline,theline);
	memcpy(thenewline,QUEUE_MON_GRAY_TUPLE_COLOR,3);
	fl_replace_browser_line(_queues[queue_index],i,thenewline);
	break; // get out!
      }
    } else { // Search from maxline to 1
       for (int i = fl_get_browser_maxline(_queues[queue_index]); i >= 1; i--) {
	const char* theline = fl_get_browser_line(_queues[queue_index],i);
	char* p = strstr(theline,QUEUE_MON_DEFAULT_TUPLE_COLOR);
	if (p == NULL) continue; // @C2 not found in this tuple, move on
	char* thenewline = (char*) malloc(strlen(theline) + 1);
	strcpy(thenewline,theline);
	memcpy(thenewline,QUEUE_MON_GRAY_TUPLE_COLOR,3);
	fl_replace_browser_line(_queues[queue_index],i,thenewline);
	break; // get out!
      }

    }

    /**
    char* theline;
    char* thenewline;
    if (_tuples_from_above) //from above, gray out top line
      theline = (char*) fl_get_browser_line(_queues[queue_index],fl_get_browser_maxline(_queues[queue_index]) - _currline[queue_index] + 1);
    else
      theline = (char*) fl_get_browser_line(_queues[queue_index],_currline[queue_index]);
    
    cout << "[QueueMon] Line to be grayed out is {"<<theline<<"}"<<endl;
    // Build the new, grayed out, line
    thenewline = (char*) malloc(strlen(theline) + 1);
    strcpy(thenewline,theline);
    char* p = strstr(thenewline,"@C2");
    if (p == NULL) {
      cout << "[QueueMon] WARNING! Color tag not found, cannot gray out removed tuple!" << endl;
    } else {
      memcpy(p,"@C3",3);
      if (_tuples_from_above) fl_replace_browser_line(_queues[queue_index],fl_get_browser_maxline(_queues[queue_index]) - _currline[queue_index] + 1,thenewline);
      else fl_replace_browser_line(_queues[queue_index],_currline[queue_index],thenewline);
      // Should I now free theline ?
    }
    // ---------- GRAY OUT CODE END
    */
  } else {
    // if new tuples came from above, the last tuple is at the top (line 1), else its the last line
    if (_tuples_from_above) fl_delete_browser_line(_queues[queue_index], 1);
    else fl_delete_browser_line(_queues[queue_index], fl_get_browser_maxline(_queues[queue_index]));
    //_currline[queue_index]--; // NOTE THAT if graying out, you don't use _currline
  }
  //printf("[QueueMon] removeTuple: Remove top from queue %d (Arc id %d)\n", queue_index, queue_id);
  pthread_mutex_unlock(_qmon_mutex);
}



void QueueMon::ping() {
  printf("QueueMon: PONG\n");
}
