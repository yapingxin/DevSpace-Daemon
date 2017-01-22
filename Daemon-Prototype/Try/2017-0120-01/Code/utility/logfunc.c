#include "logfunc.h"
#include <stdio.h>
#include <stdlib.h>

static FILE *global_log_file;

static void file_output_close(void);
static void file_output_callback(const zf_log_message *msg, void *arg);

void file_output_open(const char *const log_path)
{
	global_log_file = fopen(log_path, "a");
	if (!global_log_file)
	{
		ZF_LOGW("Failed to open log file %s", log_path);
		return;
	}
	atexit(file_output_close);
	zf_log_set_output_v(ZF_LOG_PUT_STD, 0, file_output_callback);
}

static void file_output_close(void)
{
	fclose(global_log_file);
}

static void file_output_callback(const zf_log_message *msg, void *arg)
{
	(void)arg;
	*msg->p = '\n';
	fwrite(msg->buf, msg->p - msg->buf + 1, 1, global_log_file);
	fflush(global_log_file);
}
