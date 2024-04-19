#include "l4-common.h"

#define BACKLOG_SIZE 10
#define MAX_CLIENT_COUNT 4
#define MAX_EVENTS 10

#define NAME_OFFSET 0
#define NAME_SIZE 64
#define MESSAGE_OFFSET NAME_SIZE
#define MESSAGE_SIZE 448
#define BUFF_SIZE (NAME_SIZE + MESSAGE_SIZE)

void usage(char *program_name) {
    fprintf(stderr, "USAGE: %s port key\n", program_name);
    exit(EXIT_FAILURE);
}

void serverWork(int localInSocket, int tcpInSocket)
{
    int epoll;

    if ((epoll = epoll_create1(0)) == -1)
        ERR("epoll_create");

    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = localInSocket;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, localInSocket, &event) == -1)
    {
        perror("epoll_ctl: listen sock");
        exit(EXIT_FAILURE);
    }
    event.data.fd = tcpInSocket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, tcpInSocket, &event) == -1)
    {
        perror("epoll_ctl: listen sock");
        exit(EXIT_FAILURE);
    }
    int nfds;
    sigset_t old, new;
    sigemptyset(&new);
    sigaddset(&new, SIGINT);
    sigprocmask(SIG_BLOCK, &new, &old);
    int running = 1;
    ssize_t size;
    int32_t data[16];
    while (running)
    {
        if ((nfds = epoll_pwait(epoll, events, MAX_EVENTS, -1, &old)) > 0)
        {
            fprintf(stderr, "%d\n", nfds);
            //running = 0;
            for (int i = 0; i < nfds; i++)
            {
                int client_socket = add_new_client(events[i].data.fd);

                if ((size = bulk_read(client_socket, (char*)data, sizeof(int32_t[16]))) < 0)
                    ERR("read:");
                if (size == sizeof(int32_t[16]))
                {
                    running = 0;
                    sleep(5);
                }
                if (TEMP_FAILURE_RETRY(close(client_socket)) < 0)
                    ERR("close");

            }
        }
        else
        {
            if (errno == EINTR)
                continue;
            else
                ERR("epoll_pwait");
        }
    }
    if (TEMP_FAILURE_RETRY(close(epoll)) < 0)
        ERR("close");
    sigprocmask(SIG_UNBLOCK, &new , NULL);

}

int main(int argc, char **argv) {
    char *program_name = argv[0];
    int localInSocket, tcpInSocket, flags;
    if (argc != 3) {
        usage(program_name);
    }

    uint16_t port = atoi(argv[1]);
    if (port == 0){
        usage(argv[0]);
    }

    char *key = argv[2];
    char* name = "test";

    if(sethandler(SIG_IGN, SIGPIPE))
        ERR("Setting SIGPIPE: ");
    if(sethandler(SIG_IGN, SIGINT))
        ERR("Setting SIGINT");

    localInSocket = bind_local_socket(name, BACKLOG_SIZE);
    flags = fcntl(localInSocket, F_GETFL) | O_NONBLOCK;
    fcntl(localInSocket, F_SETFL, flags);
    tcpInSocket = bind_tcp_socket(port, BACKLOG_SIZE);
    flags = fcntl(tcpInSocket, F_GETFL) |  O_NONBLOCK;
    fcntl(tcpInSocket, F_SETFL, flags);
    serverWork(localInSocket, tcpInSocket);

    if (TEMP_FAILURE_RETRY(close(localInSocket)) < 0)
        ERR("close");
    if(unlink(name) < 0)
        ERR("unlink");
    if (TEMP_FAILURE_RETRY(close(tcpInSocket)) < 0)
        ERR("close");

    fprintf(stderr, "Server has terminated\n");
    return EXIT_SUCCESS;
}
