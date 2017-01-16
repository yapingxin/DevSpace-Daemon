#include <stdio.h>
#include <stdlib.h>

#include "logfunc.h"

/* Socket Server Header Begin */
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

#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Tint HTTP Application Server: 0.1.0\r\n"
/* Socket Server Header End */

static int startup(unsigned short *port);
static void error_die(const char *sc);
static void accept_request(int);

int main(int argc, char *argv[])
{
	int server_sock = -1;
	unsigned short port = 80;
	int client_sock = -1;
	struct sockaddr_in client_socket_addr = { 0 };
	int client_socket_addr_len = sizeof(struct sockaddr_in);
	pthread_t newthread = NULL;

	file_output_open(global_log_file_path);

	ZF_LOGI("");
	ZF_LOGI("");
	ZF_LOGI("*****************************************************");
	ZF_LOGI("**       Tiny HTTP Application Server Start        **");
	ZF_LOGI("*****************************************************");
	ZF_LOGI("");

	ZF_LOGI("You will see the number of arguments: %i", argc);
	ZF_LOGD("You will NOT see the first argument: %s", *argv);

	ZF_LOGW("You will see this WARNING message");
	ZF_LOGI("You will NOT see this INFO message");

	server_sock = startup(&port);

	printf("http server running on port %d\n", port);
	ZF_LOGI("http server running on port %i", port);

	while (1)
	{
		client_sock = accept(server_sock, (struct sockaddr *)&client_socket_addr, &client_socket_addr_len);
		if (client_sock == -1)
		{
			error_die("accept() failed.");
		}

		/* accept_request(client_sock); */
		//pthread_create(&newthread, NULL, accept_request, (void**)client_sock);
		//pthread_create(&newthread, NULL, accept_request, NULL);
		
		
		if (pthread_create(&newthread, NULL, accept_request, client_sock) != 0)
		{
			error_die("pthread_create() failed.");
		}
		
	}

	printf("Server is ruinning.\n");
    return 0;
}


static int startup(unsigned short *port)
{
	const int listen_count = 5;

	int httpd = 0;
	struct sockaddr_in server_socket_addr = { 0 };
	int addr_size;

	httpd = socket(PF_INET, SOCK_STREAM, 0);
	if (httpd == -1)
	{
		error_die("socket() initialize failed.");
	}

	memset(&server_socket_addr, 0, sizeof(struct sockaddr_in));
	server_socket_addr.sin_family = AF_INET;
	server_socket_addr.sin_port = htons(*port);
	server_socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(httpd, (struct sockaddr *)&server_socket_addr, sizeof(struct sockaddr_in)) < 0)
	{
		error_die("socket bind() failed.");
	}

	/* if dynamically allocating a port */
	if (*port == 0)
	{
		addr_size = sizeof(struct sockaddr_in);
		if (getsockname(httpd, (struct sockaddr *)&server_socket_addr, &addr_size) == -1)
		{
			error_die("getsockname() failed.");
		}

		*port = ntohs(server_socket_addr.sin_port);
	}

	if (listen(httpd, listen_count) < 0)
	{
		error_die("listen() failed.");
	}

	return httpd;
}

/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
* on value of errno, which indicates system call errors) and exit the
* program indicating an error. */
/**********************************************************************/
void error_die(const char *sc)
{
	ZF_LOGF("%s", sc);

	perror(sc);
	exit(1);
}

/**********************************************************************/
/* A request has caused a call to accept() on the server port to
* return.  Process the request appropriately.
* Parameters: the socket connected to the client */
/**********************************************************************/
static void accept_request(int client)
{

}
