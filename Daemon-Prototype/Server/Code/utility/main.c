#include <stdio.h>
#include <stdlib.h>

//#define ZF_LOG_LEVEL ZF_LOG_INFO
//#define ZF_LOG_TAG "MAIN"
#include "../lib/log/zf_log.h"

FILE *g_log_file;

static void file_output_open(const char *const log_path);
static void file_output_close(void);
static void file_output_callback(const zf_log_message *msg, void *arg);

int main(int argc, char *argv[])
{
	file_output_open("http_server.log");

	ZF_LOGI("");
	ZF_LOGI("");
	ZF_LOGI("*****************************************************");
	ZF_LOGI("**       Tiny HTTP Application Server Start        **");
	ZF_LOGI("*****************************************************");
	ZF_LOGI("");

	ZF_LOGI("You will see the number of arguments: %i", argc);
	ZF_LOGD("You will NOT see the first argument: %s", *argv);

	printf("hello from Server!\n");

	ZF_LOGW("You will see this WARNING message");
	ZF_LOGI("You will NOT see this INFO message");

    return 0;
}


static void file_output_open(const char *const log_path)
{
	g_log_file = fopen(log_path, "a");
	if (!g_log_file)
	{
		ZF_LOGW("Failed to open log file %s", log_path);
		return;
	}
	atexit(file_output_close);
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, file_output_callback);
}

static void file_output_close(void)
{
	fclose(g_log_file);
}

static void file_output_callback(const zf_log_message *msg, void *arg)
{
	(void)arg;
	*msg->p = '\n';
	fwrite(msg->buf, msg->p - msg->buf + 1, 1, g_log_file);
	fflush(g_log_file);
}
