#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include "message.h"
#include "helper.h"
#include "rpc.h"

int getArgLen(int argType){
	int arr_size = argType & 0xFFFF;
	return max(arr_size,1);
}

int getArgSize(int argType){
	int type = (argType >> 16) & 0xFF;
	switch(type)
	{
		case ARG_CHAR:
			return CHAR_SIZE;
		case ARG_SHORT:
			return SHORT_SIZE;
		case ARG_INT:
			return INT_SIZE;
		case ARG_LONG:
			return LONG_SIZE;
		case ARG_DOUBLE:
			return DOUBLE_SIZE;
		case ARG_FLOAT:
			return FLOAT_SIZE;
		default:
			return CHAR_SIZE;
	}
}

int getArgType(int argType){
	int type = (argType >> 16) & 0xFF;
	return type;
}

bool operator < (const functionInfo &l, const functionInfo &r){
	if (strcmp(l.fnName,r.fnName) == 0){
		if (l.numArgTypes == r.numArgTypes){
			for(int i = 0; i < l.numArgTypes; i++){
				if (l.argTypes[i] != r.argTypes[i]){
					return l.argTypes[i] < r.argTypes[i];
				}
			}			
		}
		else{
			return (strcmp(l.fnName, r.fnName) < 0);
		}
	}
	else{
		return (strcmp(l.fnName, r.fnName) < 0);
	}
	return false;
}