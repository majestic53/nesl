/**
 * NESL
 * Copyright (C) 2022 David Jolly
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/nesl.h"

enum {
    FLAG_HELP = 0,
    FLAG_VERSION,
    FLAG_MAX,
};

static const char *FLAG[] = {
    "-h",
    "-v",
    };

static const char *DESCRIPTION[] = {
    "Show help information",
    "Show version information",
    };

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static int nesl_read_file(nesl_t *configuration, const char *path)
{
    int result;

    /* TODO */
    result = EXIT_SUCCESS;
    /* --- */

    return result;
}

static void nesl_show_version(FILE *stream, bool verbose)
{
    const nesl_version_t *version = nesl_version();

    if(verbose) {
        fprintf(stream, "NESL ");
    }

    fprintf(stream, "%i.%i.%i\n", version->major, version->minor, version->patch);

    if(verbose) {
        fprintf(stream, "Copyright (C) 2022 David Jolly\n");
    }
}

static void nesl_show_help(FILE *stream, bool verbose)
{

    if(verbose) {
        nesl_show_version(stream, true);
        fprintf(stream, "\n");
    }

    fprintf(stream, "nesl [options] file\n");

    if(verbose) {
        fprintf(stream, "\n");

        for(int flag = 0; flag < FLAG_MAX; ++flag) {
            fprintf(stream, "%s\t%s\n", FLAG[flag], DESCRIPTION[flag]);
        }
    }
}

int main(int argc, char *argv[])
{
    char *path = NULL;
    nesl_t configuration = {};
    int option, result = EXIT_SUCCESS;

    opterr = 1;

    while((option = getopt(argc, argv, "hv")) != -1) {

        switch(option) {
            case 'h':
                nesl_show_help(stdout, true);
                goto exit;
            case 'v':
                nesl_show_version(stdout, false);
                goto exit;
            case '?':
            default:
                result = EXIT_FAILURE;
                goto exit;
        }
    }

    for(option = optind; option < argc; ++option) {
        path = argv[option];
        break;
    }

    if((result = nesl_read_file(&configuration, path)) != EXIT_SUCCESS) {
        goto exit;
    }

    if((result = nesl(&configuration)) != EXIT_SUCCESS) {
        fprintf(stderr, "Error: %s\n", nesl_error());
        goto exit;
    }

exit:

    if(configuration.data) {
        free(configuration.data);
        configuration.data = NULL;
    }

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
