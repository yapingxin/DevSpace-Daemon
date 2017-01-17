#include <stdio.h>

#include "service_config.h"
#include "logfunc.h"
#include "recvlogic.h"
#include "http_response.h"

/**********************************************************************/

int main(int argc, char *argv[])
{
	int server_sockfd = -1;
	unsigned char service_poweron = 1;

	server_sockfd = startup(main_service_port);
	printf("httpd running on port %d\n", main_service_port);

	file_output_open(global_log_file_path);

	ZF_LOGI("");
	ZF_LOGI("");
	ZF_LOGI("*****************************************************");
	ZF_LOGI("**       Tiny HTTP Application Server Start        **");
	ZF_LOGI("*****************************************************");
	ZF_LOGI("");

	mainloop_recv(server_sockfd, service_poweron);

	close(server_sockfd);
	return(0);
}
