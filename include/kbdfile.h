/**
 * @file kbdfile.h
 *
 * @brief Declaration for detect input devices acting as keyboards on
 *        a Linux system
 */

#ifndef KBDFILE_H
#define KBDFILE_H


/* Public interface */
/**
 * @brief Find the path to the keyboard event device file
 *
 * @return Pointer to a string containing the path to the keyboard event
 *         device, or @c NULL if none found
 *
 * @note Scans the @c /dev/input/ directory, checks which devices are
 *       keyboards, and returns the path to the first matching device.
 */
char *kbd_event_file(void);


#endif  /* ! KBDFILE_H */
