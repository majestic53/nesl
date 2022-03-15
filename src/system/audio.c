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
 * @file audio.c
 * @brief Audio subsystem.
 */

#include <audio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Get audio data callback.
 * @param[in,out] context Pointer to audio context
 * @param[out] data Pointer to data array
 * @param[in] length Length of data array in bytes
 */
static void nesl_audio_get_data(void *context, uint8_t *data, int length)
{
    nesl_audio_buffer_t *buffer = &((nesl_audio_t *)context)->buffer;

    memset(data, 0, length);

    if(nesl_audio_buffer_readable(buffer) >= 512) {
        nesl_audio_buffer_read(buffer, (float *)data, length / sizeof(float));
    }
}

/*!
 * @brief Get audio status register.
 * @param[in] audio Pointer to audio subsystem context
 * @return Audio status data
 */
static uint8_t nesl_audio_get_status(nesl_audio_t *audio)
{
    nesl_audio_status_t result = {};

    /* TODO: GET STATUS */

    return result.raw;
}

/*!
 * @brief Set audio frame register.
 * @param[in] audio Pointer to audio subsystem context
 * @param[in] data Audio frame data
 */
static void nesl_audio_set_frame(nesl_audio_t *audio, uint8_t data)
{
    audio->frame.raw = data;

    /* TODO: SET FRAME */
}

/*!
 * @brief Set audio status register.
 * @param[in] audio Pointer to audio subsystem context
 * @param[in] data Audio status data
 */
static void nesl_audio_set_status(nesl_audio_t *audio, uint8_t data)
{
    audio->status.raw = data;

    /* TODO: SET STATUS */
}

void nesl_audio_cycle(nesl_audio_t *audio, uint64_t cycle)
{

    if(!(cycle % 6)) {
        float elapsed = (cycle / (float)89342) / (float)60;

        nesl_audio_square_cycle(&audio->synthesizer.square[SYNTHESIZER_SQUARE_1], elapsed);
        nesl_audio_square_cycle(&audio->synthesizer.square[SYNTHESIZER_SQUARE_2], elapsed);
        nesl_audio_triangle_cycle(&audio->synthesizer.triangle, elapsed);
        nesl_audio_noise_cycle(&audio->synthesizer.noise, elapsed);
        nesl_audio_dmc_cycle(&audio->synthesizer.dmc, elapsed);

        if((nesl_audio_square_readable(&audio->synthesizer.square[SYNTHESIZER_SQUARE_1]) >= 128)
                || (nesl_audio_square_readable(&audio->synthesizer.square[SYNTHESIZER_SQUARE_2]) >= 128)
                || (nesl_audio_triangle_readable(&audio->synthesizer.triangle) >= 128)
                || (nesl_audio_noise_readable(&audio->synthesizer.noise) >= 128)
                || (nesl_audio_dmc_readable(&audio->synthesizer.dmc) >= 128)) {
            float data[SYNTHESIZER_MAX + 1][128] = {};

            nesl_audio_square_read(&audio->synthesizer.square[SYNTHESIZER_SQUARE_1], data[SYNTHESIZER_SQUARE_1], 128);
            nesl_audio_square_read(&audio->synthesizer.square[SYNTHESIZER_SQUARE_2], data[SYNTHESIZER_SQUARE_2], 128);
            nesl_audio_triangle_read(&audio->synthesizer.triangle, data[SYNTHESIZER_TRIANGLE], 128);
            nesl_audio_noise_read(&audio->synthesizer.noise, data[SYNTHESIZER_NOISE], 128);
            nesl_audio_dmc_read(&audio->synthesizer.dmc, data[SYNTHESIZER_DMC], 128);

            for(int sample = 0; sample < 128; ++sample) {

                for(nesl_synthesizer_e channel = SYNTHESIZER_SQUARE_1; channel < SYNTHESIZER_MAX; ++channel) {

                    if(data[channel][sample]) {

                        /* TODO: ASSIGN CHANNEL VOLUME */
                        data[SYNTHESIZER_MAX][sample] += data[channel][sample] * 1.f;
                        /* --- */
                    }
                }

                /* TODO: ASSIGN MASTER VOLUME */
                data[SYNTHESIZER_MAX][sample] *= 1.f;
                /* --- */

                if(data[SYNTHESIZER_MAX][sample] < -1.f) {
                    data[SYNTHESIZER_MAX][sample] = -1.f;
                } else if(data[SYNTHESIZER_MAX][sample] > 1.f) {
                    data[SYNTHESIZER_MAX][sample] = 1.f;
                }
            }

            nesl_audio_buffer_write(&audio->buffer, data[SYNTHESIZER_MAX], 128);
        }


    }
}

