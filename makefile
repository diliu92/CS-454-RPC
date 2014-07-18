all:
	g++ test.cc helper.cc rpc.cc message.cc server_function_skels.c server_functions.c -o test -lpthread
	g++ server.c helper.cc rpc.cc message.cc server_function_skels.c server_functions.c -o server -lpthread
	g++ client1.c helper.cc rpc.cc message.cc server_function_skels.c server_functions.c -o client -lpthread
	g++ binder.cc helper.cc rpc.cc message.cc server_function_skels.c server_functions.c -o binder -lpthread
