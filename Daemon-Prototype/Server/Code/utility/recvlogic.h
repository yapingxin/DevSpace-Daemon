#ifndef _UTILITY_RECVLOGIC_H_
#define _UTILITY_RECVLOGIC_H_

int startup(unsigned short *);
int get_line(int sockfd, char *buf, int size);
void error_die(const char *);
void accept_request(int);

#endif // _UTILITY_RECVLOGIC_H_
