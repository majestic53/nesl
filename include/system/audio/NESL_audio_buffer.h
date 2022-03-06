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
 * @file NESL_audio_buffer.h
 * @brief Audio circular-buffer.
 */

#ifndef NESL_AUDIO_BUFFER_H_
#define NESL_AUDIO_BUFFER_H_

#include "../../NESL_common.h"

/**
 * @struct nesl_audio_buffer_t
 * @brief Audio circular-buffer context.
 */
typedef struct {
    pthread_mutex_t lock;   /*< Mutex */
    float *data;            /*< Audio data buffer */
    int length;             /*< Audio data length in bytes */
    int read;               /*< Read index */
    int write;              /*< Write index */
    bool full;              /*< Full flag */
} nesl_audio_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initialize audio buffer.
 * @param buffer Constant pointer to audio buffer context
 * @param length Max number of entries
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_AudioBufferInit(nesl_audio_buffer_t *buffer, int length);

/**
 * @brief Read bytes from audio buffer.
 * @param buffer Constant pointer to audio buffer context
 * @param data Pointer to data array
 * @param length Maximum number of entries in data array
 * @return Number of entries read
 */
int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, float *data, int length);

/**
 * @brief Readable bytes in audio buffer.
 * @param buffer Constant pointer to audio buffer context
 * @return Number of entries readable
 */
int NESL_AudioBufferReadable(nesl_audio_buffer_t *buffer);

/**
 * @brief Reset audio buffer.
 * @param buffer Constant pointer to audio buffer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_AudioBufferReset(nesl_audio_buffer_t *buffer);

/**
 * @brief Uninitialize audio buffer.
 * @param buffer Constant pointer to audio buffer context
 */
void NESL_AudioBufferUninit(nesl_audio_buffer_t *buffer);

/**
 * @brief Write bytes to audio buffer.
 * @param buffer Constant pointer to audio buffer context
 * @param data Pointer to data array
 * @param length Bumber of entries in data array
 * @return Number of entries written
 */
int NESL_AudioBufferWrite(nesl_audio_buffer_t *buffer, float *data, int length);

/**
 * @brief Writable bytes in audio buffer.
 * @param buffer Constant pointer to audio buffer context
 * @return Number of entries writable
 */
int NESL_AudioBufferWritable(nesl_audio_buffer_t *buffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_AUDIO_BUFFER_H_ */