nesl_error_e nesl_audio_initialize(nesl_audio_t *audio)
{
    nesl_error_e result;

    if((result = nesl_audio_buffer_initialize(&audio->buffer, 1024)) == NESL_FAILURE) {
        goto exit;
    }

    for(nesl_synthesizer_e channel = SYNTHESIZER_SQUARE_1; channel <= SYNTHESIZER_SQUARE_2; ++channel) {

        if((result = nesl_audio_square_initialize(&audio->synthesizer.square[channel])) == NESL_FAILURE) {
            goto exit;
        }
    }

    if((result = nesl_audio_triangle_initialize(&audio->synthesizer.triangle)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_audio_noise_initialize(&audio->synthesizer.noise)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_audio_dmc_initialize(&audio->synthesizer.dmc)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_audio_reset(audio)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

uint8_t nesl_audio_read(nesl_audio_t *audio, uint16_t address)
{
    uint8_t result = 0;

    switch(address) {
        case 0x4015:
            result = nesl_audio_get_status(audio);
            break;
        default:
            break;
    }

    return result;
}

nesl_error_e nesl_audio_reset(nesl_audio_t *audio)
{
    nesl_error_e result;

    audio->frame.raw = 0;
    audio->status.raw = 0;

    for(nesl_synthesizer_e channel = SYNTHESIZER_SQUARE_1; channel <= SYNTHESIZER_SQUARE_2; ++channel) {

        if((result = nesl_audio_square_reset(&audio->synthesizer.square[channel])) == NESL_FAILURE) {
            goto exit;
        }
    }

    if((result = nesl_audio_triangle_reset(&audio->synthesizer.triangle)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_audio_noise_reset(&audio->synthesizer.noise)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_audio_dmc_reset(&audio->synthesizer.dmc)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_audio_buffer_reset(&audio->buffer)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_service_set_audio(nesl_audio_get_data, audio)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

void nesl_audio_uninitialize(nesl_audio_t *audio)
{
    nesl_audio_dmc_uninitialize(&audio->synthesizer.dmc);
    nesl_audio_noise_uninitialize(&audio->synthesizer.noise);
    nesl_audio_triangle_uninitialize(&audio->synthesizer.triangle);

    for(nesl_synthesizer_e channel = SYNTHESIZER_SQUARE_1; channel <= SYNTHESIZER_SQUARE_2; ++channel) {
        nesl_audio_square_uninitialize(&audio->synthesizer.square[channel]);
    }

    nesl_audio_buffer_uninitialize(&audio->buffer);
    memset(audio, 0, sizeof(*audio));
}

void nesl_audio_write(nesl_audio_t *audio, uint16_t address, uint8_t data)
{
    nesl_synthesizer_e channel;

    switch(address) {
        case 0x4000 ... 0x4007:

            if((channel = (address > 0x4003) ? SYNTHESIZER_SQUARE_2 : SYNTHESIZER_SQUARE_1) == SYNTHESIZER_SQUARE_2) {
                address -= 4;
            }

            nesl_audio_square_write(&audio->synthesizer.square[channel], address, data);
            break;
        case 0x4008 ... 0x400B:
            nesl_audio_triangle_write(&audio->synthesizer.triangle, address, data);
            break;
        case 0x400C ... 0x400F:
            nesl_audio_noise_write(&audio->synthesizer.noise, address, data);
            break;
        case 0x4010 ... 0x4013:
            nesl_audio_dmc_write(&audio->synthesizer.dmc, address, data);
            break;
        case 0x4015:
            nesl_audio_set_status(audio, data);
            break;
        case 0x4017:
            nesl_audio_set_frame(audio, data);
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
