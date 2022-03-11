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

/**
 * @file main.c
 * @brief Application with a CLI interface, used to launch NESL.
 */

#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <NESL.h>

/**
 * @enum nesl_bank_e
 * @brief Interface options.
 */
typedef enum {
    OPTION_FULLSCREEN = 0,  /*< Set window fullscreen */
    OPTION_HELP,            /*< Show help information */
    OPTION_LINEAR,          /*< Set linear scaling */
    OPTION_SCALE,           /*< Set window scaling */
    OPTION_VERSION,         /*< Show version information */
    OPTION_MAX,             /*< Maximum option */
} nesl_option_e;

/**
 * @brief Color tracing macro.
 * @param _RESULT_ Error code
 * @param _FORMAT_ Error string format, followed by some number of arguments
 */
#define TRACE(_RESULT_, _FORMAT_, ...) \
    fprintf(((_RESULT_) != NESL_SUCCESS) ? stderr : stdout, "%s" _FORMAT_ "\x1b[0m", \
        ((_RESULT_) != NESL_SUCCESS) ? "\x1b[91m" : "\x1b[0m", __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Read file at path.
 * @param input Pointer to NESL context
 * @param base Pointer to base path string
 * @param path Pointer to path string
 * @param NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e ReadFile(nesl_t *input, char *base, char *path)
{
    FILE *file = NULL;
    nesl_error_e result = NESL_SUCCESS;

    if(!(file = fopen(path, "rb"))) {
        TRACE(NESL_FAILURE, "%s: File does not exist -- %s\n", base, path);
        result = NESL_FAILURE;
        goto exit;
    }

    fseek(file, 0, SEEK_END);

    if((input->length = ftell(file)) <= 0) {
        TRACE(NESL_FAILURE, "%s: File is empty -- %s\n", base, path);
        result = NESL_FAILURE;
        goto exit;
    }

    fseek(file, 0, SEEK_SET);

    if(!(input->data = calloc(input->length, sizeof(uint8_t)))) {
        TRACE(NESL_FAILURE, "%s: Failed to allocate buffer -- %.2f KB (%i bytes)\n", base, input->length / 1024.f, input->length);
        result = NESL_FAILURE;
        goto exit;
    }

    if(fread(input->data, sizeof(uint8_t), input->length, file) != input->length) {
        TRACE(NESL_FAILURE, "%s: Failed to read file -- %s\n", base, path);
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

/**
 * @brief Show version string in stream.
 * @param stream File stream
 * @param verbose Verbose output
 */
static void ShowVersion(FILE *stream, bool verbose)
{
    const nesl_version_t *version = NESL_GetVersion();

    if(verbose) {
        TRACE(NESL_SUCCESS, "%s", "NESL ");
    }

    TRACE(NESL_SUCCESS, "%i.%i.%i\n", version->major, version->minor, version->patch);

    if(verbose) {
        TRACE(NESL_SUCCESS, "%s", "Copyright (C) 2022 David Jolly\n");
    }
}

/**
 * @brief Show help string in stream.
 * @param stream File stream
 * @param verbose Verbose output
 */
static void ShowHelp(FILE *stream, bool verbose)
{

    if(verbose) {
        ShowVersion(stream, true);
        TRACE(NESL_SUCCESS, "%s", "\n");
    }

    TRACE(NESL_SUCCESS, "%s", "nesl [options] file\n");

    if(verbose) {
        static const char *OPTION[] = { "-f", "-h", "-l", "-s", "-v", },
            *DESCRIPTION[] = { "Set window fullscreen", "Show help information", "Set linear scaling", "Set window scaling", "Show version information", };

        TRACE(NESL_SUCCESS, "%s", "\n");

        for(int flag = 0; flag < OPTION_MAX; ++flag) {

            TRACE(NESL_SUCCESS, "%s\t%s\n", OPTION[flag], DESCRIPTION[flag]);
        }
    }
}

int main(int argc, char *argv[])
{
    int option;
    nesl_t input = {};
    nesl_error_e result = NESL_SUCCESS;

    opterr = 1;

    while((option = getopt(argc, argv, "fhls:v")) != -1) {

        switch(option) {
            case 'f':
                input.fullscreen = true;
                break;
            case 'h':
                ShowHelp(stdout, true);
                goto exit;
            case 'l':
                input.linear = true;
                break;
            case 's':
                input.scale = strtol(optarg, NULL, 10);
                break;
            case 'v':
                ShowVersion(stdout, false);
                goto exit;
            case '?':
            default:
                result = NESL_FAILURE;
                goto exit;
        }
    }

    for(option = optind; option < argc; ++option) {

        if((result = ReadFile(&input, argv[0], argv[option])) == NESL_FAILURE) {
            goto exit;
        }
        break;
    }

    if((result = NESL_Run(&input)) == NESL_FAILURE) {
        TRACE(NESL_FAILURE, "%s: %s\n", argv[0], NESL_GetError());
        goto exit;
    }

exit:

    if(input.data) {
        free(input.data);
        input.data = NULL;
    }

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
