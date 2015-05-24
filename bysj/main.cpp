#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <tchar.h>

#include "loop.h"


#pragma comment(lib, "ws2_32")

#define SERV_PORT 9898


void write_address_to_file(){
	printf("%p %p %p %p\n", &forwardFlag, &backFlag, &leftFlag, &rightFlag);

	FILE *fp = fopen("address.txt","w");
	if (!fp) {
		printf("open file error\n");
		exit(-1);
	}
	int addr = (int)&forwardFlag;
	fwrite(&addr, 4, 1, fp);
	fclose(fp);
}

int main(int argc, char *argv[]) {
	//因为用gta，已经不需要按键精灵了
	//write_address_to_file();

	//初始化winsock
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
	
	SOCKET serverSocket, clientSocket;
	struct sockaddr_in serverAddr, clientAddr;

	serverSocket =  socket(AF_INET, SOCK_STREAM, 0);
	
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERV_PORT);

	bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	listen(serverSocket, 5);

	while(1) {
		
		int clientLen = sizeof(clientAddr); //需要初始化的
		printf("listening...\n");
		clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
		if (clientSocket<0) {
			printf("accept error");
			return -1;
		}

		printf("accepted a client, %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		
		loop(clientSocket);

		printf("a client quited\n");
		

	} 
	return 0;
}