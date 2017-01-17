
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
#include <pthread.h>

#include <inttypes.h>
#include <linux/tcp.h> /* #define TCP_NODELAY 1 */

#include "recvlogic.h"
#include "logfunc.h"

#define ISspace(x) isspace((int)(x))

static void accept_request(const int client_sockfd);


int startup(unsigned short port)
{
	int httpd = 0;
	struct sockaddr_in sock_addr;

	httpd = socket(PF_INET, SOCK_STREAM, 0);
	if (httpd == -1)
	{
		error_die("socket() failed.");
	}

	memset(&sock_addr, 0, sizeof(struct sockaddr_in));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(httpd, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr_in)) < 0)
	{
		error_die("bind() failed.");
	}
	
	if (listen(httpd, 5) < 0)
	{
		error_die("listen() failed.");
	}

	return(httpd);
}


void error_die(const char *msg)
{
	perror(msg);
	exit(1);
}


void mainloop_recv(const int server_sockfd, const unsigned char service_poweron)
{
	int client_sockfd = -1;
	int client_addr_len = sizeof(struct sockaddr_in);
	struct sockaddr_in client_addr = { 0 };
	pthread_t request_thread = NULL;

	while (service_poweron)
	{
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (client_sockfd == -1)
		{
			error_die("accept");
		}

		/* accept_request(client_sockfd); */
		if (pthread_create(&request_thread, NULL, accept_request, client_sockfd) != 0)
		{
			perror("pthread_create");
		}
	}
}

int get_line(const int sockfd, char *buf, const int buf_size)
{
	int i = 0;
	char c = '\0';
	int n;

	while ((i < buf_size - 1) && (c != '\n'))
	{
		n = recv(sockfd, &c, 1, 0);
		/* DEBUG printf("%02X\n", c); */
		if (n > 0)
		{
			if (c == '\r')
			{
				n = recv(sockfd, &c, 1, MSG_PEEK);
				/* DEBUG printf("%02X\n", c); */
				if ((n > 0) && (c == '\n'))
					recv(sockfd, &c, 1, 0);
				else
					c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else
			c = '\n';
	}
	buf[i] = '\0';

	return(i);
}


static void accept_request(const int client_sockfd)
{
	char buf[1024];
	int numchars;
	char method[255];
	char url[255];
	char path[512];
	size_t i, j;
	struct stat status;
	int cgi = 0;      /* becomes true if server decides this is a CGI
					  * program */
	char *query_string = NULL;

	numchars = get_line(client_sockfd, buf, sizeof(buf));
	i = 0; j = 0;
	while (!ISspace(buf[j]) && (i < sizeof(method) - 1))
	{
		method[i] = buf[j];
		i++; j++;
	}
	method[i] = '\0';

	if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	{
		unimplemented(client_sockfd);
		return;
	}

	if (strcasecmp(method, "POST") == 0)
		cgi = 1;

	i = 0;
	while (ISspace(buf[j]) && (j < sizeof(buf)))
		j++;
	while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))
	{
		url[i] = buf[j];
		i++; j++;
	}
	url[i] = '\0';

	if (strcasecmp(method, "GET") == 0)
	{
		query_string = url;
		while ((*query_string != '?') && (*query_string != '\0'))
			query_string++;
		if (*query_string == '?')
		{
			cgi = 1;
			*query_string = '\0';
			query_string++;
		}
	}

	sprintf(path, "htdocs%s", url);
	if (path[strlen(path) - 1] == '/')
		strcat(path, "index.html");
	if (stat(path, &status) == -1) {
		while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
			numchars = get_line(client_sockfd, buf, sizeof(buf));
		not_found(client_sockfd);
	}
	else
	{
		//if ((st.st_mode & S_IFMT) == S_IFDIR)
		if (S_ISDIR(status.st_mode))
			strcat(path, "/index.html");
		if ((status.st_mode & S_IXUSR) ||
			(status.st_mode & S_IXGRP) ||
			(status.st_mode & S_IXOTH))
			cgi = 1;
		if (!cgi)
			serve_file(client_sockfd, path);
		else
			execute_cgi(client_sockfd, path, method, query_string);
	}

	close(client_sockfd);
}

