/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
 * @file redirect.c
 *
 * This implements redirection of stdio over a local socket.
 */

#include <gs/gosh/redirect/redirect.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <gs/util/gosh/console.h>
#include <gs/util/thread.h>
#include <gs/util/log.h>
#include <gs/util/string.h>
#include "iputil.h"

#define READ_END	0
#define WRITE_END	1

#define KEY_REDIRECT_PORT 200

typedef struct {
    gs_thread_t redirect_thread;
    bool stop_redirect_thread;
    uint16_t redirect_port;

    int pipe_stdout[2];
    int pipe_stdin[2];

    int sockfd;

    int saved_stdout;
    int saved_stdin;

} redirect_context_t;

static redirect_context_t redirect_context;

static gs_error_t redirect_setup(redirect_context_t * ctx)
{
    /* Create pipe sets */
    if (pipe(ctx->pipe_stdout) < 0) {
        log_error("%s: failed to create pipes for stdout, error: %d", __FUNCTION__, errno);
        return GS_ERROR_IO;
    }
    if (pipe(ctx->pipe_stdin) < 0) {
        log_error("%s: failed to create pipes for stdin, error: %d", __FUNCTION__, errno);
        return GS_ERROR_IO;
    }

    /* Set pipes nonblocking - stdin reads should still be blocking */
    fcntl(ctx->pipe_stdout[READ_END], F_SETFL, O_NONBLOCK);
    fcntl(ctx->pipe_stdout[WRITE_END], F_SETFL, O_NONBLOCK);
    fcntl(ctx->pipe_stdin[WRITE_END], F_SETFL, O_NONBLOCK);

    /* Create accept socket */
    ctx->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->sockfd < 0) {
        log_error("%s: failed to create socket, error: %d", __FUNCTION__, errno);
        return GS_ERROR_IO;
    }

    /* Enable port reuse */
    int optval = 1;
    setsockopt(ctx->sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    /* Set socket nonblocking */
    fcntl(ctx->sockfd, F_SETFL, O_NONBLOCK);

    /* Initialize socket structure */
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(ctx->redirect_port);

    /* Now bind the host address using bind() call.*/
    if (bind(ctx->sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        log_error("%s: failed to bind socket, error: %d", __FUNCTION__, errno);
        return GS_ERROR_IO;
    }

    /* Store current stdio for restore */
    ctx->saved_stdout = dup(STDOUT_FILENO);
    ctx->saved_stdin = dup(STDIN_FILENO);

    /* Redirect stdio, see you on the other side */
    dup2(ctx->pipe_stdout[WRITE_END], STDOUT_FILENO);
    dup2(ctx->pipe_stdin[READ_END], STDIN_FILENO);

    /* Start listening for connections */
    listen(ctx->sockfd, 5);

    return GS_OK;
}

static void * gosh_redirect_thread(void * param)
{
    redirect_context_t * ctx = param;

    /* Prepare poll events */
    struct pollfd fds[3];
    fds[0].fd = ctx->sockfd;
    fds[0].events = POLLIN;
    int nfds = 1;

    int connectionfd = -1;
    const char * disconnect = NULL;
    char buffer[1];

    while (ctx->stop_redirect_thread == false) {
        /* Wait for event on fds */
        poll(fds, nfds, 500);

        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents != POLLIN)
                continue;

            if (fds[i].fd == ctx->sockfd) {
                if (connectionfd >= 0) {
                    fprintf(stderr, "Disconnect, due to: new connection\n");
                    close(connectionfd);
                }
                struct sockaddr_in cli_addr;
                socklen_t clilen = sizeof(cli_addr);
                connectionfd = accept(ctx->sockfd, (struct sockaddr *)&cli_addr, &clilen);
                if (connectionfd < 0) {
                    fprintf(stderr, "Failed to accept incoming connection, error=%d\n", errno);
                    disconnect = "accept failed";
                    break;
                }
                fprintf(stderr, "Connection from %s accepted\n", ip_addr_to_string(&cli_addr));

                /* Set socket nonblocking */
                fcntl(connectionfd, F_SETFL, O_NONBLOCK);

                /* Add connection to poll list */
                fds[1].fd = ctx->pipe_stdout[READ_END];
                fds[1].events = POLLIN;
                fds[2].fd = connectionfd;
                fds[2].events = POLLIN;
                nfds = 3;

                /* Refresh gosh */
                gs_console_update();

                break;
            } else if (fds[i].fd == connectionfd) {
                int n = read(connectionfd, buffer, sizeof(buffer));
                if (n < 1) {
                    disconnect = "read from socket failed";
                    break;
                }
                n = write(ctx->pipe_stdin[WRITE_END], buffer, n);
                if (n < 1) {
                    disconnect = "write to 'stdin' failed";
                    break;
                }
            } else if (fds[i].fd == ctx->pipe_stdout[READ_END]) {
                int n = read(ctx->pipe_stdout[READ_END], buffer, sizeof(buffer));
                if (n < 1) {
                    disconnect = "read from 'stdout' failed";
                    break;
                }
                n = write(connectionfd, buffer, n);
                if (n < 1) {
                    disconnect = "write to socket failed";
                    break;
                }
            }
        }

        /* Remove sockets from poll on error */
        if (ctx->stop_redirect_thread) {
            disconnect = "stop request";
        }
        
        if (disconnect) {
            fprintf(stderr, "Disconnect, due to: %s\n", disconnect);
            if (connectionfd >= 0) {
                close(connectionfd);
            }
            connectionfd = -1;
            nfds = 1;
            disconnect = NULL;
        }
    }

    /* Restore stdio */
    dup2(ctx->saved_stdout, STDOUT_FILENO);
    dup2(ctx->saved_stdin, STDIN_FILENO);

    close(ctx->saved_stdout);
    close(ctx->saved_stdin);

    /* Close listen socket */
    close(ctx->sockfd);

    /* Close pipes */
    close(ctx->pipe_stdout[WRITE_END]);
    close(ctx->pipe_stdout[READ_END]);
    close(ctx->pipe_stdin[WRITE_END]);
    close(ctx->pipe_stdin[READ_END]);

    return NULL;
}

