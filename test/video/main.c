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

#include "../../include/system/NESL_video.h"
#include "../include/NESL_common.h"

typedef struct {
    nesl_video_t video;

    struct {
        uint16_t address;
        int type;
        uint8_t data[16 * 1024];
        int int_type;
        int mirror;
    } bus;
} nesl_test_t;

typedef int (*NESL_TestPort)(uint16_t address);

static nesl_test_t g_test = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_BusInterrupt(int type)
{
    g_test.bus.int_type = type;

    return NESL_SUCCESS;
}

uint8_t NESL_BusRead(int type, uint16_t address)
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

void NESL_BusWrite(int type, uint16_t address, uint8_t data)
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

static int NESL_TestInit(bool initialize, int mirror)
{
    int result = NESL_SUCCESS;

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

static int NESL_TestVideoNametableAddress(uint16_t address, int mirror, int *bank, uint16_t *addr)
{
    int result = NESL_SUCCESS;

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

static int NESL_TestVideoCycle(void)
{
    uint64_t cycles = 0;
    int result = NESL_SUCCESS;

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

static int NESL_TestVideoInit(void)
{
    int result = NESL_SUCCESS;

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

static int NESL_TestVideoPortGetData(uint16_t address)
{
    int result = NESL_SUCCESS;

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
            value = NESL_VideoPortRead(&g_test.video, address);

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

static int NESL_TestVideoPortGetOamData(uint16_t address)
{
    int result = NESL_SUCCESS;

    ((uint8_t *)g_test.video.ram.oam)[g_test.video.port.oam_address.low] = g_test.video.port.oam_address.low;

    if(NESL_ASSERT(NESL_VideoPortRead(&g_test.video, address) == g_test.video.port.oam_address.low)) {
        result = NESL_FAILURE;
        goto exit;
    }

    ++g_test.video.port.oam_address.low;

exit:
    return result;
}

static int NESL_TestVideoPortGetStatus(uint16_t address)
{
    int result = NESL_SUCCESS;

    g_test.video.port.data.low = 0x0A;
    g_test.video.port.status.sprite_overflow = true;
    g_test.video.port.status.sprite_0_hit = true;
    g_test.video.port.status.vertical_blank = true;
    g_test.video.port.latch = true;

    if(NESL_ASSERT((NESL_VideoPortRead(&g_test.video, address) == 0xEA)
            && (g_test.video.port.status.raw == 0x60)
            && (g_test.video.port.latch == false))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static int NESL_TestVideoPortGetUnused(uint16_t address)
{
    int result = NESL_SUCCESS;

    g_test.video.port.data.low = 0xAC;

    if(NESL_ASSERT(NESL_VideoPortRead(&g_test.video, address) == 0xAC)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static const NESL_TestPort TEST_PORT_GET[] = {
    NESL_TestVideoPortGetUnused,
    NESL_TestVideoPortGetUnused,
    NESL_TestVideoPortGetStatus,
    NESL_TestVideoPortGetUnused,
    NESL_TestVideoPortGetOamData,
    NESL_TestVideoPortGetUnused,
    NESL_TestVideoPortGetUnused,
    NESL_TestVideoPortGetData,
    };

static int NESL_TestVideoPortRead(void)
{
    int result = NESL_SUCCESS;

    for(uint16_t address = 0x2000; address <= 0x3FFF; ++address) {

        if((result = NESL_TestInit(true, NESL_MIRROR_HORIZONTAL)) == NESL_FAILURE) {
            goto exit;
        }

        if((result = TEST_PORT_GET[address & 7](address)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestVideoPortSetAddress(uint16_t address)
{
    int result = NESL_SUCCESS;

    NESL_VideoPortWrite(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT((g_test.video.address.t.high == (address & 0x3F))
            && (g_test.video.port.latch == true))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_VideoPortWrite(&g_test.video, address, (address + 1) & 0xFF);

    if(NESL_ASSERT((g_test.video.address.t.low == ((address + 1) & 0xFF))
            && (g_test.video.address.v.word == g_test.video.address.t.word)
            && (g_test.video.port.latch == false))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static int NESL_TestVideoPortSetControl(uint16_t address)
{
    int result = NESL_SUCCESS;

    NESL_VideoPortWrite(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT((g_test.video.port.control.raw == (address & 0xFF))
            && (g_test.video.address.t.nametable_x == g_test.video.port.control.nametable_x)
            && (g_test.video.address.t.nametable_y == g_test.video.port.control.nametable_y))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static int NESL_TestVideoPortSetData(uint16_t address)
{
    uint16_t addr;
    int result = NESL_SUCCESS;

    for(int increment = 0; increment <= 1; ++increment) {
        uint8_t data = 0;
        g_test.video.port.control.increment = increment;

        for(addr = 0x0000; addr <= 0x3FFF; addr += (g_test.video.port.control.increment ? 32 : 1), ++data) {

            if(NESL_ASSERT(g_test.video.address.v.word == addr)) {
                result = NESL_FAILURE;
                goto exit;
            }

            NESL_VideoPortWrite(&g_test.video, address, data);

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

static int NESL_TestVideoPortSetMask(uint16_t address)
{
    int result = NESL_SUCCESS;

    NESL_VideoPortWrite(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT(g_test.video.port.mask.raw == (address & 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static int NESL_TestVideoPortSetOamAddress(uint16_t address)
{
    int result = NESL_SUCCESS;

    NESL_VideoPortWrite(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT(g_test.video.port.oam_address.low == (address & 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static int NESL_TestVideoPortSetOamData(uint16_t address)
{
    uint16_t addr;
    int result = NESL_SUCCESS;

    addr = g_test.video.port.oam_address.low;
    g_test.video.port.status.vertical_blank = false;
    NESL_VideoPortWrite(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT((((uint8_t *)g_test.video.ram.oam)[addr] == (address & 0xFF))
            && (g_test.video.port.oam_address.low == ((addr + 1) & 0xFF)))) {
        result = NESL_FAILURE;
        goto exit;
    }

    addr = g_test.video.port.oam_address.low;
    g_test.video.port.status.vertical_blank = true;
    NESL_VideoPortWrite(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT((((uint8_t *)g_test.video.ram.oam)[addr] == (address & 0xFF))
            && (g_test.video.port.oam_address.low == (addr & 0xFF)))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static int NESL_TestVideoPortSetScroll(uint16_t address)
{
    int result = NESL_SUCCESS;

    NESL_VideoPortWrite(&g_test.video, address, address & 0xFF);

    if(NESL_ASSERT((g_test.video.address.t.coarse_x == ((address & 0xFF) >> 3))
            && (g_test.video.address.fine_x == ((address & 0xFF) & 7))
            && (g_test.video.port.latch == true))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_VideoPortWrite(&g_test.video, address, (address + 1) & 0xFF);

    if(NESL_ASSERT((g_test.video.address.t.coarse_y == (((address + 1) & 0xFF) >> 3))
            && (g_test.video.address.t.fine_y == (((address + 1) & 0xFF) & 7))
            && (g_test.video.port.latch == false))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static int NESL_TestVideoPortSetUnused(uint16_t address)
{
    int result = NESL_SUCCESS;

    NESL_VideoPortWrite(&g_test.video, address, 0xAC);

    if(NESL_ASSERT(g_test.video.port.data.low == 0xAC)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

static const NESL_TestPort TEST_PORT_SET[] = {
    NESL_TestVideoPortSetControl,
    NESL_TestVideoPortSetMask,
    NESL_TestVideoPortSetUnused,
    NESL_TestVideoPortSetOamAddress,
    NESL_TestVideoPortSetOamData,
    NESL_TestVideoPortSetScroll,
    NESL_TestVideoPortSetAddress,
    NESL_TestVideoPortSetData,
    };

static int NESL_TestVideoPortWrite(void)
{
    int result = NESL_SUCCESS;

    for(uint16_t address = 0x2000; address <= 0x3FFF; ++address) {

        if((result = NESL_TestInit(true, NESL_MIRROR_HORIZONTAL)) == NESL_FAILURE) {
            goto exit;
        }

        if((result = TEST_PORT_SET[address & 7](address)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestVideoRead(void)
{
    uint16_t addr, addr_offset;
    int bank = NESL_MIRROR_MAX, result = NESL_SUCCESS;

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

static int NESL_TestVideoReset(void)
{
    int result = NESL_SUCCESS;

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

static int NESL_TestVideoUninit(void)
{
    int result = NESL_SUCCESS;

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

static int NESL_TestVideoWrite(void)
{
    uint16_t addr, addr_offset;
    int bank = NESL_MIRROR_MAX, result = NESL_SUCCESS;

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

static const NESL_Test TEST[] = {
    NESL_TestVideoCycle,
    NESL_TestVideoInit,
    NESL_TestVideoPortRead,
    NESL_TestVideoPortWrite,
    NESL_TestVideoRead,
    NESL_TestVideoReset,
    NESL_TestVideoUninit,
    NESL_TestVideoWrite,
    };

int main(void)
{
    int result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
