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

#include "../include/system/input.h"
#include "../include/system/mapper.h"
#include "../include/service.h"

typedef struct {
    uint64_t cycle;
    nesl_input_t input;
    nesl_mapper_t mapper;
    /*nesl_processor_t processor;
    nesl_video_t video;*/
} nesl_bus_t;

static nesl_bus_t g_bus = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static int nesl_bus_reset(void)
{
    int result;

    if((result = nesl_mapper_reset(&g_bus.mapper)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_input_reset(&g_bus.input)) == NESL_FAILURE) {
        goto exit;
    }

    /*if((result = nesl_processor_reset(&g_bus.processor)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_video_reset(&g_bus.video, &g_bus.mapper.mirror)) == NESL_FAILURE) {
        goto exit;
    }*/

    if((result = nesl_service_reset()) == NESL_FAILURE) {
        goto exit;
    }

    g_bus.cycle = 0;

exit:
    return result;
}

bool nesl_bus_cycle(void)
{
    /*bool result = nesl_video_cycle(&g_bus.video);

    nesl_processor_cycle(&g_bus.processor, g_bus.cycle);
    ++g_bus.cycle;

    return result;*/
    return true;
}

int nesl_bus_initialize(const void *data, int length)
{
    int result;

    if((result = nesl_mapper_initialize(&g_bus.mapper, data, length)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_input_initialize(&g_bus.input)) == NESL_FAILURE) {
        goto exit;
    }

    /*if((result = nesl_processor_initialize(&g_bus.processor)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_video_initialize(&g_bus.video, &g_bus.mapper.mirror)) == NESL_FAILURE) {
        goto exit;
    }*/

    if((result = nesl_bus_reset()) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int nesl_bus_interrupt(int type)
{
    int result = NESL_FAILURE;

    switch(type) {
        case NESL_INTERRUPT_MASKABLE:
        case NESL_INTERRUPT_NON_MASKABLE:

            /*if((result = nesl_processor_interrupt(&g_bus.processor, type == NESL_INTERRUPT_MASKABLE)) == NESL_FAILURE) {
                goto exit;
            }*/
            break;
        case NESL_INTERRUPT_RESET:

            if((result = nesl_bus_reset()) == NESL_FAILURE) {
                goto exit;
            }
            break;
        case NESL_INTERRUPT_MAPPER:

            if((result = nesl_mapper_interrupt(&g_bus.mapper)) == NESL_FAILURE) {
                goto exit;
            }
            break;
        default:
            break;
    }

exit:
    return result;
}

uint8_t nesl_bus_read(int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BUS_PROCESSOR:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    //result = nesl_processor_read(&g_bus.processor, address);
                    break;
                case 0x2000 ... 0x3FFF:
                    //result = nesl_video_port_read(&g_bus.video, address);
                    break;
                case 0x4016 ... 0x4017:
                    result = nesl_input_read(&g_bus.input, address);
                    break;
                case 0x6000 ... 0x7FFF:
                    result = nesl_mapper_read(&g_bus.mapper, NESL_BANK_RAM_PROGRAM, address);
                    break;
                case 0x8000 ... 0xFFFF:
                    result = nesl_mapper_read(&g_bus.mapper, NESL_BANK_ROM_PROGRAM, address);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = nesl_mapper_read(&g_bus.mapper, NESL_BANK_ROM_CHARACTER, address);
                    break;
                case 0x2000 ... 0x3FFF:
                    //result = nesl_video_read(&g_bus.video, address);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO_OAM:

            switch(address) {
                case 0x0000 ... 0x00FF:
                    //result = nesl_video_oam_read(&g_bus.video, address);
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
    /*nesl_video_uninitialize(&g_bus.video);
    nesl_processor_uninitialize(&g_bus.processor);*/
    nesl_input_uninitialize(&g_bus.input);
    nesl_mapper_uninitialize(&g_bus.mapper);
    memset(&g_bus, 0, sizeof(g_bus));
}

void nesl_bus_write(int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BUS_PROCESSOR:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                case 0x4014:
                    //nesl_processor_write(&g_bus.processor, address, data);
                    break;
                case 0x2000 ... 0x3FFF:
                    //nesl_video_port_write(&g_bus.video, address, data);
                    break;
                case 0x4016:
                    nesl_input_write(&g_bus.input, address, data);
                    break;
                case 0x6000 ... 0x7FFF:
                    nesl_mapper_write(&g_bus.mapper, NESL_BANK_RAM_PROGRAM, address, data);
                    break;
                case 0x8000 ... 0xFFFF:
                    nesl_mapper_write(&g_bus.mapper, NESL_BANK_ROM_PROGRAM, address, data);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    nesl_mapper_write(&g_bus.mapper, NESL_BANK_ROM_CHARACTER, address, data);
                    break;
                case 0x2000 ... 0x3FFF:
                    //nesl_video_write(&g_bus.video, address, data);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BUS_VIDEO_OAM:

            switch(address) {
                case 0x0000 ... 0x00FF:
                    //nesl_video_oam_write(&g_bus.video, address, data);
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
