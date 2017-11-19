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

#define PORT_NUM 6341
#define SERVER_IP "192.168.0.65"

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
	int shmid_recspeed, shmid_recdist; //shared memory of receive data
	int semid; //세마포어
	union semun sem_union;
	void *shared_memory_recspeed = (void *) 0; //receive data
	void *shared_memory_recdist = (void *) 0;
	char buff[1024];
	int skey_recspeed = 3456; //shared memory_receive data key
	int skey_recdist = 4567;

	int *process_num;
	int local_num;
	int mode_count = 0;
	int test_count = 0;

	FILE *pFile;

	//Create Shared Memory _ receive Data
	shmid_recspeed = shmget((key_t) skey_recspeed, sizeof(int),
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
	shmid_recdist = shmget((key_t) skey_recdist, sizeof(int), 0777 | IPC_CREAT);
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

	//shared memory_receive data value

	double* r_speed = (double*) shared_memory_recspeed;
	double* r_dist = (double*) shared_memory_recdist;

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

	/* Hyerim */
	pFile = fopen("data.txt", "a");
	while (1) {
		/* Hyerim */
		strLen = read(s, ReceiveData, sizeof(ReceiveData));
		if(argv[1] == "1") {
			printf("1\n");
			fprintf(pFile, "%s", ReceiveData);
		}
		if(argv[1] == "2") {
			printf("1\n");
			// TODO : send
		}
		//  write(s,(void*)send_data,sizeof(send_data));//메세지 보내기
		usleep(50000);
	}

	close(s);

	return 0;

}
