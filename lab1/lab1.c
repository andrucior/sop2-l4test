#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define N 10

extern char* optarg;

void usage(char* pname);

int main(int argc, char** argv)
{
    int c;
    char dir[N][N];
    strcpy(dir[0],getenv("PWD"));
    //printf("%s", dir[0]);
    int i = 0;
    int rec = 0;
    int if_r = 0;
    int if_o = 0;
    char* to_save = NULL;

    while ((c = getopt(argc,argv, "p:ro:")) != -1)
    {
        switch(c)
        {
            case 'p':
                strcpy(dir[i],optarg);
                i++;
                break;
            case 'r':
                if (!if_r)
                {
                    rec = 1;
                    if_r = 1;
                }
                else
                    ERR("INVALID FLAG REPETITION");
                break;
            case 'o':
                if (!if_o)
                {
                    to_save = (char*)malloc(sizeof(char) * (strlen(optarg)+1));
                    strcpy(to_save,optarg);
                    if_o = 1;
                }
                else
                    ERR("INVALID FLAG REPETITION");           
                break;
            case'?':
            default:
                usage(argv[0]);
        }
    }
    char command[50] = "ls -s -F -h";
    for (int j = 0; j < i; j++)
    {
         strcat(command, " ");
         strcat(command, dir[j]);
         strcat(command, " ");
    }
    if (to_save)
    {
        strcat(command, " >");
        strcat(command, to_save);
    }
    if (rec)
        strcat(command, " -R");
    system(command); 
    return EXIT_SUCCESS;  
}

void usage(char* pname)
{
    fprintf(stderr, "USAGE:%s ([-p <DIRECTORY> -r Recursively -o <FILE>] ...)\n", pname);
    exit(EXIT_FAILURE);
}