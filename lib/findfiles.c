#include "findfiles.h"



struct FileName * readFileList(char *basePath)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1000];
    struct FileName *f_head = NULL;
    struct FileName *cur = f_head;

    if ((dir = opendir(basePath)) == NULL)
    {
        perror("directory path does not exist");
        exit(-1);
    }

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0 || strcmp(ptr->d_name, ".gitignore") == 0) //current dir OR parrent dir
            continue;
        else if (ptr->d_type == 8) //file
        {
            if (cur==NULL)
            {
                f_head = (struct FileName *)malloc(sizeof(struct FileName));
                f_head->next = NULL;
                cur = f_head;
            }
            else
            {
                cur->next = (struct FileName *)malloc(sizeof(struct FileName));
                cur = cur->next;
                cur->next = NULL;
            }
            strcpy(cur->filename, ptr->d_name);
        }
    }
    closedir(dir);
    return f_head;
}