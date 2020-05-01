#include "FidelityAlarmAF.H"
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

FidelityAlarmAF::FidelityAlarmAF(const char *att) {
  // THIS IS NEEDED TO HAVE A UNIQUE SHARED MEMORY AREA PER "BOX"
 // you can optimize this - look at the last character only, hehe
  if (strcmp(att,"counter1") == 0) {
    shm_key = 31337;
  } else if (strcmp(att,"counter2") == 0) {
    shm_key = 31338;
  } else if (strcmp(att,"counter3") == 0) {
    shm_key = 31339;
  } else if (strcmp(att,"counter4") == 0) {
    shm_key = 31340;
  }
  // remember the att name too, needed for makenew
  att_store = (char*) malloc(strlen(att)+1);
  strcpy(att_store,att);
}

FidelityAlarmAF::~FidelityAlarmAF() {}

void FidelityAlarmAF::init() {
  // Do nuthin
}

void FidelityAlarmAF::incr(char *tuple) {
  // Do nuthin
}

char* FidelityAlarmAF::final() {

  //cout << " ALARM ("<<att_store<<"): My pid is " << getpid() << ", parent pid is " << getppid() << endl;
  // Access shared memory
  int seg_id = shmget(shm_key, 4, IPC_CREAT | S_IRUSR | S_IWUSR);
  if (seg_id == -1) {
    perror("FidelityAlarmAF - shmget failed:");
    exit(1);
  }
  // You know its the first time at this shared segment if your current process id
  // doesn't match what the shared memory segment says
  // NO! This won't work if different threads keep calling this function, since they have a different process id
  //  (if only I could SET the pid of the shared memory segment...)
  //  What I will try is to trust "nattach", how many people are attached to this.
  //   Each call of this FidelityAlarm will attach, but NOT detach from it. We rely
  //   on the fact that exit() [abort too or not?] detaches everyone. Upon
  //   the first attach, we also immediately mark this segment for DELETION, so that
  //   after exit(), after all detaches, the segment is also deleted...
  //  FINAL UPDATE: can't mark it for deletion, it gets deleted. But if I don't, the number
  //   of attaches can be used very nicely indeed! When the program quits, it is always set back
  //   to 0... automagically. I'm no fan of doing this, but I got no choice.
  // funny thing, I could almost just use the n_attch to keep track of the count, instead of bother with an int haha!
  //  KEEP THIS IN MIND: I could always make the shared segment 8 bytes to store parent process id too.
  bool is_first = false;
  struct shmid_ds buf;
  shmctl(seg_id,IPC_STAT,&buf);
  //cout << " ALARM ("<<att_store<<"): Current attached: " << buf.shm_nattch << endl;
  //if (buf.shm_lpid != getpid()) { is_first = true; } 
  if (buf.shm_nattch == 0) {
    // No one attached to it yet!
    is_first = true;
  }
  
  // Attach to it
  void* addr = shmat(seg_id, NULL, 0);
  if (*(int*) addr == -1 ) {
    perror("FidelityAlarmAF - shmat failed:");
    exit(1);
  }
  // first time? write a 1
  int val = 1;
  if (is_first) {
    // Mark it for deletion right now (see paragraph above for why, and why not after all)
    //if (shmctl(seg_id,IPC_RMID,NULL) < 0) {
    //perror("FidelityAlarmAF - shmctl(IPCRMID) failed:");
    //exit(1);
    //}
    //cout << " ALARM ("<<att_store<<"): FIRST TIME FOR THIS EXECUTION! Writing a 1 " << endl;
    memcpy(addr,&val,sizeof(int));
    

  } else { // else read the value and then increment the shared value
    val = (*((int*)addr))++; // I believe this is all atomic, so no need for locks...
    val++; // gotta increment this one too (why? i thought above line does it, but if not, I get twice value of 1)
    //cout << " ALARM ("<<att_store<<"): NOT FIRST! Returning a " << val << endl;
  }
  // Done, detach
  // NO! Let the system do this for us
  //shmdt(addr);

  // UPDATE JUNE 6 - Filter boxes cannot do X mod Y so that will be computed here,
  //  and the filter box will just check the value as a flag (0 or 1).
  char *ret = new char[sizeof(int)];
  *(int*)(ret) = (val % 100) == 0 ? 1 : 0; // or we could compare against some alarm value now... but
                      // by passing the count, you leave it up to a filter box later
                      // to determine
  

  return ret;
}

char* FidelityAlarmAF::evaluate(char *tuple) {
  incr(tuple);
  return final();
}

int FidelityAlarmAF::getReturnedSize() { return (sizeof(int)); }
FidelityAlarmAF* FidelityAlarmAF::makeNew() { return (new FidelityAlarmAF(att_store)); }
