#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments\n");
        exit(1);
    }

    int master_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (master_sock == -1) {
        fprintf(stderr, "Fatal error\n");
        exit(1);
    }

    struct sockaddr_in serveraddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(2130706433),
        .sin_port = htons(atoi(argv[1]))
    };

    bind(master_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(master_sock, 128);

    fd_set all_fds, read_fds, write_fds;
    int max_fd = master_sock;
    FD_ZERO(&all_fds);
    FD_SET(master_sock, &all_fds);

    while (1) {
        read_fds = write_fds = all_fds;

        if (select(max_fd + 1, &read_fds, &write_fds, NULL, NULL) <= 0) {
            continue;
        }

        for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == master_sock) {
                    int new_fd = accept(fd, NULL, NULL);
                    FD_SET(new_fd, &all_fds);
                    if (new_fd > max_fd) {
                        max_fd = new_fd;
                    }
                    continue;
                }

                char buffer[4096 * 42];
                int recv_size = recv(fd, buffer, sizeof(buffer), 0);
                if(recv_size <= 0) {
                    FD_CLR(fd, &all_fds);
                    close(fd);
                    continue;
                }

                buffer[recv_size] = '\0';
                send(fd, buffer, strlen(buffer), 0);
            }
        }
    }

    return 0;
}

