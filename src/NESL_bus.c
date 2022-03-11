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
 * @file NESL_bus.c
 * @brief Common bus used by the various subsystems for communication.
 */

#include <NESL_audio.h>
#include <NESL_input.h>
#include <NESL_mapper.h>
#include <NESL_processor.h>
#include <NESL_video.h>
#include <NESL_service.h>

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
static nesl_error_e NESL_BusReset(void)
{
    nesl_error_e result;

    if((result = NESL_ServiceReset()) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_MapperReset(&g_bus.subsystem.mapper)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_AudioReset(&g_bus.subsystem.audio)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_InputReset(&g_bus.subsystem.input)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_ProcessorReset(&g_bus.subsystem.processor)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_VideoReset(&g_bus.subsystem.video, &g_bus.subsystem.mapper.mirror)) == NESL_FAILURE) {
        goto exit;
    }

    g_bus.cycle = 0;

exit:
    return result;
}

bool NESL_BusCycle(void)
{
    NESL_ProcessorCycle(&g_bus.subsystem.processor, g_bus.cycle);
    NESL_AudioCycle(&g_bus.subsystem.audio, g_bus.cycle);
    ++g_bus.cycle;

    return NESL_VideoCycle(&g_bus.subsystem.video);
}

nesl_error_e NESL_BusInitialize(const void *data, int length)
{
    nesl_error_e result;

    if((result = NESL_MapperInitialize(&g_bus.subsystem.mapper, data, length)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_AudioInitialize(&g_bus.subsystem.audio)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_InputInitialize(&g_bus.subsystem.input)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_ProcessorInitialize(&g_bus.subsystem.processor)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_VideoInitialize(&g_bus.subsystem.video, &g_bus.subsystem.mapper.mirror)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

nesl_error_e NESL_BusInterrupt(nesl_interrupt_e type)
{
    int result = NESL_FAILURE;

    switch(type) {
        case NESL_INTERRUPT_MASKABLE:
        case NESL_INTERRUPT_NON_MASKABLE:

            if((result = NESL_ProcessorInterrupt(&g_bus.subsystem.processor, type == NESL_INTERRUPT_MASKABLE)) == NESL_FAILURE) {
                goto exit;
            }
            break;
        case NESL_INTERRUPT_RESET:

            if((result = NESL_BusReset()) == NESL_FAILURE) {
                goto exit;
            }
            break;
        case NESL_INTERRUPT_MAPPER:

            if((result = NESL_MapperInterrupt(&g_bus.subsystem.mapper)) == NESL_FAILURE) {
                goto exit;
            }
            break;
        default:
            break;
    }

exit:
    return result;
}

uint8_t NESL_BusRead(nesl_bus_e type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BUS_PROCESSOR:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = NESL_ProcessorRead(&g_bus.subsystem.processor, address);
                    break;
                case 0x2000 ... 0x3FFF:
                    result = NESL_VideoReadPort(&g_bus.subsystem.video, address);
                    break;
                case 0x4015:
                    result = NESL_AudioRead(&g_bus.subsystem.audio, address);
                    break;
                case 0x4016 ... 0x4017:
                    result = NESL_InputRead(&g_bus.subsystem.input, address);
                    break;
                case 0x6000 ... 0x7FFF:
                    result = NESL_MapperRead(&g_bus.subsystem.mapper, NESL_BANK_PROGRAM_RAM, address);
                    break;
                case 0x8000 ... 0xFFFF:
                    result = NESL_MapperRead(&g_bus.subsystem.mapper, NESL_BANK_PROGRAM_ROM, address);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = NESL_MapperRead(&g_bus.subsystem.mapper, NESL_BANK_CHARACTER_ROM, address);
                    break;
                case 0x2000 ... 0x3FFF:
                    result = NESL_VideoRead(&g_bus.subsystem.video, address);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO_OAM:

            switch(address) {
                case 0x0000 ... 0x00FF:
                    result = NESL_VideoReadOam(&g_bus.subsystem.video, address);
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

void NESL_BusUninitialize(void)
{
    NESL_VideoUninitialize(&g_bus.subsystem.video);
    NESL_ProcessorUninitialize(&g_bus.subsystem.processor);
    NESL_InputUninitialize(&g_bus.subsystem.input);
    NESL_AudioUninitialize(&g_bus.subsystem.audio);
    NESL_MapperUninitialize(&g_bus.subsystem.mapper);
    memset(&g_bus, 0, sizeof(g_bus));
}

void NESL_BusWrite(nesl_bus_e type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BUS_PROCESSOR:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                case 0x4014:
                    NESL_ProcessorWrite(&g_bus.subsystem.processor, address, data);
                    break;
                case 0x2000 ... 0x3FFF:
                    NESL_VideoWritePort(&g_bus.subsystem.video, address, data);
                    break;
                case 0x4000 ... 0x4013:
                case 0x4015:
                case 0x4017:
                    NESL_AudioWrite(&g_bus.subsystem.audio, address, data);
                    break;
                case 0x4016:
                    NESL_InputWrite(&g_bus.subsystem.input, address, data);
                    break;
                case 0x6000 ... 0x7FFF:
                    NESL_MapperWrite(&g_bus.subsystem.mapper, NESL_BANK_PROGRAM_RAM, address, data);
                    break;
                case 0x8000 ... 0xFFFF:
                    NESL_MapperWrite(&g_bus.subsystem.mapper, NESL_BANK_PROGRAM_ROM, address, data);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    NESL_MapperWrite(&g_bus.subsystem.mapper, NESL_BANK_CHARACTER_ROM, address, data);
                    break;
                case 0x2000 ... 0x3FFF:
                    NESL_VideoWrite(&g_bus.subsystem.video, address, data);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO_OAM:

            switch(address) {
                case 0x0000 ... 0x00FF:
                    NESL_VideoWriteOam(&g_bus.subsystem.video, address, data);
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
