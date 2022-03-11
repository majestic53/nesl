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
 * @file bus.c
 * @brief Common bus used by subsystems for communication.
 */

#include <audio.h>
#include <input.h>
#include <mapper.h>
#include <processor.h>
#include <video.h>

/**
 * @struct nesl_bus_t
 * @brief Bus and subsystem contexts.
 */
typedef struct {
    uint64_t cycle;                 /*< Cycle-count since start of emulation */

    struct {
        nesl_audio_t audio;         /*< Audio subsystem context */
        nesl_input_t input;         /*< Input subsystem context */
        nesl_mapper_t mapper;       /*< Mapper subsystem context */
        nesl_processor_t processor; /*< Processor subsystem context */
        nesl_video_t video;         /*< Video subsystem context */
    } subsystem;
} nesl_bus_t;

static nesl_bus_t g_bus = {};       /*< Bus context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Reset Bus/Service and subsystems.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_bus_reset(void)
{
    nesl_error_e result;

    if((result = nesl_service_reset()) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_mapper_reset(&g_bus.subsystem.mapper)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_audio_reset(&g_bus.subsystem.audio)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_input_reset(&g_bus.subsystem.input)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_processor_reset(&g_bus.subsystem.processor)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_video_reset(&g_bus.subsystem.video, &g_bus.subsystem.mapper.mirror)) == NESL_FAILURE) {
        goto exit;
    }

    g_bus.cycle = 0;

exit:
    return result;
}

bool nesl_bus_cycle(void)
{
    nesl_processor_cycle(&g_bus.subsystem.processor, g_bus.cycle);
    nesl_audio_cycle(&g_bus.subsystem.audio, g_bus.cycle);
    ++g_bus.cycle;

    return nesl_video_cycle(&g_bus.subsystem.video);
}

nesl_error_e nesl_bus_initialize(const void *data, int length)
{
    nesl_error_e result;

    if((result = nesl_mapper_initialize(&g_bus.subsystem.mapper, data, length)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_audio_initialize(&g_bus.subsystem.audio)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_input_initialize(&g_bus.subsystem.input)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_processor_initialize(&g_bus.subsystem.processor)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_video_initialize(&g_bus.subsystem.video, &g_bus.subsystem.mapper.mirror)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

nesl_error_e nesl_bus_interrupt(nesl_interrupt_e type)
{
    int result = NESL_FAILURE;

    switch(type) {
        case INTERRUPT_MASKABLE:
        case INTERRUPT_NON_MASKABLE:

            if((result = nesl_processor_interrupt(&g_bus.subsystem.processor, type == INTERRUPT_MASKABLE)) == NESL_FAILURE) {
                goto exit;
            }
            break;
        case INTERRUPT_RESET:

            if((result = nesl_bus_reset()) == NESL_FAILURE) {
                goto exit;
            }
            break;
        case INTERRUPT_MAPPER:

            if((result = nesl_mapper_interrupt(&g_bus.subsystem.mapper)) == NESL_FAILURE) {
                goto exit;
            }
            break;
        default:
            break;
    }

exit:
    return result;
}

uint8_t nesl_bus_read(nesl_bus_e type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case BUS_PROCESSOR:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = nesl_processor_read(&g_bus.subsystem.processor, address);
                    break;
                case 0x2000 ... 0x3FFF:
                    result = nesl_video_read_port(&g_bus.subsystem.video, address);
                    break;
                case 0x4015:
                    result = nesl_audio_read(&g_bus.subsystem.audio, address);
                    break;
                case 0x4016 ... 0x4017:
                    result = nesl_input_read(&g_bus.subsystem.input, address);
                    break;
                case 0x6000 ... 0x7FFF:
                    result = nesl_mapper_read(&g_bus.subsystem.mapper, BANK_PROGRAM_RAM, address);
                    break;
                case 0x8000 ... 0xFFFF:
                    result = nesl_mapper_read(&g_bus.subsystem.mapper, BANK_PROGRAM_ROM, address);
                    break;
                default:
                    break;
            }
            break;
        case BUS_VIDEO:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = nesl_mapper_read(&g_bus.subsystem.mapper, BANK_CHARACTER_ROM, address);
                    break;
                case 0x2000 ... 0x3FFF:
                    result = nesl_video_read(&g_bus.subsystem.video, address);
                    break;
                default:
                    break;
            }
            break;
        case BUS_VIDEO_OAM:

            switch(address) {
                case 0x0000 ... 0x00FF:
                    result = nesl_video_read_oam(&g_bus.subsystem.video, address);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return result;
}

void nesl_bus_uninitialize(void)
{
    nesl_video_uninitialize(&g_bus.subsystem.video);
    nesl_processor_uninitialize(&g_bus.subsystem.processor);
    nesl_input_uninitialize(&g_bus.subsystem.input);
    nesl_audio_uninitialize(&g_bus.subsystem.audio);
    nesl_mapper_uninitialize(&g_bus.subsystem.mapper);
    memset(&g_bus, 0, sizeof(g_bus));
}

void nesl_bus_write(nesl_bus_e type, uint16_t address, uint8_t data)
{

    switch(type) {
        case BUS_PROCESSOR:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                case 0x4014:
                    nesl_processor_write(&g_bus.subsystem.processor, address, data);
                    break;
                case 0x2000 ... 0x3FFF:
                    nesl_video_write_port(&g_bus.subsystem.video, address, data);
                    break;
                case 0x4000 ... 0x4013:
                case 0x4015:
                case 0x4017:
                    nesl_audio_write(&g_bus.subsystem.audio, address, data);
                    break;
                case 0x4016:
                    nesl_input_write(&g_bus.subsystem.input, address, data);
                    break;
                case 0x6000 ... 0x7FFF:
                    nesl_mapper_write(&g_bus.subsystem.mapper, BANK_PROGRAM_RAM, address, data);
                    break;
                case 0x8000 ... 0xFFFF:
                    nesl_mapper_write(&g_bus.subsystem.mapper, BANK_PROGRAM_ROM, address, data);
                    break;
                default:
                    break;
            }
            break;
        case BUS_VIDEO:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    nesl_mapper_write(&g_bus.subsystem.mapper, BANK_CHARACTER_ROM, address, data);
                    break;
                case 0x2000 ... 0x3FFF:
                    nesl_video_write(&g_bus.subsystem.video, address, data);
                    break;
                default:
                    break;
            }
            break;
        case BUS_VIDEO_OAM:

            switch(address) {
                case 0x0000 ... 0x00FF:
                    nesl_video_write_oam(&g_bus.subsystem.video, address, data);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
