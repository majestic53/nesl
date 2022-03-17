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
 * @brief Test application for input subsystem.
 */

#include <input.h>
#include <test.h>

/*!
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_input_t input;         /*!< Input context */
    bool state[BUTTON_MAX];     /*!< Controller/button state */
    bool sensor;                /*!< Sensor state */
    bool trigger;               /*!< Trigger state */
} nesl_test_t;

static nesl_test_t g_test = {}; /*!< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool nesl_service_get_button(nesl_button_e button)
{
    static const uint32_t KEY[BUTTON_MAX] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        };

    return g_test.state[KEY[button] - 1];
}

bool nesl_service_get_sensor(void)
{
    return g_test.sensor;
}

bool nesl_service_get_trigger(void)
{
    return g_test.trigger;
}

/*!
 * @brief Initialize test context.
 */
static void nesl_test_initialize(void)
{
    memset(&g_test, 0, sizeof(g_test));
}

/*!
 * @brief Test input subsystem initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_input_initialize(void)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_test_initialize();

    if(ASSERT(nesl_input_initialize(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {

        if(ASSERT(g_test.input.controller.state[button].button == false)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(ASSERT(g_test.input.controller.position == BUTTON_MAX)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(ASSERT(!g_test.input.zapper.raw)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(ASSERT(g_test.input.strobe == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test input subsystem read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_input_read(void)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_test_initialize();

    if(ASSERT(nesl_input_initialize(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {

        if(ASSERT((nesl_input_read(&g_test.input, 0x4016) == 0x41)
                && (g_test.input.controller.state[button].button == false))) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(ASSERT(g_test.input.controller.position == BUTTON_MAX)) {
        result = NESL_FAILURE;
        goto exit;
    }

    g_test.sensor = false;
    g_test.trigger = false;

    if(ASSERT((nesl_input_read(&g_test.input, 0x4017) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    g_test.sensor = true;
    g_test.trigger = false;

    if(ASSERT((nesl_input_read(&g_test.input, 0x4017) == 0x08))) {
        result = NESL_FAILURE;
        goto exit;
    }

    g_test.sensor = false;
    g_test.trigger = true;

    if(ASSERT((nesl_input_read(&g_test.input, 0x4017) == 0x10))) {
        result = NESL_FAILURE;
        goto exit;
    }

    g_test.sensor = true;
    g_test.trigger = true;

    if(ASSERT((nesl_input_read(&g_test.input, 0x4017) == 0x18))) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int index = 0; index < BUTTON_MAX; ++index) {
        g_test.state[index] = (index & 1) ? true : false;
    }

    nesl_input_write(&g_test.input, 0x4016, 1);
    nesl_input_write(&g_test.input, 0x4016, 0);

    for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {

        if(ASSERT((nesl_input_read(&g_test.input, 0x4016) == (0x40 | (g_test.state[button] ? 1 : 0)))
                        && (g_test.input.controller.position == (button + 1)))) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(ASSERT(g_test.input.controller.position == BUTTON_MAX)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test input subsystem reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_input_reset(void)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_test_initialize();

    for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {
        g_test.input.controller.state[button].button = true;
    }

    g_test.input.controller.position = BUTTON_A;
    nesl_input_reset(&g_test.input);

    for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {

        if(ASSERT(g_test.input.controller.state[button].button == false)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(ASSERT(g_test.input.controller.position == BUTTON_MAX)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(ASSERT(!g_test.input.zapper.raw)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(ASSERT(g_test.input.strobe == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test input subsystem uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_input_uninitialize(void)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_test_initialize();

    if(ASSERT(nesl_input_initialize(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_input_uninitialize(&g_test.input);

    for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {

        if(ASSERT(g_test.input.controller.state[button].button == false)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(ASSERT(g_test.input.controller.position == 0)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(ASSERT(!g_test.input.zapper.raw)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(ASSERT(g_test.input.strobe == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test input subsystem write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_input_write(void)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_test_initialize();

    if(ASSERT(nesl_input_initialize(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_input_write(&g_test.input, 0x4016, 0);

    for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {

        if(ASSERT(g_test.input.controller.state[button].button == false)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(ASSERT(g_test.input.controller.position == BUTTON_MAX)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(ASSERT(g_test.input.strobe == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_input_write(&g_test.input, 0x4016, 1);

    for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {

        if(ASSERT(g_test.input.controller.state[button].button == false)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(ASSERT(g_test.input.controller.position == BUTTON_MAX)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(ASSERT(g_test.input.strobe == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int index = 0; index < BUTTON_MAX; ++index) {
        g_test.state[index] = (index & 1) ? true : false;
    }

    nesl_input_write(&g_test.input, 0x4016, 0);

    if(ASSERT(g_test.input.strobe == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {

        if(ASSERT(g_test.input.controller.state[button].button == g_test.state[button])) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(ASSERT(g_test.input.controller.position == BUTTON_A)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

int main(void)
{
    static const test TEST[] = {
        nesl_test_input_initialize, nesl_test_input_read, nesl_test_input_reset, nesl_test_input_uninitialize,
        nesl_test_input_write,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
