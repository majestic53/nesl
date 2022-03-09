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
 * @brief Test application for bus.
 */

#include "../../include/system/NESL_audio.h"
#include "../../include/system/NESL_input.h"
#include "../../include/system/NESL_mapper.h"
#include "../../include/system/NESL_processor.h"
#include "../../include/system/NESL_video.h"
#include "../include/NESL_common.h"

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_bank_e type;               /*< Bank type */
    uint16_t address;               /*< Bank address */
    uint8_t data;                   /*< Bank data */

    struct {
        bool reset;                 /*< Reset state */
    } service;

    struct {

        struct {
            bool reset;                 /*< Reset state */
        } audio;

        struct {
            bool reset;                 /*< Reset state */
        } input;

        struct {
            bool interrupt;             /*< Interrupt state */
            bool reset;                 /*< Reset state */
        } mapper;

        struct {
            bool interrupt;             /*< Interrupt state */
            bool interrupt_maskable;    /*< Maskable interrupt state */
            bool reset;                 /*< Reset state */
        } processor;

        struct {
            bool reset;                 /*< Reset state */
        } video;
    } subsystem;
} nesl_test_t;

static nesl_test_t g_test = {};     /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void NESL_AudioCycle(nesl_audio_t *audio, uint64_t cycle)
{
    return;
}

nesl_error_e NESL_AudioInit(nesl_audio_t *audio)
{
    return NESL_SUCCESS;
}

