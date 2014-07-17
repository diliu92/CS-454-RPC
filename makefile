all:
	g++ test.cc rpc.cc message.cc server_function_skels.c server_functions.c -o test -lpthread
