#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "rpc.h"

using namespace std;

  /* create sockets and connect to the binder */
int rpcInit()
{	
	int acpt_soc, reg_soc;
	struct sockaddr_in server_addr, binder_addr;
	struct hostent *binder;
	socklen_t addr_len;

	memset(&server_addr, '0', sizeof(server_addr));
	memset(&binder_addr, '0', sizeof(binder_addr));
	addr_len = sizeof(sockaddr_in);

	if ((acpt_soc = socket(AF_INET, SOCK_STREAM, 0)) == -1 ||
		(acpt_soc = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		cout << "Unable to create socket" << endl;
		return -1;
	}

	// Open connection to binder
	char *mach_name = getenv("BINDER_ADDRESS");
	int port_num = atoi(getenv("BINDER_PORT"));
	if ((binder = gethostbyname(mach_name)) == NULL)
	{
		cout << "Invalid host provided" << endl;
		return -4;
	}
	binder_addr.sin_family = AF_INET;
	binder_addr.sin_port = htons(port_num);
	memcpy((char *)&binder_addr.sin_addr.s_addr, (char *)binder->h_addr, binder->h_length);

	if (connect(reg_soc, (sockaddr *)&binder_addr, addr_len) == -1)
	{
		cout << "Unable to connect to binder" << endl;
		return -5;
	}

	// Open connection to accept client requests
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(0);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(acpt_soc, (sockaddr *)&server_addr, addr_len) == -1)
	{
		cout << "Unable to bind to accept socket" << endl;
		return -2;
	}

	if (listen(acpt_soc, 5) == -1)
	{
		cout << "unable to listen" << endl;
		return -3;
	}
	return 0;
}

// Register server functions 
int rpcRegister(char* name, int* argTypes, skeleton f)
{

}

int rpcCall(char* name, int* argTypes, void** args)
{
	
}