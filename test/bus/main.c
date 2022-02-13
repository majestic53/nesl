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
#include "../../include/system/mapper.h"
#include "../include/common.h"

typedef struct {
    int type;
    uint16_t address;
    uint8_t data;

    struct {
        bool reset;
    } input;

    struct {
        int type;
        uint64_t data;
        bool interrupt;
        bool reset;
    } mapper;

    struct {
        bool reset;
    } service;
} nesl_test_t;

static nesl_test_t g_test = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int nesl_input_initialize(nesl_input_t *input)
{
    return NESL_SUCCESS;
}

uint8_t nesl_input_read(nesl_input_t *input, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

int nesl_input_reset(nesl_input_t *input)
{
    g_test.input.reset = true;

    return NESL_SUCCESS;
}

void nesl_input_uninitialize(nesl_input_t *input)
{
    return;
}

void nesl_input_write(nesl_input_t *input, uint16_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

int nesl_mapper_initialize(nesl_mapper_t *mapper, const void *data, int length)
{
    return NESL_SUCCESS;
}

int nesl_mapper_interrupt(nesl_mapper_t *mapper)
{
    g_test.mapper.interrupt = true;

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    g_test.type = type;
    g_test.address = address;

    return g_test.data;
}

int nesl_mapper_reset(nesl_mapper_t *mapper)
{
    g_test.mapper.reset = true;

    return NESL_SUCCESS;
}

void nesl_mapper_uninitialize(nesl_mapper_t *mapper)
{
    return;
}

void nesl_mapper_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    g_test.type = type;
    g_test.address = address;
    g_test.data = data;
}

int nesl_service_reset(void)
{
    g_test.service.reset = true;

    return NESL_SUCCESS;
}

static void nesl_test_initialize(void)
{
    memset(&g_test, 0, sizeof(g_test));
}

static int nesl_test_bus_interrupt(void)
{
    int result = NESL_SUCCESS;

    for(int type = 0; type < NESL_INTERRUPT_MAX; ++type) {
        nesl_test_initialize();
        nesl_bus_interrupt(type);

        switch(type) {
            case NESL_INTERRUPT_MASKABLE:
            case NESL_INTERRUPT_NON_MASKABLE:

                /*if(NESL_ASSERT((g_test.processor.interrupt == true)
                        && (g_test.processor.interrupt_maskable == (type == NESL_INTERRUPT_MASKABLE)))) {
                    result = NESL_FAILURE;
                    goto exit;
                }*/
                break;
            case NESL_INTERRUPT_RESET:

                if(NESL_ASSERT(/*(g_test.audio.reset == true)
                        && */(g_test.input.reset == true)
                        && (g_test.mapper.reset == true)
                        /*&& (g_test.processor.reset == true)*/
                        && (g_test.service.reset == true)
                        /*&& (g_test.video.reset == true)*/)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case NESL_INTERRUPT_MAPPER:

                if(NESL_ASSERT(g_test.mapper.interrupt == true)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            default:
                break;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_bus_read(void)
{
    int result = NESL_SUCCESS;

    for(int type = 0; type < NESL_BUS_MAX; ++type) {
        uint8_t data = 0;

        switch(type) {
            case NESL_BUS_PROCESSOR:

                for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {
                    nesl_test_initialize();
                    g_test.data = data;

                    switch(address) {
                        /*case 0x0000 ... 0x3FFF:
                        case 0x4015 ... 0x4017:*/
                        case 0x4016 ... 0x4017:

                            if(NESL_ASSERT((nesl_bus_read(NESL_BUS_PROCESSOR, address) == data)
                                    && (g_test.address == address))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x6000 ... 0x7FFF:

                            if(NESL_ASSERT((nesl_bus_read(NESL_BUS_PROCESSOR, address) == data)
                                    && (g_test.address == address)
                                    && (g_test.type == NESL_BANK_RAM_PROGRAM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x8000 ... 0xFFFF:

                            if(NESL_ASSERT((nesl_bus_read(NESL_BUS_PROCESSOR, address) == data)
                                    && (g_test.address == address)
                                    && (g_test.type == NESL_BANK_ROM_PROGRAM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        default:

                            if(NESL_ASSERT(nesl_bus_read(NESL_BUS_PROCESSOR, address) == 0)) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                    }
                }
                break;
            case NESL_BUS_VIDEO:

                for(uint32_t address = 0x0000; address <= 0x3FFF; ++address, ++data) {
                    nesl_test_initialize();
                    g_test.data = data;

                    switch(address & 0x3FFF) {
                        case 0x0000 ... 0x1FFF:

                            if(NESL_ASSERT((nesl_bus_read(NESL_BUS_VIDEO, address) == data)
                                    && (g_test.address == (address & 0x3FFF))
                                    && (g_test.type == NESL_BANK_ROM_CHARACTER))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        /*case 0x2000 ... 0x3FFF:

                            if(NESL_ASSERT((nesl_bus_read(NESL_BUS_VIDEO, address) == data)
                                    && (g_test.address == (address & 0x3FFF)))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;*/
                        default:

                            if(NESL_ASSERT(nesl_bus_read(NESL_BUS_VIDEO, address) == 0)) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                    }
                }
                break;
            /*case NESL_BUS_VIDEO_OAM:

                for(uint32_t address = 0x0000; address <= 0x00FF; ++address, ++data) {
                    nesl_test_initialize();
                    g_test.data = data;

                    if(NESL_ASSERT((nesl_bus_read(NESL_BUS_VIDEO_OAM, address) == data)
                            && (g_test.address == (address & 0x00FF)))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;*/
            default:
                break;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_bus_write(void)
{
    int result = NESL_SUCCESS;

    for(int type = 0; type < NESL_BUS_MAX; ++type) {
        uint8_t data = 0;

        switch(type) {
            case NESL_BUS_PROCESSOR:

                for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {
                    nesl_test_initialize();

                    switch(address) {
                        /*case 0x0000 ... 0x4008:
                        case 0x400A ... 0x400C:
                        case 0x400E ... 0x4017:*/
                        case 0x4016:
                            nesl_bus_write(NESL_BUS_PROCESSOR, address, data);

                            if(NESL_ASSERT((g_test.address == address)
                                    && (g_test.data == data))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x6000 ... 0x7FFF:
                            nesl_bus_write(NESL_BUS_PROCESSOR, address, data);

                            if(NESL_ASSERT((g_test.address == address)
                                    && (g_test.data == data)
                                    && (g_test.type == NESL_BANK_RAM_PROGRAM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x8000 ... 0xFFFF:
                            nesl_bus_write(NESL_BUS_PROCESSOR, address, data);

                            if(NESL_ASSERT((g_test.address == address)
                                    && (g_test.data == data)
                                    && (g_test.type == NESL_BANK_ROM_PROGRAM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        default:
                            nesl_bus_write(NESL_BUS_PROCESSOR, address, data);

                            if(NESL_ASSERT((g_test.address == 0)
                                    && (g_test.data == 0)
                                    && (g_test.type == 0))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                    }
                }
                break;
            case NESL_BUS_VIDEO:

                for(uint32_t address = 0x0000; address <= 0x3FFF; ++address, ++data) {
                    nesl_test_initialize();

                    switch(address & 0x3FFF) {
                        case 0x0000 ... 0x1FFF:
                            nesl_bus_write(NESL_BUS_VIDEO, address, data);

                            if(NESL_ASSERT((g_test.type == NESL_BANK_ROM_CHARACTER)
                                    && (g_test.address == (address & 0x3FFF))
                                    && (g_test.data == data))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        /*case 0x2000 ... 0x3FFF:
                            nesl_bus_write(NESL_BUS_VIDEO, address, data);

                            if(NESL_ASSERT((g_test.address == (address & 0x3FFF))
                                    && (g_test.data == data))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;*/
                        default:
                            nesl_bus_write(NESL_BUS_VIDEO, address, data);

                            if(NESL_ASSERT((g_test.type == 0)
                                    && (g_test.address == 0)
                                    && (g_test.data == 0))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                    }
                }
                break;
            /*case NESL_BUS_VIDEO_OAM:

                for(uint32_t address = 0x0000; address <= 0x00FF; ++address, ++data) {
                    nesl_test_initialize();
                    nesl_bus_write(NESL_BUS_VIDEO_OAM, address, data);

                    if(NESL_ASSERT((g_test.address == (address & 0x00FF))
                            && (g_test.data == data))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;*/
            default:
                break;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static const nesl_test TEST[] = {
    nesl_test_bus_interrupt,
    nesl_test_bus_read,
    nesl_test_bus_write,
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
