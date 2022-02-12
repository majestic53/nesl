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
#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/nesl.h"

enum {
    OPTION_FULLSCREEN = 0,
    OPTION_HELP,
    OPTION_LINEAR,
    OPTION_SCALE,
    OPTION_VERSION,
    OPTION_MAX,
};

static const char *OPTION[] = {
    "-f",
    "-h",
    "-l",
    "-s",
    "-v",
    };

static const char *DESCRIPTION[] = {
    "Set window fullscreen",
    "Show help information",
    "Set linear scaling",
    "Set window scaling",
    "Show version information",
    };

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static int nesl_read_file(nesl_t *input, char *base, char *path)
{
    FILE *file = NULL;
    int result = NESL_SUCCESS;

    if(!(file = fopen(path, "rb"))) {
        fprintf(stderr, "%s: File does not exist -- %s\n", base, path);
        result = NESL_FAILURE;
        goto exit;
    }

    fseek(file, 0, SEEK_END);

    if((input->length = ftell(file)) <= 0) {
        fprintf(stderr, "%s: File is empty -- %s\n", base, path);
        result = NESL_FAILURE;
        goto exit;
    }

    fseek(file, 0, SEEK_SET);

    if(!(input->data = calloc(input->length, sizeof(uint8_t)))) {
        fprintf(stderr, "%s: Failed to allocate buffer -- %.2f KB (%u bytes)\n", base, input->length / 1024.f, input->length);
        result = NESL_FAILURE;
        goto exit;
    }

    if(fread(input->data, sizeof(uint8_t), input->length, file) != input->length) {
        fprintf(stderr, "%s: Failed to read file -- %s\n", base, path);
        result = NESL_FAILURE;
        goto exit;
    }

    input->title = basename(path);

exit:

    if(file) {
        fclose(file);
        file = NULL;
    }

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

        for(int flag = 0; flag < OPTION_MAX; ++flag) {
            fprintf(stream, "%s\t%s\n", OPTION[flag], DESCRIPTION[flag]);
        }
    }
}

int main(int argc, char *argv[])
{
    nesl_t input = {};
    int option, result = NESL_SUCCESS;

    opterr = 1;

    while((option = getopt(argc, argv, "fhls:v")) != -1) {

        switch(option) {
            case 'f':
                input.fullscreen = true;
                break;
            case 'h':
                nesl_show_help(stdout, true);
                goto exit;
            case 'l':
                input.linear = true;
                break;
            case 's':
                input.scale = strtol(optarg, NULL, 10);
                break;
            case 'v':
                nesl_show_version(stdout, false);
                goto exit;
            case '?':
            default:
                result = NESL_FAILURE;
                goto exit;
        }
    }

    for(option = optind; option < argc; ++option) {

        if((result = nesl_read_file(&input, argv[0], argv[option])) == NESL_FAILURE) {
            goto exit;
        }

        break;
    }

    if((result = nesl(&input)) == NESL_FAILURE) {
        fprintf(stderr, "%s: %s\n", argv[0], nesl_error());
        goto exit;
    }

exit:

    if(input.data) {
        free(input.data);
        input.data = NULL;
    }

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
