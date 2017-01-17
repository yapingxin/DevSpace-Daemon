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
#include <signal.h>
#include <inttypes.h>

#define MAXPENDING 5


#include "logfunc.h"
#include "recvlogic.h"

#define SERVER_STRING "Tint HTTP Application Server: 0.1.0\r\n"
/* Socket Server Header End */

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

static int child_process_count = 0;

void sig_child(int signo);


int main(int argc, char *argv[])
{
	uint16_t server_port = 8001;
	int server_sockfd = 0;
	int client_sockfd = 0;
	struct sockaddr_in server_addr = { 0 };
	struct sockaddr_in client_addr = { 0 };
	socklen_t client_addr_len = sizeof(struct sockaddr_in);
	
	int ret;

	pid_t childpid;

	file_output_open(global_log_file_path);

	ZF_LOGI("");
	ZF_LOGI("");
	ZF_LOGI("*****************************************************");
	ZF_LOGI("**       Tiny HTTP Application Server Start        **");
	ZF_LOGI("*****************************************************");
	ZF_LOGI("");

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
	
	ret = disable_tcp_nagle(server_sockfd);
	if (ret == -1)
	{
		ZF_LOGW("Couldn't setsockopt(TCP_NODELAY)");
	}
	else
	{
		ZF_LOGI("setsockopt(TCP_NODELAY) success.");
	}

	memset(&server_addr, 0, sizeof(struct sockaddr_in));
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

	//signal(SIGCHLD, sig_child);

	while (1)
	{
		memset(&client_addr, 0, sizeof(struct sockaddr_in));
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (client_sockfd < 0)
		{
			error_die("accept() failed.");
		}

		if ((childpid = fork()) == 0)
		{
			// child process

			close(server_sockfd);

			log_client_info(&client_addr);
			accept_request(client_sockfd);

			exit(0);
		}
		else if (childpid < 0)
		{
			error_die("fork() failed.");
		}
		else
		{
			// parent process

			// Do not count in child process :: begin
			//child_process_count++;
			//ZF_LOGI("[mainloop] child_process_count: %i", child_process_count);
			//printf("[mainloop] child_process_count: %d\n", child_process_count);
			// Do not count in child process :: end

			close(client_sockfd);
		}
	}

    return 0;
}

void sig_child(int signo)
{
	pid_t pid = 0;
	int status = 0;

	ZF_LOGI("sig_child entering.");
	printf("sig_child entering.\n");

	
	//while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
	//while ((pid = waitpid(-1, NULL, WNOHANG)) > 0);

	switch (signo)
	{
	case SIGCHLD:
		signal(SIGCHLD, sig_child);
		while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
		{
			child_process_count--;
			ZF_LOGI("[sig_child] child_process_count: %i", child_process_count);
			printf("[sig_child] child_process_count: %d\n", child_process_count);
		}

		signal(signo, sig_child);
		ZF_LOGI("[sig_child] signal re-add");
		printf("[sig_child] signal re-add\n");

		break;
	case SIGTSTP:
		exit(0);
		break;
	}

	ZF_LOGI("Client exit.");
	printf("Client exit.\n");
	
	return;
}