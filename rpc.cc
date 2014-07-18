#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <map>
#include "rpc.h"
#include "message.h"
#include "helper.h"

using namespace std;

#define SOC_CREATE   	-1
#define BIND_ERR   	 	-2
#define LISTEN_ERR   	-3
#define HOSTNAME_GET    -4
#define CON_ERR  		-5
#define ACPT_ERR  		-6
#define NO_FUNCTION 	-7
#define THREAD_CREATE	-8

map<functionInfo, skeleton> svrfns;
int acpt_soc, reg_soc;
bool connected = false;
bool termination = false;

void *wait_term(void *reg_soc)
{
	int soc_fd = *((int *)reg_soc);
	int rcv_len, rcv_type;
	recv(soc_fd, &rcv_len, INT_SIZE, 0);
	recv(soc_fd, &rcv_type, INT_SIZE, 0);
	if (rcv_type == TERMINATE)
	{
		exit(0);	
	}
}

void *exec_fn(void *conn){
	int soc_fd = *((int *)conn);
	struct message msg;
	int rcv_len, rcv_type;
	while (true)
	{
		if (recv(soc_fd, &rcv_len, INT_SIZE, 0) <= 0 ||
			recv(soc_fd, &rcv_type, INT_SIZE, 0) <= 0){
			close(soc_fd);
			break;
		}
		char rcv_data[rcv_len];
		if (recv(soc_fd, rcv_data, rcv_len, 0) <= 0){
			close(soc_fd);
			break;
		}
		char name[FN_NAME_LEN];
		memcpy(name, rcv_data, FN_NAME_LEN);
		int *argTypes = (int *)(rcv_data + FN_NAME_LEN);

		int offset = FN_NAME_LEN;
		int i = 0;
		while (*((int *)(rcv_data + offset)) != 0){
			offset = offset + INT_SIZE;
			i++;
		}
		offset = offset + INT_SIZE;

		char **args;

		args = (char **) malloc(i * sizeof(char *));
		for (int x = 0; x < i; x++){
			int argType = argTypes[x];
			int size = getArgLen(argType) * getArgSize(argType);
			args[x] = new char[size];
			memcpy(args[x], rcv_data + offset, size);
			offset += size;
		}

		functionInfo fnInfo;
		memcpy(fnInfo.fnName, name, FN_NAME_LEN);
		fnInfo.argTypes = new int[i+1];
		i=0;
		while (*(argTypes+i)!=0)
		{
			memcpy(fnInfo.argTypes+i, argTypes+i, INT_SIZE);
			i++;
		}
		memcpy(fnInfo.argTypes+i, argTypes+i, INT_SIZE);
		fnInfo.numArgTypes = i + 1;

		int result = NO_FUNCTION;
		if (svrfns.find(fnInfo) != svrfns.end()){
			skeleton fn = svrfns[fnInfo];
			result = fn(argTypes, (void **) args);
		}

		if (result == 0){
			struct message msg2 = createExecSuc(name, argTypes, (void **)args);
			int send_len2 = msg2.length;
			int send_type2 = msg2.type;
			send(soc_fd, &send_len2, INT_SIZE, 0);
			send(soc_fd, &send_type2, INT_SIZE, 0);
			send(soc_fd, msg2.data, send_len2, 0);
		}
		else{
			struct message msg3 = createExecFail(result);
			int send_len3 = msg3.length;
			int send_type3 = msg3.type;
			send(soc_fd, &send_len3, INT_SIZE, 0);
			send(soc_fd, &send_type3, INT_SIZE, 0);
			send(soc_fd, msg3.data, send_len3, 0);
		}

	}
	pthread_exit(NULL);
}

  /* create sockets and connect to the binder */
