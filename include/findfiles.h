#ifndef _FINDFILES_H_
#define _FINDFILES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "safe_connect.h"
#define DEFAULT_FILENAME_LEN 256

struct FileName
{
    char filename[DEFAULT_FILENAME_LEN];
    struct FileName *next;
};


struct FileName * readFileList(char *basePath);

#endif