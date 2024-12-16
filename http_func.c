#include "http_func.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "http_message.h"

#define MAX_BUFFER_SIZE 1024

void static_file(int sock_fd, const char *file_path) {
    total_requests++;

    FILE *file = fopen(file_path, "rb");
    if (!file) {
        char *error_response =
            "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        write(sock_fd, error_response, strlen(error_response));
        total_sent_bytes += strlen(error_response);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", file_size);
    write(sock_fd, header, strlen(header));
    total_sent_bytes += strlen(header);

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        write(sock_fd, buffer, bytes_read);
        total_sent_bytes += bytes_read;
    }

    fclose(file);
}

void stats(int sock_fd, unsigned long total_requests,
           unsigned long total_received_bytes, unsigned long total_sent_bytes) {
    total_requests++;

    char response[MAX_BUFFER_SIZE];

    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n"
             "Total Requests: %lu\n"
             "Total Received Bytes: %lu\n"
             "Total Sent Bytes: %lu\n",
             total_requests, total_received_bytes, total_sent_bytes);

    write(sock_fd, response, strlen(response));
    total_sent_bytes += strlen(response);
}

void calc(int sock_fd, const char *query_params) {
    total_requests++;

    if (query_params == NULL) {
        write(sock_fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n",
              42);
        total_sent_bytes += 42;
        return;
    }

    int num_1 = 0, num_2 = 0;
    int num_params = sscanf(query_params, "a=%d&b=%d", &num_1, &num_2);

    if (num_params != 2) {
        write(sock_fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n",
              42);
        total_sent_bytes += 42;
        return;
    }

    int result = num_1 + num_2;

    char response[MAX_BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "<html><body><h1>Calculation Result</h1><p>%d</p></body></html>\n",
             result);

    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: "
             "text/html\r\n\r\n",
             strlen(response));

    write(sock_fd, header, strlen(header));
    write(sock_fd, response, strlen(response));
    total_sent_bytes += strlen(header) + strlen(response);
}