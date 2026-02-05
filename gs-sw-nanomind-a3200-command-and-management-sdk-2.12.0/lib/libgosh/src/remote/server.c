/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#define GS_GOSH_REMOTE_INTERNAL_USE 1

#include <gs/gosh/remote/remote.h>
#include <gs/gosh/internal/remote.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <utmp.h>
#include <pty.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <gs/csp/csp.h>
#include <gs/util/string.h>
#include <gs/util/log.h>

static GS_LOG_GROUP(log_gosh_remote, "gosh-remote", GS_LOG_CAT_DEFAULT, LOG_DEFAULT_MASK);
#define LOG_DEFAULT log_gosh_remote

static pthread_t handle_remote;
static pthread_t treader, twriter;
static struct thread_args g_args;

struct thread_args {
    csp_conn_t *conn;
    unsigned int timeout;
    int fdpipe;
    pid_t child;
    pthread_t *reader;
    pthread_t *writer;
};

static int spawn(char *shell, char *command, pid_t *pd, bool noecho)
{
    pid_t pid;
    int amaster;
    char name[50];
    char command_copy[SHELL_INP_SIZE];
    char shell_option[] = "-c";
    char *argv[] = {shell, shell_option, command_copy, NULL};
    struct termios orig_termios;

    /* Create local copy of command */
    strcpy(command_copy, command);

    /* If we're not passed any command go into interactive mode */
    if (!command[0] || command[0] == '\n')
        argv[1] = NULL;

    pid = forkpty(&amaster, name, NULL, NULL);
    if (pid < 0) {
        log_error("failed to fork pty: %s", strerror(errno));
        return -errno;
    }

    if (pid == 0) {
        /* In child process */
        if (noecho) {
            /* Ensure that terminal echo is switched off so that we
             * do not get back from the spawned process the same
             * messages that we have sent it. */
            if (tcgetattr(STDOUT_FILENO, &orig_termios) < 0) {
                log_error("error getting current terminal's attributes");
                exit(EXIT_FAILURE);
            }

            orig_termios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
            orig_termios.c_oflag &= ~(ONLCR);

            if (tcsetattr(STDOUT_FILENO, TCSANOW, &orig_termios) < 0) {
                log_error("error setting current terminal's attributes");
                exit(EXIT_FAILURE);
            }
        }

        execvp(argv[0], argv);

        // only coming here, if command fails
        log_error("error spawning command: [%s]", command);
        exit(EXIT_FAILURE);

    } else {
        /* In parent process */
        log_debug("pty is at %s", name);
        *pd = pid;
    }

    return amaster;
}

static void *reader(void *params)
{
    struct thread_args *args;
    csp_packet_t *packet;
    struct gosh_sys_shell_req *req;

    args = (struct thread_args *) params;

    while (1) {
        packet = csp_read(args->conn, args->timeout);
        if (!packet) {
            log_warning("server reader timeout");
            break;
        }

        req = (struct gosh_sys_shell_req *) packet->data;
        if (req->type != GOSH_TYPE_SYS_SHELL_REQ) {
            csp_buffer_free(packet);
            break;
        }

        if (req->flags & GOSH_FLAG_DONE) {
            csp_buffer_free(packet);
            break;
        }

        if (write(args->fdpipe, req->input, strlen(req->input)) < 0) {
            csp_buffer_free(packet);
            log_warning("write failed");
            break;
        }

        csp_buffer_free(packet);
    }

    /* Kill child to force writer to exit */
    kill(args->child, SIGKILL);

    log_debug("server reader exit");

    pthread_exit(NULL);
}

