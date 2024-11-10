#include "http_message.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

bool is_complete_http_message(char* buffer) {
  if (strlen(buffer) < 10) {
    return true;
  }
  if (strncmp(buffer, "GET", 4) != 0) {
    return false;
  }
  if (strncmp(buffer + strlen(buffer) - 2, "\n\n", 4) != 0) {
    return false;
  }
  return false;
}

void read_http_client_message(int client_socket,
                              http_client_message_t** message,
                              http_read_result_t* result) {
  *message = malloc(sizeof(http_client_message_t));
  char buffer[1024];
  strcpy(buffer, "");
  while (!is_complete_http_message(buffer)) {
    int bytes_read = read(client_socket, buffer + strlen(buffer),
                          sizeof(buffer) - strlen(buffer));
    if (bytes_read == 0) {
      *result = CLOSE_CONNECTION;
      return;
    }
    if (bytes_read < 0) {
      *result = BAD_REQUEST;
      return;
    }
  }
  (*message)->method = strdup(buffer + 4);
  *result = MESSAGE;
}

void http_client_message_free(http_client_message_t* message) { free(message); }