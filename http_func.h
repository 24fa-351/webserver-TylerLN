#ifndef HTTP_FUNC_H
#define HTTP_FUNC_H

extern unsigned long total_requests;
extern unsigned long total_received_bytes;
extern unsigned long total_sent_bytes;

void static_file(int sock_fd, const char* file_path);
void stats(int sock_fd, unsigned long total_requests,
           unsigned long total_received_bytes, unsigned long total_sent_bytes);
void calc(int sock_fd, const char* query);

#endif
