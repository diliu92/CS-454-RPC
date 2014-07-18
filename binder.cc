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
#include "helper.h"

using namespace std;

pthread_t thread;
pthread_mutex_t mutexLock;

deque <int> server_connections;
map<functionInfo, deque<ServerInfo *> *> fn2server;

void register_handler(int socket_fd, int len, char* msg){
	ServerInfo *svrInfo = new ServerInfo();
	memcpy(svrInfo->hostname, msg, HOST_LEN);
	memcpy(&(svrInfo->port), msg+HOST_LEN, INT_SIZE);

	functionInfo fnInfo;
	memcpy(fnInfo.fnName, msg+HOST_LEN+INT_SIZE, FN_NAME_LEN);
	int offset = HOST_LEN + INT_SIZE + FN_NAME_LEN;
	fnInfo.numArgTypes = (len - offset) / INT_SIZE;
	fnInfo.argTypes = new int[fnInfo.numArgTypes];
	for(int i = 0; i < fnInfo.numArgTypes; i++){
		memcpy(fnInfo.argTypes+i, msg+offset+i*INT_SIZE, INT_SIZE);
	}

	pthread_mutex_lock(&mutexLock);
	if (fn2server.find(fnInfo) == fn2server.end()){	//the fn has never been registered before
		fn2server[fnInfo] = new deque<ServerInfo *>();
	}

	deque <ServerInfo *> *servers = fn2server[fnInfo];

	for(deque<ServerInfo *>::iterator it = servers->begin();
		it != servers->end();
		it++){
		ServerInfo *cur = *it;
		if ((strcmp(cur->hostname, svrInfo->hostname) == 0) && 
			cur->port == svrInfo->port){
			servers->erase(it);		
			break;
		}
	}
	servers->push_back(svrInfo);

	pthread_mutex_unlock(&mutexLock);

	struct message reply = createRegSuc(0);
	send(socket_fd, &reply.length, sizeof(int), 0);
	send(socket_fd, &reply.type, sizeof(int), 0);
	send(socket_fd, &reply.data, reply.length, 0);

	return;
}

void loc_request_handler(int socket_fd, int len, char* msg){
	functionInfo fnInfo;
	memcpy(fnInfo.fnName, msg, FN_NAME_LEN);
	fnInfo.numArgTypes = (len - FN_NAME_LEN) / 4;
	fnInfo.argTypes = new int[fnInfo.numArgTypes];

	for(int i = 0; i < fnInfo.numArgTypes; i++){
		memcpy(fnInfo.argTypes+i, msg+FN_NAME_LEN+i*4, 4);
	}


	bool success = false;
	pthread_mutex_lock(&mutexLock);

	ServerInfo *svrInfo;
	if (fn2server.find(fnInfo) != fn2server.end()){
		deque <ServerInfo *> *servers = fn2server[fnInfo];
		svrInfo = servers->front();
		servers->pop_front();
		servers->push_back(svrInfo);
		success = true;
	}

	pthread_mutex_unlock(&mutexLock);

	if (success){
		struct message reply = createLocSuc(svrInfo->hostname, svrInfo->port);
		send(socket_fd, &reply.length, sizeof(int), 0);
		send(socket_fd, &reply.type, sizeof(int), 0);
		send(socket_fd, &reply.data, reply.length, 0);
	}
	else{
		struct message reply = createLocFail(-1);
		send(socket_fd, &reply.length, sizeof(int), 0);
		send(socket_fd, &reply.type, sizeof(int), 0);
		send(socket_fd, &reply.data, reply.length, 0);
	}

	return;
}

void terminate_server_handler(int len, int type){
	while(!server_connections.empty()){
		int fd = server_connections.front();
		server_connections.pop_front();
		send(fd, &len, sizeof(int), 0);
		send(fd, &type, sizeof(int), 0);
	}
	exit(0);
}

void *handler(void *arguments){
	int socket_fd = *((int *) arguments);
	int len;
	int type;
	while (true){
		//get length
		if (recv(socket_fd, &len, sizeof(int), 0) <= 0){
			close(socket_fd);
			break;
		}

		//get type
		if (recv(socket_fd, &type, sizeof(int), 0) <= 0){
			close(socket_fd);
			break;
		}

		//get message
		char msg[len];
		if(len > 0){
			if (recv(socket_fd, msg, len, 0) <= 0){
				close(socket_fd);
				break;
			}
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
				terminate_server_handler(len, type);
				break;
		}
	}
}

int main()
{
	int binder_socket_fd;
	int new_socket_fd;
	struct sockaddr_in binder_addr, client_addr;
	socklen_t binder_addr_len, client_addr_len;

	memset(&binder_addr, '0', sizeof(binder_addr));
	binder_addr_len = sizeof(binder_addr);

	if ((binder_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cerr << "Unable to create socket" << endl;
		return -1;
	}

	binder_addr.sin_family = AF_INET;
	binder_addr.sin_port = htons(0);
	binder_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(binder_socket_fd, (struct sockaddr *)&binder_addr, binder_addr_len) == -1)
	{
		cerr << "Unable to bind" << endl;
		return -1;
	}

	char hostname[HOST_LEN];
	gethostname(hostname,HOST_LEN);
	getsockname(binder_socket_fd, (struct sockaddr *)&binder_addr, &binder_addr_len);
	int binder_port = ntohs(binder_addr.sin_port);
	// Print out binder address and port
	cout << "BINDER_ADDRESS: " << hostname << endl;
	cout << "BINDER_PORT: " << binder_port << endl;

	if (listen(binder_socket_fd,5) == -1)
	{
		cerr << "Unable to listen" << endl;
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

