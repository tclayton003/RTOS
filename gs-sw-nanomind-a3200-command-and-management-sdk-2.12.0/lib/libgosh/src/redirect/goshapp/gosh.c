/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/gosh/redirect/redirect.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <termios.h>
#include <fcntl.h>
#include <poll.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include "../iputil.h"

static struct termios original;
static bool gosh_exit = false;

static void tty_raw(void)
{
    struct termios current;

    if (tcgetattr(STDIN_FILENO, &current) < 0)
        perror("tcgetattr");

    original = current;

    current.c_iflag &= ~(IGNCR | ICRNL);
    current.c_lflag &= ~(ECHO | ICANON | IEXTEN);

    current.c_cc[VMIN] = 1;
    current.c_cc[VTIME] = 0;

    /* Put terminal in raw mode after flushing */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &current) < 0)
        perror("tcsetattr");
}

static void tty_restore(void)
{
    /* Restore termios settings */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original) < 0)
        perror("tcsetattr");
}

static void exit_handler(void)
{
    tty_restore();
}

static void sigint_handler(int signo)
{
    printf("\n");
    gosh_exit = true;
}

static int do_connection(struct hostent *server, short port, bool reconnect)
{
    /* Create a socket point */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    /* Now connect to the server */
    bool noticed = false;
    bool connected = false;
    do {
        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == 0) {
            connected = true;
            break;
        }

        if (reconnect) {
            if (!noticed) {
                printf("Waiting for remote...");
                noticed = true;
            }
            fflush(stdout);
            struct timespec ts = { .tv_sec = 0, .tv_nsec = (200000 * 1000)};
            nanosleep(&ts, NULL);
        } else {
            fprintf(stderr, "ERROR connecting to remote: %s\n", ip_addr_to_string(&serv_addr));
            return -1;
        }
    } while (reconnect && !gosh_exit);

    if (connected) {
        printf("connected to %s\n", ip_addr_to_string(&serv_addr));
    }

    /* Set socket nonblocking */
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    /* Setup poll parameters */
    bool disconnect = false;
    struct pollfd fds[2];
    int nfds = 2;

    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    while (!gosh_exit) {
        char buffer[256];

        poll(fds, nfds, -1);

        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents != POLLIN)
                continue;

            if (fds[i].fd == STDIN_FILENO) {
                /* Handle stdout */
                int n = read(STDIN_FILENO, buffer, sizeof(buffer));
                if (n < 1) {
                    disconnect = true;
                    break;
                }
                n = write(sockfd, buffer, n);
                if (n < 1) {
                    disconnect = true;
                    break;
                }
            } else if (fds[i].fd == sockfd) {
                /* Handle stdin */
                int n = read(sockfd, buffer, sizeof(buffer));
                if (n < 1) {
                    disconnect = true;
                    break;
                }
                n = write(STDOUT_FILENO, buffer, n);
                if (n < 1) {
                    disconnect = true;
                    break;
                }
            }
        }

        if (disconnect)
            break;
    }

    if (disconnect)
        fprintf(stderr, "Remote end disconnected\n");

    close(sockfd);

    return 0;
}

static void usage(void)
{
    printf("usage: [host] [port] [-r]\r\n");
    printf("host      hostname or IP, default is localhost\r\n");
    printf("port      IP port, default is %d\r\n", GOSH_DEFAULT_REDIRECT_PORT);
    printf("-r        reconnect\r\n");
    printf("-h -?     show help\r\n");
}

int main(int argc, char *argv[])
{
    int c, ret, remain, index;
    const char * host = "localhost";
    unsigned short port = GOSH_DEFAULT_REDIRECT_PORT;
    bool reconnect = false;

    while ((c = getopt(argc, argv, "rh?")) != -1) {
        switch (c) {
            case 'r':
                reconnect = true;
                break;
            case '?':
            case 'h':
            default:
                usage();
                return 1;
        }
    }

    remain = argc - optind;
    index = optind;

    if (remain > 2) {
        usage();
        return 1;
    }

    if (remain > 0) {
        host = argv[index];
    }
    if (remain > 1) {
        port = atoi(argv[index + 1]);
    }

    // cppcheck-suppress gethostbynameCalled
    struct hostent *server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr,"ERROR, cannot resolve IP for host: [%s]\n", host);
        return 1;
    }

    /* Put TTY in raw mode */
    signal(SIGINT, sigint_handler);
    atexit(exit_handler);
    tty_raw();

    do {
        ret = do_connection(server, port, reconnect);
    } while (ret == 0 && reconnect && !gosh_exit);

    return 0;
}
