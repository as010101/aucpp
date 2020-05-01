//static char *cvs_id="@(#) $Id: shm_util.C,v 1.2 2003/03/26 19:06:26 cjc Exp $";
#include "shm_util.H"

int shm_create(key_t key, int size)
{
    return (shmget(key, size, PERMS | IPC_CREAT));
}
int shm_open(key_t key, int size)
{
    return (shmget(key, size, 0));
}
void *shm_attach(int shmid)
{
	return( shmat(shmid, (char *) 0, 0));
}
int shm_rm(int shmid)
{
    return (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) < 0);
}
