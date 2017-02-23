

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>

/*shared memory & semaphore values*/
int shmid;
int semid;
int skey = 5678;
int sekey = 1234;
void *shared_memory = (void *)0;
int* torcs_steer;

struct sembuf semopen = {0,-1,SEM_UNDO};
struct sembuf semclose = {0,1,SEM_UNDO};
/*******************************/
