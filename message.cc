#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include "message.h"
#include "helper.h"
#include "rpc.h"

using namespace std;

struct message createReg(char *host, int port, char *name, int *argTypes)
{
	struct message msg;
	char msg_data[MAX_DATA_LEN];
	int l = HOST_LEN+INT_SIZE+FN_NAME_LEN;
	memcpy(msg_data, host, HOST_LEN);
	memcpy(msg_data+HOST_LEN, &port, INT_SIZE);
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

	int offset = FN_NAME_LEN+((i+1) * INT_SIZE);
	for (int j = 0; j < i; j++)
	{
		int size = getArgSize(argTypes[j]);
		for (int x = 0; x < getArgLen(argTypes[j]); x++){
			memcpy(msg_data+offset, (*(args + j) + (x * size)), size);
			offset += size;
		}
	}

	msg.length = offset;
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


	int offset = FN_NAME_LEN+((i+1) * INT_SIZE);
	for (int j = 0; j < i; j++)
	{
		int size = getArgSize(argTypes[j]);
		for (int x = 0; x < getArgLen(argTypes[j]); x++){
			memcpy(msg_data+offset, (*(args + j) + (x * size)), size);
			offset += size;
		}
	}

	msg.length = offset;
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
