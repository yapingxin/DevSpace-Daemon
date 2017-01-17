
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
	static char buf_send[BUFSIZE] = { 0 };

	size_t num_bytes_rcvd = 0;
	char* msgBack = "recv() executed.";

	char *statusCode = "200";
	char *contentType = "text/html";

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

	//send(client_sockfd, msgBack, strlen(msgBack), 0);

	memset(buf_send, 0, BUFSIZE);
	strcat(buf_send, "<html>");
	strcat(buf_send, "<head>");
	strcat(buf_send, "<title>");
	strcat(buf_send, "Response from Server");
	strcat(buf_send, "</title>");
	strcat(buf_send, "</head>");
	strcat(buf_send, "<body>");
	strcat(buf_send, "<p>");
	strcat(buf_send, "The request is: \n\n");
	strcat(buf_send, buffer);
	strcat(buf_send, "</p>");
	strcat(buf_send, "</body>");
	strcat(buf_send, "</html>");

	sendHTML(statusCode, contentType, buf_send, strlen(buf_send), client_sockfd);

	ZF_LOGI("[sendHTML] %s", buf_send);

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


void sendHeader(char *Status_code, char *Content_Type, int TotalSize, int socket)
{
	char *head = "\r\nHTTP/1.1 ";
	char *content_head = "\r\nContent-Type: ";
	char *server_head = "\r\nServer: PT06";
	char *length_head = "\r\nContent-Length: ";
	char *date_head = "\r\nDate: ";
	char *newline = "\r\n";
	char contentLength[100];

	time_t rawtime;

	time(&rawtime);

	// int contentLength = strlen(HTML);
	sprintf(contentLength, "%i", TotalSize);

	char *message = malloc((
		strlen(head) +
		strlen(content_head) +
		strlen(server_head) +
		strlen(length_head) +
		strlen(date_head) +
		strlen(newline) +
		strlen(Status_code) +
		strlen(Content_Type) +
		strlen(contentLength) +
		28 +
		sizeof(char)) * 2);

	if (message != NULL)
	{

		strcpy(message, head);

		strcat(message, Status_code);

		strcat(message, content_head);
		strcat(message, Content_Type);
		strcat(message, server_head);
		strcat(message, length_head);
		strcat(message, contentLength);
		strcat(message, date_head);
		strcat(message, (char*)ctime(&rawtime));
		strcat(message, newline);

		sendString(message, socket);

		free(message);
	}
}


void sendString(char *message, int socket)
{
	int length, bytes_sent;
	length = strlen(message);

	bytes_sent = send(socket, message, length, 0);

	return bytes_sent;
}

void sendHTML(char *statusCode, char *contentType, char *content, int size, int socket)
{
	sendHeader(statusCode, contentType, size, socket);
	sendString(content, socket);
}
