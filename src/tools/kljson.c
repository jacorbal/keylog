/**
 * @file tools/kljson.c
 *
 * @brief Convert `keylog` output to JSON format
 *
 * Each line in the input log file represents a key event and follows
 * the structure:
 *
 *      YYYY-MM-DD HH:MM:SS.mmm +/-HHMM  KEY_X - ACTION
 *
 * Where:
 *  - `YYYY-MM-DD`: The date of the event in the format of year
 *          4 digits), month (2 digits), and day (2 digits)
 *  - `HH:MM:SS.mmm`: The time of the event formatted as hours (2 digits),
 *          minutes (2 digits), seconds (2 digits), a period and the 
 *          milliseconds (3 digits)
 *  - `+/-HHMM`: The time zone offset from UTC
 *  - Two spaces
 *  - `KEY_X`: Represents the key that was pressed or released
 *  - Hyphen: Literal hyphen character that separates the key from the
 *          action, surrounded by one space character on each side
 *  - `ACTION`: Action associated with the key event, such as "pressed"
 *          or "released"
 *
 * @author J. A. Corbal <jacorbal@gmail.com>
 * @version 1.0.4
 * @date Creation date: Fri Feb 13 08:40:22 AM UTC 2026
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
#include <stdio.h>      /* fgets, fprintf, perror, FILE, fopen, fclose */
#include <stdlib.h>     /* free */
#include <unistd.h>     /* getopt, optarg */

/* JSON includes */
#include <cjson/cJSON.h>


/* Local definitions */
#define PROG_NAME         "kljson"
#define PROG_VERSION      "1.0.4"
#define PROG_VERSION_DATE "2026-02-13"


/**
 * @brief Log entry about the key event
 */
typedef struct {
    char timestamp[30]; /**< Timestamp: 'YYYY-MM-DD HH:MM:SS.mmm +/-HHMM' */
    char key[20];       /**< Key pressed or released during the event */
    char action[10];    /**< Action associated with the event */
} keylog_td;


/**
 * @brief Print usage information for the program
 *
 * @param arg0 Name of the executable
 */
static void fprint_usage(FILE *fp, const char *arg0)
{
    fprintf(fp,
            "Usage: %s [-h | -v]\n"
            "       %s -f <filename>\n"
            "Options:\n"
            "   -h                Show this help message and exit\n"
            "   -v                Show version message and exit\n"
            "   -f <filename>     File to converto to JSON\n",
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
    char *filename = NULL;
    int opt;

    /* Get options */
    while ((opt = getopt(argc, argv, "hvf:")) != -1) {
        switch (opt) {
            case 'h':
                fprint_usage(stdout, argv[0]);
                return 0;
            case 'v':
                fprint_version(stdout, argv[0]);
                return 0;
            case 'f':
                filename = optarg;
                break;
            default:
                fprint_usage(stderr, argv[0]);
                return 1;
        }
    }

    if (filename == NULL) {
        fprint_usage(stderr, argv[0]);
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    cJSON *json_logs = cJSON_CreateArray();
    char line[256];

    /* Read every line in the filename */
    while (fgets(line, sizeof(line), file)) {
        keylog_td log;

        /* Get fields */
        sscanf(line, "%29[^\n]  %[^ ] - %[^\n]",
                log.timestamp, log.key, log.action);

        /* Create JSON output */
        cJSON *json_log = cJSON_CreateObject();

        cJSON_AddStringToObject(json_log, "timestamp", log.timestamp);
        cJSON_AddStringToObject(json_log, "key", log.key);
        cJSON_AddStringToObject(json_log, "action", log.action);
        cJSON_AddItemToArray(json_logs, json_log);
    }

    fclose(file);

    /* Convert JSON to string */
    char *json_str = cJSON_Print(json_logs);
    printf("%s\n", json_str);

    cJSON_Delete(json_logs);
    free(json_str);
 
    return 0;
}
