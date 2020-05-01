#ifndef SHM_UTIL_H
#define SHM_UTIL_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "check_ip.H"

// change this back to 8888! -cjc
#define SHM_KEY IPC_PRIVATE
#define PERMS 0666

int shm_create(key_t key, int size);
int shm_open(key_t key, int size);
void* shm_attach(key_t key);
int shm_rm(int shmid);
//  SHM_UTIL_H
#endif
