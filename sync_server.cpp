/* TCP echo server */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

volatile sig_atomic_t running = 0;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        running = 0;
    }
}

int handle_client(int cfd);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    int server_fd, client_fd, err;
    struct sockaddr_in server, client;

    server_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    if (err < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    err = listen(server_fd, 1);
    if (err < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on %d\n", port);

    struct sigaction action;
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (-1 == sigaction(SIGINT, &action, NULL)) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    for (running = 1; running;) {
        socklen_t client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

        if (client_fd == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            exit(EXIT_FAILURE);
        }
        char client_addr[INET6_ADDRSTRLEN];
        printf("new client connected from '%s:%d'\n",
               inet_ntop(client.sin_family, &client.sin_addr, client_addr,
                         INET6_ADDRSTRLEN),
               ntohs(client.sin_port));

        if (-1 == handle_client(client_fd)) {
            if (errno != EINTR) {
                fprintf(stderr, "Error: %s", strerror(errno));
            }
        }
        printf("Client disconnected\n");
        close(client_fd);
    }
    printf("Stopping server");
    close(server_fd);

    return 0;
}

int handle_client(int cfd) {
    char buff[BUFFER_SIZE];

    while (running) {
        ssize_t rb = read(cfd, buff, BUFFER_SIZE);
        if (rb == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        } else if (rb == 0) {
            /* client close session */
            return 0;
        }

        char *bufp = buff;
        while (rb) {
            ssize_t wb = write(cfd, bufp, rb);
            if (wb == -1) {
                if (errno == EINTR) {
                    continue;
                }
                return -1;
            }
            bufp += wb;
            rb -= wb;
        }
    }
    return -1;
}
