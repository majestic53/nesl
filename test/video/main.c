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
 * @brief Test application for video subsystem.
 */

#include <cartridge.h>
#include <video.h>
#include <test.h>

/*!
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_video_t video;             /*!< Video context */

    struct {
        nesl_bus_e type;            /*!< Bus type */
        uint16_t address;           /*!< Bus address */
        uint8_t data[16 * 1024];    /*!< Bus data */
        nesl_interrupt_e int_type;  /*!< Bus interrupt */
        nesl_mirror_e mirror;       /*!< Bus mirror */
    } bus;
} nesl_test_t;

/*!
 * @brief Video port test function.
 * @param[in] address Test address
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
typedef nesl_error_e (*test_port)(uint16_t address);

static nesl_test_t g_test = {};     /*!< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e nesl_bus_interrupt(nesl_interrupt_e type)
{
    g_test.bus.int_type = type;

    return NESL_SUCCESS;
}

uint8_t nesl_bus_read(nesl_bus_e type, uint16_t address)
{
    uint8_t result = 0;

    g_test.bus.address = address;
    g_test.bus.type = type;

    switch(g_test.bus.type) {
        case BUS_VIDEO:
            result = g_test.bus.data[g_test.bus.address];
            break;
        default:
            break;
    }

    return result;
}

void nesl_bus_write(nesl_bus_e type, uint16_t address, uint8_t data)
{
    g_test.bus.address = address;
    g_test.bus.type = type;

    switch(g_test.bus.type) {
        case BUS_VIDEO:
            g_test.bus.data[g_test.bus.address] = data;
            break;
        default:
            break;
    }
}

void nesl_service_set_pixel(uint8_t color, bool red_emphasis, bool green_emphasis, bool blue_emphasis, uint8_t x, uint8_t y)
{
    return;
}

/*!
 * @brief Test video subsystem get data port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_get_port_data(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    for(int increment = 0; increment <= 1; ++increment) {
        uint8_t data = 0;
        g_test.video.port.control.increment = increment;

        for(uint16_t addr = 0x0000; addr <= 0x3FFF; addr += (g_test.video.port.control.increment ? 32 : 1), ++data) {
            uint8_t value;

            if(ASSERT(g_test.video.address.v.word == addr)) {
                result = NESL_FAILURE;
                goto exit;
            }

            g_test.video.port.data.low = data + 1;
            g_test.bus.data[g_test.video.address.v.word] = data;
            value = nesl_video_read_port(&g_test.video, address);

            if(ASSERT((g_test.bus.address == addr)
                    && (g_test.bus.type == BUS_VIDEO))) {
                result = NESL_FAILURE;
                goto exit;
            }

            switch(addr) {
                case 0x3F00 ... 0x3FFF:

                    if(ASSERT(value == data)
                            && (g_test.video.port.data.low == data)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                    break;
                default:

                    if(ASSERT((value != data)
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

/*!
 * @brief Test video subsystem get OAM-data port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_get_port_oam_data(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    ((uint8_t *)g_test.video.ram.oam)[g_test.video.port.oam_address.low] = g_test.video.port.oam_address.low;

    if(ASSERT(nesl_video_read_port(&g_test.video, address) == g_test.video.port.oam_address.low)) {
        result = NESL_FAILURE;
        goto exit;
    }

    ++g_test.video.port.oam_address.low;

exit:
    return result;
}

/*!
 * @brief Test video subsystem get status port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_get_port_status(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    g_test.video.port.data.low = 0x0A;
    g_test.video.port.status.sprite_overflow = true;
    g_test.video.port.status.sprite_0_hit = true;
    g_test.video.port.status.vertical_blank = true;
    g_test.video.port.latch = true;

    if(ASSERT((nesl_video_read_port(&g_test.video, address) == 0xEA)
            && (g_test.video.port.status.raw == 0x60)
            && (g_test.video.port.latch == false))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

/*!
 * @brief Test video subsystem get unused port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_get_port_unused(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    g_test.video.port.data.low = 0xAC;

    if(ASSERT(nesl_video_read_port(&g_test.video, address) == 0xAC)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

/*!
 * @brief Initialize test context.
 * @param[in] mirror Mapper mirror
 * @param[in] initialize Initialize video
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_initialize(nesl_mirror_e mirror, bool initialize)
{
    nesl_error_e result = NESL_SUCCESS;

    memset(&g_test, 0, sizeof(g_test));
    g_test.bus.mirror = mirror;

    if(initialize) {

        if((result = nesl_video_initialize(&g_test.video, &g_test.bus.mirror)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    return result;
}

/*!
 * @brief Calculate video nametable address.
 * @param[in] address Desired address
 * @param[in] mirror Mapper mirror
 * @param[out] bank Calcuated bank
 * @param[out] addr Calculated address
 * @return Calculated address
 */
