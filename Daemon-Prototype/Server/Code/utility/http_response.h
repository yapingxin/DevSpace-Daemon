#ifndef _UTILITY_HTTP_RESPONSE_H_
#define _UTILITY_HTTP_RESPONSE_H_

#include <stdio.h>

#define SERVER_STRING "Server: Tiny HTTP Application Server/0.2.1\r\n"

void bad_request(int client_sockfd);
void cannot_execute(int client_sockfd);
void not_found(int);
void unimplemented(int);
void serve_file(int, const char *);
void cat(int, FILE *);
void execute_cgi(const int, const char *, const char *, const char *);

void default_http_response(const int client_sockfd, const char * msg);

#endif // _UTILITY_HTTP_RESPONSE_H_
