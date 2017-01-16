#include <stdio.h>
#include <stdlib.h>

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

#include <inttypes.h>
#include <linux/tcp.h> /* #define TCP_NODELAY 1 */

#define MAXPENDING 5
#define BUFSIZE 1024

#include "logfunc.h"

#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Tint HTTP Application Server: 0.1.0\r\n"
/* Socket Server Header End */

static void error_die(const char *sc);

/*

- socket
  sockfd = socket(int socket_family, int socket_type, int protocol);
  http://man7.org/linux/man-pages/man7/socket.7.html

- setsockopt
  int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
  http://man7.org/linux/man-pages/man2/setsockopt.2.html

- bind
  int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
  http://man7.org/linux/man-pages/man2/bind.2.html

- listen - listen for connections on a socket
  int listen(int sockfd, int backlog);
  http://man7.org/linux/man-pages/man2/listen.2.html

- accept, accept4 - accept a connection on a socket
  int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
  http://man7.org/linux/man-pages/man2/accept.2.html
  https://linux.die.net/man/2/accept

- inet_ntop - convert IPv4 and IPv6 addresses from binary to text form
  const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
  http://man7.org/linux/man-pages/man3/inet_ntop.3.html
  https://linux.die.net/man/3/inet_ntop

*/

static void log_client_info(struct sockaddr_in * p_client_addr);
static void accept_request(int client_sockfd);


int main(int argc, char *argv[])
{
	uint16_t server_port = 8001;
	int server_sockfd = 0;
	int client_sockfd = 0;
	struct sockaddr_in server_addr = { 0 };
	struct sockaddr_in client_addr = { 0 };
	socklen_t client_addr_len = sizeof(struct sockaddr_in);
	int flag;
	int ret;

	file_output_open(global_log_file_path);

	ZF_LOGI("");
	ZF_LOGI("");
	ZF_LOGI("*****************************************************");
	ZF_LOGI("**       Tiny HTTP Application Server Start        **");
	ZF_LOGI("*****************************************************");
	ZF_LOGI("");

	//ZF_LOGI("You will see the number of arguments: %i", argc);
	//ZF_LOGD("You will NOT see the first argument: %s", *argv);

	//ZF_LOGW("You will see this WARNING message");
	//ZF_LOGI("You will NOT see this INFO message");

	server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_sockfd < 0)
	{
		error_die("socket() failed.");
	}
	else
	{
		ZF_LOGI("socket() ==> server_sockfd = 0x%08x", server_sockfd);
	}

	/* Disable the Nagle (TCP No Delay) algorithm */
	flag = 1;
	ret = setsockopt(server_sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
	if (ret == -1)
	{
		ZF_LOGW("Couldn't setsockopt(TCP_NODELAY)");
	}
	else
	{
		ZF_LOGI("setsockopt(TCP_NODELAY) success.");
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(server_port);

	ret = bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if (ret < 0)
	{
		error_die("bind() failed.");
	}
	else
	{
		ZF_LOGI("bind() result: bind to port %i", server_port);
		printf("bind() result: bind to port %d\n", server_port);
	}

	ret = listen(server_sockfd, MAXPENDING);
	if (ret < 0)
	{
		error_die("listen() failed.");
	}
	else
	{
		ZF_LOGI("listen() at port %i with backlog = %i", server_port, MAXPENDING);
		printf("listen() at port %d with backlog = %d\n", server_port, MAXPENDING);
	}

	ZF_LOGI("Server is ruinning.");
	printf("Server is ruinning.\n");

	while (1)
	{
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (client_sockfd < 0)
		{
			error_die("accept() failed.");
		}

		log_client_info(&client_addr);

		accept_request(client_sockfd);
	}

    return 0;
}

static void accept_request(int client_sockfd)
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

static void log_client_info(struct sockaddr_in *p_client_addr)
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
static void error_die(const char *sc)
{
	ZF_LOGF("%s", sc);

	printf("%s\n", sc);

	perror(sc);
	exit(1);
}

