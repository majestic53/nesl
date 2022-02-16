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

#include "../../include/system/cartridge.h"
#include "../include/common.h"

typedef struct {
    nesl_cartridge_t cartridge;

    struct {
        nesl_header_t header;
        uint8_t program[2][16 * 1024];
        uint8_t character[1][8 * 1024];
    } data;
} nesl_test_t;

static nesl_test_t g_test = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int nesl_error_set(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

static void nesl_test_uninitialize(void)
{
    nesl_cartridge_uninitialize(&g_test.cartridge);
    memset(&g_test, 0, sizeof(g_test));
}

static int nesl_test_initialize(void)
{
    int bank, result;

    nesl_test_uninitialize();
    memcpy(g_test.data.header.magic, "NES\x1A", 4);
    g_test.data.header.rom.program = 2;
    g_test.data.header.rom.character = 1;

    for(bank = 0; bank < 2; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (16 * 1024); ++address) {
            g_test.data.program[bank][address] = data++;
        }
    }

    for(bank = 0; bank < 1; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {
            g_test.data.character[bank][address] = data++;
        }
    }

    if((result = nesl_cartridge_initialize(&g_test.cartridge, &g_test.data.header, sizeof(g_test.data))) == NESL_FAILURE) {
        goto exit;
    }

    for(bank = 0; bank < 1; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {
            g_test.cartridge.ram.program[(bank * 8 * 1024) + address] = data++;
        }
    }

exit:
    return result;
}

static int nesl_test_cartridge_read(void)
{
    int bank, result = NESL_SUCCESS;

    for(bank = 0; bank < 1; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {

            if(NESL_ASSERT(nesl_cartridge_ram_read(&g_test.cartridge, NESL_BANK_RAM_PROGRAM, (bank * 8 * 1024) + address) == data++)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

    for(bank = 0; bank < 1; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {

            if(NESL_ASSERT(nesl_cartridge_rom_read(&g_test.cartridge, NESL_BANK_ROM_CHARACTER, (bank * 8 * 1024) + address) == data++)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

    for(bank = 0; bank < 2; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (16 * 1024); ++address) {

            if(NESL_ASSERT(nesl_cartridge_rom_read(&g_test.cartridge, NESL_BANK_ROM_PROGRAM, (bank * 16 * 1024) + address) == data++)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_cartridge_write(void)
{
    uint8_t data = 0xFF;
    int result = NESL_SUCCESS;

    g_test.cartridge.ram.character = g_test.data.character[0];

    for(uint16_t address = 0; address < (8 * 1024); ++address) {
        nesl_cartridge_ram_write(&g_test.cartridge, NESL_BANK_RAM_CHARACTER, address, data);

        if(NESL_ASSERT(g_test.cartridge.ram.character[address] == data--)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    g_test.cartridge.ram.character = NULL;

    for(int bank = 0; bank < 1; ++bank) {
        data = 0xFF;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {
            nesl_cartridge_ram_write(&g_test.cartridge, NESL_BANK_RAM_PROGRAM, (bank * 8 * 1024) + address, data);

            if(NESL_ASSERT(g_test.cartridge.ram.program[(bank * 8 * 1024) + address] == data--)) {
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
    nesl_test_cartridge_read,
    nesl_test_cartridge_write,
    };

int main(void)
{
    int result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(nesl_test_initialize() == NESL_FAILURE) {
            result = NESL_FAILURE;
            continue;
        }

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    nesl_test_uninitialize();

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
