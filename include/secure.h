#ifndef _SECURE_H_
#define _SECURE_H_ 
#include <openssl/ssl.h>
#include <openssl/err.h>

void init_openssl();
void cleanup_openssl();
SSL_CTX *create_context();
void configure_context(SSL_CTX *ctx);

#endif