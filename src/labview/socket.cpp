/*
 * socket.cpp
 *
 *  Created on: 2017. 2. 22.
 *      Author: NaYeon Kim
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys\types.h>
#include <sys\socket.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <arpa\inet.h>
#include <unistd.h>
#include <sstream>



#define PORT_NUM 6341
#define SERVER_IP "192.168.0.8"

using namespace std;

template <typename T>
string ToString(T val)
{
	stringstream stream;
	stream << val;
	return stream.str();

}

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
};





int main(void)
{


	char ReceiveData[1024];
	int strLen;
	int shmid;
	int semid;//세마포어
	union semun sem_union;
	void *shared_memory = (void *)0;
	char buff[1024];
	int skey = 5678;
	int sekey = 1234;

	int *process_num;
	int local_num;

	struct sembuf semopen = {0,-1,SEM_UNDO};
	struct sembuf semclose = {0,1,SEM_UNDO};


	//공유메모리 공간 만들기
	shmid = shmget((key_t)skey, sizeof(int), 0666|IPC_CREAT);
	if(shmid == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

	semid = semget((key_t)sekey,1,0666);
	if(semid == -1)
	{
		perror("semget failed : ");
		return 1;
	}

	//세마포어 초기화
	sem_union.val = 1;
	if(-1 == semctl(semid,0,SETVAL,sem_union))
	{
		return 1;
	}



	//공유메모리 맵핑
	shared_memory = shmat(shmid, (void *)0, 0);
	if(!shared_memory)
	{
		perror("shmat failed");
		exit(0);
	}


	//공유메모리 변수 생성
	  int* w_steer;
	  w_steer = (int *)shared_memory;









	  int s = socket(AF_INET,SOCK_STREAM,0);//socket 생성
	  if (s == -1)
	  {
		  cout << "Socket 오류입니다" << endl;
		  exit(1);
	  }

	  struct sockaddr_in addr;
	  addr.sin_family = AF_INET;
	  addr.sin_port = htons(PORT_NUM);
	  addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	  if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	  {
		  cout << "서버 연결 오류" << endl;
		  exit(1);
	  }

	  cout << "서버 연결 성공" << endl;



 /*************** TCP Setting ****************/

 int car_autosteer = 0;
 int car_autogear = 0;
 int car_speed = 10;
 int car_rpm = 110;
 int car_gear = 0;
 int car_steer = 0;
 int car_odometer = 0;



 int count = 0;
 string str2 = "";

 string test[19] = {"Empty", "Accel","Brake", "Steer","gear","UpperRampStatic","UpperRamp","Ramp","RampAuto","WiperMove","WiperTrigger","WiperOff","WiperAuto","WiperLow","WiperHigh","RightTurn","LeftTurn","Bright","LampTrigger" };





 while (1)
 {


	 if(semop(semid,&semopen,1) == -1)
	 {
		return 1;
	 }
	 car_steer = *w_steer;
	 sleep(1);

  string str= "dataautosteer" + ToString(car_autosteer) + +"autogear" + ToString(car_autogear)
   + "speed" + ToString(car_speed)
   + "rpm" + ToString(car_rpm)
   + "gear" + ToString(car_gear)
   + "steer" + ToString(car_steer)
   + "odometer" + ToString(car_odometer)
   + "eom";
  const char* cstr = str.c_str();






  send(s,cstr,60, 0);//메세지 보내기


  strLen = recv(s, ReceiveData, sizeof(ReceiveData) - 1, 0);//메세지 받기


  ReceiveData[strLen] = 0;




  char *ptr = strtok(ReceiveData, ".");//각각 값으로 쪼개기

  while (ptr != NULL)
  {
//   str2 += test[count];
//   str2 += " : ";
//   str2 += ptr;
//   str2 += " ";

	 if(count == 3)
	 {
		 *w_steer = atoi(ptr);
	 }

   ptr = strtok(NULL, ".");
   count++;
  }







  cout << str2 << '\n';
  //printf("Msg : %s \n", ReceiveData);

  semop(semid, &semclose, 1);

  sleep(100);



 }



 close(s);

 return 0;
}




