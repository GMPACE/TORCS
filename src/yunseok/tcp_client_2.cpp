/*
 * socket.cpp
 *
 *  Created on: 2017. 4. 03
 *      Author: Hwancheol
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

using namespace std;

#define PORT_NUM 6342
#define SERVER_IP "192.168.0.65"
#define NUM_OF_DATA 3
enum data {
	ACC, LKAS, TARGET_SPEED
};

template<typename T>
string ToString(T val) {
	stringstream stream;
	stream << val;
	return stream.str();

}

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
};

int main(void) {

	char ReceiveData[256];
	int strLen;

	int shmid[NUM_OF_DATA];
	int skey[NUM_OF_DATA];
	void* shared_memory[NUM_OF_DATA];
	int* r_value[NUM_OF_DATA];
	char buff[1024];

	skey[ACC] = 4055;
	skey[LKAS] = 6243;
	skey[TARGET_SPEED] = 5136;

	int *process_num;
	int local_num;

	struct sembuf semopen = { 0, -1, SEM_UNDO };
	struct sembuf semclose = { 0, 1, SEM_UNDO };

	/******************************************Receive Data **********************************************/
	for (int i = 0; i < NUM_OF_DATA; i++) {
		//Create Shared Memory _ receive Data
		shmid[i] = shmget((key_t) skey[i], sizeof(int), 0777 | IPC_CREAT);
		if (shmid[i] == -1) {
			perror("shmget failed : ");
			exit(0);
		}

		//Shared Memory Mapping _ receive Data
		shared_memory[i] = shmat(shmid[i], (void *) 0, 0);
		if (!shared_memory[i]) {
			perror("shmat of shared memory failed ");
			exit(0);
		}
		//shared memory_receive data value

		r_value[i] = (int *) shared_memory[i];
	}

	/*************************************socket connecting*************************************/

	int s = socket(AF_INET, SOCK_STREAM, 0); //socket 생성
	if (s == -1) {
		cout << "Socket 오류입니다" << endl;
		exit(1);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT_NUM);
	addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	if (connect(s, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
		cout << "서버 연결 오류" << endl;
		exit(1);
	}

	cout << "서버 연결 성공" << endl;

	/*************** TCP Setting ****************/
	string car_acc_s;
	string car_lkas_s;
	string car_targetspeed_s;

	char send_data[] = "dataACC0LKAS0TARGETSPEED000eom";
	while (1) {

		car_acc_s = ToString(*(r_value[0]));
		car_lkas_s = ToString(*(r_value[1]));
		car_targetspeed_s = ToString(*(r_value[2]));

		send_data[7] = car_acc_s.at(0);
		send_data[12] = car_lkas_s.at(0);
		if (car_targetspeed_s.length() == 3) {
			send_data[24] = car_targetspeed_s.at(0);
			send_data[25] = car_targetspeed_s.at(1);
			send_data[26] = car_targetspeed_s.at(2);
		}
		if (car_targetspeed_s.length() == 2) {
			send_data[25] = car_targetspeed_s.at(0);
			send_data[26] = car_targetspeed_s.at(1);
		}

		cout << "acc : " << *(r_value[0]) << "*r_lkas : " << *(r_value[1])
				<< "*r_targetspeed : " << *(r_value[2]) << endl;

		write(s, (void*) send_data, 256);
	}

	close(s);

	return 0;
}