gs_error_t gs_gosh_redirect_stdio(uint16_t port)
{
    if (port == 0) {
        log_error("%s: no redirect port specified", __FUNCTION__);
        return GS_ERROR_ARG;
    }

    redirect_context.redirect_port = port;
    redirect_context.pipe_stdout[0] = -1;
    redirect_context.pipe_stdout[1] = -1;
    redirect_context.pipe_stdin[0] = -1;
    redirect_context.pipe_stdin[1] = -1;
    redirect_context.sockfd = -1;
    redirect_context.saved_stdout = -1;
    redirect_context.saved_stdin = -1;

    gs_error_t error = redirect_setup(&redirect_context);
    if (error) {
        log_error("%s: Failed to setup pipes and sockets, port %u, error: %s",
                  __FUNCTION__, redirect_context.redirect_port, gs_error_string(error));
        return error;
    }

    error = gs_thread_create("redirect", gosh_redirect_thread, &redirect_context, 0, GS_THREAD_PRIORITY_NORMAL,
                             GS_THREAD_CREATE_JOINABLE, &redirect_context.redirect_thread);
    if (error) {
        log_error("%s: Failed to create redirect thread stdio to port %u, error: %s",
                  __FUNCTION__, redirect_context.redirect_port, gs_error_string(error));
    }

    return error;
}

gs_error_t gs_gosh_redirect_stdio_stop(void)
{
    if (redirect_context.redirect_thread == 0) {
        return GS_ERROR_HANDLE;
    }

    redirect_context.stop_redirect_thread = true;
    gs_error_t error = gs_thread_join(redirect_context.redirect_thread, NULL);
    if (error == GS_OK) {
        redirect_context.redirect_thread = 0;
        redirect_context.stop_redirect_thread = false;
    }
    return error;
}

gs_error_t gs_gosh_redirect_stdio_command_line_option(void)
{
    if (redirect_context.redirect_port) {
        return gs_gosh_redirect_stdio(redirect_context.redirect_port);
    }
    return GS_ERROR_NOT_FOUND;
}

static int parser(int key, char *arg, struct argp_state *state)
{
    switch (key) {
        case KEY_REDIRECT_PORT:
            if (arg) {
                return gs_string_to_uint16(arg, &redirect_context.redirect_port);
            } else {
                redirect_context.redirect_port = GOSH_DEFAULT_REDIRECT_PORT;
            }
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static const struct argp_option options[] = {
    {
        .name = "redirect-stdio",
        .key = KEY_REDIRECT_PORT,
        .arg = "PORT",
        .flags = OPTION_ARG_OPTIONAL,
        .doc = "Redirect std in/out to a TCP/IP PORT (default: " GS_DEF2STRING(GOSH_DEFAULT_REDIRECT_PORT) ")"},
    {0}
};

static const struct argp argp = {.options = options, .parser = parser};

const struct argp_child gs_gosh_redirect_stdio_command_line_argp = {.argp = &argp};
