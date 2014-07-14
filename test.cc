#include <string.h>
#include <iostream>
#include "message.h"

using namespace std;

int main(){
	// int a[] = {1,2,3,4,5,6,7,8,9,0};
	// int argTypes[] = {1,2,3,4,5,6,7,8,9,0};
	// char *host = "127.0.0.1";
	// int port = 9999;
	// char *name = "sum";
	// char msg_data[1024];
	// memcpy(msg_data, host, 15);
	// memcpy(msg_data+15, &port, 4);
	// memcpy(msg_data+19, name, 64);
	// int i =0;
	// while (*(argTypes+i)!=0)
	// {
	// 	memcpy(msg_data+83+(i*4), argTypes+i, 4);
	// 	i++;
	// }
	// memcpy(msg_data+83+(i*4), argTypes+i, 4);
	// i++;
	
	// char host2[15];
	// int port2;
	// char name2[64];
	// int argTypes2[10];
	// memcpy(host2, msg_data, 15);
	// cout << host2 << endl;
	// memcpy(&port2, msg_data+15, 4);
	// cout << port2 << endl;
	// memcpy(name2, msg_data+19, 64);
	// cout << name2 << endl;
	// for(int x = 0; x < i; x++){
	// 	memcpy(argTypes2+x, msg_data+83+(x*4), 4);
	// 	cout << argTypes2[x] << endl;
	// }

	// int i = 0;
	// i = sizeof(void *);
	// cout << i << endl;

	// int argTypes[] = {1,2,3,4,5,6,7,8,9,0};
	// char *d;
	// char data[1024];
	// int da[1024];
	// char host[15], name[64];
	// struct message msg1 = createReg("127.0.0.1", 9999, "sum", argTypes);
	// d = getTCPData(msg1);
	// int len, type, port;
	// memcpy(&len, d, 4);
	// memcpy(&type, d+4, 4);
	// memcpy(data, d+8, len);
	// memcpy(host, data, 15);
	// memcpy(&port, data+15, 4);
	// memcpy(name, data+19, 64);
	// cout << len << endl;
	// cout << type << endl;
	// cout << host << endl;
	// cout << port << endl;
	// cout << name << endl;

	// struct message msg2 = createRegSuc(3);
	// cout << "msg2 reg suc:" << endl;
	// cout << "length " << getLength(msg2) << endl;
	// cout << "type " << getType(msg2) << endl;
	// strcpy(data, getData(msg2)); 
	// cout << "data " << data << endl;

	// struct message msg3 = createRegFail(-3);
	// cout << "msg3 reg fail:" << endl;
	// cout << "length " << getLength(msg3) << endl;
	// cout << "type " << getType(msg3) << endl;
	// strcpy(data, getData(msg3)); 
	// cout << "data " << data << endl;

	// cout << "msg4 loc req:" << endl;
	// cout << "msg4 loc suc:" << endl;
	// cout << "msg4 loc fail:" << endl;
	// cout << "msg4 exec:" << endl;
	// cout << "msg4 exec suc:" << endl;
	// cout << "msg4 exec fail:" << endl;
	// cout << "msg4 terminate:" << endl;
}
