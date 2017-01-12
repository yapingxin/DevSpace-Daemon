#include <stdio.h>
#include <stdlib.h>

#include "logfunc.h"



int main(int argc, char *argv[])
{
	file_output_open(global_log_file_path);

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


