
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

