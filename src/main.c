/*
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

/*!
 * @file main.c
 * @brief NESL launcher application.
 */

#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <nesl.h>

/*!
 * @enum nesl_option_e
 * @brief Interface option.
 */
typedef enum {
    OPTION_HELP,            /*!< Show help information */
    OPTION_LINEAR,          /*!< Set linear scaling */
    OPTION_SCALE,           /*!< Set window scaling */
    OPTION_VERSION,         /*!< Show version information */
    OPTION_MAX,             /*!< Maximum option */
} nesl_option_e;

/*!
 * @brief Color tracing macro.
 * @param[in] _RESULT_ Error code
 * @param[in] _FORMAT_ Error string format, followed by some number of arguments
 */
#define TRACE(_RESULT_, _FORMAT_, ...) \
    fprintf(((_RESULT_) != NESL_SUCCESS) ? stderr : stdout, "%s" _FORMAT_ "\x1b[0m", \
        ((_RESULT_) != NESL_SUCCESS) ? "\x1b[91m" : "\x1b[0m", __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Read file at path.
 * @param[in] context Pointer to NESL context
 * @param[in] base Pointer to base path string
 * @param[in] path Pointer to path string
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e read_file(nesl_t *context, char *base, char *path)
{
    FILE *file = NULL;
    nesl_error_e result = NESL_SUCCESS;

    if(!(file = fopen(path, "rb"))) {
        TRACE(NESL_FAILURE, "%s: File does not exist -- %s\n", base, path);
        result = NESL_FAILURE;
        goto exit;
    }

    fseek(file, 0, SEEK_END);

    if((context->length = ftell(file)) <= 0) {
        TRACE(NESL_FAILURE, "%s: File is empty -- %s\n", base, path);
        result = NESL_FAILURE;
        goto exit;
    }

    fseek(file, 0, SEEK_SET);

    if(!(context->data = calloc(context->length, sizeof(uint8_t)))) {
        TRACE(NESL_FAILURE, "%s: Failed to allocate buffer -- %.2f KB (%i bytes)\n", base, context->length / 1024.f, context->length);
        result = NESL_FAILURE;
        goto exit;
    }

    if(fread(context->data, sizeof(uint8_t), context->length, file) != context->length) {
        TRACE(NESL_FAILURE, "%s: Failed to read file -- %s\n", base, path);
        result = NESL_FAILURE;
        goto exit;
    }

    context->title = basename(path);

exit:

    if(file) {
        fclose(file);
        file = NULL;
    }

    return result;
}

/*!
 * @brief Show version string in stream.
 * @param[in,out] stream File stream
 * @param[in] verbose Verbose output
 */
static void show_version(FILE *stream, bool verbose)
{
    const nesl_version_t *version = nesl_get_version();

    if(verbose) {
        TRACE(NESL_SUCCESS, "%s", "NESL ");
    }

    TRACE(NESL_SUCCESS, "%i.%i-%X\n", version->major, version->minor, version->patch);

    if(verbose) {
        TRACE(NESL_SUCCESS, "%s", "Copyright (C) 2022 David Jolly\n");
    }
}

/*!
 * @brief Show help string in stream.
 * @param[in,out] stream File stream
 * @param[in] verbose Verbose output
 */
static void show_help(FILE *stream, bool verbose)
{

    if(verbose) {
        show_version(stream, true);
        TRACE(NESL_SUCCESS, "%s", "\n");
    }

    TRACE(NESL_SUCCESS, "%s", "nesl [options] file\n");

    if(verbose) {
        const char *OPTION[] = { "-h", "-l", "-s", "-v", },
            *DESCRIPTION[] = { "Show help information", "Set linear scaling", "Set window scaling", "Show version information", };

        TRACE(NESL_SUCCESS, "%s", "\n");

        for(int flag = 0; flag < OPTION_MAX; ++flag) {

            TRACE(NESL_SUCCESS, "%s\t%s\n", OPTION[flag], DESCRIPTION[flag]);
        }
    }
}

int main(int argc, char *argv[])
{
    int option;
    nesl_t context = {};
    nesl_error_e result = NESL_SUCCESS;

    opterr = 1;

    while((option = getopt(argc, argv, "hls:v")) != -1) {

        switch(option) {
            case 'h':
                show_help(stdout, true);
                goto exit;
            case 'l':
                context.linear = true;
                break;
            case 's':
                context.scale = strtol(optarg, NULL, 10);
                break;
            case 'v':
                show_version(stdout, false);
                goto exit;
            case '?':
            default:
                result = NESL_FAILURE;
                goto exit;
        }
    }

    for(option = optind; option < argc; ++option) {

        if((result = read_file(&context, argv[0], argv[option])) == NESL_FAILURE) {
            goto exit;
        }
        break;
    }

    if((result = nesl(&context)) == NESL_FAILURE) {
        TRACE(NESL_FAILURE, "%s: %s\n", argv[0], nesl_get_error());
        goto exit;
    }

exit:

    if(context.data) {
        free(context.data);
        context.data = NULL;
    }

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
