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

int binder()
{	
	int socket_fd;
	struct sockaddr_in binder_addr;
	socklen_t addr_len;

	memset(&binder_addr, '0', sizeof(binder_addr));
	addr_len = sizeof(binder_addr);

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cout << "Unable to create socket" << endl;
		return -1;
	}

	binder_addr.sin_family = AF_INET;
	binder_addr.sin_port = htons(0);
	binder_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(socket_fd, (sockaddr *)&binder_addr, addr_len) == -1)
	{
		cout << "Unable to bind" << endl;
		return -1;
	}
	
	char hostname[64];
	gethostname(hostname,64);
	getsockname(socket_fd, (sockaddr *)&binder_addr, &server_len);
	binder_port = ntohs(binder_addr.sin_port);
	// Print out binder address and port
	cout << "BINDER_ADDRESS " << hostname << endl;
	cout << "BINDER_PORT" << binder_port << endl;

	if (listen(socket_fd,5) == -1)
	{
		cout << "Unable to listen" << endl;
		return -1;
	}
}