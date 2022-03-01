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

#ifndef NESL_AUDIO_BUFFER_H_
#define NESL_AUDIO_BUFFER_H_

#include "../../NESL_common.h"

typedef struct {
    int8_t *data;
    int length;
    int read;
    int write;
    bool full;
} nesl_audio_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_AudioBufferInit(nesl_audio_buffer_t *buffer, int length);
int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, int8_t *data, int length);
int NESL_AudioBufferReset(nesl_audio_buffer_t *buffer);
void NESL_AudioBufferUninit(nesl_audio_buffer_t *buffer);
int NESL_AudioBufferWrite(nesl_audio_buffer_t *buffer, int8_t *data, int length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_AUDIO_BUFFER_H_ */
