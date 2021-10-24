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

void sig_child(int signo) //消灭僵尸进程 僵尸进程会占用资源如果一直不释放的话
{
	int pid;
	int state;
	while ((pid = waitpid(-1, (int *)0, WNOHANG)) > 0)
		return;
}
int main(int arg, char *argv[])
{
	int server_fd, client_fd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	char recv_buffer[DEFAULT_RECV_BUFFER];
	int n;
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
	int pid;
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
	close(client_fd);
}
