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

#include "../../include/system/mapper.h"
#include "../include/common.h"

typedef struct {
    nesl_mapper_t mapper;
    int type;
    uint16_t address;
    uint8_t data;
    bool interrupt;
    bool reset;

    struct {
        const nesl_header_t *header;
        const void *data;
        int length;
        bool initialized;
        int status;
    } cartridge;

    struct {
        bool initialized;
        int status;
    } mapper_0;

    struct {
        bool initialized;
        bool reset;
        int status;
    } mapper_1;

    struct {
        bool initialized;
        bool reset;
        int status;
    } mapper_2;

    struct {
        bool initialized;
        bool reset;
        int status;
    } mapper_3;

    struct {
        bool initialized;
        bool interrupt;
        bool reset;
        int status;
    } mapper_4;

    struct {
        bool initialized;
        bool reset;
        int status;
    } mapper_30;

    struct {
        bool initialized;
        bool reset;
        int status;
    } mapper_66;
} nesl_test_t;

static nesl_test_t g_test = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static int interrupt_handler(nesl_mapper_t *mapper)
{
    g_test.interrupt = true;

    return NESL_SUCCESS;
}

static uint8_t read_handler(nesl_mapper_t *mapper, int type, uint16_t address)
{
    g_test.type = type;
    g_test.address = address;

    return g_test.data;
}

static int reset_handler(nesl_mapper_t *mapper)
{
    g_test.reset = true;

    return NESL_SUCCESS;
}

static void write_handler(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    g_test.type = type;
    g_test.address = address;
    g_test.data = data;
}

int nesl_cartridge_initialize(nesl_cartridge_t *cartridge, const void *data, int length)
{
    g_test.cartridge.data = data;
    g_test.cartridge.length = length;
    g_test.cartridge.initialized = (g_test.cartridge.status == NESL_SUCCESS);

    return g_test.cartridge.status;
}

void nesl_cartridge_uninitialize(nesl_cartridge_t *cartridge)
{
    g_test.cartridge.initialized = false;
}

