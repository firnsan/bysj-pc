#ifndef FIRNSAN_LOOP_H
#define FIRNSAN_LOOP_H

#include <winsock2.h>

extern char forwardFlag, backFlag, leftFlag, rightFlag;

void loop(SOCKET clientSocket);
void vedioLoop(SOCKET clientSocket);

void snap();

#endif