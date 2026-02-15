/**
 * @file kbdfile.c
 *
 * @brief Implementation for detect input devices acting as keyboards on
 *        a Linux system
 */

#define _POSIX_C_SOURCE 200809L /* scandir, strdup */


/* System includes */
#include <dirent.h>         /* scandir, struct dirent */
#include <fcntl.h>          /* open, O_RDONLY */
#include <linux/input.h>    /* EV_KEY, EVIOCGBIT */
#include <stdint.h>         /* int32_t */
#include <stdio.h>          /* snprintf, perror */
#include <stdlib.h>         /* free */
#include <string.h>         /* snprintf, strdup */
#include <sys/ioctl.h>      /* ioctl */
#include <sys/stat.h>       /* stat, S_ISCHR */
#include <unistd.h>         /* close */

/* Local includes */
#include <kbdfile.h>


/* Local definitions */
#define INPUT_DIR "/dev/input/"


/**
 * @brief Check if a directory entry is a character device
 *
 * @param file Pointer to dirent structure representing a directory entry
 *
 * @return Status of the operation
 * @retval  1 if the entry is a character device (@c true)
 * @retval  0 otherwise (@c false)
 */
static int is_char_device(const struct dirent *file)
{
    struct stat filestat;
    char filename[512];
    int err;

    snprintf(filename, sizeof(filename), "%s%s", INPUT_DIR,
            file->d_name);

    err = stat(filename, &filestat);
    if (err) {
        return 0;   /* false */
    }

    return S_ISCHR(filestat.st_mode);
}


/* Find the path to the keyboard event device file */
char *kbd_event_file(void)
{
    char *kbd_file = NULL;
    int num;
    struct dirent **event_files;
    char filename[512];

    num = scandir(INPUT_DIR, &event_files, &is_char_device, &alphasort);
    if (num < 0) {
        return NULL;
    }

    /* Iterate through each event file, check if it acts like a keyboard */
    for (int i = 0; i < num; ++i) {
        int32_t event_bitmap = 0;
        int32_t kbd_bitmap = KEY_A | KEY_B | KEY_C | KEY_Z; 
        int fd;

        snprintf(filename, sizeof(filename), "%s%s", INPUT_DIR,
                event_files[i]->d_name);
        if ((fd = open(filename, O_RDONLY)) == -1) {
            perror("open");
            continue;
        }

        ioctl(fd, EVIOCGBIT(0, sizeof(event_bitmap)), &event_bitmap);
        if ((EV_KEY & event_bitmap) == EV_KEY) {
            /* The device acts like a keyboard */
            ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(event_bitmap)),
                    &event_bitmap);
            if ((kbd_bitmap & event_bitmap) == kbd_bitmap) {
                /* The device supports 'A', 'B', 'C', 'Z' keys, so it's
                 * probably is a keyboard */
                kbd_file = strdup(filename);
                close(fd);
                break;
            }
        }

        close(fd);
    }

    /* Cleanup scandir */
    for (int i = 0; i < num; ++i) {
        free(event_files[i]);
    }
    free(event_files);

    return kbd_file;
}
