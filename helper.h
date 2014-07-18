#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "message.h"
#include "helper.h"
#include "rpc.h"

using namespace std;

typedef struct functionInfo{
	char fnName[FN_NAME_LEN];
	int *argTypes;
	int numArgTypes;
}functionInfo;

typedef struct ServerInfo{
	char hostname[HOST_LEN];
	int port;
}ServerInfo;

int getArgLen(int argType);

int getArgSize(int argType);

int getArgType(int argType);

bool operator < (const functionInfo &l, const functionInfo &r);

#endif	/* HELPER_H */