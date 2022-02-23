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

#include "../../include/system/NESL_cartridge.h"
#include "../include/NESL_common.h"

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

int NESL_SetError(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

static void NESL_TestUninit(void)
{
    NESL_CartridgeUninit(&g_test.cartridge);
    memset(&g_test, 0, sizeof(g_test));
}

static int NESL_TestInit(void)
{
    int bank, result;

    NESL_TestUninit();
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

    if((result = NESL_CartridgeInit(&g_test.cartridge, &g_test.data.header, sizeof(g_test.data))) == NESL_FAILURE) {
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

static int NESL_TestCartridgeGetBankCount(void)
{
    int result = NESL_FAILURE;

    for(uint8_t type = 0; type < NESL_BANK_MAX; ++type) {
        uint8_t data = 0;

        if((result = NESL_TestInit()) == NESL_FAILURE) {
            goto exit;
        }

        switch(type) {
            case NESL_BANK_RAM_CHARACTER:
                break;
            case NESL_BANK_RAM_PROGRAM:
                g_test.data.header.ram.program = 11;
                data = g_test.data.header.ram.program;
                break;
            case NESL_BANK_ROM_CHARACTER:
                g_test.data.header.rom.character = 22;
                data = g_test.data.header.rom.character;
                break;
            case NESL_BANK_ROM_PROGRAM:
                g_test.data.header.rom.program = 33;
                data = g_test.data.header.rom.program;
                break;
            default:
                break;
        }

        if(NESL_ASSERT(NESL_CartridgeGetBankCount(&g_test.cartridge, type) == data)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestCartridgeGetMapper(void)
{
    int result;

    if((result = NESL_TestInit()) == NESL_FAILURE) {
        goto exit;
    }

    g_test.data.header.flag_6.type_low = NESL_MAPPER_4 & 0x0F;
    g_test.data.header.flag_7.type_high = (NESL_MAPPER_4 & 0xF0) >> 4;

    if(NESL_ASSERT(NESL_CartridgeGetMapper(&g_test.cartridge) == NESL_MAPPER_4)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit()) == NESL_FAILURE) {
        goto exit;
    }

    g_test.data.header.flag_6.type_low = NESL_MAPPER_66 & 0x0F;
    g_test.data.header.flag_7.type_high = (NESL_MAPPER_66 & 0xF0) >> 4;

    if(NESL_ASSERT(NESL_CartridgeGetMapper(&g_test.cartridge) == NESL_MAPPER_66)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestCartridgeGetMirror(void)
{
    int result = NESL_FAILURE;

    for(uint8_t type = 0; type < NESL_MIRROR_ONE_LOW; ++type) {

        if((result = NESL_TestInit()) == NESL_FAILURE) {
            goto exit;
        }

        g_test.data.header.flag_6.mirror = type;

        if(NESL_ASSERT(NESL_CartridgeGetMirror(&g_test.cartridge) == type)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestCartridgeRead(void)
{
    int bank, result = NESL_SUCCESS;

    for(bank = 0; bank < 1; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {

            if(NESL_ASSERT(NESL_CartridgeRamRead(&g_test.cartridge, NESL_BANK_RAM_PROGRAM, (bank * 8 * 1024) + address) == data++)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

    for(bank = 0; bank < 1; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {

            if(NESL_ASSERT(NESL_CartridgeRomRead(&g_test.cartridge, NESL_BANK_ROM_CHARACTER, (bank * 8 * 1024) + address) == data++)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

    for(bank = 0; bank < 2; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (16 * 1024); ++address) {

            if(NESL_ASSERT(NESL_CartridgeRomRead(&g_test.cartridge, NESL_BANK_ROM_PROGRAM, (bank * 16 * 1024) + address) == data++)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestCartridgeWrite(void)
{
    uint8_t data = 0xFF;
    int result = NESL_SUCCESS;

    g_test.cartridge.ram.character = g_test.data.character[0];

    for(uint16_t address = 0; address < (8 * 1024); ++address) {
        NESL_CartridgeRamWrite(&g_test.cartridge, NESL_BANK_RAM_CHARACTER, address, data);

        if(NESL_ASSERT(g_test.cartridge.ram.character[address] == data--)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    g_test.cartridge.ram.character = NULL;

    for(int bank = 0; bank < 1; ++bank) {
        data = 0xFF;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {
            NESL_CartridgeRamWrite(&g_test.cartridge, NESL_BANK_RAM_PROGRAM, (bank * 8 * 1024) + address, data);

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

static const NESL_Test TEST[] = {
    NESL_TestCartridgeGetBankCount,
    NESL_TestCartridgeGetMapper,
    NESL_TestCartridgeGetMirror,
    NESL_TestCartridgeRead,
    NESL_TestCartridgeWrite,
    };

int main(void)
{
    int result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(NESL_TestInit() == NESL_FAILURE) {
            result = NESL_FAILURE;
            continue;
        }

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    NESL_TestUninit();

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
