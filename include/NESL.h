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
 * @file NESL.h
 * @brief NESL API.
 */

#ifndef NESL_H_
#define NESL_H_

/**
 * NESL API version
 */
#define NESL_API_VERSION_1 1
#define NESL_API_VERSION NESL_API_VERSION_1

/**
 * @enum nesl_error_e
 * @brief NESL Error codes.
 */
typedef enum {
    NESL_FAILURE = -1,  /*< Operation failed, call NESL_GetError */
    NESL_SUCCESS,       /*< Operation succeeded */
    NESL_QUIT,          /*< Internal event, assume operation succeeded */
} nesl_error_e;

/**
 * @struct nes_t
 * @brief NESL context struct, allocated by the caller and passed into NESL_Run.
 */
typedef struct {
    void *data;         /*< Data */
    int length;         /*< Data length in bytes */
    char *title;        /*< Window title (can be NULL) */
    int fullscreen;     /*< Window fullscreen (default:false) */
    int linear;         /*< Window linear scaling (default:false) */
    int scale;          /*< Window scaling [1-4] (default:1) */
} nesl_t;

/**
 * @struct nesl_version_t
 * @brief NESL version struct, returned from NESL_GetVersion.
 */
typedef struct {
    int major;          /*< Major version */
    int minor;          /*< Minor version */
    int patch;          /*< Patch version */
} nesl_version_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Run NESL with a caller defined context.
 * @param context Constant pointer to caller defined context struct
 * @return NESL_FAILURE on failure, NESL_SUCCESS or NESL_QUIT otherwise
 */
nesl_error_e NESL_Run(const nesl_t *context);

/**
 * @brief Get NESL error string.
 * @return Constant pointer to NESL error string
 */
const char *NESL_GetError(void);

/**
 * @brief Get NESL version struct.
 * @return Constant pointer to NESL version struct
 */
const nesl_version_t *NESL_GetVersion(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_H_ */
