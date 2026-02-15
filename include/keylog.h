/**
 * @file keylog.h
 *
 * @brief Declaration of keylogger functions to capture keystrokes and
 *        write them to a file
 */

#ifndef KEYLOGGER_H
#define KEYLOGGER_H


/* System includes */
#include <stdbool.h>    /* bool */


/* Local definitions */
#define NUM_EVENTS (128)    /**< Number of input events processed per
                                 read cycle */


/* Public interface */
/**
 * @brief Capture keystrokes by reading from the keyboard resource
 *
 * Captures key press and writing to the output file, and continues
 * reading until @c SIGINT is recieved.
 *
 * @param kbd       File descriptor for the keyboard device
 * @param output_fd File descriptor to write keystrokes out to
 * @param timestamp Whether to prepend the timestamp to the keystroke
 *
 * @note A newline is appended to the end of the file.
 */
void keylogger_start(int kbd, int output_fd, bool timestamp);

/**
 * @brief Stop the keylogger
 */
void keylogger_stop(void);


#endif  /* ! KEYLOGGER_H */
