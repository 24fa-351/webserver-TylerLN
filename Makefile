server: server.c http_message.c http_message.h http_func.c http_func.h
	gcc -o server server.c http_message.c http_func.c -lpthread

clean:
	rm -f server

