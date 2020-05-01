#ifndef SEM_UTIL_H
#define SEM_UTIL_H


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "check_ip.H"

#define SEMKEY 4567L /* key value for semget() */
#define PERMS  0666

extern struct sembuf op_lock[2];
extern struct sembuf op_unlock[2];


int sem_create(key_t key);

int sem_open(key_t key);

void sem_rm(int id);

int sem_lock(int id);

void sem_unlock(int id);

void sem_set_val(int id, int mem_num, int val);

int sem_get_val(int id, int mem_num);
// SEM_UTIL_H
#endif
