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
#include "rpc.h"
#include "message.h"

using namespace std;

#define CHAR_SIZE 		sizeof(char)
#define SHORT_SIZE 		sizeof(short)
#define LONG_SIZE 		sizeof(long)
#define DOUBLE_SIZE 	sizeof(double)
#define FLOAT_SIZE 		sizeof(float)

#define SOC_CREATE   	-1
#define BIND_ERR   	 	-2
#define LISTEN_ERR   	-3
#define HOSTNAME_GET    -4
#define CON_ERR  		-5
#define ACPT_ERR  		-6
#define TERM_ERR 		-7
#define NO_FUNCTION		-8

struct svr_fns {
	char *name;		
	int *argTypes;		
	skeleton function;	
};

int acpt_soc, reg_soc, n=0;
struct svr_fns svfns[32];	
bool connected = false;		

void *exec_fn(void *conn){
	int soc_fd = *((int *)conn);
	struct message msg;
	int rcv_len, rcv_type;	
	while (true)
	{
		recv(soc_fd, &rcv_len, INT_SIZE, 0);
		recv(soc_fd, &rcv_type, INT_SIZE, 0);
		char rcv_data[rcv_len];
		recv(soc_fd, rcv_data, rcv_len, 0);
		char name[FN_NAME_LEN];
		int *argTypes;
		memcpy(name, rcv_data, FN_NAME_LEN);

		int i =0;
		int cur_len = FN_NAME_LEN;
		while (*((int *)(rcv_data+i))!=0)
		{
			memcpy(argTypes+i, rcv_data+cur_len, INT_SIZE);
			cur_len += INT_SIZE; 
			i++;
		}
		memcpy(argTypes+i, rcv_data+cur_len, INT_SIZE);
		cur_len += INT_SIZE; 
		i++;
		void **args = (void **)malloc(i * PTR_SIZE);

		for (int j=0; j<i; j++)
		{
			int t = *(argTypes+j);
			int type = (t << 8) & 0xFF;
			int arr_size = (t << 16) | ((t << 24) & 0xFF);
			switch(type)
			{
				case ARG_CHAR:
					if (arr_size == 0) {
						char c;
						memcpy(&c, rcv_data+cur_len, CHAR_SIZE);
						cur_len += CHAR_SIZE; 
						*(args+j) = &c;
					} else {
						char *c;
						memcpy(c, rcv_data+cur_len, PTR_SIZE);
						cur_len += PTR_SIZE; 
						*(args+j) = c;
					} break;
				case ARG_SHORT:
					if (arr_size == 0) {
						short s;
						memcpy(&s, rcv_data+cur_len, SHORT_SIZE);
						cur_len += SHORT_SIZE; 
						*(args+j) = &s;
					} else {
						short *s;
						memcpy(s, rcv_data+cur_len, PTR_SIZE);
						cur_len += PTR_SIZE; 
						*(args+j) = s;
					} break;
				case ARG_INT:
					if (arr_size == 0) {
						int i2;
						memcpy(&i2, rcv_data+cur_len, INT_SIZE);
						cur_len += INT_SIZE; 
						*(args+j) = &i2;
					} else {
						int *i2;
						memcpy(i2, rcv_data+cur_len, PTR_SIZE);
						cur_len += PTR_SIZE; 
						*(args+j) = i2;
					} break;
				case ARG_LONG:
					if (arr_size == 0) {
						long l;
						memcpy(&l, rcv_data+cur_len, LONG_SIZE);
						cur_len += SHORT_SIZE; 
						*(args+j) = &l;
					} else {
						long *l;
						memcpy(l, rcv_data+cur_len, PTR_SIZE);
						cur_len += PTR_SIZE; 
						*(args+j) = l;
					} break;
				case ARG_DOUBLE:
					if (arr_size == 0) {
						double d;
						memcpy(&d, rcv_data+cur_len, DOUBLE_SIZE);
						cur_len += DOUBLE_SIZE; 
						*(args+j) = &d;
					} else {
						double *d;
						memcpy(d, rcv_data+cur_len, PTR_SIZE);
						cur_len += PTR_SIZE; 
						*(args+j) = d;
					} break;
				case ARG_FLOAT:		
					if (arr_size == 0) {
						float f;
						memcpy(&f, rcv_data+cur_len, FLOAT_SIZE);
						cur_len += FLOAT_SIZE; 
						*(args+j) = &f;
					} else {
						float *f;
						memcpy(f, rcv_data+cur_len, PTR_SIZE);
						cur_len += PTR_SIZE; 
						*(args+j) = f;
					} break;
			}
			int result;
			int k=0;
			while (k<32){
				struct svr_fns sf = svfns[k];
				if (strcmp(sf.name, name) == 0)
				{
					int l =0;
					while (*(argTypes+l)!=0 && *(sf.argTypes+l) !=0)
					{
						if (*(argTypes+l) != *(sf.argTypes+l)) {
							break;
						} else {
							l++;							
						}
					}
					if (*(argTypes+l)==0 && *(sf.argTypes+l) ==0)
					{
						result = (sf.function)(argTypes, args);
						break;
					}
				}
				k++;
			}
			free(args);
			if (k == 32)
			{
				struct message msg1 = createExecFail(NO_FUNCTION);
				int send_len1 = msg1.length;
				int send_type1 = msg1.type;
				send(soc_fd, &send_len1, INT_SIZE, 0);
				send(soc_fd, &send_type1, INT_SIZE, 0);
				send(soc_fd, msg1.data, send_len1, 0);
			}

			else if (result == 0)
			{
				struct message msg2 = createExecSuc(name, argTypes, args);
				int send_len2 = msg2.length;
				int send_type2 = msg2.type;
				send(soc_fd, &send_len2, INT_SIZE, 0);
				send(soc_fd, &send_type2, INT_SIZE, 0);
				send(soc_fd, msg2.data, send_len2, 0);
			}
			else
			{
				struct message msg3 = createExecFail(result);
				int send_len3 = msg3.length;
				int send_type3 = msg3.type;
				send(soc_fd, &send_len3, INT_SIZE, 0);
				send(soc_fd, &send_type3, INT_SIZE, 0);
				send(soc_fd, msg3.data, send_len3, 0);
			}	
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
		cout << "Unable to create socket" << endl;
		return -1;
	}

	// // Change to non-block so socket will not be blocked by recv
	// fcntl(reg_soc, F_SETFL, O_NONBLOCK);	
	// fcntl(acpt_soc, F_SETFL, O_NONBLOCK);

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
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(0);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(acpt_soc, (sockaddr *)&server_addr, addr_len) == -1)
	{
		cout << "Unable to bind to accept socket" << endl;
		return BIND_ERR;
	}
	if (bind(reg_soc, (sockaddr *)&server_addr, addr_len) == -1)
	{
		cout << "Unable to bind to registration socket" << endl;
		return BIND_ERR;
	}
	if (connect(reg_soc, (sockaddr *)&binder_addr, addr_len) == -1)
	{
		cout << "Unable to connect to binder" << endl;
		return CON_ERR;
	}
	int length = 0;
	int type = INITIALIZE;
	char data[] = "";
	send(reg_soc, &length, INT_SIZE, 0);
	send(reg_soc, &type, INT_SIZE, 0);
	send(reg_soc, data, 0, 0);
	connected = true;
	return 0;
}

