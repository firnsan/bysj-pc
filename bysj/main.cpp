#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <tchar.h>

#include "loop.h"


#pragma comment(lib, "ws2_32")

#define SERV_PORT 9898

int MakeKeyLparam(int VirtualKey, int flag)
{
	UINT sCode;
	//Firstbyte ; lparam ������ 24-31λ
	UINT Firstbyte;
	switch (flag)
	{
	case WM_KEYDOWN:    Firstbyte = 0;   break;
	case WM_KEYUP:      Firstbyte = 0xC0;break;
	case WM_CHAR:       Firstbyte = 0x20;break;
	case WM_SYSKEYDOWN: Firstbyte = 0x20;break;
	case WM_SYSKEYUP:   Firstbyte = 0xE0;break;
	case WM_SYSCHAR:    Firstbyte = 0xE0;break;
	}

	// 16�C23 Specifies the scan code. 
	UINT iKey = MapVirtualKey(VirtualKey, 0);

	// 0�C15 Specifies the repeat count for the current message. 
	sCode = (Firstbyte << 24) + (iKey << 16) + 20; 
	return sCode;
}


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
	//��Ϊ��gta���Ѿ�����Ҫ����������
	//write_address_to_file();

	//��ʼ��winsock
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
		
		int clientLen = sizeof(clientAddr); //��Ҫ��ʼ����
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