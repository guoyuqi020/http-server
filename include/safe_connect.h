#ifndef _SAFE_CONNECT_H_
#define _SAFE_CONNECT_H_

#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "secure.h"

#define BURSIZE 2048
int socket_s(int __domain, int __type, int __protocol) __THROW;
ssize_t recv_s(SSL * __fd, void *__buf, size_t __n, int __flags);
ssize_t send_s(SSL * __fd, void *__buf, size_t __n, int __flags);
void urlencode(char url[]);
void urldecode(char url[]);
#endif