// Register server functions 
int rpcRegister(char* name, int* argTypes, skeleton f)
{	
	struct sockaddr_in server_addr;
	memset(&server_addr, '0', sizeof(server_addr));
	socklen_t addr_len = sizeof(sockaddr_in);

	char host[64];
	gethostname(host,64);
	getsockname(reg_soc, (sockaddr *)&server_addr, &addr_len);
	int port = ntohs(server_addr.sin_port);

	struct message msg = createReg(host, port, name, argTypes);
	int send_len = msg.length;
	int send_type = msg.type;
	svfns[n].name = name;
	svfns[n].argTypes = argTypes;
	svfns[n].function = f;
	n++;

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
		cout << "Unable to connected to binder" << endl;
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

	if (conn_soc = socket(AF_INET, SOCK_STREAM, 0) == -1 )
	{
		cout << "Unable to create socket" << endl;
		return -1;
	}
	// fcntl(conn_soc, F_SETFL, O_NONBLOCK);

	char *binder_machine = getenv("BINDER_ADDRESS");
	int binder_port = atoi(getenv("BINDER_PORT"));
	if ((binder = gethostbyname(binder_machine)) == NULL)
	{
		cout << "Invalid host provided" << endl;
		return HOSTNAME_GET;
	}
	binder_addr.sin_family = AF_INET;
	binder_addr.sin_port = htons(binder_port);
	memcpy((char *)&binder_addr.sin_addr.s_addr, (char *)binder->h_addr, binder->h_length);
	
	if (connect(conn_soc, (sockaddr *)&binder_addr, addr_len) == -1)
	{
		cout << "Unable to connect to binder" << endl;
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
		return reasonCode;
	}
	else if (rcv_type == LOC_SUCCESS)
	{
		char data[rcv_len];
		recv(conn_soc, data, rcv_len, 0);
		char *server_host;
		int server_port;
		memcpy(server_host, data, HOST_LEN);
		memcpy(&server_port, data+HOST_LEN, INT_SIZE);
		server = gethostbyname(server_host);

		memset(&server_addr, '0', sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(server_port);
		memcpy((char *)&server_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
		
		if (connect(conn_soc, (sockaddr *)&server_addr, addr_len) == -1)
		{
			cout << "Unable to connect to server" << endl;
			return CON_ERR;
		}

		struct message msg2 = createExec(name, argTypes, args);
		int send_len2 = msg2.length;
		int send_type2 = msg2.type;
		send(conn_soc, &send_len2, INT_SIZE, 0);
		send(conn_soc, &send_type2, INT_SIZE, 0);
		send(conn_soc, msg2.data, send_len2, 0);

		int rcv_len2, rcv_type2, reasonCode2;
		recv(conn_soc, &rcv_len2, INT_SIZE, 0);
		recv(conn_soc, &rcv_type2, INT_SIZE, 0);
		char rcv_data2[rcv_len2];
		if (rcv_type2 == EXECUTE_FAILURE)
		{
			recv(conn_soc, &reasonCode2, INT_SIZE, 0);
			return reasonCode2;
		}
		else if (rcv_type2 == EXECUTE_SUCCESS)
		{
			recv(conn_soc, rcv_data2, rcv_len2, 0);
			return 0;
		}
	}
}

int rpcExecute()
{
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(sockaddr_in);
	int conn;
	pthread_t thread[1];

	if (listen(acpt_soc, 5) == -1)
	{
		cout << "Unable to listen" << endl;
		return LISTEN_ERR;
	}
	if ((conn = accept(acpt_soc, (sockaddr *)&client_addr, &addr_len)) == -1)
	{
		cout << "Fail to connect with client" << endl;
		return ACPT_ERR;
	}
	if (pthread_create(&thread[0], NULL, exec_fn, (void *)&conn) != 0)
	{
		cout << "Error when create thread" << endl;
		return -1;
	}
	pthread_exit(NULL);

	int rcv_len, rcv_type;
	recv(reg_soc, &rcv_len, INT_SIZE, 0);
	recv(reg_soc, &rcv_type, INT_SIZE, 0);
	if (rcv_type == TERMINATE)
	{
		char rcv_data[rcv_len];
		recv(reg_soc, rcv_data, rcv_len, 0);
		close(reg_soc);
		close(acpt_soc);
	}
	return 0;
}

int rpcTerminate()
{
	struct sockaddr_in binder_addr;
	struct hostent *binder;
	socklen_t addr_len = sizeof(sockaddr_in);
	int conn_soc;

	if (conn_soc = socket(AF_INET, SOCK_STREAM, 0) == -1 )
	{
		cout << "Unable to create socket" << endl;
		return -1;
	}

	char *mach_name = getenv("BINDER_ADDRESS");
	int binder_port = atoi(getenv("BINDER_PORT"));
	if ((binder = gethostbyname(mach_name)) == NULL)
	{
		cout << "Invalid host provided" << endl;
		return HOSTNAME_GET;
	}

	binder_addr.sin_family = AF_INET;
	binder_addr.sin_port = htons(binder_port);
	memcpy((char *)&binder_addr.sin_addr.s_addr, (char *)binder->h_addr, binder->h_length);
	
	if (connect(conn_soc, (sockaddr *)&binder_addr, addr_len) == -1)
	{
		cout << "Unable to connect to binder" << endl;
		return CON_ERR;
	}

	struct message msg = createTerm();
	int send_len = msg.length;
	int send_type = msg.type;
	send(conn_soc, &send_len, INT_SIZE, 0);
	send(conn_soc, &send_type, INT_SIZE, 0);
	send(conn_soc, msg.data, send_len, 0);
}