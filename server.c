#include "connection.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define LISTEN_BACKLOG 5
#define DEFAULT_PORT 80

int main(int argc, char* argv[]) {
  int port = DEFAULT_PORT;

  if (argc > 1) {
    port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
      perror(
          "Invalid port number. Please provide a port between 1 and 65535.\n");
      return 1;
    }
  }

  printf("Server Listening on Port: %d\n", port);

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    perror("Failed to create socket");
    return 1;
  }

  struct sockaddr_in server_address;
  memset(&server_address, '\0', sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(port);

  int return_val = bind(socket_fd, (struct sockaddr*)&server_address,
                        sizeof(server_address));
  if (return_val == -1) {
    perror("Failed to bind socket");
    return 1;
  }

  return_val = listen(socket_fd, LISTEN_BACKLOG);
  if (return_val == -1) {
    perror("Failed to listen on socket");
    return 1;
  }

  struct sockaddr_in client_address;
  socklen_t client_address_len = sizeof(client_address);

  while (1) {
    pthread_t thread;
    int* client_fd_buf = malloc(sizeof(int));
    if (!client_fd_buf) {
      perror("Failed to allocate memory for client file descriptor");
      continue;
    }

    *client_fd_buf = accept(socket_fd, (struct sockaddr*)&client_address,
                            &client_address_len);
    if (*client_fd_buf == -1) {
      perror("Failed to accept connection");
      free(client_fd_buf);
      continue;
    }

    printf("Accepted connection on %d\n", *client_fd_buf);

    pthread_create(&thread, NULL, (void* (*)(void*))handle_connection,
                   (void*)client_fd_buf);
    pthread_detach(thread);
  }

  close(socket_fd);
  return 0;
}
