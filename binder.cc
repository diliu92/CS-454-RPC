#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <deque>
#include <map>
#include "rpc.h"
#include "message.h"

using namespace std;

pthread_t thread;
pthread_mutex_t mutexLock;

typedef struct functionInfo{
	char *fnName;
	int *argTypes;
	int numArgTypes;
}functionInfo;

typedef struct ServerInfo{
	char *hostname;
	int port;
}ServerInfo;

deque <int> server_connections;
map<functionInfo *, deque<ServerInfo *> *> fn2server;


void register_handler(int socket_fd, int len, char* msg){
	char hostname[15];
	int port;
	char fn_name[64];
	int num_arg_types = (len - 15 - 4 - 64) / 4;
	int argTypes[num_arg_types];

	memcpy(hostname, msg, 15);
	memcpy(&port, msg+15, 4);
	memcpy(fn_name, msg+19, 64);
	for(int i = 0; i < num_arg_types; i++){
		memcpy(argTypes+i, msg+83+i*4, 4);
	}

	functionInfo *fnInfo = new functionInfo();
	fnInfo->fnName = fn_name;
	fnInfo->argTypes = argTypes;
	fnInfo->numArgTypes = num_arg_types;

	ServerInfo *srvInfo = new ServerInfo();
	srvInfo->hostname = hostname;
	srvInfo->port = port;

	pthread_mutex_lock(&mutexLock);

	if (fn2server.find(fnInfo) == fn2server.end()){	//the fn has never been registered before
		fn2server[fnInfo] = new deque<ServerInfo *>();
	}

	deque <ServerInfo *> *servers = fn2server[fnInfo];
	for(deque<ServerInfo *>::iterator it = servers->begin();
		it != servers->end();
		++it){
		ServerInfo *cur = *it;
		if (cur->hostname == srvInfo->hostname && cur->port == srvInfo->port){
			servers->erase(it);
		}
	}
	servers->push_back(srvInfo);

	pthread_mutex_unlock(&mutexLock);

	int reply_len = 0;
	int type = REGISTER_SUCCESS;
	send(socket_fd, &reply_len, sizeof(int), 0);
	send(socket_fd, &type, sizeof(int), 0);
	send(socket_fd, NULL, 0, 0);

	return;
}

void loc_request_handler(int socket_fd, int len, char* msg){
	char fn_name[64];
	int num_arg_types = (len - 64) / 4;
	int argTypes[num_arg_types];

	memcpy(fn_name, msg, 64);
	for(int i = 0; i < num_arg_types; i++){
		memcpy(argTypes+i, msg+64+i*4, 4);
	}

	functionInfo *fnInfo = new functionInfo();
	fnInfo->fnName = fn_name;
	fnInfo->argTypes = argTypes;
	fnInfo->numArgTypes = num_arg_types;

	int type = LOC_FAILURE;
	int reply_len = 0;
	pthread_mutex_lock(&mutexLock);

	ServerInfo *svrInfo;
	if (fn2server.find(fnInfo) != fn2server.end()){
		deque <ServerInfo *> *servers = fn2server[fnInfo];
		svrInfo = servers->front();
		servers->pop_front();
		servers->push_back(svrInfo);
		type = LOC_SUCCESS;
		reply_len = 19;
	}

	pthread_mutex_unlock(&mutexLock);

	if (type == LOC_SUCCESS){
		char reply[reply_len];	//hostname 15 + port 4;
		memcpy(reply, svrInfo->hostname, 15);
		memcpy(reply+15, &(svrInfo->port), 4);

		send(socket_fd, &reply_len, sizeof(int), 0);
		send(socket_fd, &type, sizeof(int), 0);
		send(socket_fd, &reply, 19, 0);
	}
	else{
		send(socket_fd, &reply_len, sizeof(int), 0);
		send(socket_fd, &type, sizeof(int), 0);
		send(socket_fd, NULL, 0, 0);
	}

	return;
}

void terminate_server_handler(){
	while(!server_connections.empty()){
		int fd = server_connections.front();
		server_connections.pop_front();
		close(fd);
	}
	return;
}

void *handler(void *arguments){
	int socket_fd = *((int *) arguments);
	int len;
	int type;
	while (true){
		//get length
		if (recv(socket_fd, &len, sizeof(int), 0) <= 0){
			close(socket_fd);
		}

		//get type
		if (recv(socket_fd, &type, sizeof(int), 0) <= 0){
			close(socket_fd);
		}

		//get message
		char msg[len];
		if (recv(socket_fd, msg, len, 0) <= 0){
			close(socket_fd);
		}

		switch(type){
			case INITIALIZE:
				server_connections.push_back(socket_fd);
				break;
			case REGISTER:
				register_handler(socket_fd, len, msg);
				break;
			case LOC_REQUEST:
				loc_request_handler(socket_fd, len, msg);
				break;
			case TERMINATE:
				terminate_server_handler();
				break;
		}
	}
}

int binder()
{
	int binder_socket_fd;
	int new_socket_fd;
	struct sockaddr_in binder_addr, client_addr;
	socklen_t binder_addr_len, client_addr_len;

	memset(&binder_addr, '0', sizeof(binder_addr));
	binder_addr_len = sizeof(binder_addr);

	if ((binder_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cout << "Unable to create socket" << endl;
		return -1;
	}

	binder_addr.sin_family = AF_INET;
	binder_addr.sin_port = htons(0);
	binder_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(binder_socket_fd, (struct sockaddr *)&binder_addr, binder_addr_len) == -1)
	{
		cout << "Unable to bind" << endl;
		return -1;
	}

	char hostname[15];
	gethostname(hostname,15);
	getsockname(binder_socket_fd, (struct sockaddr *)&binder_addr, &binder_addr_len);
	int binder_port = ntohs(binder_addr.sin_port);
	// Print out binder address and port
	cout << "BINDER_ADDRESS: " << hostname << endl;
	cout << "BINDER_PORT: " << binder_port << endl;

	if (listen(binder_socket_fd,5) == -1)
	{
		cout << "Unable to listen" << endl;
		return -1;
	}


	while(true){
		new_socket_fd = accept(binder_socket_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		if (new_socket_fd < 0){
			cerr << "Error on Accecpt" << endl;
			exit(0);
		}

		if (pthread_create(&thread, NULL, handler, (void *) &new_socket_fd)){
			cerr << "Cannot create new thread" << endl;
			exit(0);
		}
	}


	close(binder_socket_fd);
	return 0;
}

