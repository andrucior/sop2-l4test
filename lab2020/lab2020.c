#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__))
#define N 1024

extern char* optarg;

void save(char* file_name);
void usage(char* pname);

int main (int argc, char** argv)
{
    srand(time(NULL));
    int c = 0;
    char* path = NULL;
    int if_p = 0;
    int if_f = 0;
    int if_t = 0;
    int if_s = 0;
    char* file_name = NULL;
    char* which = NULL;

    while ((c=getopt(argc,argv,"p:t:s:f:"))  != -1)
    {
        switch (c)
        {
            case 'p':
                //path = (char*)malloc(sizeof(char)*(strlen(optarg)+1));
                path = strdup(optarg);
                if_p = 1;
                if (!path) ERR("Path does not exist");
                break;
            case 't':
                //which = (char*)malloc(sizeof(char));
                if (!which) ERR("Allocation error");
                which = strdup(optarg);
                if_t = 1;
                break;
            case 's':
                int check = setenv("SIZE", optarg, 1);
                if (check == -1) ERR("Error with setenv");
                if_s = 1;
                break;
            case 'f':
                //file_name = (char*)malloc(sizeof(char)*(strlen(optarg)+1));
                if (!file_name) ERR("Allocation error");
                file_name = strdup(optarg);
                save(file_name);
                if_f = 1;
                break;
            case '?':
            default:
                usage(argv[0]);
        }
    }
    if (!if_p) ERR("Error: no path");
    char command[N] = "find .";
    char prev_command[N] = "cd ";
    //strcat(prev_command,path);
    printf("%s",path);
    system(prev_command);
    
   /*  if (if_f)
    {
        strcat(command, " > ");
        strcat(command, file_name);
    }
    if (if_t)
    {
        strcat(command, " -type ");
        strcat(command, which);
    }
    if (if_s)
    {
        strcat(command," -size +");
        strcat(command,getenv("size"));
        strcat(command, "c");
    } */
    printf("%s", command);
    //system(command);
    return EXIT_SUCCESS;
}
void save(char* file_name)
{
    FILE* to_save;
    char* name = (char*)malloc(sizeof(char)*(strlen(file_name)+5));
    strcpy(name,file_name);
    int los = rand() % 10;
    if (los < 4)
        strcat(name,".out");
    else
        strcat(name,".txt");
    to_save = fopen(name, "ab");
    if (!to_save)
        ERR("File error");
}

void usage(char* pname)
{
    fprintf(stderr, "USAGE: %s (-p PATH [-t TYPE -s SIZE -f FILE])\n", pname);
    exit(EXIT_FAILURE);
}