#ifndef _SHARED_MEMORY_H_
#define _SHARED_MEMORY_H_

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>

/*shared memory & semaphore values*/
int shmid,shmid2,shmid3, shmid4;
int shmid_recrpm,shmid_recspeed,shmid_recsteer;
int shmid_acc, shmid_lkas, shmid_targetspeed;
int semid;
const int skey = 5678;
const int skey2 = 3147;
const int skey3 = 2345;
const int skey4 = 2015;
const int skey_recspeed = 3456;
const int skey_recrpm = 4567;
const int skey_recsteer = 6789;
const int skey_acc = 4055;
const int skey_lkas = 6243;
const int skey_targetspeed = 5136;
//const int sekey = 1234;


void *shared_memory;
void *shared_memory2;
void *shared_memory3;
void *shared_memory4;
void *shared_memory_recspeed;
void *shared_memory_recrpm;
void *shared_memory_recsteer;
void *shared_memory_acc;
void *shared_memory_lkas;
void *shared_memory_targetspeed;

int* torcs_steer;
int steer_value;
int* ptr_accel;
int accel_value;
int* ptr_brake;
int brake_value;
int* ptr_brake2;


int* rec_rpm;
int* rec_speed;
int* rec_steer;

int* rec_acc;
int* rec_lkas;
int* rec_targetspeed;

/*******************************/

#endif _SHARED_MEMORY_H_
