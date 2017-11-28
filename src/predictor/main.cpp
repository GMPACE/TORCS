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
#include <fstream>
#include <algorithm>
#define PORT_NUM 9998
#define PORT_TALKER 9999
#define SERVER_IP "192.168.43.194"


using namespace std;

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

int main(int argc, char* argv[]) {
	char ReceiveData[256];
	int strLen;
	int shmid_recspeed, shmid_recdist, shmid_recintent; //shared memory of receive data
	void *shared_memory_recspeed = (void *) 0; //receive data
	void *shared_memory_recdist = (void *) 0;
	void *shared_memory_recintent = (void *) 0;
	int skey_recspeed = 7712; //shared memory_receive data key
	int skey_recdist = 4539;
	int skey_recintent = 9998;

	int *process_num;
	int local_num;
	int mode_count = 0;
	int test_count = 0;

	ofstream pFILE("data.txt");

	//Create Shared Memory _ receive Data
	shmid_recspeed = shmget((key_t) skey_recspeed, sizeof(float),
			0777 | IPC_CREAT);
	if (shmid_recspeed == -1) {
		perror("shmget failed of shmid_recspeed : ");
		exit(0);
	}

	//Shared Memory Mapping _ receive Data
	shared_memory_recspeed = shmat(shmid_recspeed, (void *) 0, 0);
	if (!shared_memory_recspeed) {
		perror("shmat of shared memory_recspeed failed ");
		exit(0);
	}

	//Create Shared Memory _ receive Data
	shmid_recdist = shmget((key_t) skey_recdist, sizeof(float),
			0777 | IPC_CREAT);
	if (shmid_recspeed == -1) {
		perror("shmget failed of shmid_recdist : ");
		exit(0);
	}

	//Shared Memory Mapping _ receive Data
	shared_memory_recdist = shmat(shmid_recdist, (void *) 0, 0);
	if (!shared_memory_recdist) {
		perror("shmat of shared memory_recdist failed ");
		exit(0);
	}
	//Create Shared Memory _ receive Data
	shmid_recintent = shmget((key_t) skey_recintent, sizeof(int),
			0777 | IPC_CREAT);
	if (shmid_recintent == -1) {
		perror("shmget failed of shmid_recdist : ");
		exit(0);
	}

	//Shared Memory Mapping _ receive Data
	shared_memory_recintent = shmat(shmid_recintent, (void *) 0, 0);
	if (!shared_memory_recintent) {
		perror("shmat of shared memory_recdist failed ");
		exit(0);
	}
	//shared memory_receive data value

	float* r_speed = (float*) shared_memory_recspeed;
	float* r_dist = (float*) shared_memory_recdist;
	int* r_intent = (int*) shared_memory_recintent;

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
	cout << "EyeTracker(Server) 연결 성공" << endl;

	/* Hwancheol : Connect to talker */
	int s_t = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in addr_talker;
	addr_talker.sin_family = AF_INET;
	addr_talker.sin_port = htons(PORT_TALKER);
	addr_talker.sin_addr.s_addr = htonl(INADDR_ANY);

	 //binds connection
	if (bind(s_t, (struct sockaddr *)&addr_talker, sizeof(addr_talker)) < 0) {
		perror("bind error");
	}
	if (listen(s_t, SOMAXCONN)) {
		perror("listen error");
	}
	int s_c;
	struct sockaddr_in addr_client;
	int addlen;
	addlen = sizeof(addr_client);
	s_c = accept(s_t, (struct sockaddr*)&addr_client, (socklen_t *)&addlen);
	/*************** TCP Setting ****************/

	/* Hyerim */
	while (1) {
		strLen = read(s, ReceiveData, sizeof(ReceiveData));
		string str_speed = ToString((int)*r_speed);
		string str_dist = ToString((int)*r_dist);
		string str_intent = ToString((int)*r_intent);
		//printf("str_speed : %s\n", str_speed.c_str());
		//printf("str_dist : %s\n", str_dist.c_str());
		strcat(ReceiveData, str_speed.c_str());
		strcat(ReceiveData, "/");
		strcat(ReceiveData, str_dist.c_str());
		strcat(ReceiveData, "/");
		strcat(ReceiveData, str_intent.c_str());
		strcat(ReceiveData, "/");
		if (!strcmp(argv[1], "fw")) {
			printf("%s\n", ReceiveData);
			std: string string_RD(ReceiveData);
			if (std::count(string_RD.begin(), string_RD.end(), '/') == 4) {
				pFILE << ReceiveData << endl;
			}
		}
		if (!strcmp(argv[1], "tf")) {
			char SendData[64];
			snprintf(SendData, 64, "%c#%s#%s#", ReceiveData[0], str_speed.c_str(), str_dist.c_str());
			printf("SendData : %s\n", SendData);
			write(s_c, SendData, 64);
			char temp[1];
			read(s_c, temp, 1);
			memset(SendData, 10x00, 64);
		}
		memset(ReceiveData, 0x00, 256);
		usleep(50000);
	}
	pFILE.close();
	close(s_t);
	close(s);
	return 0;

}
