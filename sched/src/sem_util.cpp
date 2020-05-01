#include <errno.h>
#include <stdio.h>
#include "sem_util.H"


struct sembuf op_lock[2] = {
	{0, 0, 0},            /* wait for sem#0 to become 0					*/
	{0, 1, SEM_UNDO}      /* then increment sem#0 to 1                    */
						/* UNDO to release the lock if process exits    */
						/* before explicitly unlocking                  */
};

struct sembuf op_unlock[2] = {
	{0, -1, SEM_UNDO} 	/* decrement sem#0 by 1 (sets it to 0)			*/
};



int sem_create(key_t key)
{
  register int	id; //, semval;

	if (key == IPC_PRIVATE)
		return(-1);			/* not intended for private semaphores */
	else if (key == (key_t) -1)
		return(-1);			/* probably an ftok() error by caller */

	if ( (id = semget(key, 3, PERMS | IPC_CREAT)) < 0)
		return(-1);			/* permission problem or tables full */

	return(id);

}/* end sem_create */

int sem_open(key_t key)
{
	register int id;
	
	if (key == IPC_PRIVATE)
		return(-1);
	else if (key == (key_t) -1)
		return(-1);

	if ( (id = semget(key, 3, 0)) < 0)
		return(-1);

	return(id);
}/* end sem_open */

void sem_rm(int id)
{
	if (semctl(id, 0, IPC_RMID, 0) < 0)
		printf("can't IPC_RMID\n");
}/* end sem_rm */

int sem_lock(int id)
{
	if (semop(id, &op_lock[0], 2) < 0)
	{
	  // Fixed code to eliminate a bad format string. -cjc 17 Feb 2003
	  // printf("lock failed\n",check_ip(errno));
	  printf("lock failed\n");
	  return (-1);
	}
}
void sem_unlock(int id)
{
	if (semop(id, &op_unlock[0], 1) < 0)
		printf("unlock failed\n");
}
void sem_set_val(int id, int mem_num, int val)
{
    union semun {
        int             val;
        struct semid_ds *buf;
        ushort          *array;
    } semctl_arg;

    semctl_arg.val = val;
    if ( (semctl(id, mem_num, SETVAL, semctl_arg)) < 0)
	{
        printf("cant SETVAL errno :: %s\n",check_ip(errno));
	}
}
int sem_get_val(int id, int mem_num)
{
    register int    semval;

    if ( (semval = semctl(id, mem_num, GETVAL, 0)) < 0)
        printf("cant GETVAL\n");

    return(semval);
}

