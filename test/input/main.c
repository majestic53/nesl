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

#include "../../include/system/input.h"
#include "../include/common.h"

static uint32_t KEY[NESL_CONTROLLER_MAX][NESL_BUTTON_MAX] = {
    { 1, 2, 3, 4, 5, 6, 7, 8, },
    { 9, 10, 11, 12, 13, 14, 15, 16, },
};

typedef struct {
    nesl_input_t input;
    bool state[NESL_CONTROLLER_MAX * NESL_BUTTON_MAX];
} nesl_test_t;

static nesl_test_t g_test = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool nesl_service_button(int controller, int button)
{
    return g_test.state[KEY[controller][button] - 1];
}

static void nesl_test_initialize(void)
{
    memset(&g_test, 0, sizeof(g_test));
}

static int nesl_test_input_initialize(void)
{
    int result = NESL_SUCCESS;

    nesl_test_initialize();

    if(NESL_ASSERT(nesl_input_initialize(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        if(NESL_ASSERT(g_test.input.button[controller].position == NESL_BUTTON_MAX)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_input_read(void)
{
    int result = NESL_SUCCESS;

    nesl_test_initialize();

    if(NESL_ASSERT(nesl_input_initialize(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(int button = 0; button < NESL_BUTTON_MAX; ++button) {

            if(NESL_ASSERT((nesl_input_read(&g_test.input, 0x4016 + controller) == 0x41)
                    && (g_test.input.button[controller].state[button] == false))) {
                result = NESL_FAILURE;
                goto exit;
            }
        }

        if(NESL_ASSERT(g_test.input.button[controller].position == NESL_BUTTON_MAX)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    for(int index = 0; index < NESL_CONTROLLER_MAX * NESL_BUTTON_MAX; ++index) {
        g_test.state[index] = (index & 1) ? true : false;
    }

    nesl_input_write(&g_test.input, 0x4016, 1);
    nesl_input_write(&g_test.input, 0x4016, 0);

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(int button = 0; button < NESL_BUTTON_MAX; ++button) {

            if(NESL_ASSERT((nesl_input_read(&g_test.input, 0x4016 + controller)
                    == (0x40 | (g_test.state[(NESL_BUTTON_MAX * controller) + button] ? 1 : 0)))
                            && (g_test.input.button[controller].position == (button + 1)))) {
                result = NESL_FAILURE;
                goto exit;
            }
        }

        if(NESL_ASSERT(g_test.input.button[controller].position == NESL_BUTTON_MAX)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_input_reset(void)
{
    int result = NESL_SUCCESS;

    nesl_test_initialize();

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(int button = 0; button < NESL_BUTTON_MAX; ++button) {
            g_test.input.button[controller].state[button] = true;
        }

        g_test.input.button[controller].position = NESL_BUTTON_A;
    }

    nesl_input_reset(&g_test.input);

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(int button = 0; button < NESL_BUTTON_MAX; ++button) {

            if(NESL_ASSERT(g_test.input.button[controller].state[button] == false)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }

        if(NESL_ASSERT(g_test.input.button[controller].position == NESL_BUTTON_MAX)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(NESL_ASSERT(g_test.input.strobe == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}
static int nesl_test_input_uninitialize(void)
{
    int result = NESL_SUCCESS;

    nesl_test_initialize();

    if(NESL_ASSERT(nesl_input_initialize(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_input_uninitialize(&g_test.input);

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(int button = 0; button < NESL_BUTTON_MAX; ++button) {

            if(NESL_ASSERT(g_test.input.button[controller].state[button] == false)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }

        if(NESL_ASSERT(g_test.input.button[controller].position == 0)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(NESL_ASSERT(g_test.input.strobe == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_input_write(void)
{
    int result = NESL_SUCCESS;

    nesl_test_initialize();

    if(NESL_ASSERT(nesl_input_initialize(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_input_write(&g_test.input, 0x4016, 0);

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(int button = 0; button < NESL_BUTTON_MAX; ++button) {

            if(NESL_ASSERT(g_test.input.button[controller].state[button] == false)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }

        if(NESL_ASSERT(g_test.input.button[controller].position == NESL_BUTTON_MAX)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(NESL_ASSERT(g_test.input.strobe == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_input_write(&g_test.input, 0x4016, 1);

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(int button = 0; button < NESL_BUTTON_MAX; ++button) {

            if(NESL_ASSERT(g_test.input.button[controller].state[button] == false)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }

        if(NESL_ASSERT(g_test.input.button[controller].position == NESL_BUTTON_MAX)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(NESL_ASSERT(g_test.input.strobe == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int index = 0; index < NESL_CONTROLLER_MAX * NESL_BUTTON_MAX; ++index) {
        g_test.state[index] = (index & 1) ? true : false;
    }

    nesl_input_write(&g_test.input, 0x4016, 0);

    if(NESL_ASSERT(g_test.input.strobe == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(int button = 0; button < NESL_BUTTON_MAX; ++button) {

            if(NESL_ASSERT(g_test.input.button[controller].state[button] == g_test.state[(NESL_BUTTON_MAX * controller) + button])) {
                result = NESL_FAILURE;
                goto exit;
            }
        }

        if(NESL_ASSERT(g_test.input.button[controller].position == NESL_BUTTON_A)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static const nesl_test TEST[] = {
    nesl_test_input_initialize,
    nesl_test_input_read,
    nesl_test_input_reset,
    nesl_test_input_uninitialize,
    nesl_test_input_write,
    };

int main(void)
{
    int result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(TEST[index]() != NESL_SUCCESS) {
            result = NESL_FAILURE;
        }
    }

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
