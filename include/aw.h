#ifndef _AW_H_
#define _AW_H_
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

#define DEFAULT_MSG_BUFFER_SIZE 100
#define DEFAULT_URI_BUFFER_SIZE 100
#define DEFAULT_HEADER_LEN 2048
#define DEFAULT_RECV_BUFFER 4096 * 10
#define DEFAULT_SEND_BUFFER 4096 * 10
#define DEFAULT_BUFFER_SIZE 2 * DEFAULT_RECV_BUFFER
#define DEFAULT_RESOURCE_DIR "./resources/"

#define min(a, b) ((a < b) ? a : b)

void handle(int fd1, char *url, int valid_len);
#endif
