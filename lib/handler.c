#include "handler.h"
#include "findfiles.h"

char v1[100];
char value1[200];
char v2[100];
char value2[200];
int filed1;

void status_to_text(int status, char *msg)
{
	switch (status)
	{
	case 200:
		strcpy(msg, "ok");
		break;
	case 404:
		strcpy(msg, "not found");
		break;
	case 403:
		strcpy(msg, "fobbiden permission");
		break;
	case 501:
		strcpy(msg, "not implement");
		break;
	case 204:
		strcpy(msg, "no content");
		break;
	case 303:
		strcpy(msg, "see other");
		break;
	case 202:
		strcpy(msg, "accepted");
		break;
	case 500:
		strcpy(msg, "internal error");
		break;
	default:
		printf("unknown status code %d", status);
		exit(-1);
	}
	return;
}
int cut_params(char *uri, char *params)
{
	char *start_pos;
	if ((start_pos = strstr(uri, "?")) == NULL)
	{
		return 0;
	}
	strcpy(params, start_pos + 1);
	*start_pos = (char)0;
	strcat(params, "&");
	return 0;
}

// Returns 0 if it finds "boundary" in buffer, and copies it into value.
// Else it returns -1.
int get_value(char *buffer, const char *key, char *value, char *end_str)
{
	char *pos;
	if ((pos = strstr(buffer, key)) != NULL)
	{
		char *start_pos = pos + strlen(key) + 1;
		char *end_pos = strstr(start_pos, end_str);
		int value_len = end_pos - start_pos;
		memcpy(value, start_pos, value_len);
		value[value_len] = (char)0;
		return 0;
	}
	return -1;
}

void simple_format(int client_fd, int status)
{
	char msg[DEFAULT_MSG_BUFFER_SIZE];
	char send_buffer[DEFAULT_SEND_BUFFER];
	memset(send_buffer, 0, sizeof(char) * DEFAULT_SEND_BUFFER);
	status_to_text(status, msg);
	sprintf(send_buffer, "HTTP/1.1 %d %s\r\n", status, msg);
	sprintf(send_buffer, "%sContent-Length: %d\r\n", send_buffer, 0);
	sprintf(send_buffer, "%sContent-Type:text/html\r\n\r\n", send_buffer);
	send(client_fd, send_buffer, strlen(send_buffer), 0);
	return;
}

void generate_html(char *buff, char *msg)
{
	struct FileName *f_head = readFileList("./resources");
	struct FileName *cur = f_head;
	sprintf(buff, "%s<!DOCTYPE html>\r\n<html>\r\n\r\n<head>\r\n<meta charset=\"utf-8\">\r\n<title>xx.xx</title>\r\n</head>\r\n\r\n<body>\r\n<p>%s</p><br>\r\n", buff, msg);
	while (cur != NULL)
	{
		char encoded_filename[DEFAULT_FILENAME_LEN * 2];
		strcpy(encoded_filename, cur->filename);
		urlencode(encoded_filename);
		sprintf(buff, "%s<a href=\"/download?filename=%s\">%s</a><br>\r\n", buff, encoded_filename, cur->filename);
		cur = cur->next;
	}
	sprintf(buff, "%s<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">\r\n上传文件： <input type=\"file\" name=\"upload\"><br>\r\n<input type=\"submit\">\r\n</form>\r\n</body>\r\n</html>", buff);
	cur = f_head;
	while (cur != NULL)
	{
		cur = cur->next;
		free(f_head);
		f_head = cur;
	}
}

void homepage(int client_fd, char *print_msg)
{
	urldecode(print_msg);
	char send_buffer[DEFAULT_SEND_BUFFER];
	memset(send_buffer, 0, sizeof(char) * DEFAULT_SEND_BUFFER);
	char buff[DEFAULT_BUFFER_SIZE];
	memset(buff, 0, sizeof(char) * DEFAULT_BUFFER_SIZE);
	generate_html(buff, print_msg);
	int status = 200;
	char msg[DEFAULT_MSG_BUFFER_SIZE];
	status_to_text(status, msg);
	sprintf(send_buffer, "HTTP/1.1 %d %s\r\n", status, msg);
	sprintf(send_buffer, "%sContent-Length: %ld\r\n", send_buffer, strlen(buff));
	sprintf(send_buffer, "%sContent-Type:text/html,charset:utf-8;\r\n\r\n", send_buffer);
	sprintf(send_buffer, "%s%s", send_buffer, buff);
	send(client_fd, send_buffer, strlen(send_buffer), 0);
	return;
}

