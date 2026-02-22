/**
 * @file tools/klserver.c
 *
 * @brief Implementation of a simple TCP server that accepts connections
 *        and log data
 *
 * @author J. A. Corbal <jacorbal@gmail.com>
 * @version 1.0.4
 * @date Creation date: Mon Feb  9 10:15:47 UTC 2026
 * @date Updated 1.0.1: Thu Feb 12 10:22:51 UTC 2026
 * @date Updated 1.0.2: Thu Feb 12 14:26:29 UTC 2026
 * @date Updated 1.0.3: Thu Feb 12 10:26:32 UTC 2026
 * @date Updated 1.0.4: Fri Feb 13 00:01:02 UTC 2026
 *
 * @copyright Copyright (c) 2026, J. A. Corbal.
 *            ISC License <https://opensource.org/license/isc-license-txt>
 *
 * @note Built with `gcc` 14.2.0 and `clang` 19.1.7
 */
/* Copyright (c) 2026, J. A. Corbal <jacorbal@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#define _POSIX_C_SOURCE 200809L /* getopt, optarg */


/* System includes */
#include <arpa/inet.h>  /* inet_ntop, INET_ADDRSTRLEN */
#include <signal.h>     /* sigaction, SIGCHLD, sigemptyset */
#include <stdbool.h>    /* bool, true, false */
#include <stdio.h>      /* fprintf, printf, perror, FILE, fopen, fclose */
#include <sys/socket.h> /* listen, accept, recv */
#include <sys/wait.h>   /* waitpid */
#include <unistd.h>     /* close, fork, getopt, optarg */

/* Project includes */
#include <network.h>


/* Local definitions */
#define PROG_NAME         "klserver"
#define PROG_VERSION      "1.0.4"
#define PROG_VERSION_DATE "2026-02-13"

/* 'SA_RESTART' is not defined with this '_POSIX_C_SOURCE' */
#ifndef SA_RESTART
#define SA_RESTART (0x10000000) /**< 'SA_RESTART' causes interrupted
                                      system calls to automatically
                                      restart instead of failing with
                                      'EINTR' */
#endif  /* ! SA_RESTART */
#define PORT "3491"             /**< Port number as a string for
                                     listening socket */
#define BACKLOG (15)            /**< Number of pending connections the
                                     queue will hold */
#define BUFFER_SIZE (1000)      /**< Size of buffer for receiving data */


/* Static variables */
static volatile bool loop = true;   /**< Flag controlling the main
                                         server loop; set to 'false' to
                                         terminate */


/**
 * @brief Signal handler to reap zombie processes
 *
 * @param sig Signal number (@c SIGCHLD)
 */
static void sigchld_handler(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}


/**
 * @brief Print usage information for the program
 *
 * @param arg0 Name of the executable
 */
static void fprint_usage(FILE *fp, const char *arg0)
{
    fprintf(fp,
            "Usage: %s [-h | -v]\n"
            "       %s [-q | -s] [-f <filename>]\n"
            "Options:\n"
            "   -h                Show this help message and exit\n"
            "   -v                Show version message and exit\n"
            "   -q                Quiet; suppress 'stdout', keep 'stderr'\n"
            "   -s                Silent; suppress all output\n"
            "   -f <filename>     Write output to specified file\n",
            arg0, arg0);
}


/**
 * @brief Print version information for the program
 *
 * @param arg0 Name of the executable
 */
static void fprint_version(FILE *fp, const char *arg0)
{
    (void) arg0;

    fprintf(fp,
            "%s -- version %s (%s)\n"
            "Licensed under 'ISC License';"
            " Copyright (c) 2026, J. A. Corbal\n",
            PROG_NAME, PROG_VERSION, PROG_VERSION_DATE);
}


/* Main entry */
int main(int argc, char *const argv[])
{
    int sockfd, new_fd = -1;            /* Listening & connection sockets */
    struct sigaction sa;                /* Signal action structure */
    struct sockaddr_storage their_addr; /* Client address information */
    socklen_t sin_size;                 /* Size of socket address */
    char s[INET_ADDRSTRLEN];            /* String for IP address */
    char buffer[BUFFER_SIZE];           /* Buffer for received data */
    ssize_t bytes_received;             /* Number of received bytes */
    FILE *fp = NULL;                    /* File pointer for output file */
    bool file = false;
    int option = 0;

    /* Get options */
    while ((option = getopt(argc, argv,"hvqsf:")) != -1) {
        switch (option){
            case 'h':
                fprint_usage(stdout, argv[0]);
                return 0;
            case 'v':
                fprint_version(stdout, argv[0]);
                return 0;
            case 'q':
                freopen("/dev/null", "w", stdout);
                break;
            case 's':
                freopen("/dev/null", "w", stdout);
                freopen("/dev/null", "w", stderr);
                break;
            case 'f':
                file = true;
                if ((fp = fopen(optarg, "a")) == NULL) {
                    perror("opening");
                }
                break;
            default:
                fprint_usage(stderr, argv[0]);
                return 1;
        }
    }

    sockfd = nwk_listener_socket_file_descriptor((char *) PORT);
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        return 2;
    }

    sa.sa_handler = sigchld_handler;    /* Reap dead processes */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 2;
    }

    printf("%s\n", "server: waiting for connections");

    /* Event loop */
    while (loop) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd,
                (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                &(((struct sockaddr_in *) &their_addr)->sin_addr),
                s, sizeof(s));
        printf("server: got connection from %s\n", s);

        if (!fork()) {  /* We are the child process */
            close(sockfd);

            bytes_received = recv(new_fd, buffer, sizeof(buffer), 0);
            if (bytes_received < 0) {
                perror("recv");
                return 3;
            }

            while (bytes_received > 0) {
                for (int i = 0; i < bytes_received; ++i) {
                    if (file) {
                        fprintf(fp, "%c", buffer[i]);
                    } else {
                        printf("%c", buffer[i]);
                    }
                }
                bytes_received = recv(new_fd, buffer, sizeof(buffer), 0);
            }
            if (file) {
                fflush(fp);
            }

            close(new_fd);
            return 0;
        }
    } /* ! while (loop) */

    close(new_fd);
    if (file) {
        fclose(fp);
    }

    return 0;

}
