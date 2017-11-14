/*
 * socket.cpp
 *
 *  Created on: 2017. 4. 3
 *  Last Modified : 2017. 7. 14
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



#define PORT_NUM 6341
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
	int shmid,shmid2,shmid3; //shared memory
	int shmid_recspeed,shmid_recrpm,shmid_recwheel; //shared memory of receive data
	int shmid_recmode;
	int semid;//세마포어
	union semun sem_union;
	void *shared_memory = (void *)0;//~3 -> send data
	void *shared_memory2 = (void *)0;
	void *shared_memory3 = (void *)0;
	void *shared_memory_recspeed = (void *)0;//receive data
	void *shared_memory_recrpm = (void *)0;
	void *shared_memory_recwheel = (void *)0;
	void *shared_memory_recmode = (void *)0;
	char buff[1024];
	int skey = 5678;//shared memory1-> key
	int skey2 = 1234;
	int skey3 = 2345;
	int skey_recspeed = 3456;//shared memory_receive data key
	int skey_recrpm = 4567;
	int skey_recwheel = 6789;
	int skey_recmode = 6243;
//	int sekey = 1234;//semaphore key

	int *process_num;
	int local_num;
	int mode_count=0;
	int test_count=0;

//	struct sembuf semopen = {0,-1,SEM_UNDO};
//	struct sembuf semclose = {0,1,SEM_UNDO};


	//공유메모리 공간 만들기 1
	shmid = shmget((key_t)skey, sizeof(int), 0777|IPC_CREAT);
	if(shmid == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

//	semid = semget((key_t)sekey,1,IPC_CREAT|0777);
//	if(semid == -1)
//	{
//		perror("semget failed : ");
//		return 1;
//	}

	//세마포어 초기화
//	sem_union.val = 1;
//	if(-1 == semctl(semid,0,SETVAL,sem_union))
//	{
//		return 1;
//	}



	//공유메모리 맵핑 1
	shared_memory = shmat(shmid, (void *)0, 0);
	if(!shared_memory)
	{
		perror("shmat failed");
		exit(0);
	}

	//공유메모리 공간 만들기 2
	shmid2 = shmget((key_t)skey2, sizeof(int), 0777|IPC_CREAT);
	if(shmid2 == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

	//공유메모리 맵핑 2
	shared_memory2 = shmat(shmid2, (void *)0, 0);
	if(!shared_memory2)
	{
		perror("shmat failed");
		exit(0);
	}

	//공유메모리 공간 만들기 3
	shmid3 = shmget((key_t)skey3, sizeof(int), 0777|IPC_CREAT);
	if(shmid3 == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

	//공유메모리 맵핑 3
	shared_memory3 = shmat(shmid3, (void *)0, 0);
	if(!shared_memory3)
	{
		perror("shmat failed");
		exit(0);
	}
	//공유메모리 변수 생성
	  int* w_steer;
	  w_steer = (int *)shared_memory;
	  int* w_brake = (int*)shared_memory2;
	  int* w_accel = (int*)shared_memory3;



/********************************Receive Data (Accel,Brake, Wheel) **************************************/
	 



	//Create Shared Memory _ receive Data
	shmid_recspeed = shmget((key_t)skey_recspeed, sizeof(int), 0777|IPC_CREAT);
	if(shmid_recspeed == -1)
	{
		perror("shmget failed of shmid_recspeed : ");
		exit(0);
	}

	//Shared Memory Mapping _ receive Data
	shared_memory_recspeed = shmat(shmid_recspeed, (void *)0, 0);
	if(!shared_memory_recspeed)
	{
		perror("shmat of shared memory_recspeed failed ");
		exit(0);
	}

	//Create Shared Memory _ receive Data
	shmid_recrpm = shmget((key_t)skey_recrpm, sizeof(int), 0777|IPC_CREAT);
	if(shmid_recrpm == -1)
	{
		perror("shmget failed of shmid_recrpm : ");
		exit(0);
	}

	//Shared Memory Mapping _ receive Data
	shared_memory_recrpm = shmat(shmid_recrpm, (void *)0, 0);
	if(!shared_memory_recrpm)
	{
		perror("shmat of shared memory_recrpm failed ");
		exit(0);
	}


	//Create Shared Memory _ receive Data
	shmid_recwheel = shmget((key_t)skey_recwheel, sizeof(int), 0777|IPC_CREAT);
	if(shmid_recwheel == -1)
	{
		perror("shmget failed of shmid_recwheel : ");
		exit(0);
	}

	//Shared Memory Mapping _ receive Data
	shared_memory_recwheel = shmat(shmid_recwheel, (void *)0, 0);
	if(!shared_memory_recwheel)
	{
		perror("shmat of shared memory_recwheel failed ");
		exit(0);
	}
	

	//shared memory_receive data value
	  
	  int* r_speed = (int *)shared_memory_recspeed;
	  int* r_rpm = (int*)shared_memory_recrpm;
	  int* r_steer = (int*)shared_memory_recwheel;



