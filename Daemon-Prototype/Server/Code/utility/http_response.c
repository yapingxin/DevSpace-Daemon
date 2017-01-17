#include "http_response.h"

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


/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
*             the name of the file */
/**********************************************************************/
void headers(int client_sockfd, const char *filename)
{
	char buf[1024];
	(void)filename;  /* could use filename to determine file type */

	strcpy(buf, "HTTP/1.0 200 OK\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	strcpy(buf, SERVER_STRING);
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Inform the client that a request it has made has a problem.
* Parameters: client socket */
/**********************************************************************/
void bad_request(int client_sockfd)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
	send(client_sockfd, buf, sizeof(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client_sockfd, buf, sizeof(buf), 0);
	sprintf(buf, "\r\n");
	send(client_sockfd, buf, sizeof(buf), 0);
	sprintf(buf, "<P>Your browser sent a bad request, ");
	send(client_sockfd, buf, sizeof(buf), 0);
	sprintf(buf, "such as a POST without a Content-Length.\r\n");
	send(client_sockfd, buf, sizeof(buf), 0);
}


/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
* Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute(int client_sockfd)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
}


/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void not_found(int client_sockfd)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "your request because the resource specified\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "is unavailable or nonexistent.\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
}


/**********************************************************************/
/* Inform the client that the requested web method has not been
* implemented.
* Parameter: the client socket */
/**********************************************************************/
void unimplemented(int client_sockfd)
{
	char buf[1024];

	sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, SERVER_STRING);
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "</TITLE></HEAD>\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
	sprintf(buf, "</BODY></HTML>\r\n");
	send(client_sockfd, buf, strlen(buf), 0);
}

/**********************************************************************/
/* Send a regular file to the client.  Use headers, and report
* errors to client if they occur.
* Parameters: a pointer to a file structure produced from the socket
*              file descriptor
*             the name of the file to serve */
/**********************************************************************/
void serve_file(int client_sockfd, const char *filename)
{
	FILE *resource = NULL;
	int numchars = 1;
	char buf[1024];

	buf[0] = 'A'; buf[1] = '\0';
	while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
		numchars = get_line(client_sockfd, buf, sizeof(buf));

	resource = fopen(filename, "r");
	if (resource == NULL)
		not_found(client_sockfd);
	else
	{
		headers(client_sockfd, filename);
		cat(client_sockfd, resource);
	}
	fclose(resource);
}

/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
* is named after the UNIX "cat" command, because it might have been
* easier just to do something like pipe, fork, and exec("cat").
* Parameters: the client socket descriptor
*             FILE pointer for the file to cat */
/**********************************************************************/
void cat(int client_sockfd, FILE *resource)
{
	char buf[1024];

	fgets(buf, sizeof(buf), resource);
	while (!feof(resource))
	{
		send(client_sockfd, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
}

/**********************************************************************/
/* Execute a CGI script.  Will need to set environment variables as
* appropriate.
* Parameters: client socket descriptor
*             path to the CGI script */
/**********************************************************************/
void execute_cgi(int client_sockfd, const char *path,
	const char *method, const char *query_string)
{
	char buf[1024];
	int cgi_output[2];
	int cgi_input[2];
	pid_t pid;
	int status;
	int i;
	char c;
	int numchars = 1;
	int content_length = -1;

	buf[0] = 'A'; buf[1] = '\0';
	if (strcasecmp(method, "GET") == 0)
		while ((numchars > 0) && strcmp("\n", buf))  /* read & discard headers */
			numchars = get_line(client_sockfd, buf, sizeof(buf));
	else    /* POST */
	{
		numchars = get_line(client_sockfd, buf, sizeof(buf));
		while ((numchars > 0) && strcmp("\n", buf))
		{
			buf[15] = '\0';
			if (strcasecmp(buf, "Content-Length:") == 0)
				content_length = atoi(&(buf[16]));
			numchars = get_line(client_sockfd, buf, sizeof(buf));
		}
		if (content_length == -1) {
			bad_request(client_sockfd);
			return;
		}
	}

	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	send(client_sockfd, buf, strlen(buf), 0);

	if (pipe(cgi_output) < 0) {
		cannot_execute(client_sockfd);
		return;
	}
	if (pipe(cgi_input) < 0) {
		cannot_execute(client_sockfd);
		return;
	}

	if ((pid = fork()) < 0) {
		cannot_execute(client_sockfd);
		return;
	}
	if (pid == 0)  /* child: CGI script */
	{
		char meth_env[255];
		char query_env[255];
		char length_env[255];

		dup2(cgi_output[1], 1);
		dup2(cgi_input[0], 0);
		close(cgi_output[0]);
		close(cgi_input[1]);
		sprintf(meth_env, "REQUEST_METHOD=%s", method);
		putenv(meth_env);
		if (strcasecmp(method, "GET") == 0) {
			sprintf(query_env, "QUERY_STRING=%s", query_string);
			putenv(query_env);
		}
		else {   /* POST */
			sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
			putenv(length_env);
		}
		execl(path, path, NULL);
		exit(0);
	}
	else {    /* parent */
		close(cgi_output[1]);
		close(cgi_input[0]);
		if (strcasecmp(method, "POST") == 0)
			for (i = 0; i < content_length; i++) {
				recv(client_sockfd, &c, 1, 0);
				write(cgi_input[1], &c, 1);
			}
		while (read(cgi_output[0], &c, 1) > 0)
			send(client_sockfd, &c, 1, 0);

		close(cgi_output[0]);
		close(cgi_input[1]);
		waitpid(pid, &status, 0);
	}
}
