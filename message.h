#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

#define REGISTER    	 	1
#define REGISTER_SUCCESS    2
#define REGISTER_FAILURE    3
#define LOC_REQUEST   	 	4
#define LOC_SUCCESS      	5
#define LOC_FAILURE      	6
#define EXECUTE  		 	7
#define EXECUTE_SUCCESS  	8
#define EXECUTE_FAILURE  	9
#define TERMINATE  	 	 	10

struct message {
	int length;		// message length
	int type;		// type
	char data[1024];	// Char array with length length
	// string msg;
};

struct message createReg(char *host, int port, char *name, int *argTypes);
struct message createRegSuc(int Warn_Err);
struct message createRegFail(int Warn_Err);
struct message createLocReq(char* name, int* argTypes);
struct message createLocSuc(char *host, int port);
struct message createLocFail(int reasonCode);
struct message createExec(char* name, int* argTypes, void** args);
struct message createExecSuc(char* name, int* argTypes, void** args);
struct message createExecFail(int reasonCode);
struct message createTerm();

// Get a message's length, type, and data
int getLength(message msg);
int getType(message msg);
char *getData(message msg);

// Stores message information in a char array
// char* getTCPData(message msg);
// Retrieve information from a char array and make packet
// struct message parseTCPData(char* msgdata);

#endif	/* MESSAGE_H */