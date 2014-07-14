#include "message.h"
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>

#define HOST_LEN 15
#define FN_NAME_LEN 64
#define INT_SIZE 4
#define VOID_PTR_SIZE 8
#define MAX_DATA_LEN 1024

using namespace std;

struct message createReg(char *host, int port, char *name, int *argTypes)
{
	struct message msg;
	char msg_data[MAX_DATA_LEN];
	int l = HOST_LEN+INT_SIZE+FN_NAME_LEN;
	memcpy(msg_data, host, HOST_LEN);
	memcpy(msg_data+HOST_LEN, &port, INT_SIZE);
	cout << "createReg: " << *((int *)(msg_data+HOST_LEN)) << endl;
	memcpy(msg_data+HOST_LEN+INT_SIZE, name, FN_NAME_LEN);

	int i =0;
	while (*(argTypes+i)!=0)
	{
		memcpy(msg_data+l+(i*INT_SIZE), argTypes+i, INT_SIZE);
		i++;
	}
	memcpy(msg_data+l+(i*INT_SIZE), argTypes+i, INT_SIZE);
	i++;

	msg.length = l + i*INT_SIZE;
	msg.type = REGISTER;
	memcpy(msg.data, msg_data, msg.length);
	return msg;
}

struct message createRegSuc(int Warn_Err)
{
	struct message msg;
	msg.length = INT_SIZE;
	msg.type = REGISTER_SUCCESS;
	memcpy(msg.data, &Warn_Err, INT_SIZE);
	return msg;
}

struct message createRegFail(int Warn_Err)
{
	struct message msg;
	msg.length = INT_SIZE;
	msg.type = REGISTER_FAILURE;
	memcpy(msg.data, &Warn_Err, INT_SIZE);
	return msg;
}

struct message createLocReq(char* name, int* argTypes)
{
	struct message msg;
	char msg_data[MAX_DATA_LEN];
	memcpy(msg_data, name, FN_NAME_LEN);

	int i =0;
	while (*(argTypes+i)!=0)
	{
		memcpy(msg_data+FN_NAME_LEN+(i*INT_SIZE), argTypes+i, INT_SIZE);
		i++;
	}
	memcpy(msg_data+FN_NAME_LEN+(i*INT_SIZE), argTypes+i, INT_SIZE);
	i++;

	msg.length = FN_NAME_LEN + i*INT_SIZE;
	msg.type = LOC_REQUEST;
	memcpy(msg.data, msg_data, msg.length);
	return msg;
}

struct message createLocSuc(char *host, int port)
{
	struct message msg;
	char msg_data[MAX_DATA_LEN];
	memcpy(msg_data, host, HOST_LEN);
	memcpy(msg_data+HOST_LEN, &port, INT_SIZE);
	msg.length = HOST_LEN+INT_SIZE;
	msg.type = LOC_SUCCESS;
	memcpy(msg.data, msg_data, msg.length);
	return msg;
}

struct message createLocFail(int reasonCode)
{
	struct message msg;
	msg.length = INT_SIZE;
	msg.type = LOC_FAILURE;
	memcpy(msg.data, &reasonCode, INT_SIZE);
	return msg;
}

struct message createExec(char* name, int* argTypes, void** args)
{
	struct message msg;
	char msg_data[MAX_DATA_LEN];
	memcpy(msg_data, name, FN_NAME_LEN);

	int i =0;
	while (*(argTypes+i)!=0)
	{
		memcpy(msg_data+FN_NAME_LEN+(i*INT_SIZE), argTypes+i, INT_SIZE);
		i++;
	}
	memcpy(msg_data+FN_NAME_LEN+(i*INT_SIZE), argTypes+i, INT_SIZE);
	i++;

	for (int j = 0; j < i; j++)
	{
		memcpy(msg_data+FN_NAME_LEN+(i*INT_SIZE)+(j*VOID_PTR_SIZE), 
			   *(args+j), VOID_PTR_SIZE);
	}

	msg.length = FN_NAME_LEN + i*INT_SIZE+ (i-1)*VOID_PTR_SIZE;
	msg.type = EXECUTE;
	memcpy(msg.data, msg_data, msg.length);
	return msg;
}

struct message createExecSuc(char* name, int* argTypes, void** args)
{
	struct message msg;
	char msg_data[MAX_DATA_LEN];
	memcpy(msg_data, name, FN_NAME_LEN);

	int i =0;
	while (*(argTypes+i)!=0)
	{
		memcpy(msg_data+FN_NAME_LEN+(i*INT_SIZE), argTypes+i, INT_SIZE);
		i++;
	}
	memcpy(msg_data+FN_NAME_LEN+(i*INT_SIZE), argTypes+i, INT_SIZE);
	i++;

	for (int j = 0; j < i; j++)
	{
		memcpy(msg_data+FN_NAME_LEN+(i*INT_SIZE)+(j*VOID_PTR_SIZE), 
			   *(args+j), VOID_PTR_SIZE);
	}

	msg.length = FN_NAME_LEN + i*INT_SIZE+ (i-1)*VOID_PTR_SIZE;
	msg.type = EXECUTE_SUCCESS;
	memcpy(msg.data, msg_data, msg.length);
	return msg;
}

struct message createExecFail(int reasonCode)
{
	struct message msg;
	msg.length = INT_SIZE;
	msg.type = EXECUTE_FAILURE;
	memcpy(msg.data, &reasonCode, INT_SIZE);
	return msg;
}

struct message createTerm()
{
	struct message msg;
	msg.length = 0;
	msg.type = TERMINATE;
	msg.data[0] = '\0';	
	return msg;
}


int getLength(message msg){
	return msg.length;
}

int getType(message msg){
	return msg.type;
}

char* getData(message msg){
	return msg.data;
}


// char* getTCPData(message msg)
// {
// 	int msg_len = msg.length;
// 	int msg_type = msg.type;

// 	char return_arr[MAX_DATA_LEN];
// 	memcpy(return_arr, &msg_len, INT_SIZE);
// 	memcpy(return_arr+INT_SIZE, &msg_type, INT_SIZE);
// 	memcpy(return_arr+(2*INT_SIZE), msg.data, msg_len);
// 	return return_arr;
// }

// struct message parseTCPData(char* msgdata)
// {
// 	struct message msg;
// 	int msg_len, msg_type;
// 	char msg_data[MAX_DATA_LEN];
// 	memcpy(&msg_len, msgdata, INT_SIZE);
// 	memcpy(&msg_type, msgdata+INT_SIZE, INT_SIZE);
// 	memcpy(msg_data, msgdata+2*INT_SIZE, msg_len);
// 	msg.length = msg_len;
// 	msg.type = msg_type;
// 	memcpy(msg.data, msg_data, msg.length);
// 	return msg;
// }