int nesl_error_set(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

int nesl_mapper_0_initialize(nesl_mapper_t *mapper)
{
    g_test.mapper_0.initialized = (g_test.mapper_0.status == NESL_SUCCESS);

    return g_test.mapper_0.status;
}

uint8_t nesl_mapper_0_ram_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_0_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

uint8_t nesl_mapper_0_rom_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

int nesl_mapper_1_initialize(nesl_mapper_t *mapper)
{
    g_test.mapper_1.initialized = (g_test.mapper_1.status == NESL_SUCCESS);

    return g_test.mapper_0.status;
}

uint8_t nesl_mapper_1_ram_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_1_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int nesl_mapper_1_reset(nesl_mapper_t *mapper)
{
    g_test.mapper_1.reset = true;

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_1_rom_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_1_rom_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void nesl_mapper_1_uninitialize(nesl_mapper_t *mapper)
{
    g_test.mapper_1.initialized = false;
}

int nesl_mapper_2_initialize(nesl_mapper_t *mapper)
{
    g_test.mapper_2.initialized = (g_test.mapper_2.status == NESL_SUCCESS);

    return g_test.mapper_2.status;
}

uint8_t nesl_mapper_2_ram_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_2_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int nesl_mapper_2_reset(nesl_mapper_t *mapper)
{
    g_test.mapper_2.reset = true;

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_2_rom_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_2_rom_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void nesl_mapper_2_uninitialize(nesl_mapper_t *mapper)
{
    g_test.mapper_2.initialized = false;
}

int nesl_mapper_3_initialize(nesl_mapper_t *mapper)
{
    g_test.mapper_3.initialized = (g_test.mapper_3.status == NESL_SUCCESS);

    return g_test.mapper_3.status;
}

uint8_t nesl_mapper_3_ram_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_3_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int nesl_mapper_3_reset(nesl_mapper_t *mapper)
{
    g_test.mapper_3.reset = true;

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_3_rom_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_3_rom_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void nesl_mapper_3_uninitialize(nesl_mapper_t *mapper)
{
    g_test.mapper_3.initialized = false;
}

int nesl_mapper_30_initialize(nesl_mapper_t *mapper)
{
    g_test.mapper_30.initialized = (g_test.mapper_30.status == NESL_SUCCESS);

    return g_test.mapper_30.status;
}

uint8_t nesl_mapper_30_ram_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_30_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int nesl_mapper_30_reset(nesl_mapper_t *mapper)
{
    g_test.mapper_30.reset = true;

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_30_rom_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_30_rom_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void nesl_mapper_30_uninitialize(nesl_mapper_t *mapper)
{
    g_test.mapper_30.initialized = false;
}

int nesl_mapper_4_initialize(nesl_mapper_t *mapper)
{
    g_test.mapper_4.initialized = (g_test.mapper_4.status == NESL_SUCCESS);

    return g_test.mapper_4.status;
}

int nesl_mapper_4_interrupt(nesl_mapper_t *mapper)
{
    g_test.mapper_4.interrupt = true;

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_4_ram_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_4_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int nesl_mapper_4_reset(nesl_mapper_t *mapper)
{
    g_test.mapper_4.reset = true;

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_4_rom_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_4_rom_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void nesl_mapper_4_uninitialize(nesl_mapper_t *mapper)
{
    g_test.mapper_4.initialized = false;
}

int nesl_mapper_66_initialize(nesl_mapper_t *mapper)
{
    g_test.mapper_66.initialized = (g_test.mapper_66.status == NESL_SUCCESS);

    return g_test.mapper_66.status;
}

uint8_t nesl_mapper_66_ram_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_66_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int nesl_mapper_66_reset(nesl_mapper_t *mapper)
{
    g_test.mapper_66.reset = true;

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_66_rom_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void nesl_mapper_66_rom_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void nesl_mapper_66_uninitialize(nesl_mapper_t *mapper)
{
    g_test.mapper_66.initialized = false;
}

static void nesl_test_initialize(const nesl_header_t *header)
{
    memset(&g_test, 0, sizeof(g_test));
    g_test.cartridge.header = header;
    g_test.mapper.cartridge.header = g_test.cartridge.header;
    g_test.mapper.interrupt = &interrupt_handler;
    g_test.mapper.ram_read = &read_handler;
    g_test.mapper.ram_write = &write_handler;
    g_test.mapper.reset = &reset_handler;
    g_test.mapper.rom_read = &read_handler;
    g_test.mapper.rom_write = &write_handler;
}

static int nesl_test_mapper_initialize(void)
{
    int result = NESL_SUCCESS;
    nesl_header_t header = {};

    nesl_test_initialize(&header);
    g_test.cartridge.status = NESL_FAILURE;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    g_test.mapper_0.status = NESL_FAILURE;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = 0xF;
    header.flag_7.type_high = 0xF;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    memset(&header, 0, sizeof(header));

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_0;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT((nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_0.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_0)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_1;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT((nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_1.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_1)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_2;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT((nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_2.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_2)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_3;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT((nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_3.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_3)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_30 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_30 & 0xF0) >> 4;

    if(NESL_ASSERT((nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_30.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_30)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_4;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT((nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_4.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_4)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_66 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_66 & 0xF0) >> 4;

    if(NESL_ASSERT((nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_66.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_66)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_mapper_interrupt(void)
{
    int result = NESL_SUCCESS;
    nesl_header_t header = {};

    nesl_test_initialize(&header);
    nesl_mapper_interrupt(&g_test.mapper);

    if(NESL_ASSERT(g_test.interrupt == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_4;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_interrupt(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_4.interrupt == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_mapper_read(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        for(int type = 0; type < NESL_BANK_MAX; ++type) {
            nesl_header_t header = {};

            nesl_test_initialize(&header);
            g_test.data = data;

            if(NESL_ASSERT((nesl_mapper_read(&g_test.mapper, type, address) == data)
                    && (g_test.type == type)
                    && (g_test.address == address))) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_mapper_reset(void)
{
    int result = NESL_SUCCESS;
    nesl_header_t header = {};

    nesl_test_initialize(&header);
    nesl_mapper_reset(&g_test.mapper);

    if(NESL_ASSERT(g_test.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_1;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_reset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_1.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_2;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_reset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_2.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_3;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_reset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_3.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_30 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_30 & 0xF0) >> 4;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_reset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_30.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_4;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_reset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_4.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_66 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_66 & 0xF0) >> 4;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_reset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_66.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_mapper_uninitialize(void)
{
    nesl_header_t header = {};
    int result = NESL_SUCCESS;

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_1;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_uninitialize(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_1.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_2;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_uninitialize(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_2.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_3;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_uninitialize(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_3.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_30 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_30 & 0xF0) >> 4;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_uninitialize(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_30.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_4;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_uninitialize(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_4.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header);
    header.flag_6.type_low = NESL_MAPPER_66 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_66 & 0xF0) >> 4;

    if(NESL_ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_mapper_uninitialize(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_66.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_mapper_write(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address) {

        for(int type = 0; type < NESL_BANK_MAX; ++type, ++data) {
            nesl_header_t header = {};

            nesl_test_initialize(&header);
            nesl_mapper_write(&g_test.mapper, type, address, data);

            if(NESL_ASSERT((g_test.data == data)
                    && (g_test.type == type)
                    && (g_test.address == address))) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static const nesl_test TEST[] = {
    nesl_test_mapper_initialize,
    nesl_test_mapper_interrupt,
    nesl_test_mapper_read,
    nesl_test_mapper_reset,
    nesl_test_mapper_uninitialize,
    nesl_test_mapper_write,
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
