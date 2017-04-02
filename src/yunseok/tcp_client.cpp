/*
 * tcp_client.cpp
 *
 *  Created on: 2017. 4. 2.
 *      Author: Yunseok & Hwancheol
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

/* Hwancheol */
#define PORT_NUM 6342
#define SERVER_IP "192.168.0.65"
#define NUM_OF_DATA 3
enum data {
	ACC, LKAS, TARGET_SPEED
};

class SharedMemory_Manager {
private:
	int shmid[NUM_OF_DATA];
	void* shared_memory[NUM_OF_DATA];
	int skey[NUM_OF_DATA];
public:
	int* value[NUM_OF_DATA];
	SharedMemory_Manager() {
		skey[ACC] = 4055;
		skey[LKAS] = 6243;
		skey[TARGET_SPEED] = 5136;
	}
	void map() {

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
			value[i] = (int *) shared_memory[i];
		}
	}

};
/* Hwancheol */
int main(void) {
	/* Hwancheol : Shared Memory Part */
	SharedMemory_Manager* shm_manager = new SharedMemory_Manager();
	shm_manager->map();
	/* Hwancheol */

	/* TCP/IP Part */

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

	while (1) {
		stringstream acc, lkas, target_speed, test;
		acc << *(shm_manager->value[ACC]);
		lkas << *(shm_manager->value[LKAS]);
		target_speed << *(shm_manager->value[TARGET_SPEED]);
		cout << "ACC : " << *(shm_manager->value[ACC]) << " LKAS : "
				<< *(shm_manager->value[LKAS]) << " TARGET_SPEED : "
				<< *(shm_manager->value[TARGET_SPEED]) <<  endl;

		string smessage = "ACC:" + acc.str() + "&LKAS:" + lkas.str()
				+ "&TARGETSPEED:" + target_speed.str() + "\n";
		const char* message = smessage.c_str();
		//printf(message);
		write(s, (void*) message, sizeof(message));
	}

	close(s);
	delete shm_manager;
	return 0;
}
