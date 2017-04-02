/*
 * socket.cpp
 *
 *  Created on: 2017. 3. 29.
 *      Author: NaYeon Kim
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>



#define PORT_NUM 6342
#define SERVER_IP "192.168.0.65"

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


	char ReceiveData[256];
	int strLen;

	int shmid_acc,shmid_lkas,shmid_targetspeed; //shared memory of receive data

//	void *shared_memory_acc = (void *)0;//receive data
//	void *shared_memory_lkas = (void *)0;
//	void *shared_memory_targetspeed = (void *)0;
	char buff[1024];

	int skey_acc = 4055;//shared memory_receive data key
	int skey_lkas = 6243;
	int skey_targetspeed = 5136;
//	int sekey = 1234;//semaphore key

	int *process_num;
	int local_num;

	struct sembuf semopen = {0,-1,SEM_UNDO};
	struct sembuf semclose = {0,1,SEM_UNDO};




/********************************Receive Data  **************************************/




	//Create Shared Memory _ receive Data
	shmid_acc = shmget((key_t)skey_acc, sizeof(int), 0777|IPC_CREAT);
	if(shmid_acc == -1)
	{
		perror("shmget failed of shmid_acc : ");
		exit(0);
	}

	//Shared Memory Mapping _ receive Data
	void* shared_memory_acc = shmat(shmid_acc, (void *)0, 0);
	if(!shared_memory_acc)
	{
		perror("shmat of shared memory_acc failed ");
		exit(0);
	}

	//Create Shared Memory _ receive Data
	shmid_lkas = shmget((key_t)skey_lkas, sizeof(int), 0777|IPC_CREAT);
	if(shmid_lkas == -1)
	{
		perror("shmget failed of shmid_lkas : ");
		exit(0);
	}

	//Shared Memory Mapping _ receive Data
	void* shared_memory_lkas = shmat(shmid_lkas, (void *)0, 0);
	if(!shared_memory_lkas)
	{
		perror("shmat of shared memory_lkas failed ");
		exit(0);
	}


	//Create Shared Memory _ receive Data
	shmid_targetspeed = shmget((key_t)skey_targetspeed, sizeof(int), 0777|IPC_CREAT);
	if(shmid_targetspeed == -1)
	{
		perror("shmget failed of shmid_targetspeed : ");
		exit(0);
	}

	//Shared Memory Mapping _ receive Data
	void* shared_memory_targetspeed = shmat(shmid_targetspeed, (void *)0, 0);
	if(!shared_memory_targetspeed)
	{
		perror("shmat of shared memory_targetspeed failed ");
		exit(0);
	}


	//shared memory_receive data value

	  int* r_acc = (int *)shared_memory_acc;
	  int* r_lkas = (int*)shared_memory_lkas;
	  int* r_targetspeed = (int*)shared_memory_targetspeed;





