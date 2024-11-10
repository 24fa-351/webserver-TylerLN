#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "http_func.h"
#include "http_message.h"

#define LISTEN_BACKLOG 5
#define MAX_BUFFER_SIZE 1024
#define DEFAULT_PORT 80

unsigned long total_requests = 0;
unsigned long total_received_bytes = 0;
unsigned long total_sent_bytes = 0;

int respond_to_http_message(int sock_fd, http_client_message_t* http_msg) {
  char* response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
  write(sock_fd, response, strlen(response));
  return 0;
}

void handle_connection(int* sock_fd_ptr) {
  int sock_fd = *sock_fd_ptr;
  free(sock_fd_ptr);

  while (1) {
    printf("Handling connection on %d\n", sock_fd);

    http_client_message_t* http_msg = NULL;
    http_read_result_t result;

    read_http_client_message(sock_fd, &http_msg, &result);

    if (http_msg != NULL) {
      total_received_bytes += http_msg->body_length;
    }

    if (result == BAD_REQUEST) {
      printf("Bad Request on %d\n", sock_fd);
      close(sock_fd);
      return;
    } else if (result == CLOSE_CONNECTION) {
      printf("Closing connection on %d\n", sock_fd);
      close(sock_fd);
      return;
    }

    printf("Received HTTP Request:\n");
    printf("Method: %s\n", http_msg->method);
    printf("Path: %s\n", http_msg->path);
    printf("HTTP Version: %s\n", http_msg->http_version);
    printf("Query Params: %s\n", http_msg->query_params);

    if (strncmp(http_msg->path, "/static", 7) == 0) {
      char file_path[MAX_BUFFER_SIZE];
      snprintf(file_path, sizeof(file_path), ".%s", http_msg->path);
      static_file(sock_fd, file_path);
    } else if (strcmp(http_msg->path, "/stats") == 0) {
      stats(sock_fd, total_requests, total_received_bytes, total_sent_bytes);
    } else if (strncmp(http_msg->path, "/calc", 5) == 0) {
      calc(sock_fd, http_msg->query_params);
    } else {
      respond_to_http_message(sock_fd, http_msg);
    }

    http_client_message_free(http_msg);
  }

  printf("Closing connection on %d\n", sock_fd);
  close(sock_fd);
}

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