static void *writer(void *params)
{
    int rd;
    struct thread_args *args;
    csp_packet_t *packet;
    struct gosh_sys_shell_rep *rep;
    char line[SHELL_OUTP_SIZE];
    bool done = false;

    args = (struct thread_args *) params;

    while (!done) {
        packet = csp_buffer_get(sizeof(*rep));
        if (!packet)
            break;

        rep = (struct gosh_sys_shell_rep *) packet->data;

        if ((rd = read(args->fdpipe, line, sizeof(line) - 1)) > 0) {
            line[rd] = 0;
            log_trace("read line [%d][%s] from process", rd, line);
            memcpy(rep->output, line, rd);
            rep->err = 0;
        } else {
            log_trace("no more bytes to read from process (read %d)", rd);
            rep->err = 1;
            done = true;
            rd = 0;
        }

        rep->type = GOSH_TYPE_SYS_SHELL_REP;
        rep->flags = 0;
        rep->output[rd] = 0;

        packet->length = shell_reply_size(rd + 1);

        if(!csp_send(args->conn, packet, args->timeout)) {
            csp_buffer_free(packet);
            log_warning("send failed");
        }
    }

    log_debug("server writer exit");

    pthread_exit(NULL);
}

static bool do_sys_shell(csp_conn_t *conn, csp_packet_t *packet)
{
    /* Read shell from environment */
    char shell[100];
    {
        const char * env_shell = getenv("SHELL");
        if (env_shell && env_shell[0]) {
            GS_STRNCPY(shell, env_shell);
        } else {
            GS_STRNCPY(shell, "/bin/sh");
        }
    }

    struct gosh_sys_shell_req * req = (struct gosh_sys_shell_req *) packet->data;
    struct gosh_sys_shell_rep * rep = (struct gosh_sys_shell_rep *) packet->data;

    char command[SHELL_INP_SIZE];
    GS_STRNCPY(command, req->input);

    log_debug("executing command [%s], using shell [%s]", command, shell);

    /* Spawn child process */
    pid_t child = 0;
    int fdpipe = spawn(shell, command, &child, !(req->flags & GOSH_FLAG_CHAR));
    if (!fdpipe) {
        log_error("spawn %s failed", shell);
        rep->type = GOSH_TYPE_SYS_SHELL_REP;
        rep->err = 1;
        rep->output[0] = '\0';

        packet->length = shell_reply_size(1);

        if(!csp_send(conn, packet, 10000))
            csp_buffer_free(packet);

        return true;
    }

    csp_buffer_free(packet);

    g_args.conn   = conn;
    g_args.fdpipe = fdpipe;
    g_args.reader = &treader;
    g_args.writer = &twriter;
    g_args.child  = child;
    g_args.timeout = 10000;

    pthread_create(&treader, NULL, reader, &g_args);
    pthread_create(&twriter, NULL, writer, &g_args);

    pthread_join(twriter, NULL);
    pthread_join(treader, NULL);

    kill(child, SIGKILL);
    waitpid(child, NULL, 0);
    close(fdpipe);

    return true;
}

static void *gosh_remote_thread(void *params)
{
    csp_conn_t *conn = params;

    log_trace("%s: starting", __FUNCTION__);

    csp_packet_t * packet;
    while ((packet = csp_read(conn, 100))) {

        uint8_t type = packet->data[0];

        log_trace("received type %u packet", type);

        bool disconnect;
        if (type == GOSH_TYPE_SYS_SHELL_REQ) {
            disconnect = do_sys_shell(conn, packet);
        } else {
            log_warning("unknown type: %u", type);
            disconnect = true;
            csp_buffer_free(packet);
        }

        if (disconnect) {
            break;
        }
    }

    csp_close(conn);

    log_trace("%s: terminating", __FUNCTION__);

    return NULL;
}

gs_error_t gs_gosh_remote_init(void)
{
    return gs_log_group_register(LOG_DEFAULT);
}

gs_error_t gs_gosh_remote_csp_service_handler(csp_conn_t *conn)
{
    pthread_attr_t attr;
    log_debug("starting GOSH remote shell");

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&handle_remote, &attr, gosh_remote_thread, conn);
    pthread_attr_destroy(&attr);

    return GS_OK;
}
