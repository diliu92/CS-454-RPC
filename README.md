CS-454-RPC
==========

Group Member: 
	Silin Li - 20387220 
	Di Liu - 20369479

How to build:
	run the following command in the folder:
		- make
		- g++ -L. client.o -lrpc -lpthread -o client
		- g++ -L. server.o -lrpc -lpthread -o server

How to run:
	- "./binder" to run the binder
	- Set environment variable for every terminal with: 
		"setenv BINDER_ADDRESS <binder_address>"
		"setenv BINDER_PORT <binder_port>"
	- "./server" to run a server
	- "./client" to run a client 

