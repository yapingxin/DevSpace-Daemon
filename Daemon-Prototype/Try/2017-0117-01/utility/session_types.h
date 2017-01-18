#ifndef _UTILITY_SESSION_TYPES_H_
#define _UTILITY_SESSION_TYPES_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>

#include <inttypes.h>
#include <linux/tcp.h> /* #define TCP_NODELAY 1 */

typedef struct
{
	struct sockaddr_in client_addr;
	int server_sockfd;
	int client_sockfd;

} session_data_t;


#endif // _UTILITY_SESSION_TYPES_H_