void file_upload(int client_fd, char *buffer, int valid_len)
{
	char boundary[DEFAULT_MSG_BUFFER_SIZE];
	char filename[DEFAULT_URI_BUFFER_SIZE];
	char content_len_str[DEFAULT_MSG_BUFFER_SIZE];
	int recv_len, content_len, file_buffer_valid_len;
	char *file_start_pos, *file_end_pos;

	get_value(buffer, "boundary", boundary + 2, "\r\n");
	boundary[0] = boundary[1] = '-';
	get_value(buffer, "Content-Length", content_len_str, "\r\n");
	content_len = atoi(content_len_str);
	char file_buffer[DEFAULT_BUFFER_SIZE];
	memset(file_buffer, 0, DEFAULT_BUFFER_SIZE);
	if (strstr(buffer, "\r\n\r\n") + 4 == buffer + valid_len)
	{
		recv_len = recv_s(client_fd, file_buffer, DEFAULT_RECV_BUFFER, 0);
		file_buffer_valid_len = recv_len;
	}
	else
	{
		char *content_start_pos = strstr(buffer, "\r\n\r\n") + 4;
		recv_len = buffer + valid_len - content_start_pos;
		memcpy(file_buffer, content_start_pos, recv_len);
		file_buffer_valid_len = recv_len;
	}

	// printf("file buffer:\n%s\n", file_buffer);

	// we set default recv buffer to 4K (valid HTTP header is less than 2K),
	// thus this header must be in the buffer.
	get_value(file_buffer, "filename", filename, "\r\n");
	filename[strlen(filename) - 1] = (char)0;
	memmove(filename, filename + 1, strlen(filename));
	// open write file fd
	char filepath[2 * DEFAULT_URI_BUFFER_SIZE] = DEFAULT_RESOURCE_DIR;
	strcat(filepath, filename);
	FILE *fd = fopen(filepath, "wb+");

	file_start_pos = strstr(file_buffer, "\r\n\r\n");
	if (file_start_pos == NULL)
	{
		printf("Error: can not find fild startpos\n");
		exit(-1);
	}
	file_start_pos += 4;
	file_buffer_valid_len = file_buffer + file_buffer_valid_len - file_start_pos;
	memmove(file_buffer, file_start_pos, file_buffer_valid_len);
	printf("%d/%d/%d\n", recv_len, content_len, file_buffer_valid_len);
	while (recv_len < content_len)
	{
		printf("%d/%d\n", recv_len, content_len);
		int cur_recv_len = recv_s(client_fd, file_buffer + file_buffer_valid_len, DEFAULT_RECV_BUFFER, 0);
		recv_len += cur_recv_len;
		file_buffer_valid_len += cur_recv_len;
		printf("recv:%d content:%d valid:%d\n", recv_len, content_len, file_buffer_valid_len);
		if (recv_len < content_len)
		{
			// just receive DEFAULT_RECV_BUFFER bytes.
			// write DEFAULT_RECV_BUFFER bytes from file buffer to file.
			// !!! NOTE !!!
			// the recv function may not receive DEFAULT_RECV_BUFFER bytes.
			// use min(DEFAULT_RECV_BUFFER, file_buffer_valid_len) instead.
			int write_len = fwrite(file_buffer, sizeof(char), min(DEFAULT_RECV_BUFFER, file_buffer_valid_len), fd);
			file_buffer_valid_len -= write_len;
			memmove(file_buffer, file_buffer + write_len, file_buffer_valid_len);
		}
	}
	fwrite(file_buffer, sizeof(char), file_buffer_valid_len - strlen(boundary) - 6, fd);
	fclose(fd);
	char msg[DEFAULT_MSG_BUFFER_SIZE];
	char send_buffer[DEFAULT_SEND_BUFFER];
	memset(send_buffer, 0, sizeof(char) * DEFAULT_SEND_BUFFER);
	status_to_text(303, msg);
	sprintf(send_buffer, "HTTP/1.1 %d %s\r\n", 303, msg);
	sprintf(send_buffer, "%sContent-Length: %d\r\n", send_buffer, 0);
	sprintf(send_buffer, "%sLocation: /?print_msg=upload file \"%s\" success!\r\n\r\n", send_buffer, filename);
	send(client_fd, send_buffer, strlen(send_buffer), 0);
	return;
}

