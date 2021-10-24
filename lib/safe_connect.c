#include <safe_connect.h>

int socket_s(int __domain, int __type, int __protocol)
{
    int server_fd;
    if ((server_fd = socket(__domain, __type, __protocol)) == -1)
    {
        perror("create socket failed");
        exit(-1);
    }
    return server_fd;
}

ssize_t recv_s(int __fd, void *__buf, size_t __n, int __flags)
{
    int recv_len;
    if ((recv_len = recv(__fd, __buf, __n, __flags)) == -1)
    {
        printf("recv failed\n");
        exit(-1);
    }
    return recv_len;
}