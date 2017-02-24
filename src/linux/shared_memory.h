#ifndef _SHARED_MEMORY_H_
#define _SHARED_MEMORY_H_

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>

/*shared memory & semaphore values*/
int shmid;
int shmid2;
int shmid3;
int semid;
const int skey = 5678;
const int skey2 = 1234;
const int skey3 = 2345;
//const int sekey = 1234;
void *shared_memory;
void *shared_memory2;
void *shared_memory3;

int* torcs_steer;
int steer_value;
int* ptr_accel;
int accel_value;
int* ptr_brake;
int brake_value;


/*******************************/

#endif _SHARED_MEMORY_H_