void file_download(int client_fd, char *filename)
{
	urldecode(filename);
	printf("%s\n", filename);
	char msg[DEFAULT_MSG_BUFFER_SIZE];
	char send_buffer[DEFAULT_SEND_BUFFER];
	memset(send_buffer, 0, sizeof(char) * DEFAULT_SEND_BUFFER);
	char filepath[DEFAULT_URI_BUFFER_SIZE] = "./resources/";
	strcpy(filepath + strlen(filepath), filename);
	FILE *fd = fopen(filepath, "rb");
	if (fd == NULL)
	{
		simple_format(client_fd, 500);
		return;
	}
	fseek(fd, 0, SEEK_END);
	int file_len = ftell(fd);
	rewind(fd);
	int status = 200;
	status_to_text(status, msg);
	sprintf(send_buffer, "HTTP/1.1 %d %s\r\n", status, msg);
	sprintf(send_buffer, "%sContent-Length: %d\r\n", send_buffer, file_len);
	sprintf(send_buffer, "%sContent-Type: application/octet-stream\r\n", send_buffer);
	sprintf(send_buffer, "%sContent-Disposition: attachment;filename=%s\r\n\r\n", send_buffer, filename);
	send(client_fd, send_buffer, strlen(send_buffer), 0);
	int valid_buffer_size = 0;
	while ((valid_buffer_size = fread(send_buffer, sizeof(char), DEFAULT_SEND_BUFFER, fd)) > 0)
	{
		send(client_fd, send_buffer, valid_buffer_size, 0);
	}
	fclose(fd);
	return;
}

void router(int client_fd, char *method, char *uri, char *buffer, int valid_len)
{
	char params[DEFAULT_URI_BUFFER_SIZE];
	memset(params, 0, sizeof(char) * DEFAULT_URI_BUFFER_SIZE);
	if (cut_params(uri, params) == -1)
	{
		printf("can not parse uri!\n%s\n%s\n", uri, params);
		simple_format(client_fd, 500);
		return;
	}
	printf("path: %s\nparams: %s\n", uri, params);
	if (strcmp(uri, "/") == 0)
	{
		if (strcmp(method, "GET") != 0)
		{
			simple_format(client_fd, 500);
		}
		else
		{
			char print_msg[DEFAULT_MSG_BUFFER_SIZE];
			memset(print_msg, 0, sizeof(char) * DEFAULT_MSG_BUFFER_SIZE);
			if (get_value(params, "print_msg", print_msg, "&") == -1)
				strcpy(print_msg, "hello");
			homepage(client_fd, print_msg);
		}
	}
	else if (strcmp(uri, "/upload") == 0)
	{
		if (strcmp(method, "POST") != 0)
		{
			simple_format(client_fd, 500);
		}
		else
		{
			file_upload(client_fd, buffer, valid_len);
		}
	}
	else if (strcmp(uri, "/download") == 0)
	{
		char filename[DEFAULT_URI_BUFFER_SIZE];
		if (get_value(params, "filename", filename, "&") == -1)
		{
			printf("download parameters error\n");
			exit(-1);
		}

		file_download(client_fd, filename);
	}
	else
	{
		int status = 404;
		simple_format(client_fd, status);
	}
	return;
}

void handle(int client_fd, char *buffer, int valid_len) //获取返回的响应报文
{
	char uri[DEFAULT_URI_BUFFER_SIZE];
	char method[5];
	char version[10];
	char header[DEFAULT_HEADER_LEN];
	int status;
	char msg[DEFAULT_MSG_BUFFER_SIZE];
	sscanf(buffer, "%s %s %s", method, uri, version);
	printf("receive request: method:%s uri:%s\n", method, uri);
	if (strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0)
	{
		status = 501;
		simple_format(client_fd, status);
		return;
	}
	else
	{
		router(client_fd, method, uri, buffer, valid_len);
	}
}
