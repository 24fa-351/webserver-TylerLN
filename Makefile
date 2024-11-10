server: server.c http_message.c http_message.h
	gcc -o server server.c http_message.c -lpthread