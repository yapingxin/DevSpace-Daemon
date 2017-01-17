#ifndef _UTILITY_RECVLOGIC_H_
#define _UTILITY_RECVLOGIC_H_

int startup(unsigned short port);
int get_line(const int sockfd, char *buf, const int buf_size);
void error_die(const char *msg);
void accept_request(const int client_sockfd);
void mainloop_recv(const int server_sockfd, const unsigned char service_poweron);

#endif // _UTILITY_RECVLOGIC_H_
