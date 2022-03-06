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
 * @brief Test application for video subsystem.
 */

#include "../../include/system/NESL_video.h"
#include "../include/NESL_common.h"

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_video_t video; /*< Video context */

    struct {
        nesl_bus_e type;            /*< Bus type */
        uint16_t address;           /*< Bus address */
        uint8_t data[16 * 1024];    /*< Bus data */
        nesl_interrupt_e int_type;  /*< Bus interrupt */
        nesl_mirror_e mirror;       /*< Bus mirror */
    } bus;
} nesl_test_t;

/**
 * @brief Video port test function.
 * @param address Test address
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
typedef nesl_error_e (*NESL_TestPort)(uint16_t address);

static nesl_test_t g_test = {}; /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e NESL_BusInterrupt(nesl_interrupt_e type)
{
    g_test.bus.int_type = type;

    return NESL_SUCCESS;
}

uint8_t NESL_BusRead(nesl_bus_e type, uint16_t address)
{
    uint8_t result = 0;

    g_test.bus.address = address;
    g_test.bus.type = type;

    switch(g_test.bus.type) {
        case NESL_BUS_VIDEO:
            result = g_test.bus.data[g_test.bus.address];
            break;
        default:
            break;
    }

    return result;
}

void NESL_BusWrite(nesl_bus_e type, uint16_t address, uint8_t data)
{
    g_test.bus.address = address;
    g_test.bus.type = type;

    switch(g_test.bus.type) {
        case NESL_BUS_VIDEO:
            g_test.bus.data[g_test.bus.address] = data;
            break;
        default:
            break;
    }
}

void NESL_ServiceSetPixel(uint8_t color, bool red_emphasis, bool green_emphasis, bool blue_emphasis, uint8_t x, uint8_t y)
{
    return;
}

static nesl_error_e NESL_TestVideoGetPortData(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    for(int increment = 0; increment <= 1; ++increment) {
        uint8_t data = 0;
        g_test.video.port.control.increment = increment;

        for(uint16_t addr = 0x0000; addr <= 0x3FFF; addr += (g_test.video.port.control.increment ? 32 : 1), ++data) {
            uint8_t value;

            if(NESL_ASSERT(g_test.video.address.v.word == addr)) {
                result = NESL_FAILURE;
                goto exit;
            }

            g_test.video.port.data.low = data + 1;
            g_test.bus.data[g_test.video.address.v.word] = data;
            value = NESL_VideoReadPort(&g_test.video, address);

            if(NESL_ASSERT((g_test.bus.address == addr)
                    && (g_test.bus.type == NESL_BUS_VIDEO))) {
                result = NESL_FAILURE;
                goto exit;
            }

            switch(addr) {
                case 0x3F00 ... 0x3FFF:

                    if(NESL_ASSERT(value == data)
                            && (g_test.video.port.data.low == data)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                    break;
                default:

                    if(NESL_ASSERT((value != data)
                            && (g_test.video.port.data.low == data))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                    break;
            }
        }

        g_test.video.address.v.word = 0;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoGetPortOamData(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    ((uint8_t *)g_test.video.ram.oam)[g_test.video.port.oam_address.low] = g_test.video.port.oam_address.low;

    if(NESL_ASSERT(NESL_VideoReadPort(&g_test.video, address) == g_test.video.port.oam_address.low)) {
        result = NESL_FAILURE;
        goto exit;
    }

    ++g_test.video.port.oam_address.low;

exit:
    return result;
}

static nesl_error_e NESL_TestVideoGetPortStatus(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    g_test.video.port.data.low = 0x0A;
    g_test.video.port.status.sprite_overflow = true;
    g_test.video.port.status.sprite_0_hit = true;
    g_test.video.port.status.vertical_blank = true;
    g_test.video.port.latch = true;

    if(NESL_ASSERT((NESL_VideoReadPort(&g_test.video, address) == 0xEA)
            && (g_test.video.port.status.raw == 0x60)
            && (g_test.video.port.latch == false))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoGetPortUnused(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    g_test.video.port.data.low = 0xAC;

    if(NESL_ASSERT(NESL_VideoReadPort(&g_test.video, address) == 0xAC)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestInit(bool initialize, nesl_mirror_e mirror)
{
    nesl_error_e result = NESL_SUCCESS;

    memset(&g_test, 0, sizeof(g_test));
    g_test.bus.mirror = mirror;

    if(initialize) {

        if((result = NESL_VideoInit(&g_test.video, &g_test.bus.mirror)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoNametableAddress(uint16_t address, nesl_mirror_e mirror, int *bank, uint16_t *addr)
{
    nesl_error_e result = NESL_SUCCESS;

    switch(mirror) {
        case NESL_MIRROR_HORIZONTAL:

            switch(address) {
                case 0x2000 ... 0x23FF:
                case 0x2400 ... 0x27FF:
                    *bank = 0;
                    break;
                case 0x2800 ... 0x2BFF:
                case 0x2C00 ... 0x2FFF:
                    *bank = 1;
                    break;
                default:
                    NESL_ASSERT((address >= 0x2000) && (address <= 0x2FFF));
                    result = NESL_FAILURE;
                    goto exit;
            }
            break;
        case NESL_MIRROR_VERTICAL:

            switch(address) {
                case 0x2000 ... 0x23FF:
                case 0x2800 ... 0x2BFF:
                    *bank = 0;
                    break;
                case 0x2400 ... 0x27FF:
                case 0x2C00 ... 0x2FFF:
                    *bank = 1;
                    break;
                default:
                    NESL_ASSERT((address >= 0x2000) && (address <= 0x2FFF));
                    result = NESL_FAILURE;
                    goto exit;
            }
            break;
        case NESL_MIRROR_ONE_LOW:
            *bank = 0;
            break;
        case NESL_MIRROR_ONE_HIGH:
            *bank = 1;
            break;
        default:
            NESL_ASSERT(mirror < NESL_MIRROR_MAX);
            result = NESL_FAILURE;
            goto exit;
    }

    *addr = (address & 0x03FF);

exit:
    return result;
}

static uint16_t NESL_TestVideoPaletteAddress(uint16_t address)
{

    switch((address &= 0x1F)) {
        case 0x10:
        case 0x14:
        case 0x18:
        case 0x1C:
            address -= 0x10;
            break;
        default:
            break;
    }

    return address;
}

static nesl_error_e NESL_TestVideoSetPortAddress(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_VideoWritePort(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT((g_test.video.address.t.high == (address & 0x3F))
            && (g_test.video.port.latch == true))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_VideoWritePort(&g_test.video, address, (address + 1) & 0xFF);

    if(NESL_ASSERT((g_test.video.address.t.low == ((address + 1) & 0xFF))
            && (g_test.video.address.v.word == g_test.video.address.t.word)
            && (g_test.video.port.latch == false))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoSetPortControl(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_VideoWritePort(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT((g_test.video.port.control.raw == (address & 0xFF))
            && (g_test.video.address.t.nametable_x == g_test.video.port.control.nametable_x)
            && (g_test.video.address.t.nametable_y == g_test.video.port.control.nametable_y))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoSetPortData(uint16_t address)
{
    uint16_t addr;
    nesl_error_e result = NESL_SUCCESS;

    for(int increment = 0; increment <= 1; ++increment) {
        uint8_t data = 0;
        g_test.video.port.control.increment = increment;

        for(addr = 0x0000; addr <= 0x3FFF; addr += (g_test.video.port.control.increment ? 32 : 1), ++data) {

            if(NESL_ASSERT(g_test.video.address.v.word == addr)) {
                result = NESL_FAILURE;
                goto exit;
            }

            NESL_VideoWritePort(&g_test.video, address, data);

            if(NESL_ASSERT((g_test.bus.data[addr] == data)
                    && (g_test.bus.address == addr)
                    && (g_test.bus.type == NESL_BUS_VIDEO))) {
                result = NESL_FAILURE;
                goto exit;
            }
        }

        g_test.video.address.v.word = 0;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoSetPortMask(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_VideoWritePort(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT(g_test.video.port.mask.raw == (address & 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoSetPortOamAddress(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_VideoWritePort(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT(g_test.video.port.oam_address.low == (address & 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoSetPortOamData(uint16_t address)
{
    uint16_t addr;
    nesl_error_e result = NESL_SUCCESS;

    addr = g_test.video.port.oam_address.low;
    g_test.video.port.status.vertical_blank = false;
    NESL_VideoWritePort(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT((((uint8_t *)g_test.video.ram.oam)[addr] == (address & 0xFF))
            && (g_test.video.port.oam_address.low == ((addr + 1) & 0xFF)))) {
        result = NESL_FAILURE;
        goto exit;
    }

    addr = g_test.video.port.oam_address.low;
    g_test.video.port.status.vertical_blank = true;
    NESL_VideoWritePort(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT((((uint8_t *)g_test.video.ram.oam)[addr] == (address & 0xFF))
            && (g_test.video.port.oam_address.low == (addr & 0xFF)))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoSetPortScroll(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_VideoWritePort(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT((g_test.video.address.t.coarse_x == ((address & 0xFF) >> 3))
            && (g_test.video.address.fine_x == ((address & 0xFF) & 7))
            && (g_test.video.port.latch == true))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_VideoWritePort(&g_test.video, address, (address + 1) & 0xFF);

    if(NESL_ASSERT((g_test.video.address.t.coarse_y == (((address + 1) & 0xFF) >> 3))
            && (g_test.video.address.t.fine_y == (((address + 1) & 0xFF) & 7))
            && (g_test.video.port.latch == false))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoSetPortUnused(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    NESL_VideoWritePort(&g_test.video, address, 0xAC);

    if(NESL_ASSERT(g_test.video.port.data.low == 0xAC)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static nesl_error_e NESL_TestVideoCycle(void)
{
    uint64_t cycles = 0;
    nesl_error_e result = NESL_SUCCESS;

    if((result = NESL_TestInit(true, NESL_MIRROR_HORIZONTAL)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.video.port.status.sprite_overflow = true;
    g_test.video.port.status.sprite_0_hit = true;
    g_test.video.port.status.vertical_blank = true;
    g_test.video.port.control.interrupt = true;
    g_test.video.port.mask.background_show = true;
    g_test.video.port.mask.sprite_show = true;

    for(int16_t scanline = -1; scanline < 261; ++scanline) {

        for(uint16_t cycle = 0; cycle <= 340; ++cycle) {

            if(NESL_ASSERT((g_test.video.scanline == scanline)
                    && (g_test.video.cycle == cycle))) {
                result = NESL_SUCCESS;
                goto exit;
            }

            NESL_VideoCycle(&g_test.video);

            if((scanline == -1) && (cycle == 1)) {

                if(NESL_ASSERT((g_test.video.port.status.sprite_overflow == false)
                        && (g_test.video.port.status.sprite_0_hit == false)
                        && (g_test.video.port.status.vertical_blank == false))) {
                    result = NESL_SUCCESS;
                    goto exit;
                }
            }

            if((scanline == 241) && (cycle == 1)) {

                if(NESL_ASSERT((g_test.video.port.status.sprite_overflow == false)
                        && (g_test.video.port.status.sprite_0_hit == false)
                        && (g_test.video.port.status.vertical_blank == true)
                        && (g_test.bus.int_type == NESL_INTERRUPT_NON_MASKABLE))) {
                    result = NESL_SUCCESS;
                    goto exit;
                }
            }

            if((scanline > 0) && (scanline < 240) && (cycle == 260)) {

                if(NESL_ASSERT(g_test.bus.int_type == NESL_INTERRUPT_MAPPER)) {
                    result = NESL_SUCCESS;
                    goto exit;
                }
            }

            ++cycles;
        }
    }

    if(NESL_ASSERT(cycles == 89342)) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestVideoInit(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if((result = NESL_TestInit(false, NESL_MIRROR_VERTICAL)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_VideoInit(&g_test.video, &g_test.bus.mirror) == NESL_SUCCESS)
            && (g_test.video.cycle == 0)
            && (g_test.video.scanline == -1)
            && (*g_test.video.mirror == NESL_MIRROR_VERTICAL)
            && (g_test.video.address.v.word == 0)
            && (g_test.video.address.t.word == 0)
            && (g_test.video.address.fine_x == 0)
            && (g_test.video.port.latch == false)
            && (g_test.video.port.control.raw == 0)
            && (g_test.video.port.mask.raw == 0)
            && (g_test.video.port.status.raw == 0)
            && (g_test.video.port.oam_address.word == 0)
            && (g_test.video.port.data.word == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestVideoRead(void)
{
    uint16_t addr, addr_offset;
    int bank = NESL_MIRROR_MAX;
    nesl_error_e result = NESL_SUCCESS;

    if((result = NESL_TestInit(true, NESL_MIRROR_HORIZONTAL)) == NESL_FAILURE) {
        goto exit;
    }

    for(uint16_t address = 0x2000; address <= 0x3FFF; ++address) {
        addr_offset = 0;

        if((g_test.bus.mirror == NESL_MIRROR_HORIZONTAL)
                && (address == 0x3F00)) {
            g_test.bus.mirror = NESL_MIRROR_VERTICAL;
            address = 0x2000;
            NESL_VideoReset(&g_test.video, &g_test.bus.mirror);
        }

        switch(address) {
            case 0x3000 ... 0x3EFF:
                addr_offset = 0x1000;
            case 0x2000 ... 0x2FFF:

                if((result = NESL_TestVideoNametableAddress(address - addr_offset, g_test.bus.mirror, &bank, &addr)) == NESL_FAILURE) {
                    goto exit;
                }

                if(NESL_ASSERT(addr < 0x0400)) {
                    result = NESL_FAILURE;
                    goto exit;
                }

                g_test.video.ram.nametable[bank & 1][addr & 0x03FF] = address & 0xFF;

                if(NESL_ASSERT(NESL_VideoRead(&g_test.video, address - addr_offset) == ((address - addr_offset) & 0xFF))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x3F00 ... 0x3FFF:

                if(NESL_ASSERT((addr = NESL_TestVideoPaletteAddress(address)) < 0x20)) {
                    result = NESL_FAILURE;
                    goto exit;
                }

                g_test.video.ram.palette[addr & 0x1F] = address & 0x1F;

                if(NESL_ASSERT(NESL_VideoRead(&g_test.video, address) == (address & 0x1F))) {
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

static nesl_error_e NESL_TestVideoReadPort(void)
{
    static const NESL_TestPort TEST_PORT[] = {
        NESL_TestVideoGetPortUnused, NESL_TestVideoGetPortUnused, NESL_TestVideoGetPortStatus, NESL_TestVideoGetPortUnused,
        NESL_TestVideoGetPortOamData, NESL_TestVideoGetPortUnused, NESL_TestVideoGetPortUnused, NESL_TestVideoGetPortData,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(uint16_t address = 0x2000; address <= 0x3FFF; ++address) {

        if((result = NESL_TestInit(true, NESL_MIRROR_HORIZONTAL)) == NESL_FAILURE) {
            goto exit;
        }

        if((result = TEST_PORT[address & 7](address)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestVideoReset(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if((result = NESL_TestInit(true, NESL_MIRROR_HORIZONTAL)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.bus.mirror = NESL_MIRROR_VERTICAL;
    NESL_VideoReset(&g_test.video, &g_test.bus.mirror);

    if(NESL_ASSERT((g_test.video.cycle == 0)
            && (g_test.video.scanline == -1)
            && (*g_test.video.mirror == NESL_MIRROR_VERTICAL)
            && (g_test.video.address.v.word == 0)
            && (g_test.video.address.t.word == 0)
            && (g_test.video.address.fine_x == 0)
            && (g_test.video.port.latch == false)
            && (g_test.video.port.control.raw == 0)
            && (g_test.video.port.mask.raw == 0)
            && (g_test.video.port.status.raw == 0)
            && (g_test.video.port.oam_address.word == 0)
            && (g_test.video.port.data.word == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestVideoUninit(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if((result = NESL_TestInit(true, 0)) == NESL_FAILURE) {
        goto exit;
    }

    NESL_VideoUninit(&g_test.video);

    if(NESL_ASSERT((g_test.video.cycle == 0)
            && (g_test.video.scanline == 0)
            && (g_test.video.mirror == NULL)
            && (g_test.video.address.v.word == 0)
            && (g_test.video.address.t.word == 0)
            && (g_test.video.address.fine_x == 0)
            && (g_test.video.port.latch == false)
            && (g_test.video.port.control.raw == 0)
            && (g_test.video.port.mask.raw == 0)
            && (g_test.video.port.status.raw == 0)
            && (g_test.video.port.oam_address.word == 0)
            && (g_test.video.port.data.word == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestVideoWrite(void)
{
    uint16_t addr, addr_offset;
    int bank = NESL_MIRROR_MAX;
    nesl_error_e result = NESL_SUCCESS;

    if((result = NESL_TestInit(true, NESL_MIRROR_HORIZONTAL)) == NESL_FAILURE) {
        goto exit;
    }

    for(uint16_t address = 0x2000; address <= 0x3FFF; ++address) {
        addr_offset = 0;

        if((g_test.bus.mirror == NESL_MIRROR_HORIZONTAL)
                && (address == 0x3F00)) {
            g_test.bus.mirror = NESL_MIRROR_VERTICAL;
            address = 0x2000;
            NESL_VideoReset(&g_test.video, &g_test.bus.mirror);
        }

        switch(address) {
            case 0x3000 ... 0x3EFF:
                addr_offset = 0x1000;
            case 0x2000 ... 0x2FFF:

                if((result = NESL_TestVideoNametableAddress(address - addr_offset, g_test.bus.mirror, &bank, &addr)) == NESL_FAILURE) {
                    goto exit;
                }

                if(NESL_ASSERT(addr < 0x0400)) {
                    result = NESL_FAILURE;
                    goto exit;
                }

                NESL_VideoWrite(&g_test.video, address - addr_offset, (address - addr_offset) & 0xFF);

                if(NESL_ASSERT(g_test.video.ram.nametable[bank & 1][addr & 0x03FF] == ((address - addr_offset) & 0xFF))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x3F00 ... 0x3FFF:

                if(NESL_ASSERT((addr = NESL_TestVideoPaletteAddress(address)) < 0x20)) {
                    result = NESL_FAILURE;
                    goto exit;
                }

                NESL_VideoWrite(&g_test.video, address, address & 0x1F);

                if(NESL_ASSERT(g_test.video.ram.palette[addr & 0x1F] == (address & 0x1F))) {
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

static nesl_error_e NESL_TestVideoWritePort(void)
{
    static const NESL_TestPort TEST_PORT[] = {
        NESL_TestVideoSetPortControl, NESL_TestVideoSetPortMask, NESL_TestVideoSetPortUnused, NESL_TestVideoSetPortOamAddress,
        NESL_TestVideoSetPortOamData, NESL_TestVideoSetPortScroll, NESL_TestVideoSetPortAddress, NESL_TestVideoSetPortData,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(uint16_t address = 0x2000; address <= 0x3FFF; ++address) {

        if((result = NESL_TestInit(true, NESL_MIRROR_HORIZONTAL)) == NESL_FAILURE) {
            goto exit;
        }

        if((result = TEST_PORT[address & 7](address)) == NESL_FAILURE) {
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
        NESL_TestVideoCycle, NESL_TestVideoInit, NESL_TestVideoRead, NESL_TestVideoReadPort,
        NESL_TestVideoReset, NESL_TestVideoUninit, NESL_TestVideoWrite, NESL_TestVideoWritePort,
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
