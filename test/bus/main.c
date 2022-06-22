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
 * @brief Test application for bus.
 */

#include <audio.h>
#include <input.h>
#include <mapper.h>
#include <processor.h>
#include <video.h>
#include <test.h>

/*!
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_bank_e type;                   /*!< Bank type */
    uint16_t address;                   /*!< Bank address */
    uint8_t data;                       /*!< Bank data */

    struct {
        bool reset;                     /*!< Reset state */
    } service;

    struct {

        struct {
            bool reset;                 /*!< Reset state */
        } audio;

        struct {
            bool reset;                 /*!< Reset state */
        } input;

        struct {
            bool interrupt;             /*!< Interrupt state */
            bool reset;                 /*!< Reset state */
        } mapper;

        struct {
            bool interrupt;             /*!< Interrupt state */
            bool interrupt_maskable;    /*!< Maskable interrupt state */
            bool reset;                 /*!< Reset state */
        } processor;

        struct {
            bool reset;                 /*!< Reset state */
        } video;
    } subsystem;
} nesl_test_t;

static nesl_test_t g_test = {};         /*!< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void nesl_audio_cycle(nesl_audio_t *audio, uint64_t cycle)
{
    return;
}

nesl_error_e nesl_audio_initialize(nesl_audio_t *audio)
{
    return NESL_SUCCESS;
}

uint8_t nesl_audio_read(nesl_audio_t *audio, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

nesl_error_e nesl_audio_reset(nesl_audio_t *audio)
{
    g_test.subsystem.audio.reset = true;

    return NESL_SUCCESS;
}

void nesl_audio_uninitialize(nesl_audio_t *audio)
{
    return;
}

void nesl_audio_write(nesl_audio_t *audio, uint16_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

nesl_error_e nesl_input_initialize(nesl_input_t *input)
{
    return NESL_SUCCESS;
}

uint8_t nesl_input_read(nesl_input_t *input, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

nesl_error_e nesl_input_reset(nesl_input_t *input)
{
    g_test.subsystem.input.reset = true;

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

nesl_error_e nesl_mapper_initialize(nesl_mapper_t *mapper, const void *data, int length)
{
    return NESL_SUCCESS;
}

nesl_error_e nesl_mapper_interrupt(nesl_mapper_t *mapper)
{
    g_test.subsystem.mapper.interrupt = true;

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_read(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    g_test.type = type;
    g_test.address = address;

    return g_test.data;
}

nesl_error_e nesl_mapper_reset(nesl_mapper_t *mapper)
{
    g_test.subsystem.mapper.reset = true;

    return NESL_SUCCESS;
}

void nesl_mapper_uninitialize(nesl_mapper_t *mapper)
{
    return;
}

void nesl_mapper_write(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{
    g_test.type = type;
    g_test.address = address;
    g_test.data = data;
}

void nesl_processor_cycle(nesl_processor_t *processor, uint64_t cycle)
{
    return;
}

nesl_error_e nesl_processor_initialize(nesl_processor_t *processor)
{
    return NESL_SUCCESS;
}

nesl_error_e nesl_processor_interrupt(nesl_processor_t *processor, bool maskable)
{
    g_test.subsystem.processor.interrupt = true;
    g_test.subsystem.processor.interrupt_maskable = maskable;

    return NESL_SUCCESS;
}

uint8_t nesl_processor_read(nesl_processor_t *processor, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

nesl_error_e nesl_processor_reset(nesl_processor_t *processor)
{
    g_test.subsystem.processor.reset = true;

    return NESL_SUCCESS;
}

void nesl_processor_uninitialize(nesl_processor_t *processor)
{
    return;
}

void nesl_processor_write(nesl_processor_t *processor, uint16_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

nesl_error_e nesl_service_reset(void)
{
    g_test.service.reset = true;

    return NESL_SUCCESS;
}

bool nesl_video_cycle(nesl_video_t *video)
{
    return true;
}

nesl_error_e nesl_video_initialize(nesl_video_t *video, const nesl_mirror_e *mirror)
{
    return NESL_SUCCESS;
}

uint8_t nesl_video_read(nesl_video_t *video, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

uint8_t nesl_video_read_oam(nesl_video_t *video, uint8_t address)
{
    g_test.address = address;

    return g_test.data;
}

uint8_t nesl_video_read_port(nesl_video_t *video, uint16_t address)
{
    g_test.address = address;

    return g_test.data;
}

nesl_error_e nesl_video_reset(nesl_video_t *video, const nesl_mirror_e *mirror)
{
    g_test.subsystem.video.reset = true;

    return NESL_SUCCESS;
}

void nesl_video_uninitialize(nesl_video_t *video)
{
    return;
}

void nesl_video_write(nesl_video_t *video, uint16_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

void nesl_video_write_oam(nesl_video_t *video, uint8_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

void nesl_video_write_port(nesl_video_t *video, uint16_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;
}

/*!
 * @brief Initialize test context.
 */
static void nesl_test_initialize(void)
{
    memset(&g_test, 0, sizeof(g_test));
}

