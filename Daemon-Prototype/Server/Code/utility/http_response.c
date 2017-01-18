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

#include "service_config.h"
#define ISspace(x) isspace((int)(x))

static void headers(int client_sockfd, const char *filename);


static char outbuf[out_to_browser_buf_size] = { 0 };
static char method[accept_method_buf_size] = { 0 };
static char url[accept_url_buf_size] = { 0 };
static char protocol[accept_protocol_buf_size] = { 0 };


/**********************************************************************/
/* Inform the client that a request it has made has a problem.
* Parameters: client socket */
/**********************************************************************/
void bad_request(int client_sockfd)
{
	memset(outbuf, 0, out_to_browser_buf_size);

	sprintf(outbuf, "HTTP/1.0 400 BAD REQUEST\r\n");
	send(client_sockfd, outbuf, sizeof(outbuf), 0);
	sprintf(outbuf, "Content-type: text/html\r\n");
	send(client_sockfd, outbuf, sizeof(outbuf), 0);
	sprintf(outbuf, "\r\n");
	send(client_sockfd, outbuf, sizeof(outbuf), 0);
	sprintf(outbuf, "<P>Your browser sent a bad request, ");
	send(client_sockfd, outbuf, sizeof(outbuf), 0);
	sprintf(outbuf, "such as a POST without a Content-Length.\r\n");
	send(client_sockfd, outbuf, sizeof(outbuf), 0);
}


/**********************************************************************/
/* Inform the client that a CGI script could not be executed.
* Parameter: the client socket descriptor. */
/**********************************************************************/
void cannot_execute(int client_sockfd)
{
	memset(outbuf, 0, out_to_browser_buf_size);

	sprintf(outbuf, "HTTP/1.0 500 Internal Server Error\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "Content-type: text/html\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "<P>Error prohibited CGI execution.\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
}


/**********************************************************************/
/* Give a client a 404 not found status message. */
/**********************************************************************/
void not_found(int client_sockfd)
{
	memset(outbuf, 0, out_to_browser_buf_size);

	sprintf(outbuf, "HTTP/1.0 404 NOT FOUND\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, SERVER_STRING);
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "Content-Type: text/html\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "<HTML><TITLE>Not Found</TITLE>\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "<BODY><P>The server could not fulfill\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "your request because the resource specified\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "is unavailable or nonexistent.\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "</BODY></HTML>\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
}


/**********************************************************************/
/* Inform the client that the requested web method has not been
* implemented.
* Parameter: the client socket */
/**********************************************************************/
void unimplemented(int client_sockfd)
{
	memset(outbuf, 0, out_to_browser_buf_size);

	sprintf(outbuf, "HTTP/1.0 501 Method Not Implemented\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, SERVER_STRING);
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "Content-Type: text/html\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "</TITLE></HEAD>\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "<BODY><P>HTTP request method not supported.\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "</BODY></HTML>\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
}


void default_http_response(const int client_sockfd, const char * msg)
{
	size_t i = 0;
	size_t j = 0;

	memset(outbuf, 0, out_to_browser_buf_size);
	memset(method, 0, accept_method_buf_size);
	memset(url, 0, accept_url_buf_size);
	memset(protocol, 0, accept_protocol_buf_size);

	while (!ISspace(msg[i]) && (i < accept_method_buf_size))
	{
		method[i] = msg[i];
		i++;
	}

	if (strcasecmp(method, "GET") && strcasecmp(method, "POST"))
	{
		memset(method, 0, accept_method_buf_size);
		strcpy(method, "Unknown");
	}
	else
	{
		j = i;
		while (ISspace(msg[j]) && j < accept_line_buf_size)
		{
			j++;
		}

		i = 0;
		while (!ISspace(msg[j]) && (j < accept_line_buf_size) && (i < accept_url_buf_size))
		{
			url[i] = msg[j];
			i++;
			j++;
		}

		while (ISspace(msg[j]) && j < accept_line_buf_size)
		{
			j++;
		}

		i = 0;
		while (!ISspace(msg[j]) && (j < accept_line_buf_size) && (i < accept_protocol_buf_size))
		{
			protocol[i] = msg[j];
			i++;
			j++;
		}
	}

	sprintf(outbuf, "HTTP/1.0 200 OK\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, SERVER_STRING);
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "Content-Type: text/html\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "<HTML><HEAD><TITLE>HTTP Response | client_sockfd = 0x%08X </TITLE></HEAD>\r\n", client_sockfd);
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "<BODY><h1>Your request:</h1><hr /><p> Method: %s <br /> URL: %s <br /> protocol: %s </p><pre>%s</pre><hr />\r\n", method, url, protocol, msg);
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "</BODY></HTML>\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
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
	
	memset(outbuf, 0, out_to_browser_buf_size);

	outbuf[0] = 'A'; outbuf[1] = '\0';
	while ((numchars > 0) && strcmp("\n", outbuf))  /* read & discard headers */
		numchars = get_line(client_sockfd, outbuf, sizeof(outbuf));

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
	memset(outbuf, 0, out_to_browser_buf_size);

	fgets(outbuf, sizeof(outbuf), resource);
	while (!feof(resource))
	{
		send(client_sockfd, outbuf, strlen(outbuf), 0);
		fgets(outbuf, sizeof(outbuf), resource);
	}
}

/**********************************************************************/
/* Execute a CGI script.  Will need to set environment variables as
* appropriate.
* Parameters: client socket descriptor
*             path to the CGI script */
/**********************************************************************/
void execute_cgi(const int client_sockfd, const char *path, const char *method, const char *query_string)
{
	int cgi_output[2];
	int cgi_input[2];
	pid_t pid;
	int status;
	int i;
	char c;
	int numchars = 1;
	int content_length = -1;

	memset(outbuf, 0, out_to_browser_buf_size);

	outbuf[0] = 'A'; outbuf[1] = '\0';
	if (strcasecmp(method, "GET") == 0)
		while ((numchars > 0) && strcmp("\n", outbuf))  /* read & discard headers */
			numchars = get_line(client_sockfd, outbuf, sizeof(outbuf));
	else    /* POST */
	{
		numchars = get_line(client_sockfd, outbuf, sizeof(outbuf));
		while ((numchars > 0) && strcmp("\n", outbuf))
		{
			outbuf[15] = '\0';
			if (strcasecmp(outbuf, "Content-Length:") == 0)
				content_length = atoi(&(outbuf[16]));
			numchars = get_line(client_sockfd, outbuf, sizeof(outbuf));
		}
		if (content_length == -1) {
			bad_request(client_sockfd);
			return;
		}
	}

	sprintf(outbuf, "HTTP/1.0 200 OK\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);

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


/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket to print the headers on
*             the name of the file */
/**********************************************************************/
static void headers(int client_sockfd, const char *filename)
{
	(void)filename;  /* could use filename to determine file type */

	memset(outbuf, 0, out_to_browser_buf_size);

	strcpy(outbuf, "HTTP/1.0 200 OK\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	strcpy(outbuf, SERVER_STRING);
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	sprintf(outbuf, "Content-Type: text/html\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
	strcpy(outbuf, "\r\n");
	send(client_sockfd, outbuf, strlen(outbuf), 0);
}