uint8_t NESL_AudioRead(nesl_audio_t *audio, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

nesl_error_e NESL_AudioReset(nesl_audio_t *audio)
{
    g_test.subsystem.audio.reset = true;

    return NESL_SUCCESS;
}

void NESL_AudioUninit(nesl_audio_t *audio)
{
    return;
}

void NESL_AudioWrite(nesl_audio_t *audio, uint16_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

nesl_error_e NESL_InputInit(nesl_input_t *input)
{
    return NESL_SUCCESS;
}

uint8_t NESL_InputRead(nesl_input_t *input, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

nesl_error_e NESL_InputReset(nesl_input_t *input)
{
    g_test.subsystem.input.reset = true;

    return NESL_SUCCESS;
}

void NESL_InputUninit(nesl_input_t *input)
{
    return;
}

void NESL_InputWrite(nesl_input_t *input, uint16_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

nesl_error_e NESL_MapperInit(nesl_mapper_t *mapper, const void *data, int length)
{
    return NESL_SUCCESS;
}

nesl_error_e NESL_MapperInterrupt(nesl_mapper_t *mapper)
{
    g_test.subsystem.mapper.interrupt = true;

    return NESL_SUCCESS;
}

uint8_t NESL_MapperRead(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    g_test.type = type;
    g_test.address = address;

    return g_test.data;
}

nesl_error_e NESL_MapperReset(nesl_mapper_t *mapper)
{
    g_test.subsystem.mapper.reset = true;

    return NESL_SUCCESS;
}

void NESL_MapperUninit(nesl_mapper_t *mapper)
{
    return;
}

void NESL_MapperWrite(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{
    g_test.type = type;
    g_test.address = address;
    g_test.data = data;
}

void NESL_ProcessorCycle(nesl_processor_t *processor, uint64_t cycle)
{
    return;
}

nesl_error_e NESL_ProcessorInit(nesl_processor_t *processor)
{
    return NESL_SUCCESS;
}

nesl_error_e NESL_ProcessorInterrupt(nesl_processor_t *processor, bool maskable)
{
    g_test.subsystem.processor.interrupt = true;
    g_test.subsystem.processor.interrupt_maskable = maskable;

    return NESL_SUCCESS;
}

uint8_t NESL_ProcessorRead(nesl_processor_t *processor, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

nesl_error_e NESL_ProcessorReset(nesl_processor_t *processor)
{
    g_test.subsystem.processor.reset = true;

    return NESL_SUCCESS;
}

void NESL_ProcessorUninit(nesl_processor_t *processor)
{
    return;
}

void NESL_ProcessorWrite(nesl_processor_t *processor, uint16_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

nesl_error_e NESL_ServiceReset(void)
{
    g_test.service.reset = true;

    return NESL_SUCCESS;
}

bool NESL_VideoCycle(nesl_video_t *video)
{
    return true;
}

nesl_error_e NESL_VideoInit(nesl_video_t *video, const nesl_mirror_e *mirror)
{
    return NESL_SUCCESS;
}

uint8_t NESL_VideoRead(nesl_video_t *video, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

uint8_t NESL_VideoReadOam(nesl_video_t *video, uint8_t address)
{
    g_test.address = address;

    return g_test.data;
}

uint8_t NESL_VideoReadPort(nesl_video_t *video, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

nesl_error_e NESL_VideoReset(nesl_video_t *video, const nesl_mirror_e *mirror)
{
    g_test.subsystem.video.reset = true;

    return NESL_SUCCESS;
}

void NESL_VideoUninit(nesl_video_t *video)
{
    return;
}

void NESL_VideoWrite(nesl_video_t *video, uint16_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

void NESL_VideoWriteOam(nesl_video_t *video, uint8_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

void NESL_VideoWritePort(nesl_video_t *video, uint16_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

/**
 * @brief Initialize test context.
 */
static void NESL_TestInit(void)
{
    memset(&g_test, 0, sizeof(g_test));
}

/**
 * @brief Test bus interrupt.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestBusInterrupt(void)
{
    nesl_error_e result = NESL_SUCCESS;

    for(nesl_interrupt_e type = 0; type < NESL_INTERRUPT_MAX; ++type) {
        NESL_TestInit();
        NESL_BusInterrupt(type);

        switch(type) {
            case NESL_INTERRUPT_MASKABLE:
            case NESL_INTERRUPT_NON_MASKABLE:

                if(NESL_ASSERT((g_test.subsystem.processor.interrupt == true)
                        && (g_test.subsystem.processor.interrupt_maskable == (type == NESL_INTERRUPT_MASKABLE)))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case NESL_INTERRUPT_RESET:

                if(NESL_ASSERT((g_test.service.reset == true)
                        && (g_test.subsystem.audio.reset == true)
                        && (g_test.subsystem.input.reset == true)
                        && (g_test.subsystem.mapper.reset == true)
                        && (g_test.subsystem.processor.reset == true)
                        && (g_test.subsystem.video.reset == true))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case NESL_INTERRUPT_MAPPER:

                if(NESL_ASSERT(g_test.subsystem.mapper.interrupt == true)) {
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

/**
 * @brief Test bus read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestBusRead(void)
{
    nesl_error_e result = NESL_SUCCESS;

    for(nesl_bus_e type = 0; type < NESL_BUS_MAX; ++type) {
        uint8_t data = 0;

        switch(type) {
            case NESL_BUS_PROCESSOR:

                for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {
                    NESL_TestInit();
                    g_test.data = data;

                    switch(address) {
                        case 0x0000 ... 0x3FFF:
                        case 0x4015 ... 0x4017:

                            if(NESL_ASSERT((NESL_BusRead(NESL_BUS_PROCESSOR, address) == data)
                                    && (g_test.address == address))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x6000 ... 0x7FFF:

                            if(NESL_ASSERT((NESL_BusRead(NESL_BUS_PROCESSOR, address) == data)
                                    && (g_test.address == address)
                                    && (g_test.type == NESL_BANK_PROGRAM_RAM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x8000 ... 0xFFFF:

                            if(NESL_ASSERT((NESL_BusRead(NESL_BUS_PROCESSOR, address) == data)
                                    && (g_test.address == address)
                                    && (g_test.type == NESL_BANK_PROGRAM_ROM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        default:

                            if(NESL_ASSERT(NESL_BusRead(NESL_BUS_PROCESSOR, address) == 0)) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                    }
                }
                break;
            case NESL_BUS_VIDEO:

                for(uint32_t address = 0x0000; address <= 0x3FFF; ++address, ++data) {
                    NESL_TestInit();
                    g_test.data = data;

                    switch(address & 0x3FFF) {
                        case 0x0000 ... 0x1FFF:

                            if(NESL_ASSERT((NESL_BusRead(NESL_BUS_VIDEO, address) == data)
                                    && (g_test.address == (address & 0x3FFF))
                                    && (g_test.type == NESL_BANK_CHARACTER_ROM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x2000 ... 0x3FFF:

                            if(NESL_ASSERT((NESL_BusRead(NESL_BUS_VIDEO, address) == data)
                                    && (g_test.address == (address & 0x3FFF)))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        default:

                            if(NESL_ASSERT(NESL_BusRead(NESL_BUS_VIDEO, address) == 0)) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                    }
                }
                break;
            case NESL_BUS_VIDEO_OAM:

                for(uint32_t address = 0x0000; address <= 0x00FF; ++address, ++data) {
                    NESL_TestInit();
                    g_test.data = data;

                    if(NESL_ASSERT((NESL_BusRead(NESL_BUS_VIDEO_OAM, address) == data)
                            && (g_test.address == (address & 0x00FF)))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
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

/**
 * @brief Test bus write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestBusWrite(void)
{
    int result = NESL_SUCCESS;

    for(nesl_bus_e type = 0; type < NESL_BUS_MAX; ++type) {
        uint8_t data = 0;

        switch(type) {
            case NESL_BUS_PROCESSOR:

                for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {
                    NESL_TestInit();

                    switch(address) {
                        case 0x0000 ... 0x4017:
                            NESL_BusWrite(NESL_BUS_PROCESSOR, address, data);

                            if(NESL_ASSERT((g_test.address == address)
                                    && (g_test.data == data))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x6000 ... 0x7FFF:
                            NESL_BusWrite(NESL_BUS_PROCESSOR, address, data);

                            if(NESL_ASSERT((g_test.address == address)
                                    && (g_test.data == data)
                                    && (g_test.type == NESL_BANK_PROGRAM_RAM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x8000 ... 0xFFFF:
                            NESL_BusWrite(NESL_BUS_PROCESSOR, address, data);

                            if(NESL_ASSERT((g_test.address == address)
                                    && (g_test.data == data)
                                    && (g_test.type == NESL_BANK_PROGRAM_ROM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        default:
                            NESL_BusWrite(NESL_BUS_PROCESSOR, address, data);

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
                    NESL_TestInit();

                    switch(address & 0x3FFF) {
                        case 0x0000 ... 0x1FFF:
                            NESL_BusWrite(NESL_BUS_VIDEO, address, data);

                            if(NESL_ASSERT((g_test.type == NESL_BANK_CHARACTER_ROM)
                                    && (g_test.address == (address & 0x3FFF))
                                    && (g_test.data == data))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x2000 ... 0x3FFF:
                            NESL_BusWrite(NESL_BUS_VIDEO, address, data);

                            if(NESL_ASSERT((g_test.address == (address & 0x3FFF))
                                    && (g_test.data == data))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        default:
                            NESL_BusWrite(NESL_BUS_VIDEO, address, data);

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
            case NESL_BUS_VIDEO_OAM:

                for(uint32_t address = 0x0000; address <= 0x00FF; ++address, ++data) {
                    NESL_TestInit();
                    NESL_BusWrite(NESL_BUS_VIDEO_OAM, address, data);

                    if(NESL_ASSERT((g_test.address == (address & 0x00FF))
                            && (g_test.data == data))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
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

int main(void)
{
    static const NESL_Test TEST[] = {
        NESL_TestBusInterrupt, NESL_TestBusRead, NESL_TestBusWrite,
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
