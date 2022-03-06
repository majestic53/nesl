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
 * @file main.c
 * @brief Test application for input subsystem.
 */

#include "../../include/system/NESL_input.h"
#include "../include/NESL_common.h"

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_input_t input;                                 /*< Input context */
    bool state[NESL_CONTROLLER_MAX * NESL_BUTTON_MAX];  /*< Controller/button state */
} nesl_test_t;

static nesl_test_t g_test = {}; /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool NESL_ServiceGetButton(nesl_controller_e controller, nesl_button_e button)
{
    static const uint32_t KEY[NESL_CONTROLLER_MAX][NESL_BUTTON_MAX] = {
        { 1, 2, 3, 4, 5, 6, 7, 8, },
        { 9, 10, 11, 12, 13, 14, 15, 16, },
        };

    return g_test.state[KEY[controller][button] - 1];
}

static void NESL_TestInit(void)
{
    memset(&g_test, 0, sizeof(g_test));
}

static nesl_error_e NESL_TestInputInit(void)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_TestInit();

    if(NESL_ASSERT(NESL_InputInit(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(nesl_controller_e controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        if(NESL_ASSERT(g_test.input.button[controller].position == NESL_BUTTON_MAX)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestInputRead(void)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_TestInit();

    if(NESL_ASSERT(NESL_InputInit(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(nesl_controller_e controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(nesl_button_e button = 0; button < NESL_BUTTON_MAX; ++button) {

            if(NESL_ASSERT((NESL_InputRead(&g_test.input, 0x4016 + controller) == 0x41)
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

    NESL_InputWrite(&g_test.input, 0x4016, 1);
    NESL_InputWrite(&g_test.input, 0x4016, 0);

    for(nesl_controller_e controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(nesl_button_e button = 0; button < NESL_BUTTON_MAX; ++button) {

            if(NESL_ASSERT((NESL_InputRead(&g_test.input, 0x4016 + controller)
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

static nesl_error_e NESL_TestInputReset(void)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_TestInit();

    for(nesl_controller_e controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(nesl_button_e button = 0; button < NESL_BUTTON_MAX; ++button) {
            g_test.input.button[controller].state[button] = true;
        }

        g_test.input.button[controller].position = NESL_BUTTON_A;
    }

    NESL_InputReset(&g_test.input);

    for(nesl_controller_e controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(nesl_button_e button = 0; button < NESL_BUTTON_MAX; ++button) {

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

static nesl_error_e NESL_TestInputUninit(void)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_TestInit();

    if(NESL_ASSERT(NESL_InputInit(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_InputUninit(&g_test.input);

    for(nesl_controller_e controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(nesl_button_e button = 0; button < NESL_BUTTON_MAX; ++button) {

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

static nesl_error_e NESL_TestInputWrite(void)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_TestInit();

    if(NESL_ASSERT(NESL_InputInit(&g_test.input) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_InputWrite(&g_test.input, 0x4016, 0);

    for(nesl_controller_e controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(nesl_button_e button = 0; button < NESL_BUTTON_MAX; ++button) {

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

    NESL_InputWrite(&g_test.input, 0x4016, 1);

    for(nesl_controller_e controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(nesl_button_e button = 0; button < NESL_BUTTON_MAX; ++button) {

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

    NESL_InputWrite(&g_test.input, 0x4016, 0);

    if(NESL_ASSERT(g_test.input.strobe == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(nesl_controller_e controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(nesl_button_e button = 0; button < NESL_BUTTON_MAX; ++button) {

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

int main(void)
{
    static const NESL_Test TEST[] = {
        NESL_TestInputInit, NESL_TestInputRead, NESL_TestInputReset, NESL_TestInputUninit,
        NESL_TestInputWrite,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
