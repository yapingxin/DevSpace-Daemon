#ifndef _UTILITY_RECVLOGIC_H_
#define _UTILITY_RECVLOGIC_H_

int startup(unsigned short port);
void mainloop_recv(const int server_sockfd, const unsigned char service_poweron);
void error_die(const char *msg);
int get_line(const int sockfd, char *buf, const int buf_size);

#endif // _UTILITY_RECVLOGIC_H_
