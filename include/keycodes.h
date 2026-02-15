/**
 * @file keycodes.h
 *
 * @brief Array of keycode string representations
 */

#ifndef KEYCODES_H
#define KEYCODES_H


/* Local definitions */
#define NUM_KEYCODES (249)      /**< Number of keycodes supported and
                                     used in the keycode array */


/**
 * @brief Array of keycode strings, indexed by key code numbers from
 *        input events
 */
/* See '/usr/include/linux/input-event-codes.h' */
extern const char *kl_keycodes[];


#endif  /* ! KEYCODES_H */
