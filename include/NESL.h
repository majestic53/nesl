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

#ifndef NESL_H_
#define NESL_H_

enum {
    NESL_FAILURE = -1,
    NESL_SUCCESS,
    NESL_QUIT,
};

typedef struct {
    void *data;
    int length;
    char *title;
    int fullscreen;
    int linear;
    int scale;
} nesl_t;

typedef struct {
    int major;
    int minor;
    int patch;
} nesl_version_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_Run(const nesl_t *);
const char *NESL_GetError(void);
const nesl_version_t *NESL_GetVersion(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_H_ */