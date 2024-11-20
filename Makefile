server: server.c http_message.c http_message.h http_func.c http_func.h connection.h connection.c
	gcc -o server server.c http_message.c http_func.c connection.c connection.h -lpthread

clean:
	rm -f server

