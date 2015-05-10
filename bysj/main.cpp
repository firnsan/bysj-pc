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
	//Firstbyte ; lparam 参数的 24-31位
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

	// 16–23 Specifies the scan code. 
	UINT iKey = MapVirtualKey(VirtualKey, 0);

	// 0–15 Specifies the repeat count for the current message. 
	sCode = (Firstbyte << 24) + (iKey << 16) + 20; 
	return sCode;
}

int main(int argc, char *argv[]) {

	printf("%p %p %p %p\n", &forwardFlag, &backFlag, &leftFlag, &rightFlag);

	//snap(); //截图
	//getchar();
	//return 0;

	FILE *fp = fopen("address.txt","w");
	if (!fp) {
		printf("open file error\n");
		exit(-1);
	}
	int addr = (int)&forwardFlag;
	fwrite(&addr, 4, 1, fp);
	fclose(fp);

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

	/*
	//无标题 - 记事本
	HWND hGame = FindWindow(NULL, _T("Euro Truck Simulator 2"));
	

	if (!hGame) {
		printf("cannot find the game\n");
		exit(-1);
	}*/


	//long x = static_cast<long>(65535.0f / (GetSystemMetrics(SM_CXSCREEN) - 1) * 100);
	


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
		
		
		//发送alt+f 到 notepad：
		/*
		::PostMessage(hGame, WM_SYSKEYDOWN, VK_MENU, MakeKeyLparam(VK_MENU, WM_SYSKEYDOWN));
		::PostMessage(hGame, WM_SYSKEYDOWN, 'F', MakeKeyLparam('F', WM_SYSKEYDOWN));
		::PostMessage(hGame, WM_SYSKEYUP, 'F', MakeKeyLparam('F', WM_SYSKEYUP));
		::PostMessage(hGame, WM_KEYUP, VK_MENU, MakeKeyLparam(VK_MENU, WM_KEYUP));*/

		
		/*
		// 不行
		if (!::PostMessage(hGame, WM_KEYDOWN, 'F', MakeKeyLparam('F', WM_KEYDOWN))) 
			{printf("aaa\n");}
		if (!::PostMessage(hGame, WM_CHAR, 'F', MakeKeyLparam('F', WM_CHAR)))
			{printf("aaa\n");}
		if (!::PostMessage(hGame, WM_KEYUP, 'F', MakeKeyLparam('F', WM_KEYUP)))
			{printf("aaa\n");}*/
		
		
		/*
		INPUT input[2];
		memset(input, 0, 2 * sizeof(INPUT));
		input[0].type = INPUT_KEYBOARD;
		input[0].ki.wVk = 'F';
		input[0].ki.dwFlags
		SendInput(1, input, sizeof(INPUT));

		input[1].type = INPUT_KEYBOARD;
		input[1].ki.wVk = 'F';
		input[1].ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, input + 1, sizeof(INPUT));*/


		/*
		keybd_event(16,0,0,0); //按下Shift键
		keybd_event('A',0,0,0); //按下a键
		keybd_event('A',0,KEYEVENTF_KEYUP,0); //松开a键
		keybd_event(16,0,KEYEVENTF_KEYUP,0); //松开Shift键*/


		
		/* //可以
		INPUT input;
		input.type = INPUT_MOUSE;
		input.mi.dx = x;
		input.mi.dy = x;
		input.mi.dwFlags =  MOUSEEVENTF_MOVE;
		SendInput(1, &input, sizeof(INPUT));
		*/

		//printf("sended\n");
		//Sleep(3000);

	} 
	return 0;
}