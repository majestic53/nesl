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
 * @file NESL_service.h
 * @brief Common service used by subsystems for input/output.
 */

#ifndef NESL_SERVICE_H_
#define NESL_SERVICE_H_

#include "./NESL_common.h"

/**
 * @brief Audio callback routine used to collect audio samples.
 * @param context Constant pointer to audio context
 * @param data Pointer to data buffer
 * @param length Data buffer length in bytes
 */
typedef void (*NESL_ServiceGetAudio)(void *context, uint8_t *data, int length);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Get controller button state.
 * @param controller Controller type
 * @param button Button type
 * @return true if pressed, false if released
 */
bool NESL_ServiceGetButton(nesl_controller_e controller, nesl_button_e button);

/**
 * @brief Initialize service.
 * @param title Constant pointer to window title
 * @param fullscreen Fullscreen enabled
 * @param linear Linear scaling enabled
 * @param scale Scaling value
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_ServiceInit(const char *title, int fullscreen, int linear, int scale);

/**
 * @brief Poll service state.
 * @return NESL_FAILURE on failure, NESL_SUCCESS or NESL_QUIT otherwise
 */
nesl_error_e NESL_ServicePoll(void);

/**
 * @brief Redraw service pixels to display.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_ServiceRedraw(void);

/**
 * @brief Reset service.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_ServiceReset(void);

/**
 * @brief Set service audio callback.
 * @param callback Pointer to audio callback function
 * @param context Constant pointer to audio context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_ServiceSetAudio(NESL_ServiceGetAudio callback, void *context);

/**
 * @brief Set service pixel.
 * @param color Color value (0-63)
 * @param red Emphasize red channel
 * @param green Emphasize green channel
 * @param blue Emphasize blue channel
 * @param x X-coordinate
 * @param y Y-coordinate
 */
void NESL_ServiceSetPixel(uint8_t color, bool red, bool green, bool blue, uint8_t x, uint8_t y);

/**
 * @brief Uninitialize service.
 */
void NESL_ServiceUninit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_SERVICE_H_ */
