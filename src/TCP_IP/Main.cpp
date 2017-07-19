/*
 * Main.cpp
 *
 *  Created on: 2017. 7. 19.
 *      Author: kang
 */

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>

#define TCPIP_PORT_NUM 6342
#define TCPIP_SERVER_IP "192.168.56.1"

int my_socket, conn_desc;
struct sockaddr_in addr;
struct sockaddr_in addr_client;
socklen_t size_client;

int shmid;
char* shared_memory[1024];

char buff[1024];
int skey = 7149;

char* receive_data[1024];

void tcpip();
void init_shared_memory();
void delete_shared_memory();

int main(int argc, char* argv[]) {
	init_shared_memory();
	tcpip();
	delete_shared_memory();
	return 0;
}
/* shared memory */


void init_shared_memory() {
	shmid = shmget((key_t) skey, sizeof(int), 0777 | IPC_CREAT);
	if (shmid == -1) {
		perror("shmget failed");
		exit(0);
	}
	shared_memory[0] = (char*) shmat(shmid, (void *) 0, 0);
	if (!shared_memory) {
		perror("shmat failed");
		exit(0);
	}
	receive_data[0] = shared_memory[0];
}
void delete_shared_memory() {
	if (shmdt(shared_memory) < 0) {
		perror("shmdt failed");
		exit(0);
	}
}
void tcpip() {
	my_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (my_socket == -1) {
		printf("Socket 오류\n");
		exit(1);
	}
	bzero((char *)&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(TCPIP_PORT_NUM);
	addr.sin_addr.s_addr = inet_addr(TCPIP_SERVER_IP);

	if(bind(my_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		printf("Failed to bind\n");
	}
	listen(my_socket, 1);
	printf("Waiting for connection...\n");
	/* tcp ip */
	size_client = sizeof(addr_client);
	while (true) {
		conn_desc = accept(my_socket, (struct sockaddr *) &addr_client,
				&size_client);
		if (conn_desc == -1) {
			printf("Failed accepting connection\n");
			return;
		}
		bzero(buff, sizeof(buff));
		strncpy(buff, *receive_data, sizeof(buff));
		write(conn_desc, buff, sizeof(buff));
	}
}