static nesl_error_e nesl_test_video_nametable_address(uint16_t address, nesl_mirror_e mirror, int *bank, uint16_t *addr)
{
    nesl_error_e result = NESL_SUCCESS;

    switch(mirror) {
        case MIRROR_HORIZONTAL:

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
                    ASSERT((address >= 0x2000) && (address <= 0x2FFF));
                    result = NESL_FAILURE;
                    goto exit;
            }
            break;
        case MIRROR_VERTICAL:

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
                    ASSERT((address >= 0x2000) && (address <= 0x2FFF));
                    result = NESL_FAILURE;
                    goto exit;
            }
            break;
        case MIRROR_ONE_LOW:
            *bank = 0;
            break;
        case MIRROR_ONE_HIGH:
            *bank = 1;
            break;
        default:
            ASSERT(mirror < MIRROR_MAX);
            result = NESL_FAILURE;
            goto exit;
    }

    *addr = (address & 0x03FF);

exit:
    return result;
}

/*!
 * @brief Calcluate video palette address.
 * @param[in] address Desired address
 * @return Calculated address
 */
static uint16_t nesl_test_video_palette_address(uint16_t address)
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

/*!
 * @brief Test video subsystem set address port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_set_port_address(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_video_write_port(&g_test.video, address, address & 0xFF);

    if(ASSERT((g_test.video.address.t.high == (address & 0x3F))
            && (g_test.video.port.latch == true))) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_video_write_port(&g_test.video, address, (address + 1) & 0xFF);

    if(ASSERT((g_test.video.address.t.low == ((address + 1) & 0xFF))
            && (g_test.video.address.v.word == g_test.video.address.t.word)
            && (g_test.video.port.latch == false))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

/*!
 * @brief Test video subsystem set control port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_set_port_control(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_video_write_port(&g_test.video, address, address & 0xFF);

    if(ASSERT((g_test.video.port.control.raw == (address & 0xFF))
            && (g_test.video.address.t.nametable_x == g_test.video.port.control.nametable_x)
            && (g_test.video.address.t.nametable_y == g_test.video.port.control.nametable_y))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

/*!
 * @brief Test video subsystem set data port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_set_port_data(uint16_t address)
{
    uint16_t addr;
    nesl_error_e result = NESL_SUCCESS;

    for(int increment = 0; increment <= 1; ++increment) {
        uint8_t data = 0;
        g_test.video.port.control.increment = increment;

        for(addr = 0x0000; addr <= 0x3FFF; addr += (g_test.video.port.control.increment ? 32 : 1), ++data) {

            if(ASSERT(g_test.video.address.v.word == addr)) {
                result = NESL_FAILURE;
                goto exit;
            }

            nesl_video_write_port(&g_test.video, address, data);

            if(ASSERT((g_test.bus.data[addr] == data)
                    && (g_test.bus.address == addr)
                    && (g_test.bus.type == BUS_VIDEO))) {
                result = NESL_FAILURE;
                goto exit;
            }
        }

        g_test.video.address.v.word = 0;
    }

exit:
    return result;
}

/*!
 * @brief Test video subsystem set mask port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_set_port_mask(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_video_write_port(&g_test.video, address, address & 0xFF);

    if(ASSERT(g_test.video.port.mask.raw == (address & 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

/*!
 * @brief Test video subsystem set OAM-address port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_set_port_oam_address(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_video_write_port(&g_test.video, address, address & 0xFF);

    if(ASSERT(g_test.video.port.oam_address.low == (address & 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

/*!
 * @brief Test video subsystem set OAM-data port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_set_port_oam_data(uint16_t address)
{
    uint16_t addr;
    nesl_error_e result = NESL_SUCCESS;

    addr = g_test.video.port.oam_address.low;
    g_test.video.port.status.vertical_blank = false;
    nesl_video_write_port(&g_test.video, address, address & 0xFF);

    if(ASSERT((((uint8_t *)g_test.video.ram.oam)[addr] == (address & 0xFF))
            && (g_test.video.port.oam_address.low == ((addr + 1) & 0xFF)))) {
        result = NESL_FAILURE;
        goto exit;
    }

    addr = g_test.video.port.oam_address.low;
    g_test.video.port.status.vertical_blank = true;
    nesl_video_write_port(&g_test.video, address, address & 0xFF);

    if(ASSERT((((uint8_t *)g_test.video.ram.oam)[addr] == (address & 0xFF))
            && (g_test.video.port.oam_address.low == (addr & 0xFF)))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

/*!
 * @brief Test video subsystem set scroll port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_set_port_scroll(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_video_write_port(&g_test.video, address, address & 0xFF);

    if(ASSERT((g_test.video.address.t.coarse_x == ((address & 0xFF) >> 3))
            && (g_test.video.address.fine_x == ((address & 0xFF) & 7))
            && (g_test.video.port.latch == true))) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_video_write_port(&g_test.video, address, (address + 1) & 0xFF);

    if(ASSERT((g_test.video.address.t.coarse_y == (((address + 1) & 0xFF) >> 3))
            && (g_test.video.address.t.fine_y == (((address + 1) & 0xFF) & 7))
            && (g_test.video.port.latch == false))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

/*!
 * @brief Test video subsystem set unused port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_set_port_unused(uint16_t address)
{
    nesl_error_e result = NESL_SUCCESS;

    nesl_video_write_port(&g_test.video, address, 0xAC);

    if(ASSERT(g_test.video.port.data.low == 0xAC)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    return result;
}

/*!
 * @brief Test video subsystem cycle.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_cycle(void)
{
    uint64_t cycles = 0;
    nesl_error_e result = NESL_SUCCESS;

    if((result = nesl_test_initialize(MIRROR_HORIZONTAL, true)) == NESL_FAILURE) {
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

            if(ASSERT((g_test.video.scanline == scanline)
                    && (g_test.video.cycle == cycle))) {
                result = NESL_SUCCESS;
                goto exit;
            }

            nesl_video_cycle(&g_test.video);

            if((scanline == -1) && (cycle == 1)) {

                if(ASSERT((g_test.video.port.status.sprite_overflow == false)
                        && (g_test.video.port.status.sprite_0_hit == false)
                        && (g_test.video.port.status.vertical_blank == false))) {
                    result = NESL_SUCCESS;
                    goto exit;
                }
            }

            if((scanline == 241) && (cycle == 1)) {

                if(ASSERT((g_test.video.port.status.sprite_overflow == false)
                        && (g_test.video.port.status.sprite_0_hit == false)
                        && (g_test.video.port.status.vertical_blank == true)
                        && (g_test.bus.int_type == INTERRUPT_NON_MASKABLE))) {
                    result = NESL_SUCCESS;
                    goto exit;
                }
            }

            if((scanline > 0) && (scanline < 240) && (cycle == 260)) {

                if(ASSERT(g_test.bus.int_type == INTERRUPT_MAPPER)) {
                    result = NESL_SUCCESS;
                    goto exit;
                }
            }

            ++cycles;
        }
    }

    if(ASSERT(cycles == 89342)) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test video subsystem initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_initialize(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if((result = nesl_test_initialize(MIRROR_VERTICAL, false)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_video_initialize(&g_test.video, &g_test.bus.mirror) == NESL_SUCCESS)
            && (g_test.video.cycle == 0)
            && (g_test.video.scanline == -1)
            && (*g_test.video.mirror == MIRROR_VERTICAL)
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
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test video subsystem read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_read(void)
{
    uint16_t addr, addr_offset;
    int bank = MIRROR_MAX;
    nesl_error_e result = NESL_SUCCESS;

    if((result = nesl_test_initialize(MIRROR_HORIZONTAL, true)) == NESL_FAILURE) {
        goto exit;
    }

    for(uint16_t address = 0x2000; address <= 0x3FFF; ++address) {
        addr_offset = 0;

        if((g_test.bus.mirror == MIRROR_HORIZONTAL)
                && (address == 0x3F00)) {
            g_test.bus.mirror = MIRROR_VERTICAL;
            address = 0x2000;
            nesl_video_reset(&g_test.video, &g_test.bus.mirror);
        }

        switch(address) {
            case 0x3000 ... 0x3EFF:
                addr_offset = 0x1000;
            case 0x2000 ... 0x2FFF:

                if((result = nesl_test_video_nametable_address(address - addr_offset, g_test.bus.mirror, &bank, &addr)) == NESL_FAILURE) {
                    goto exit;
                }

                if(ASSERT(addr < 0x0400)) {
                    result = NESL_FAILURE;
                    goto exit;
                }

                g_test.video.ram.nametable[bank & 1][addr & 0x03FF] = address & 0xFF;

                if(ASSERT(nesl_video_read(&g_test.video, address - addr_offset) == ((address - addr_offset) & 0xFF))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x3F00 ... 0x3FFF:

                if(ASSERT((addr = nesl_test_video_palette_address(address)) < 0x20)) {
                    result = NESL_FAILURE;
                    goto exit;
                }

                g_test.video.ram.palette[addr & 0x1F] = address & 0x1F;

                if(ASSERT(nesl_video_read(&g_test.video, address) == (address & 0x1F))) {
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
 * @brief Test video subsystem read port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_read_port(void)
{
    const test_port TEST_PORT[] = {
        nesl_test_video_get_port_unused, nesl_test_video_get_port_unused, nesl_test_video_get_port_status, nesl_test_video_get_port_unused,
        nesl_test_video_get_port_oam_data, nesl_test_video_get_port_unused, nesl_test_video_get_port_unused, nesl_test_video_get_port_data,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(uint16_t address = 0x2000; address <= 0x3FFF; ++address) {

        if((result = nesl_test_initialize(MIRROR_HORIZONTAL, true)) == NESL_FAILURE) {
            goto exit;
        }

        if((result = TEST_PORT[address & 7](address)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test video subsystem reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_reset(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if((result = nesl_test_initialize(MIRROR_HORIZONTAL, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.bus.mirror = MIRROR_VERTICAL;
    nesl_video_reset(&g_test.video, &g_test.bus.mirror);

    if(ASSERT((g_test.video.cycle == 0)
            && (g_test.video.scanline == -1)
            && (*g_test.video.mirror == MIRROR_VERTICAL)
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
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test video subsystem uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_uninitialize(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if((result = nesl_test_initialize(0, true)) == NESL_FAILURE) {
        goto exit;
    }

    nesl_video_uninitialize(&g_test.video);

    if(ASSERT((g_test.video.cycle == 0)
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
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test video subsystem write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_write(void)
{
    int bank = MIRROR_MAX;
    uint16_t addr, addr_offset;
    nesl_error_e result = NESL_SUCCESS;

    if((result = nesl_test_initialize(MIRROR_HORIZONTAL, true)) == NESL_FAILURE) {
        goto exit;
    }

    for(uint16_t address = 0x2000; address <= 0x3FFF; ++address) {
        addr_offset = 0;

        if((g_test.bus.mirror == MIRROR_HORIZONTAL)
                && (address == 0x3F00)) {
            g_test.bus.mirror = MIRROR_VERTICAL;
            address = 0x2000;
            nesl_video_reset(&g_test.video, &g_test.bus.mirror);
        }

        switch(address) {
            case 0x3000 ... 0x3EFF:
                addr_offset = 0x1000;
            case 0x2000 ... 0x2FFF:

                if((result = nesl_test_video_nametable_address(address - addr_offset, g_test.bus.mirror, &bank, &addr)) == NESL_FAILURE) {
                    goto exit;
                }

                if(ASSERT(addr < 0x0400)) {
                    result = NESL_FAILURE;
                    goto exit;
                }

                nesl_video_write(&g_test.video, address - addr_offset, (address - addr_offset) & 0xFF);

                if(ASSERT(g_test.video.ram.nametable[bank & 1][addr & 0x03FF] == ((address - addr_offset) & 0xFF))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x3F00 ... 0x3FFF:

                if(ASSERT((addr = nesl_test_video_palette_address(address)) < 0x20)) {
                    result = NESL_FAILURE;
                    goto exit;
                }

                nesl_video_write(&g_test.video, address, address & 0x1F);

                if(ASSERT(g_test.video.ram.palette[addr & 0x1F] == (address & 0x1F))) {
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
 * @brief Test video subsystem write port.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_video_write_port(void)
{
    const test_port TEST_PORT[] = {
        nesl_test_video_set_port_control, nesl_test_video_set_port_mask, nesl_test_video_set_port_unused, nesl_test_video_set_port_oam_address,
        nesl_test_video_set_port_oam_data, nesl_test_video_set_port_scroll, nesl_test_video_set_port_address, nesl_test_video_set_port_data,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(uint16_t address = 0x2000; address <= 0x3FFF; ++address) {

        if((result = nesl_test_initialize(MIRROR_HORIZONTAL, true)) == NESL_FAILURE) {
            goto exit;
        }

        if((result = TEST_PORT[address & 7](address)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

int main(void)
{
    const test TEST[] = {
        nesl_test_video_cycle, nesl_test_video_initialize, nesl_test_video_read, nesl_test_video_read_port,
        nesl_test_video_reset, nesl_test_video_uninitialize, nesl_test_video_write, nesl_test_video_write_port,
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
