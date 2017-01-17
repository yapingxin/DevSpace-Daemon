#ifndef _UTILITY_RECVLOGIC_H_
#define _UTILITY_RECVLOGIC_H_

void error_die(const char *sc);
void log_client_info(struct sockaddr_in * p_client_addr);
void accept_request(int client_sockfd);

int disable_tcp_nagle(int sockfd);

void sendString(char *message, int socket);
void sendHeader(char *Status_code, char *Content_Type, int TotalSize, int socket);
void sendHTML(char *statusCode, char *contentType, char *content, int size, int socket);

#endif // _UTILITY_RECVLOGIC_H_