int rpcInit()
{
	struct sockaddr_in server_addr, binder_addr;
	struct hostent *binder;
	socklen_t addr_len = sizeof(sockaddr_in);

	memset(&server_addr, '0', sizeof(server_addr));
	memset(&binder_addr, '0', sizeof(binder_addr));

	if ((reg_soc = socket(AF_INET, SOCK_STREAM, 0)) == -1 ||
		(acpt_soc = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		cerr << "Unable to create socket" << endl;
		return SOC_CREATE;
	}

	// // Change to non-block so socket will not be blocked by recv
	// fcntl(reg_soc, F_SETFL, O_NONBLOCK);
	// fcntl(acpt_soc, F_SETFL, O_NONBLOCK);

	char *mach_name = getenv("BINDER_ADDRESS");
	int port_num = atoi(getenv("BINDER_PORT"));

	if ((binder = gethostbyname(mach_name)) == NULL)
	{
		cerr << "Invalid host provided" << endl;
		return HOSTNAME_GET;
	}
	binder_addr.sin_family = AF_INET;
	binder_addr.sin_port = htons(port_num);
	memcpy((char *)&binder_addr.sin_addr.s_addr, (char *)binder->h_addr, binder->h_length);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(0);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(acpt_soc, (sockaddr *)&server_addr, addr_len) == -1)
	{
		cerr << "Unable to bind to accept socket" << endl;
		return BIND_ERR;
	}

	if (connect(reg_soc, (sockaddr *)&binder_addr, addr_len) == -1)
	{
		cerr << "Unable to connect to binder" << endl;
		return CON_ERR;
	}

	int length = 0;
	int type = INITIALIZE;
	send(reg_soc, &length, INT_SIZE, 0);
	send(reg_soc, &type, INT_SIZE, 0);
	connected = true;
	return 0;
}

// Register server functions
int rpcRegister(char* name, int* argTypes, skeleton f)
{
	struct sockaddr_in server_addr;
	memset(&server_addr, '0', sizeof(server_addr));
	socklen_t addr_len = sizeof(sockaddr_in);

	char host[HOST_LEN];
	gethostname(host,HOST_LEN);
	getsockname(acpt_soc, (sockaddr *)&server_addr, &addr_len);
	int port = ntohs(server_addr.sin_port);

	struct message msg = createReg(host, port, name, argTypes);
	int send_len = msg.length;
	int send_type = msg.type;

	functionInfo fnInfo;
	memcpy(fnInfo.fnName, name, FN_NAME_LEN);
	int i =0;
	while (*(argTypes+i)!=0){i++;}
	fnInfo.argTypes = new int[i+1];
	i =0;
	while (*(argTypes+i)!=0)
	{
		memcpy(fnInfo.argTypes+i, argTypes+i, INT_SIZE);
		i++;
	}
	memcpy(fnInfo.argTypes+i, argTypes+i, INT_SIZE);
	fnInfo.numArgTypes = i + 1;

	svrfns[fnInfo] = f;

	if (connected)
	{
		send(reg_soc, &send_len, INT_SIZE, 0);
		send(reg_soc, &send_type, INT_SIZE, 0);
		send(reg_soc, msg.data, send_len, 0);
		int rcv_len, rcv_type, return_code;
		recv(reg_soc, &rcv_len, INT_SIZE, 0);
		recv(reg_soc, &rcv_type, INT_SIZE, 0);
		recv(reg_soc, &return_code, INT_SIZE, 0);
		return return_code;
	}
	else
	{
		cerr << "Unable to connected to binder" << endl;
		return CON_ERR;
	}
}

