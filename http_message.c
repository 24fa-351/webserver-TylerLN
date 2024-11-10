#include "http_message.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

bool is_complete_http_message(char* buffer) {
  size_t len = strlen(buffer);
  return (len >= 4 && strstr(buffer, "\r\n\r\n") != NULL);
}

void read_http_client_message(int client_socket,
                              http_client_message_t** message,
                              http_read_result_t* result) {
  *message = malloc(sizeof(http_client_message_t));
  char buffer[1024];
  size_t buffer_len = 0;
  memset(buffer, 0, sizeof(buffer));

  while (!is_complete_http_message(buffer)) {
    int bytes_read = read(client_socket, buffer + buffer_len,
                          sizeof(buffer) - buffer_len - 1);
    if (bytes_read == 0) {
      *result = CLOSE_CONNECTION;
      return;
    }
    if (bytes_read < 0) {
      *result = BAD_REQUEST;
      return;
    }
    buffer_len += bytes_read;
    buffer[buffer_len] = '\0';
  }

  (*message)->method = strdup(buffer + 4);
  *result = MESSAGE;
}

void http_client_message_free(http_client_message_t* message) {
  free(message->method);
  free(message);
}
