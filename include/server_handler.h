#ifndef _SERVER_HANDLER_H_
#define _SERVER_HANDLER_H_
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "safe_connect.h"
#include <event.h>

#define DEFAULT_MSG_BUFFER_SIZE 256
#define DEFAULT_URI_BUFFER_SIZE 1024
#define DEFAULT_HEADER_LEN 2048
#define DEFAULT_RECV_BUFFER_SIZE 4096 * 10
#define DEFAULT_SEND_BUFFER_SIZE 4096 * 10
#define DEFAULT_BUFFER_SIZE 2 * DEFAULT_RECV_BUFFER_SIZE
#define DEFAULT_RESOURCE_DIR "./resources/"

#define min(a, b) ((a < b) ? a : b)

void handle(SSL * fd1, char *url, int valid_len);
#endif
