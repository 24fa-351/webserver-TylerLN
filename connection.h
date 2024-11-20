#ifndef CONNECTION_H
#define CONNECTION_H

#include "http_message.h"

extern unsigned long total_requests;
extern unsigned long total_received_bytes;
extern unsigned long total_sent_bytes;

int respond_to_http_message(int sock_fd, http_client_message_t* http_msg);
void handle_connection(int* sock_fd_ptr);

#endif
