/* Nayeon & Hwancheol */
#ifndef _SHARED_MEMORY_H_
#define _SHARED_MEMORY_H_

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>

/*shared memory & semaphore values*/
extern int shmid,shmid2,shmid3;
extern int shmid_recrpm,shmid_recspeed,shmid_recsteer;
extern int shmid_acc, shmid_lkas, shmid_targetspeed;
extern int shmid_speedfcar;
extern int shmid_distfcar;
extern int shmid_intent;
const int skey = 5678;
const int skey2 = 1234;
const int skey3 = 2345;
const int skey_recspeed = 3456;
const int skey_recrpm = 4567;
const int skey_recsteer = 6789;
const int skey_acc = 4055;
const int skey_lkas = 6243;
const int skey_targetspeed = 5136;
const int skey_speedfcar = 7712;
const int skey_distfcar = 4539;
const int skey_intent = 9998;
//const int sekey = 1234;


extern void *shared_memory;
extern void *shared_memory2;
extern void *shared_memory3;
extern void *shared_memory_recspeed;
extern void *shared_memory_recrpm;
extern void *shared_memory_recsteer;
extern void *shared_memory_acc;
extern void *shared_memory_lkas;
extern void *shared_memory_targetspeed;
extern void *shared_memory_speedfcar;
extern void *shared_memory_distfcar;
extern void *shared_memory_intent;
extern float* torcs_steer;
extern int* torcs_speed;
extern int steer_value;
extern int* ptr_accel;
extern int accel_value;
extern int* ptr_brake;
extern int brake_value;
extern unsigned char* torcs_img;
extern int* torcs_lock;

extern int* rec_rpm;
extern int* rec_speed;
extern float* rec_steer;

extern int* rec_acc;
extern int* rec_lkas;
extern int* rec_targetspeed;
extern float* rec_speedfcar;
extern float* rec_distfcar;
extern int* rec_intent;
/*******************************/

#endif
