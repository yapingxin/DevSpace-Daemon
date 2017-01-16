
/* Socket Server Header Begin */
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

#include <inttypes.h>
#include <linux/tcp.h> /* #define TCP_NODELAY 1 */

#include "recvlogic.h"
#include "logfunc.h"

#define ISspace(x) isspace((int)(x))

#define BUFSIZE 1024

int disable_tcp_nagle(int sockfd)
{
	int flag = 1;
	int ret = 0;

	return setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
}

void accept_request(int client_sockfd)
{
	static char buffer[BUFSIZE] = { 0 };

	size_t num_bytes_rcvd = 0;

	num_bytes_rcvd = recv(client_sockfd, buffer, BUFSIZE, 0);
	if (num_bytes_rcvd < 0)
	{
		error_die("recv() failed.");
	}

	/* if num_bytes_rcvd == 0 then the end of stream is reached. */
	while (num_bytes_rcvd > 0)
	{
		ZF_LOGI("Recv: %s", buffer);
		printf("Recv: %s\n", buffer);

		num_bytes_rcvd = recv(client_sockfd, buffer, BUFSIZE, 0);
		if (num_bytes_rcvd < 0)
		{
			error_die("recv() failed.");
		}
	}

	close(client_sockfd);
}

void log_client_info(struct sockaddr_in *p_client_addr)
{
	static char client_name[INET_ADDRSTRLEN] = { 0 };

	const char *ret_constxt = inet_ntop(AF_INET, &p_client_addr->sin_addr.s_addr, client_name, INET_ADDRSTRLEN);
	if (ret_constxt != NULL)
	{
		ZF_LOGI("Handling client %s/%i", client_name, ntohs(p_client_addr->sin_port));
		printf("Handling client %s/%d\n", client_name, ntohs(p_client_addr->sin_port));
	}
}


/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
* on value of errno, which indicates system call errors) and exit the
* program indicating an error. */
/**********************************************************************/
void error_die(const char *sc)
{
	ZF_LOGF("%s", sc);

	printf("%s\n", sc);

	perror(sc);
	exit(1);
}