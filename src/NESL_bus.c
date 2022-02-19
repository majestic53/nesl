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

#include "../include/system/NESL_input.h"
#include "../include/system/NESL_mapper.h"
#include "../include/system/NESL_processor.h"
#include "../include/system/NESL_video.h"
#include "../include/NESL_service.h"

typedef struct {
    uint64_t cycle;
    nesl_input_t input;
    nesl_mapper_t mapper;
    nesl_processor_t processor;
    //nesl_video_t video;
} nesl_bus_t;

static nesl_bus_t g_bus = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static int NESL_BusReset(void)
{
    int result;

    if((result = NESL_MapperReset(&g_bus.mapper)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_InputReset(&g_bus.input)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_ProcessorReset(&g_bus.processor)) == NESL_FAILURE) {
        goto exit;
    }

    /*if((result = NESL_VideoReset(&g_bus.video, &g_bus.mapper.mirror)) == NESL_FAILURE) {
        goto exit;
    }*/

    if((result = NESL_ServiceReset()) == NESL_FAILURE) {
        goto exit;
    }

    g_bus.cycle = 0;

exit:
    return result;
}

bool NESL_BusCycle(void)
{
    bool result = true;//NESL_VideoCycle(&g_bus.video);

    NESL_ProcessorCycle(&g_bus.processor, g_bus.cycle);
    ++g_bus.cycle;

    return result;
}

int NESL_BusInit(const void *data, int length)
{
    int result;

    if((result = NESL_MapperInit(&g_bus.mapper, data, length)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_InputInit(&g_bus.input)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_ProcessorInit(&g_bus.processor)) == NESL_FAILURE) {
        goto exit;
    }

    /*if((result = NESL_VideoInit(&g_bus.video, &g_bus.mapper.mirror)) == NESL_FAILURE) {
        goto exit;
    }*/

    if((result = NESL_BusReset()) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_BusInterrupt(int type)
{
    int result = NESL_FAILURE;

    switch(type) {
        case NESL_INTERRUPT_MASKABLE:
        case NESL_INTERRUPT_NON_MASKABLE:

            if((result = NESL_ProcessorInterrupt(&g_bus.processor, type == NESL_INTERRUPT_MASKABLE)) == NESL_FAILURE) {
                goto exit;
            }
            break;
        case NESL_INTERRUPT_RESET:

            if((result = NESL_BusReset()) == NESL_FAILURE) {
                goto exit;
            }
            break;
        case NESL_INTERRUPT_MAPPER:

            if((result = NESL_MapperInterrupt(&g_bus.mapper)) == NESL_FAILURE) {
                goto exit;
            }
            break;
        default:
            break;
    }

exit:
    return result;
}

uint8_t NESL_BusRead(int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BUS_PROCESSOR:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = NESL_ProcessorRead(&g_bus.processor, address);
                    break;
                case 0x2000 ... 0x3FFF:
                    //result = NESL_VideoPortRead(&g_bus.video, address);
                    break;
                case 0x4016 ... 0x4017:
                    result = NESL_InputRead(&g_bus.input, address);
                    break;
                case 0x6000 ... 0x7FFF:
                    result = NESL_MapperRead(&g_bus.mapper, NESL_BANK_RAM_PROGRAM, address);
                    break;
                case 0x8000 ... 0xFFFF:
                    result = NESL_MapperRead(&g_bus.mapper, NESL_BANK_ROM_PROGRAM, address);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = NESL_MapperRead(&g_bus.mapper, NESL_BANK_ROM_CHARACTER, address);
                    break;
                case 0x2000 ... 0x3FFF:
                    //result = NESL_VideoRead(&g_bus.video, address);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO_OAM:

            switch(address) {
                case 0x0000 ... 0x00FF:
                    //result = NESL_VideoOamRead(&g_bus.video, address);
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

void NESL_BusUninit(void)
{
    //NESL_VideoUninit(&g_bus.video);
    NESL_ProcessorUninit(&g_bus.processor);
    NESL_InputUninit(&g_bus.input);
    NESL_MapperUninit(&g_bus.mapper);
    memset(&g_bus, 0, sizeof(g_bus));
}

void NESL_BusWrite(int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BUS_PROCESSOR:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                case 0x4014:
                    NESL_ProcessorWrite(&g_bus.processor, address, data);
                    break;
                case 0x2000 ... 0x3FFF:
                    //NESL_VideoPortWrite(&g_bus.video, address, data);
                    break;
                case 0x4016:
                    NESL_InputWrite(&g_bus.input, address, data);
                    break;
                case 0x6000 ... 0x7FFF:
                    NESL_MapperWrite(&g_bus.mapper, NESL_BANK_RAM_PROGRAM, address, data);
                    break;
                case 0x8000 ... 0xFFFF:
                    NESL_MapperWrite(&g_bus.mapper, NESL_BANK_ROM_PROGRAM, address, data);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    NESL_MapperWrite(&g_bus.mapper, NESL_BANK_ROM_CHARACTER, address, data);
                    break;
                case 0x2000 ... 0x3FFF:
                    //NESL_VideoWrite(&g_bus.video, address, data);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO_OAM:

            switch(address) {
                case 0x0000 ... 0x00FF:
                    //NESL_VideoOamWrite(&g_bus.video, address, data);
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
