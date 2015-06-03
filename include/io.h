#ifndef IO_H
#define IO_H


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "gc.h"
//#include <sys/socket.h>
//#include <netdb.h>
//#include <arpa/inet.h>
#include "stack.h"
#include "error.h"

#define __LINUX

#ifdef __WINDOWS
//#include <winsock.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#ifdef __LINUX
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

void closesocket(int socket);

StackItem* openStream(char* address, char* mode);

StackItem* readStream(int stream, int amt);

StackItem* readFileStream(FILE* stream, int amt);

void closeStream(StackItem* item);

#endif
