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
 * @brief Test application for cartridge subsystem.
 */

#include <cartridge.h>
#include <test.h>

/*!
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_cartridge_t cartridge;         /*!< Cartridge context */

    struct {
        nesl_cartridge_header_t header; /*!< Cartridge header */
        uint8_t program[2][16 * 1024];  /*!< Program banks */
        uint8_t character[1][8 * 1024]; /*!< Character banks */
    } data;
} nesl_test_t;

static nesl_test_t g_test = {};         /*!< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e nesl_set_error(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

/*!
 * @brief Uninitialize test context.
 */
static void nesl_test_uninitialize(void)
{
    nesl_cartridge_uninitialize(&g_test.cartridge);
    memset(&g_test, 0, sizeof(g_test));
}

/*!
 * @brief Initialize test context.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_initialize(void)
{
    int bank;
    nesl_error_e result;

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

/*!
 * @brief Test cartridge subsystem bank count.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_cartridge_get_banks(void)
{
    nesl_error_e result = NESL_FAILURE;

    for(nesl_bank_e type = 0; type < BANK_MAX; ++type) {
        uint8_t data = 0;

        if((result = nesl_test_initialize()) == NESL_FAILURE) {
            goto exit;
        }

        switch(type) {
            case BANK_CHARACTER_RAM:
                break;
            case BANK_PROGRAM_RAM:
                g_test.data.header.ram.program = 11;
                data = g_test.data.header.ram.program;
                break;
            case BANK_CHARACTER_ROM:
                g_test.data.header.rom.character = 22;
                data = g_test.data.header.rom.character;
                break;
            case BANK_PROGRAM_ROM:
                g_test.data.header.rom.program = 33;
                data = g_test.data.header.rom.program;
                break;
            default:
                break;
        }

        if(ASSERT(nesl_cartridge_get_banks(&g_test.cartridge, type) == data)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test cartridge subsystem mapper.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_cartridge_get_mapper(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize()) == NESL_FAILURE) {
        goto exit;
    }

    g_test.data.header.flag_6.type_low = MAPPER_4 & 0x0F;
    g_test.data.header.flag_7.type_high = (MAPPER_4 & 0xF0) >> 4;

    if(ASSERT(nesl_cartridge_get_mapper(&g_test.cartridge) == MAPPER_4)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize()) == NESL_FAILURE) {
        goto exit;
    }

    g_test.data.header.flag_6.type_low = MAPPER_66 & 0x0F;
    g_test.data.header.flag_7.type_high = (MAPPER_66 & 0xF0) >> 4;

    if(ASSERT(nesl_cartridge_get_mapper(&g_test.cartridge) == MAPPER_66)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test cartridge subsystem mirror.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_cartridge_get_mirror(void)
{
    nesl_error_e result = NESL_FAILURE;

    for(nesl_mirror_e type = 0; type < MIRROR_ONE_LOW; ++type) {

        if((result = nesl_test_initialize()) == NESL_FAILURE) {
            goto exit;
        }

        g_test.data.header.flag_6.mirror = type;

        if(ASSERT(nesl_cartridge_get_mirror(&g_test.cartridge) == type)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test cartridge subsystem read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_cartridge_read(void)
{
    int bank;
    nesl_error_e result = NESL_SUCCESS;

    for(bank = 0; bank < 1; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {

            if(ASSERT(nesl_cartridge_read_ram(&g_test.cartridge, BANK_PROGRAM_RAM, (bank * 8 * 1024) + address) == data++)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

    for(bank = 0; bank < 1; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {

            if(ASSERT(nesl_cartridge_read_rom(&g_test.cartridge, BANK_CHARACTER_ROM, (bank * 8 * 1024) + address) == data++)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

    for(bank = 0; bank < 2; ++bank) {
        uint8_t data = 0;

        for(uint16_t address = 0; address < (16 * 1024); ++address) {

            if(ASSERT(nesl_cartridge_read_rom(&g_test.cartridge, BANK_PROGRAM_ROM, (bank * 16 * 1024) + address) == data++)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test cartridge subsystem write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_cartridge_write(void)
{
    uint8_t data = 0xFF;
    nesl_error_e result = NESL_SUCCESS;

    g_test.cartridge.ram.character = g_test.data.character[0];

    for(uint16_t address = 0; address < (8 * 1024); ++address) {
        nesl_cartridge_write_ram(&g_test.cartridge, BANK_CHARACTER_RAM, address, data);

        if(ASSERT(g_test.cartridge.ram.character[address] == data--)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    g_test.cartridge.ram.character = NULL;

    for(int bank = 0; bank < 1; ++bank) {
        data = 0xFF;

        for(uint16_t address = 0; address < (8 * 1024); ++address) {
            nesl_cartridge_write_ram(&g_test.cartridge, BANK_PROGRAM_RAM, (bank * 8 * 1024) + address, data);

            if(ASSERT(g_test.cartridge.ram.program[(bank * 8 * 1024) + address] == data--)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

int main(void)
{
    const test TEST[] = {
        nesl_test_cartridge_get_banks, nesl_test_cartridge_get_mapper, nesl_test_cartridge_get_mirror, nesl_test_cartridge_read,
        nesl_test_cartridge_write,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < TEST_COUNT(TEST); ++index) {

        if(nesl_test_initialize() == NESL_FAILURE) {
            result = NESL_FAILURE;
            continue;
        }

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    nesl_test_uninitialize();

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
