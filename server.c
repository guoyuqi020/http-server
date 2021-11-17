#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>

#include "server_handler.h"
#include "safe_connect.h"
#include <event.h>
#include "pipeline.h"

//divide buffer into several http requests, for pipeline

int divide_buffer(char *recv_buffer, int l, char reqs[N_REQ][DEFAULT_RECV_BUFFER_SIZE], int *length, int *rest_len)
{
	int n = -1;
	int i = 0;
	int k = 0;

	char method[DEFAULT_BUFFER_SIZE] = {0};
	sscanf(recv_buffer, "%s ", method);
	if (strcmp(method, "POST") == 0)
	{
		memcpy(reqs[0], recv_buffer, l);
		length[0] = l;
		return 1;
	}
	char line[DEFAULT_BUFFER_SIZE] = {0};
	// must be GET
	char *startpos = recv_buffer, *endpos = recv_buffer;
	char *req_startpos = recv_buffer;
	while (n < N_REQ && startpos - recv_buffer <= l && (endpos = strstr(startpos, "\n")) != NULL)
	{
		endpos += 1;
		sscanf(startpos, "%s ", method);
		if (strcmp(method, "GET") == 0)
		{
			// a new GET request!
			if (n >= 0)
			{
				memset(reqs[n], 0, sizeof(char) * DEFAULT_RECV_BUFFER_SIZE);
				length[n] = startpos - req_startpos;
				memcpy(reqs[n], req_startpos, length[n]);
				*rest_len = recv_buffer + l - startpos;
			}
			req_startpos = startpos;
			n += 1;
		}

		startpos = endpos;
	}
	if (n < N_REQ)
	{
		memset(reqs[n], 0, sizeof(char) * DEFAULT_RECV_BUFFER_SIZE);
		length[n] = recv_buffer + l - req_startpos;
		memcpy(reqs[n], req_startpos, length[n]);
		*rest_len = 0;
		n += 1;
	}
	return n;
}

struct event_base *base;

SSL_CTX *ctx;

void on_accept(int server_fd, short event, void *arg)
{
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	int client_fd;
	char recv_buffer[DEFAULT_RECV_BUFFER_SIZE];
	int n;
	char reqs[N_REQ][DEFAULT_RECV_BUFFER_SIZE] = {0};
	int recv_rest = 0;
	// read_ev must allocate from heap memory, otherwise the program would crash from segmant fault
	if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
							&client_addr_size)) == -1)
	{
		perror("accept failed:");
		return;
	}

	SSL *ssl = SSL_new(ctx);
	SSL_set_fd(ssl, client_fd);

	if (SSL_accept(ssl) <= 0)
	{
		perror("ssl state:");
	}
	memset(recv_buffer, 0, sizeof(char) * DEFAULT_RECV_BUFFER_SIZE);
	while (1)
	{
		if (n == 0)
			n = recv_s(ssl, recv_buffer + recv_rest, DEFAULT_RECV_BUFFER_SIZE - recv_rest, 0);
		if (n == 0)
			break;

		int n_buffer;
		int req_len[N_REQ] = {0};

		memset(reqs, 0, sizeof(char) * N_REQ * DEFAULT_RECV_BUFFER_SIZE);
		n_buffer = divide_buffer(recv_buffer, n, reqs, req_len, &recv_rest);

		for (int i = 0; i < n_buffer; i++)
		{
			handle(ssl, reqs[i], req_len[i]);
		}
		memmove(recv_buffer, recv_buffer + n - recv_rest, recv_rest);
		n = recv_rest;
	}
	SSL_shutdown(ssl);
	SSL_free(ssl);

	close(client_fd);
}

int main(int arg, char *argv[])
{
	init_openssl();

	ctx = create_context();
	configure_context(ctx);

	int server_fd;
	struct sockaddr_in server_addr;

	int server_socket_reuseable = 1;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(5000);

	server_fd = socket_s(AF_INET, SOCK_STREAM, 0);
	if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&server_socket_reuseable, sizeof(server_socket_reuseable))) < 0)
		perror("setsockopt SO_REUSEADDR failed");

	if ((bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
	{
		perror("bind failed");
		exit(-1);
	}
	if (listen(server_fd, 10) < 0)
		perror("start listening failed");
	// signal(SIGCHLD, sig_child);
	printf("------------waiting for client -----------\n");
	struct event listen_ev;
	base = event_base_new();
	event_set(&listen_ev, server_fd, EV_READ | EV_PERSIST, on_accept, NULL);
	event_base_set(base, &listen_ev);
	event_add(&listen_ev, NULL);
	event_base_dispatch(base);
}