/********************************Change Mode -> Shared_Memory********************************/

		//Create Shared Memory _ receive Mode value
		shmid_recmode = shmget((key_t)skey_recmode, sizeof(int), 0777|IPC_CREAT);
		if(shmid_recmode == -1)
		{
			perror("shmget failed of shmid_recmode : ");
			exit(0);
		}

		//Shared Memory Mapping _ receive Mode value
		shared_memory_recmode = shmat(shmid_recmode, (void *)0, 0);
		if(!shared_memory_recmode)
		{
			perror("shmat of shared memory_recmode failed ");
			exit(0);
		}

		int* r_mode = (int *)shared_memory_recmode;
		*r_mode = 0;//초기 설정은 수동모드로 설정 









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
 int car_steer = 70;
 int car_odometer = 0;

 string car_speed_s;
 string car_rpm_s;
 string car_steer_s;





 int count = 0;
 string str2 = "";

 string test[19] = {"Empty", "Accel","Brake", "Steer","gear","UpperRampStatic","UpperRamp","Ramp","RampAuto","WiperMove","WiperTrigger","WiperOff","WiperAuto"
,"WiperLow","WiperHigh","RightTurn","LeftTurn","Bright","LampTrigger"};



 char send_data[]="dataautosteer0autogear0speed030rpm0500gear0steer-020odometer0eom";


 





 while (1)	
 {
	

/*************************************자율주행모드*********************************************/
	 if(*r_mode == 1)//auto mode

	 {
		 mode_count = 0;
		 //test_count++;

		 send_data[13] = '1';//change autosteer value


		 car_speed = *r_speed;
		 car_rpm = (*r_rpm)*10;
		 car_steer = *r_steer;
		 //car_steer = 105;



//		 car_speed = car_speed;
//		 car_rpm = car_rpm;
//		 car_steer = car_steer;
//
//		 car_speed = car_speed;
//		 car_rpm = car_rpm;
//		 car_steer = car_steer;


		 car_speed_s =ToString(car_speed);
		 car_rpm_s = ToString(car_rpm);
		 car_steer_s = ToString(car_steer);


		 cout<<"mode : "<<*r_mode<<" speed : "<<*r_speed<<"*r_rpm : "<<*r_rpm<<"*r_steer : "<<*r_steer<<endl;


		  if(car_speed_s.length() == 3)
		  {
			  send_data[28] = car_speed_s.at(0);
			  send_data[29] = car_speed_s.at(1);
			  send_data[30] = car_speed_s.at(2);
		  }
		  else if(car_speed_s.length() == 2)
		  {
			  send_data[28] = '0';
			  send_data[29] = car_speed_s.at(0);
			  send_data[30] = car_speed_s.at(1);
		  }
		  else if(car_speed_s.length() == 1)
		  {
			  send_data[28] = '0';
			  send_data[29] = '0';
			  send_data[30] = car_speed_s.at(0);
		  }



		  if(car_rpm_s.length() == 4)
		  {
		  	  send_data[34] = car_rpm_s.at(0);
		  	  send_data[35] = car_rpm_s.at(1);
		  	  send_data[36] = car_rpm_s.at(2);
		  	  send_data[37] = car_rpm_s.at(3);
		  }
		  else if(car_rpm_s.length() == 3)
		  {
			  send_data[34] = '0';
			  send_data[35] = car_rpm_s.at(0);
			  send_data[36] = car_rpm_s.at(1);
		  	  send_data[37] = car_rpm_s.at(2);
		  }
		  else if(car_rpm_s.length() == 2)
		  {
			  send_data[34] = '0';
			  send_data[35] = '0';
			  send_data[36] = car_rpm_s.at(0);
		  	  send_data[37] = car_rpm_s.at(1);
		  }
		  else if(car_rpm_s.length() == 1)
		  {
			  send_data[34] = '0';
			  send_data[35] = '0';
			  send_data[36] = '0';
		  	  send_data[37] = car_rpm_s.at(0);
		  }





		  if(car_steer_s.length() == 4)//ex)-500
		   {
			  send_data[48] = '0';
			  send_data[49] = car_steer_s.at(1);
			  send_data[50] = car_steer_s.at(2);
			  send_data[51] = car_steer_s.at(3);

		   }
		  else if(car_steer_s.length() == 3)
		  {
			  if(car_steer > 0)//ex)500
			  {
				  send_data[48] = '-';
				  send_data[49] = car_steer_s.at(0);
				  send_data[50] = car_steer_s.at(1);
				  send_data[51] = car_steer_s.at(2);
			  }
			  else//ex)-40
			  {
				  send_data[48] = '0';
				  send_data[49] = '0';
				  send_data[50] = car_steer_s.at(1);
				  send_data[51] = car_steer_s.at(2);
			  }
		  }
		  else if(car_steer_s.length() == 2)
		  {
			  if(car_steer > 0)//ex)50
			  {
				  send_data[48] = '-';
				  send_data[49] = '0';
				  send_data[50] = car_steer_s.at(0);
				  send_data[51] = car_steer_s.at(1);
			  }
			  else//ex)-5
			  {
				  send_data[48] = '0';
				  send_data[49] = '0';
				  send_data[50] = '0';
				  send_data[51] = car_steer_s.at(1);
			  }

		  }
		  else if(car_steer_s.length() == 1)//ex)5 or 0
		  {
			  send_data[48] = '-';
			  send_data[49] = '0';
			  send_data[50] = '0';
			  send_data[51] = car_steer_s.at(0);
		  }



		  write(s,(void*)send_data,sizeof(send_data));//메세지 보내기



/*
		 if(test_count>9000)//모드 변환 테스트용
		 {
			printf("mode->0");
			*r_mode =0;
		 }

*/
	 }
/***************************************수동 모드**************************************/
	 else if (*r_mode == 0)
	 {
		
		 if(mode_count == 0)//자동모드에서 수동모드로 바뀌는 순간 audo->Manual로 바꿔주는 작업

		 {
			 printf("count");
			 send_data[13] = 0;//steer mode is manual
			 write(s,(void*)send_data,sizeof(send_data));//change steer mode auto -> manual
			 mode_count++;
		 }

		 cout<<"mode : "<<*r_mode<<endl;

		  strLen = read(s, (void*)ReceiveData, sizeof(ReceiveData));//메세지 받기


		 // ReceiveData[strLen] = 0;


		 strLen = 0;
		 if((strLen = read(s, ReceiveData, sizeof(ReceiveData)-1))>5 && strLen <= 160)
		 {


		   ReceiveData[strLen] = 0;

		   //printf("Msg : %s \nlength = %d\n", ReceiveData, strLen);
		 }
		  string to_string(ReceiveData);
		//  string s_steer = to_string.strsub(26,
		  char *ptr = strtok(ReceiveData, ".");//각각 값으로 쪼개기
		//  printf("%s", ptr);
		  while (ptr != NULL)
		  {
		//   str2 += test[count];
		//   str2 += " : ";
		//   str2 += ptr;
		//   str2 += " ";
			 if(count == 1)
			 {
				*w_accel = atoi(ptr);
				printf("accel : %d\n", *w_accel);
			 }
			 if(count == 2)
			 {
				*w_brake = atoi(ptr);
				printf("brake: %d\n", *w_brake);
			 }

			 if(count == 10)
			 {
				*w_steer = atoi(ptr);
				string to_string(ptr);
				if(*w_steer == 0) {
					*w_steer = atoi(to_string.substr(6, 3).c_str());
				}
				printf("steer : %d\n", *w_steer);
				break;
			 }

		  ptr = strtok(NULL, ".");
		   count++;
		  }


		count =0;





	 }



 }

 


 

 close(s);

 return 0;

}