int rpcCall(char* name, int* argTypes, void** args)
{
	struct sockaddr_in server_addr, binder_addr;
	struct hostent *binder, *server;
	socklen_t addr_len = sizeof(sockaddr_in);
	int conn_soc;

	memset(&binder_addr, '0', sizeof(binder_addr));
	memset(&server_addr, '0', sizeof(server_addr));		

	if ((conn_soc = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		cerr << "Unable to create socket" << endl;
		return SOC_CREATE;
	}
	// fcntl(conn_soc, F_SETFL, O_NONBLOCK);

	char *binder_machine = getenv("BINDER_ADDRESS");
	int binder_port = atoi(getenv("BINDER_PORT"));

	if ((binder = gethostbyname(binder_machine)) == NULL)
	{
		cerr << "Invalid host provided" << endl;
		return HOSTNAME_GET;
	}

	binder_addr.sin_family = AF_INET;
	binder_addr.sin_port = htons(binder_port);
	memcpy((char *)&binder_addr.sin_addr.s_addr, (char *)binder->h_addr, binder->h_length);

	if (connect(conn_soc, (sockaddr *)&binder_addr, addr_len) == -1)
	{
		cerr << "Unable to connect to binder" << endl;
		return CON_ERR;
	}

	struct message msg = createLocReq(name, argTypes);
	int send_len = msg.length;
	int send_type = msg.type;
	send(conn_soc, &send_len, INT_SIZE, 0);
	send(conn_soc, &send_type, INT_SIZE, 0);
	send(conn_soc, msg.data, send_len, 0);
	int rcv_len, rcv_type, reasonCode;
	recv(conn_soc, &rcv_len, INT_SIZE, 0);
	recv(conn_soc, &rcv_type, INT_SIZE, 0);

	if (rcv_type == LOC_FAILURE)
	{
		recv(conn_soc, &reasonCode, INT_SIZE, 0);
		close(conn_soc);
		return reasonCode;
	}
	else if (rcv_type == LOC_SUCCESS)
	{
		char data[rcv_len];
		recv(conn_soc, data, rcv_len, 0);
		close(conn_soc);

		int serv_soc;
		char server_host[HOST_LEN];
		int server_port;
		memcpy(server_host, data, HOST_LEN);
		memcpy(&server_port, data+HOST_LEN, INT_SIZE);

		if ((server = gethostbyname(server_host)) == NULL)
		{
			cerr << "Invalid host provided" << endl;
			return HOSTNAME_GET;
		}

		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(server_port);
		memcpy((char *)&server_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);

		if ((serv_soc = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
			{
				cerr << "Unable to create server socket" << endl;
				return SOC_CREATE;
			}

		if (connect(serv_soc, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
		{
			cerr << "Unable to connect to server" << endl;
			return CON_ERR;
		}


		struct message msg2 = createExec(name, argTypes, args);
		int send_len2 = msg2.length;
		int send_type2 = msg2.type;
		send(serv_soc, &send_len2, INT_SIZE, 0);
		send(serv_soc, &send_type2, INT_SIZE, 0);
		send(serv_soc, msg2.data, send_len2, 0);

		int rcv_len2, rcv_type2, reasonCode2;
		recv(serv_soc, &rcv_len2, INT_SIZE, 0);
		recv(serv_soc, &rcv_type2, INT_SIZE, 0);
		char rcv_data2[rcv_len2];
		if (rcv_type2 == EXECUTE_FAILURE)
		{
			recv(serv_soc, &reasonCode2, INT_SIZE, 0);
			return reasonCode2;
		}
		else if (rcv_type2 == EXECUTE_SUCCESS)
		{
			int n = recv(serv_soc, rcv_data2, rcv_len2, 0);

			int *argTypes = (int *)(rcv_data2 + FN_NAME_LEN);

			int offset = FN_NAME_LEN;
			int i = 0;
			while (*((int *)(rcv_data2 + offset)) != 0){
				offset = offset + INT_SIZE;
				i++;
			}
			offset = offset + INT_SIZE;

			for (int x = 0; x < i; x++){
				int argType = argTypes[x];
				int size = getArgLen(argType) * getArgSize(argType);
				memcpy(args[x], rcv_data2 + offset, size);
				offset += size;
			}
			return 0;
		}
		close(serv_soc);
	}
}

int rpcExecute()
{
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(sockaddr_in);
	int conn;
	pthread_t thread;

	if (listen(acpt_soc, 5) == -1)
	{
		cerr << "Unable to listen" << endl;
		return LISTEN_ERR;
	}

	if (pthread_create(&thread, NULL, wait_term, (void *)&reg_soc) != 0)
	{
		cerr << "Error when creating thread" << endl;
		return THREAD_CREATE;
	}

	while(!termination)
	{
		if ((conn = accept(acpt_soc, (sockaddr *)&client_addr, &addr_len)) == -1)
		{
			cerr << "Fail to connect with client" << endl;
			return ACPT_ERR;
		}
		if (pthread_create(&thread, NULL, exec_fn, (void *)&conn) != 0)
		{
			cerr << "Error when creating thread" << endl;
			return THREAD_CREATE;
		}
	} 
	return 0;
}

int rpcTerminate()
{
	struct sockaddr_in binder_addr;
	struct hostent *binder;
	socklen_t addr_len = sizeof(sockaddr_in);
	int conn_soc;

	if ((conn_soc = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		cerr << "Unable to create socket" << endl;
		return SOC_CREATE;
	}

	char *mach_name = getenv("BINDER_ADDRESS");
	int binder_port = atoi(getenv("BINDER_PORT"));
	if ((binder = gethostbyname(mach_name)) == NULL)
	{
		cerr << "Invalid host provided" << endl;
		return HOSTNAME_GET;
	}

	binder_addr.sin_family = AF_INET;
	binder_addr.sin_port = htons(binder_port);
	memcpy((char *)&binder_addr.sin_addr.s_addr, (char *)binder->h_addr, binder->h_length);

	if (connect(conn_soc, (sockaddr *)&binder_addr, addr_len) == -1)
	{
		cerr << "Unable to connect to binder" << endl;
		return CON_ERR;
	}

	struct message msg = createTerm();
	int send_len = msg.length;
	int send_type = msg.type;
	send(conn_soc, &send_len, INT_SIZE, 0);
	send(conn_soc, &send_type, INT_SIZE, 0);
}