/*************************************socket connecting*************************************/




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
 int car_speed = 40;
 int car_rpm = 500;
 int car_gear = 0;
 int car_steer = 30;
 int car_odometer = 0;

 string car_acc_s;
 string car_lkas_s;
 string car_targetspeed_s;





 int count = 0;
 string str2 = "";

 string test[19] = {"Empty", "Accel","Brake", "Steer","gear","UpperRampStatic","UpperRamp","Ramp","RampAuto","WiperMove","WiperTrigger","WiperOff","WiperAuto"
,"WiperLow","WiperHigh","RightTurn","LeftTurn","Bright","LampTrigger" };



 char send_data[]="dataautosteer0autogear0speed000rpm0000gear0steer-000odometer0eom";



 while (1)
 {



//  string str= "dataautosteer" + ToString(car_autosteer) + +"autogear" + ToString(car_autogear)
//   + "speed" + ToString(car_speed)
//   + "rpm" + ToString(car_rpm)
//   + "gear" + ToString(car_gear)
//   + "steer" + ToString(car_steer)
//   + "odometer" + ToString(car_odometer)
//   + "eom";
//  const char* cstr = str.c_str();



/*******************************************자율주행모드*********************************************/



	 int car_acc = *r_acc;
	 int car_lkas = *r_lkas;
	 int car_targetspeed = *r_targetspeed;
	 //car_steer = -70;


	 car_acc_s =ToString(car_acc);
	 car_lkas_s = ToString(car_lkas);
	 car_targetspeed_s = ToString(car_targetspeed);



	 cout<<"acc : "<<*r_acc<<"*r_lkas : "<<*r_lkas<<"*r_targetspeed : "<<*r_targetspeed<<endl;



//  if(car_speed_s.length() == 3)
//  {
//	  send_data[28] = car_speed_s.at(0);
//	  send_data[29] = car_speed_s.at(1);
//	  send_data[30] = car_speed_s.at(2);
//  }
//  else if(car_speed_s.length() == 2)
//  {
//	  send_data[28] = '0';
//	  send_data[29] = car_speed_s.at(0);
//	  send_data[30] = car_speed_s.at(1);
//  }
//  else if(car_speed_s.length() == 1)
//  {
//	  send_data[28] = '0';
//	  send_data[29] = '0';
//	  send_data[30] = car_speed_s.at(0);
//  }
//
//
//
//
//
//
//  if(car_rpm_s.length() == 4)
//  {
//  	  send_data[34] = car_rpm_s.at(0);
//  	  send_data[35] = car_rpm_s.at(1);
//  	  send_data[36] = car_rpm_s.at(2);
//  	  send_data[37] = car_rpm_s.at(3);
//  }
//  else if(car_rpm_s.length() == 3)
//  {
//	  send_data[34] = '0';
//	  send_data[35] = car_rpm_s.at(0);
//	  send_data[36] = car_rpm_s.at(1);
//  	  send_data[37] = car_rpm_s.at(2);
//  }
//  else if(car_rpm_s.length() == 2)
//  {
//	  send_data[34] = '0';
//	  send_data[35] = '0';
//	  send_data[36] = car_rpm_s.at(0);
//  	  send_data[37] = car_rpm_s.at(1);
//  }
//  else if(car_rpm_s.length() == 1)
//  {
//	  send_data[34] = '0';
//	  send_data[35] = '0';
//	  send_data[36] = '0';
//  	  send_data[37] = car_rpm_s.at(0);
//  }
//
//
//
//
//
//
//
//
//
//
//
//  if(car_steer_s.length() == 4)//ex)-500
//   {
//	  send_data[48] = '0';
//	  send_data[49] = car_steer_s.at(1);
//	  send_data[50] = car_steer_s.at(2);
//	  send_data[51] = car_steer_s.at(3);
//
//   }
//  else if(car_steer_s.length() == 3)
//  {
//	  if(car_steer > 0)//ex)500
//	  {
//		  send_data[48] = '-';
//		  send_data[49] = car_steer_s.at(0);
//		  send_data[50] = car_steer_s.at(1);
//		  send_data[51] = car_steer_s.at(2);
//	  }
//	  else//ex)-40
//	  {
//		  send_data[48] = '0';
//		  send_data[49] = '0';
//		  send_data[50] = car_steer_s.at(1);
//		  send_data[51] = car_steer_s.at(2);
//	  }
//  }
//  else if(car_steer_s.length() == 2)
//  {
//	  if(car_steer > 0)//ex)50
//	  {
//		  send_data[48] = '-';
//		  send_data[49] = '0';
//		  send_data[50] = car_steer_s.at(0);
//		  send_data[51] = car_steer_s.at(1);
//	  }
//	  else//ex)-5
//	  {
//		  send_data[48] = '0';
//		  send_data[49] = '0';
//		  send_data[50] = '0';
//		  send_data[51] = car_steer_s.at(1);
//	  }
//
//  }
//  else if(car_steer_s.length() == 1)//ex)5 or 0
//  {
//	  send_data[48] = '-';
//	  send_data[49] = '0';
//	  send_data[50] = '0';
//	  send_data[51] = car_steer_s.at(0);
//  }
//
//
//
//
//  write(s,(void*)send_data,sizeof(send_data));//메세지 보내기


/***********************************************수동 모드**********************************************/

//  strLen = read(s, (void*)ReceiveData, sizeof(ReceiveData));//메세지 받기
//
//
// // ReceiveData[strLen] = 0;
//
//
// strLen = 0;
// if((strLen = read(s, ReceiveData, sizeof(ReceiveData)-1))>5 && strLen <= 160)
// {
//
//
//   ReceiveData[strLen] = 0;
//
//   //printf("Msg : %s \nlength = %d\n", ReceiveData, strLen);
// }
//  string to_string(ReceiveData);
////  string s_steer = to_string.strsub(26,
//  char *ptr = strtok(ReceiveData, ".");//각각 값으로 쪼개기
////  printf("%s", ptr);
//  while (ptr != NULL)
//  {
////   str2 += test[count];
////   str2 += " : ";
////   str2 += ptr;
////   str2 += " ";
//	 if(count == 1)
//	 {
//		*w_accel = atoi(ptr);
//		printf("accel : %d\n", *w_accel);
//	 }
//	 if(count == 2)
//	 {
//		*w_brake = atoi(ptr);
//		printf("brake: %d\n", *w_brake);
//	 }
//	 if(count == 3)
//	 {
//		*w_steer = atoi(ptr);
//		string to_string(ptr);
//		if(*w_steer == 0) {
//			*w_steer = atoi(to_string.substr(6, 3).c_str());
//		}
//		printf("steer : %d\n", *w_steer);
//		break;
//	 }
//
//  ptr = strtok(NULL, ".");
//   count++;
//  }
//
//
//count =0;
//




 }








 close(s);

 return 0;
}




