#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "logfunc.h"
#include "recvlogic.h"
#include "http_response.h"

/**********************************************************************/

int main(int argc, char *argv[])
{
	unsigned short main_service_port = 8001;
	int server_sockfd = -1;
	int client_sockfd = -1;
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	pthread_t request_thread;

	server_sockfd = startup(&main_service_port);
	printf("httpd running on port %d\n", main_service_port);

	while (1)
	{
		client_sockfd = accept(server_sockfd,
			(struct sockaddr *)&client_addr,
			&client_addr_len);
		if (client_sockfd == -1)
			error_die("accept");
		/* accept_request(client_sock); */
		if (pthread_create(&request_thread, NULL, accept_request, client_sockfd) != 0)
			perror("pthread_create");
	}

	close(server_sockfd);

	return(0);
}
