#ifndef _UTILITY_LOGFUNC_H_
#define _UTILITY_LOGFUNC_H_

//#define ZF_LOG_LEVEL ZF_LOG_INFO
//#define ZF_LOG_TAG "MAIN"
#include "../lib/log/zf_log.h"

#define global_log_file_path	"/home/yapingxin/DevSpace/log/Daemon-Prototype/Server.log"

void file_output_open(const char *const log_path);

#endif // _UTILITY_LOGFUNC_H_
