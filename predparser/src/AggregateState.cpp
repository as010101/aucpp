#include "AggregateState.H"

AggregateState::AggregateState()
{
  af = NULL;
  group_by = NULL;
  unique_time = *(new Timestamp());
  slack_time = *(new Timestamp());
  window_range = 0;
  reg_hash = NULL;
  list_hash = NULL;
  last = NULL;
  trash = NULL;
  group_hash = NULL;
  unless_timeout =  *(new Timestamp()); 
  whenever_pred = NULL;
  satisfies_pred = NULL;
  buffer = NULL;
  last_emitted = NULL;
}

void AggregateState::printAll()
{
  cout << "af: " << (af == NULL) << endl;
  if (af) {
    af->init();
    cout << "Aggregate Function: "; printTuple(af->final(), "f");
  }
  cout << "Group By: " << group_by << endl;
  cout << "Unique Time: " << unique_time.toString() << endl;
  cout << "Slack Time: " << slack_time.tv_sec << endl;       // toString() did not work ... figure out later
  cout << "Window Range: " << window_range << endl;
  //if (reg_hash)
  //  cout << "Reg Hash: "; reg_hash->printAll();
  //if (last)
  //  cout << "Last Hash: "; last->printAll();
  //if (trash)
  //  cout << "Trash Hash: "; trash->printAll();
  //cout << "Group Hash: "; group_hash->printAll();
  cout << "Unless Timeout: " << unless_timeout.toString() << endl;
  cout << "Whenever Predicate: " <<  (whenever_pred != NULL) << endl;
  cout << "Satisfies Predicate: " << (satisfies_pred != NULL) << endl;
  switch (output) {
  case LAST:
    cout << "Output Time: LAST" << endl;
    break;
  case ALL:
    cout << "Output Time: ALL" << endl;
    break;
  case WHENEVER:
    cout << "Output Time: WHENEVER" << endl;
    break;
  }
  switch (until) {
  case TIMEOUT:
    cout << "Timeout Arg: TIMEOUT" << endl;
    break;
  case SATISFIES:
    cout << "Timeout Arg: SATISFIES" << endl;
    break;
  case NONE:
    cout << "Timeout Arg: NONE" << endl;
    break;
  }
}
