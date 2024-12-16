#include "connection.h"
#include "http_func.h"
#include "http_message.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024

unsigned long total_requests       = 0;
unsigned long total_received_bytes = 0;
unsigned long total_sent_bytes     = 0;

int respond_to_http_message(int sock_fd, http_client_message_t *http_msg) {
    char *response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    write(sock_fd, response, strlen(response));
    return 0;
}

void handle_connection(int *sock_fd_ptr) {
    int sock_fd = *sock_fd_ptr;
    free(sock_fd_ptr);

    while (1) {
        printf("Handling connection on %d\n", sock_fd);

        http_client_message_t *http_msg = NULL;
        http_read_result_t result;

        read_http_client_message(sock_fd, &http_msg, &result);

        if (http_msg != NULL)
            total_received_bytes += http_msg->body_length;

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
            stats(sock_fd, total_requests, total_received_bytes,
                  total_sent_bytes);
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
