#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

typedef struct functionInfo{
	char fnName[64];
	int *argTypes;
	int numArgTypes;
}functionInfo;

typedef struct ServerInfo{
	char hostname[15];
	int port;
}ServerInfo;

int getArgLen(int argType);

int getArgSize(int argType);

int getArgType(int argType);

bool operator < (const functionInfo &l, const functionInfo &r);

#endif	/* HELPER_H */