all:
	g++ test.cc binder.cc rpc.cc message.cc server_function_skels.c server_functions.c -o test -lpthread

