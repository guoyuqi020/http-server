#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include "handler.h"
#include <string.h>
#include <arpa/inet.h>
#include "safe_connect.h"
#include <event.h>

struct event_base* base;
void on_accept(int server_fd, short event, void *arg) //消灭僵尸进程 僵尸进程会占用资源如果一直不释放的话
{
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	int client_fd;
	char recv_buffer[DEFAULT_RECV_BUFFER];
	int n;
	// read_ev must allocate from heap memory, otherwise the program would crash from segmant fault
	if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
							&client_addr_size)) == -1)
	{
		perror("accept failed:");
	}
	while (1)
	{
		memset(recv_buffer, 0, sizeof(char) * DEFAULT_RECV_BUFFER);
		n = recv_s(client_fd, recv_buffer, DEFAULT_RECV_BUFFER, 0);
		if (n == 0)
			break;
		handle(client_fd, recv_buffer, n);
	}
	//printf("connection_closed\n");
	close(client_fd);
}
int main(int arg, char *argv[])
{
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
    event_set(&listen_ev, server_fd, EV_READ|EV_PERSIST, on_accept, NULL);
    event_base_set(base, &listen_ev);
    event_add(&listen_ev, NULL);
    event_base_dispatch(base);
}