/*!
 * @brief Test bus interrupt.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_bus_interrupt(void)
{
    nesl_error_e result = NESL_SUCCESS;

    for(nesl_interrupt_e type = 0; type < INTERRUPT_MAX; ++type) {
        nesl_test_initialize();
        nesl_bus_interrupt(type);

        switch(type) {
            case INTERRUPT_MASKABLE:
            case INTERRUPT_NON_MASKABLE:

                if(ASSERT((g_test.subsystem.processor.interrupt == true)
                        && (g_test.subsystem.processor.interrupt_maskable == (type == INTERRUPT_MASKABLE)))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case INTERRUPT_RESET:

                if(ASSERT((g_test.service.reset == true)
                        && (g_test.subsystem.audio.reset == true)
                        && (g_test.subsystem.input.reset == true)
                        && (g_test.subsystem.mapper.reset == true)
                        && (g_test.subsystem.processor.reset == true)
                        && (g_test.subsystem.video.reset == true))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case INTERRUPT_MAPPER:

                if(ASSERT(g_test.subsystem.mapper.interrupt == true)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            default:
                break;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test bus read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_bus_read(void)
{
    nesl_error_e result = NESL_SUCCESS;

    for(nesl_bus_e type = 0; type < BUS_MAX; ++type) {
        uint8_t data = 0;

        switch(type) {
            case BUS_PROCESSOR:

                for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {
                    nesl_test_initialize();
                    g_test.data = data;

                    switch(address) {
                        case 0x0000 ... 0x3FFF:
                        case 0x4015 ... 0x4017:

                            if(ASSERT((nesl_bus_read(BUS_PROCESSOR, address) == data)
                                    && (g_test.address == address))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x6000 ... 0x7FFF:

                            if(ASSERT((nesl_bus_read(BUS_PROCESSOR, address) == data)
                                    && (g_test.address == address)
                                    && (g_test.type == BANK_PROGRAM_RAM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x8000 ... 0xFFFF:

                            if(ASSERT((nesl_bus_read(BUS_PROCESSOR, address) == data)
                                    && (g_test.address == address)
                                    && (g_test.type == BANK_PROGRAM_ROM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        default:

                            if(ASSERT(nesl_bus_read(BUS_PROCESSOR, address) == 0)) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                    }
                }
                break;
            case BUS_VIDEO:

                for(uint32_t address = 0x0000; address <= 0x3FFF; ++address, ++data) {
                    nesl_test_initialize();
                    g_test.data = data;

                    switch(address & 0x3FFF) {
                        case 0x0000 ... 0x1FFF:

                            if(ASSERT((nesl_bus_read(BUS_VIDEO, address) == data)
                                    && (g_test.address == (address & 0x3FFF))
                                    && (g_test.type == BANK_CHARACTER_ROM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x2000 ... 0x3FFF:

                            if(ASSERT((nesl_bus_read(BUS_VIDEO, address) == data)
                                    && (g_test.address == (address & 0x3FFF)))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        default:

                            if(ASSERT(nesl_bus_read(BUS_VIDEO, address) == 0)) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                    }
                }
                break;
            case BUS_VIDEO_OAM:

                for(uint32_t address = 0x0000; address <= 0x00FF; ++address, ++data) {
                    nesl_test_initialize();
                    g_test.data = data;

                    if(ASSERT((nesl_bus_read(BUS_VIDEO_OAM, address) == data)
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
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test bus write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_bus_write(void)
{
    int result = NESL_SUCCESS;

    for(nesl_bus_e type = 0; type < BUS_MAX; ++type) {
        uint8_t data = 0;

        switch(type) {
            case BUS_PROCESSOR:

                for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {
                    nesl_test_initialize();

                    switch(address) {
                        case 0x0000 ... 0x4017:
                            nesl_bus_write(BUS_PROCESSOR, address, data);

                            if(ASSERT((g_test.address == address)
                                    && (g_test.data == data))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x6000 ... 0x7FFF:
                            nesl_bus_write(BUS_PROCESSOR, address, data);

                            if(ASSERT((g_test.address == address)
                                    && (g_test.data == data)
                                    && (g_test.type == BANK_PROGRAM_RAM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x8000 ... 0xFFFF:
                            nesl_bus_write(BUS_PROCESSOR, address, data);

                            if(ASSERT((g_test.address == address)
                                    && (g_test.data == data)
                                    && (g_test.type == BANK_PROGRAM_ROM))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        default:
                            nesl_bus_write(BUS_PROCESSOR, address, data);

                            if(ASSERT((g_test.address == 0)
                                    && (g_test.data == 0)
                                    && (g_test.type == 0))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                    }
                }
                break;
            case BUS_VIDEO:

                for(uint32_t address = 0x0000; address <= 0x3FFF; ++address, ++data) {
                    nesl_test_initialize();

                    switch(address & 0x3FFF) {
                        case 0x0000 ... 0x1FFF:
                            nesl_bus_write(BUS_VIDEO, address, data);

                            if(ASSERT((g_test.type == BANK_CHARACTER_ROM)
                                    && (g_test.address == (address & 0x3FFF))
                                    && (g_test.data == data))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        case 0x2000 ... 0x3FFF:
                            nesl_bus_write(BUS_VIDEO, address, data);

                            if(ASSERT((g_test.address == (address & 0x3FFF))
                                    && (g_test.data == data))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                        default:
                            nesl_bus_write(BUS_VIDEO, address, data);

                            if(ASSERT((g_test.type == 0)
                                    && (g_test.address == 0)
                                    && (g_test.data == 0))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                            break;
                    }
                }
                break;
            case BUS_VIDEO_OAM:

                for(uint32_t address = 0x0000; address <= 0x00FF; ++address, ++data) {
                    nesl_test_initialize();
                    nesl_bus_write(BUS_VIDEO_OAM, address, data);

                    if(ASSERT((g_test.address == (address & 0x00FF))
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
    TEST_RESULT(result);

    return result;
}

int main(void)
{
    const test TEST[] = {
        nesl_test_bus_interrupt, nesl_test_bus_read, nesl_test_bus_write,
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
