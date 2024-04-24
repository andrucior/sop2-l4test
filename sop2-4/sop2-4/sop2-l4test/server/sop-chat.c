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

void chatWork(int client_socket, char* data)
{
    ssize_t  size;
    char name[NAME_SIZE];
    char message[MESSAGE_SIZE];

    while(1) {
        if ((size = bulk_read(client_socket, data, BUFF_SIZE)) < 0)
            ERR("read");

        memcpy(name, data, NAME_SIZE);
        memcpy(message, data + MESSAGE_OFFSET, MESSAGE_SIZE);
        printf("%s wrote: %s\n", name, message);
    }
    if (TEMP_FAILURE_RETRY(close(client_socket)) < 0)
        ERR("close");
}

void serverWork(int localInSocket, int tcpInSocket, char* key) {
    int epoll;

    if ((epoll = epoll_create1(0)) == -1)
        ERR("epoll_create");

    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = localInSocket;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, localInSocket, &event) == -1) {
        perror("epoll_ctl: listen sock");
        exit(EXIT_FAILURE);
    }
    event.data.fd = tcpInSocket;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, tcpInSocket, &event) == -1) {
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
    char data[BUFF_SIZE];
    char name[NAME_SIZE];
    char message[MESSAGE_SIZE];
    int clients = 0;

    while (running) {
        if ((nfds = epoll_pwait(epoll, events, MAX_EVENTS, -1, &old)) > 0) {
            fprintf(stderr, "%d\n", nfds);
            //running = 0;
            for (int i = 0; i < nfds; i++) {
                if (clients < 4) {
                    int client_socket = add_new_client(events[i].data.fd);
                    fprintf(stderr, "Number of clients: %d\n", clients);
                    clients++;

                    if ((size = bulk_read(client_socket, (char*) data, BUFF_SIZE)) < 0)
                        ERR("read:");
                    if (size == BUFF_SIZE) {
                        memcpy(name, data, NAME_SIZE);
                        memcpy(message, data + MESSAGE_OFFSET, MESSAGE_SIZE);
                        printf("Name: %s, message: %s\n", name, message);

                        if (strcmp(message, key) == 0) {
                            if (bulk_write(client_socket, data, BUFF_SIZE) < 0 && errno != SIGPIPE)
                                ERR("write");

                            fprintf(stderr, "Authorization passed\n");

                            switch (fork()) {
                                case 0:
                                    chatWork(client_socket, data);
                                    return;
                                case -1:
                                    ERR("fork");
                                default:
                                    break;
                            }
                        }
                    }
                }
                else
                {
                    fprintf(stderr, "Could not add a client - too much already connected\n");
                    running = 0;
                }

            }
        } else {
            if (errno == EINTR)
                continue;
            else
                ERR("epoll_pwait");
        }
    }
    if (TEMP_FAILURE_RETRY(close(epoll)) < 0)
        ERR("close");
    sigprocmask(SIG_UNBLOCK, &new, NULL);

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
    char* name = "localhost";

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
    serverWork(localInSocket, tcpInSocket, key);

    if (TEMP_FAILURE_RETRY(close(localInSocket)) < 0)
        ERR("close");
    if(unlink(name) < 0)
        ERR("unlink");
    if (TEMP_FAILURE_RETRY(close(tcpInSocket)) < 0)
        ERR("close");

    fprintf(stderr, "Server has terminated\n");
    return EXIT_SUCCESS;
}
