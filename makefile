all:
	g++ -w -c helper.cc -o helper.o
	g++ -w -c message.cc -o message.o
	g++ -w -c rpc.cc -o rpc.o -lpthread
	g++ -w -c binder.cc -o binder.o -lpthread
	ar rc librpc.a helper.o message.o rpc.o
	ranlib librpc.a 
	g++ binder.o helper.o message.o -o binder -lpthread
	g++ -w -c client1.c -o client.o
	g++ -L. client.o -lrpc -lpthread -o client
	g++ -w -c server.c -o server.o
	g++ -L. client.o -lrpc -lpthread -o client