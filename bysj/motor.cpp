#include <windows.h>


char forwardFlag = 0,
backFlag = 0,
leftFlag = 0,
rightFlag = 0;


void motor_status(char cur, char pre, char acition) {
	if (cur==1)
		keybd_event(acition, 0, 0, 0);
	else if (cur==pre && cur==0) {
		//low level,do nothing
	}else if (cur!=pre && cur==0) {
		keybd_event(acition, 0, KEYEVENTF_KEYUP, 0);
	}
}

DWORD motor_thread(LPVOID *param){
	char preFoward = forwardFlag;
	char preBack = backFlag;
	char preLeft = leftFlag;
	char preRight = rightFlag;

	while (1) {
		motor_status(forwardFlag, preFoward, 'W');
		motor_status(backFlag, preBack, 'S');
		motor_status(leftFlag, preLeft, 'A');
		motor_status(rightFlag, preRight, 'D');

		preFoward = forwardFlag;
		preBack = backFlag;
		preLeft = leftFlag;
		preRight = rightFlag;

		Sleep(20);
	}
}

void motor_on() {
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)motor_thread, 
		NULL, 0, NULL);
}


void motor_off(){

}