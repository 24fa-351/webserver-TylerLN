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
  if (*message == NULL) {
    *result = BAD_REQUEST;
    return;
  }

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

  char* method_end = strchr(buffer, ' ');
  if (method_end == NULL) {
    *result = BAD_REQUEST;
    return;
  }

  *method_end = '\0';
  (*message)->method = strdup(buffer);

  char* path_start = method_end + 1;
  char* path_end = strchr(path_start, ' ');
  if (path_end == NULL) {
    *result = BAD_REQUEST;
    return;
  }

  *path_end = '\0';
  (*message)->path = strdup(path_start);

  char* query_params = strchr((*message)->path, '?');
  if (query_params != NULL) {
    *query_params = '\0';
    (*message)->query_params = strdup(query_params + 1);
  } else {
    (*message)->query_params = NULL;
  }

  char* version_start = path_end + 1;
  (*message)->http_version = strdup(version_start);

  *result = MESSAGE;
}

void http_client_message_free(http_client_message_t* message) { free(message); }
