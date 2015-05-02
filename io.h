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

void closesocket(int socket) { close(socket); }

StackItem* openStream(char* address, char* mode) {
	//printf("called openStream\n");
	int read = 0, write = 0;
	for (int i = 0 ; mode[i] != 0 ; i ++) {
		if (mode[i] == 'r') read = 1;
		if (mode[i] == 'w') write = 1;
	}
	int slash = 0, slashCount = 0, slashIndex = -1, colon = 0, colonIndex = -1;
	for (int i = 0 ; address[i] != 0 ; i ++) {
		if (address[i] == '/') {
			slash = 1;
			slashCount ++;
			if (colon) {
				if (slashCount == 3) {
					slashIndex = i;
				}
			} else if (slashIndex == -1) {
				slashIndex = i;
			}
		}
		if (address[i] == ':') {
			if (slash == 0) {
				colon = 1;
				colonIndex = i;
			}
		}
	}
	if (colon) {
        #ifdef __WINDOWS
            WSADATA wsaData;
            WSAStartup(0x0202, &wsaData);
        #endif // __WINDOWS
		char colonTemp = address[colonIndex];
		char slashTemp = address[slashIndex];
		address[colonIndex] = 0;
		address[slashIndex] = 0;
		//printf("type: %s\n", address);
		if (strcmp(address, "http") == 0) {
			char* host = calloc(strlen(&address[colonIndex + 1]) + 1, sizeof(char));
			strcpy(host, &address[colonIndex + 3]);
			address[slashIndex] = slashTemp;
			char* path = calloc(strlen(&address[slashIndex]) + 1, sizeof(char));
			strcpy(path, &address[slashIndex]);
			//printf("host: %s\n", host);
			//printf("path: %s\n", path);
			int socketDisc;
			if ((socketDisc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				snprintf(errorMessage, 1000, "%s", "Unable to create socket");
				longjmp(errBuf, 1);
			}
			//printf("socket created\n");
			struct hostent* he;
			if ((he = gethostbyname(host)) == NULL) {
				snprintf(errorMessage, 1000, "%s '%s'", "Invalid hostname", host);
				longjmp(errBuf, 1);
			}
			//printf("host found\n");
			struct sockaddr_in server;
			server.sin_addr.s_addr = inet_addr(inet_ntoa(
						*((struct in_addr**) he->h_addr_list)[0]));
			server.sin_family = AF_INET;
			server.sin_port = htons(80);
			if (connect(socketDisc, (struct sockaddr *)&server , sizeof(server)) < 0) {
				sprintf(errorMessage, "%s", "Unable to create socket");
				longjmp(errBuf, 1);
			}
			//printf("connected to host\n");
			char* message = calloc(25 + strlen(path) + strlen(host) + 1, sizeof(char));
			sprintf(message, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", path, host);
			//printf("sending:\n%s", message);
			if (send(socketDisc, message, strlen(message), 0) < 0) {
				snprintf(errorMessage, 1000, "%s", "Could not send message to server");
				longjmp(errBuf, 1);
			}
			free(message);
			int* desc = calloc(1, sizeof(int));
			*desc = socketDisc;
			StackItem* out = newStackItem(Type_STREAM, desc);
			free(host);
			free(path);
			return out;
		}
		address[colonIndex] = colonTemp;
	} else {
		//printf("file: %s\n", address);
		FILE* file;
		if ((file = fopen(address, mode)) == NULL) {
			sprintf(errorMessage, "failed to open file '%s'", address);
			longjmp(errBuf, 1);
		}
		return newStackItem(Type_FILE_STREAM, file);
	}
	return newStackItem(Type_NULL, NULL);
}

StackItem* readStream(int stream, int amt) {
	char* message = calloc(amt + 1, sizeof(char));
	int length = 0;
	if ((length = read(stream, message, amt)) < 0) {
		snprintf(errorMessage, 1000, "%s", "Read failed");
		longjmp(errBuf, 1);
	} else if (length == 0) {
		message = realloc(message, sizeof(char));
		message[0] = 0;
	}
	StackItem* out = newStackItem(Type_STRING, message);
	return out;
}

StackItem* readFileStream(FILE* stream, int amt) {
	char* message = calloc(amt + 1, sizeof(char));
	int length = 0;
	if ((length = fread(message, sizeof(char), amt, stream)) < 0) {
		snprintf(errorMessage, 1000, "%s", "Read failed");
		longjmp(errBuf, 1);
	} else if (length == 0) {
		message = realloc(message, sizeof(char));
		message[0] = 0;
	}
	StackItem* out = newStackItem(Type_STRING, message);
	return out;
}

void closeStream(StackItem* item) {
	if (item->type == Type_STREAM) {
		close(*(int*) item->item);
		free(item->item);
	} else if (item->type == Type_FILE_STREAM) {
		fclose(item->item);
	}
}

#endif
