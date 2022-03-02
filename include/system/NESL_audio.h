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

#ifndef NESL_AUDIO_H_
#define NESL_AUDIO_H_

#include "./audio/NESL_audio_buffer.h"
#include "../NESL_bus.h"

typedef union {

    struct {

        union {

            struct {
                uint8_t index : 4;
                uint8_t unused : 2;
                uint8_t loop : 1;
                uint8_t interrupt_enable : 1;
            };

            uint8_t raw;
        } control;

        union {

            struct {
                uint8_t counter : 7;
            };

            uint8_t raw;
        } load;

        uint8_t address;
        uint8_t length;
    };

    uint8_t byte[4];
} nesl_audio_dmc_t;

typedef union {

    struct {
        uint8_t unused : 6;
        uint8_t interrupt_disable : 1;
        uint8_t mode : 1;
    };

    uint8_t raw;
} nesl_audio_frame_t;

typedef union {

    struct {

        union {

            struct {
                uint8_t volume : 4;
                uint8_t volume_const : 1;
                uint8_t loop : 1;
            };

            uint8_t raw;
        } envelope;

        uint8_t unused;

        union {

            struct {
                uint8_t index : 4;
                uint8_t unused : 3;
                uint8_t loop : 1;
            };

            uint8_t raw;
        } period;

        union {

            struct {
                uint8_t unused : 3;
                uint8_t index : 5;
            };

            uint8_t raw;
        } length;
    };

    uint8_t byte[4];
} nesl_audio_noise_t;

typedef union {

    struct {

        union {

            struct {
                uint8_t volume : 4;
                uint8_t volume_const : 1;
                uint8_t loop : 1;
                uint8_t duty : 2;
            };

            uint8_t raw;
        } envelope;

        union {

            struct {
                uint8_t shift : 3;
                uint8_t negative : 1;
                uint8_t period : 3;
                uint8_t enable : 1;
            };

            uint8_t raw;
        } sweep;

        uint8_t period_low;

        union {

            struct {
                uint8_t period_high : 3;
                uint8_t counter : 5;
            };

            uint8_t raw;
        } length;
    };

    uint8_t byte[4];
} nesl_audio_square_t;

typedef union {

    struct {
        uint8_t square_0 : 1;
        uint8_t square_1 : 1;
        uint8_t triangle : 1;
        uint8_t noise : 1;
        uint8_t dmc : 1;
        uint8_t unused : 1;
        uint8_t frame_interrupt : 1;
        uint8_t dmc_interrupt : 1;
    };

    uint8_t raw;
} nesl_audio_status_t;

typedef union {

    struct {

        union {

            struct {
                uint8_t counter : 7;
                uint8_t control : 1;
            };

            uint8_t raw;
        } control;

        uint8_t unused;
        uint8_t period_low;

        union {

            struct {
                uint8_t period_high : 3;
                uint8_t counter : 5;
            };

            uint8_t raw;
        } length;
    };

    uint8_t byte[4];
} nesl_audio_triangle_t;

typedef struct {
    nesl_audio_buffer_t buffer;
    nesl_audio_frame_t frame;
    nesl_audio_status_t status;

    struct {
        nesl_audio_square_t square[2];
        nesl_audio_triangle_t triangle;
        nesl_audio_noise_t noise;
        nesl_audio_dmc_t dmc;
    } synthesizer;
} nesl_audio_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void NESL_AudioCycle(nesl_audio_t *audio, uint64_t cycle);
int NESL_AudioInit(nesl_audio_t *audio);
uint8_t NESL_AudioRead(nesl_audio_t *audio, uint16_t address);
int NESL_AudioReset(nesl_audio_t *audio);
void NESL_AudioUninit(nesl_audio_t *audio);
void NESL_AudioWrite(nesl_audio_t *audio, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_AUDIO_H_ */
