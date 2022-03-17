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
 * @file service.h
 * @brief Common service.
 */

#ifndef NESL_SERVICE_H_
#define NESL_SERVICE_H_

#include <common.h>

/*!
 * @enum nesl_button_e
 * @brief Button types.
 */
typedef enum {
    BUTTON_A = 0,       /*!< A button */
    BUTTON_B,           /*!< B button */
    BUTTON_SELECT,      /*!< Select button */
    BUTTON_START,       /*!< Start button */
    BUTTON_UP,          /*!< D-pad up button */
    BUTTON_DOWN,        /*!< D-pad down button */
    BUTTON_LEFT,        /*!< D-pad left button */
    BUTTON_RIGHT,       /*!< D-pad right button */
    BUTTON_MAX,         /*!< Maximum button */
} nesl_button_e;

/*!
 * @brief Audio callback routine used to collect audio samples.
 * @param[in,out] context Constant pointer to audio context
 * @param[out] data Pointer to data buffer
 * @param[in] length Data buffer length in bytes
 */
typedef void (*nesl_service_get_audio)(void *context, uint8_t *data, int length);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Get controller button state.
 * @param[in] button Button type
 * @return true if pressed, false if released
 */
bool nesl_service_get_button(nesl_button_e button);

/*!
 * @brief Get controller sensor state.
 * @return true if not-detected, false if detected
 */
bool nesl_service_get_sensor(void);

/*!
 * @brief Get controller trigger state.
 * @return true if pressed, false if released
 */
bool nesl_service_get_trigger(void);

/*!
 * @brief Initialize service.
 * @param[in] title Constant pointer to window title
 * @param[in] linear Linear scaling enabled
 * @param[in] scale Scaling value
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_service_initialize(const char *title, int linear, int scale);

/*!
 * @brief Poll service state.
 * @return NESL_FAILURE on failure, NESL_SUCCESS or NESL_QUIT otherwise
 */
nesl_error_e nesl_service_poll(void);

/*!
 * @brief Redraw service pixels to display.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_service_redraw(void);

/*!
 * @brief Reset service.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_service_reset(void);

/*!
 * @brief Set service audio callback.
 * @param[in] callback Pointer to audio callback function
 * @param[in] context Constant pointer to audio context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_service_set_audio(nesl_service_get_audio callback, void *context);

/*!
 * @brief Set service pixel.
 * @param[in] color Color value (0-63)
 * @param[in] red Emphasize red channel
 * @param[in] green Emphasize green channel
 * @param[in] blue Emphasize blue channel
 * @param[in] x X-coordinate
 * @param[in] y Y-coordinate
 */
void nesl_service_set_pixel(uint8_t color, bool red, bool green, bool blue, uint8_t x, uint8_t y);

/*!
 * @brief Uninitialize service.
 */
void nesl_service_uninitialize(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_SERVICE_H_ */
