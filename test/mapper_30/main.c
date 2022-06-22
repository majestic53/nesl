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
 * @brief Test application for mapper 30 extension.
 */

#include <mapper_30.h>
#include <test.h>

/*!
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_mapper_t mapper;                   /*!< Mapper type */
    nesl_interrupt_e int_type;              /*!< Mapper interrupt type */
    nesl_bank_e type;                       /*!< Bank type */
    uint32_t address;                       /*!< Bank address */
    uint8_t data;                           /*!< Bank data */

    struct {

        struct {
            uint8_t character[8 * 1024];    /*!< Character RAM */
            uint8_t program[8 * 1024];      /*!< Program RAM */
        } ram;

        struct {
            uint8_t character[8 * 1024];    /*!< Character ROM */
            uint8_t program[2 * 16 * 1024]; /*!< Program ROM */
        } rom;
    } cartridge;
} nesl_test_t;

static nesl_test_t g_test = {};             /*!< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint8_t nesl_cartridge_get_banks(nesl_cartridge_t *cartridge, nesl_bank_e type)
{
    return g_test.mapper.cartridge.header->rom.program;
}

nesl_mirror_e nesl_cartridge_get_mirror(nesl_cartridge_t *cartridge)
{
    return (nesl_mirror_e)cartridge->header->flag_6.mirror;
}

uint8_t nesl_cartridge_read_ram(nesl_cartridge_t *cartridge, nesl_bank_e type, uint32_t address)
{
    g_test.address = address;

    switch((g_test.type = type)) {
        case BANK_PROGRAM_RAM:
            g_test.data = g_test.cartridge.ram.program[g_test.address & 0xFFFF];
            break;
        default:
            break;
    }

    return g_test.data;
}

uint8_t nesl_cartridge_read_rom(nesl_cartridge_t *cartridge, nesl_bank_e type, uint32_t address)
{
    g_test.address = address;

    switch((g_test.type = type)) {
        case BANK_CHARACTER_ROM:
            g_test.data = g_test.cartridge.rom.character[g_test.address & 0xFFFF];
            break;
        case BANK_PROGRAM_ROM:
            g_test.data = g_test.cartridge.rom.program[g_test.address & 0xFFFF];
            break;
        default:
            break;
    }

    return g_test.data;
}

void nesl_cartridge_write_ram(nesl_cartridge_t *cartridge, nesl_bank_e type, uint32_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;

    switch((g_test.type = type)) {
        case BANK_CHARACTER_RAM:
            g_test.cartridge.ram.character[g_test.address & 0xFFFF] = g_test.data;
            break;
        case BANK_PROGRAM_RAM:
            g_test.cartridge.ram.program[g_test.address & 0xFFFF] = g_test.data;
            break;
        default:
            break;
    }
}

nesl_error_e nesl_set_error(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

/*!
 * @brief Uninitialize test context.
 */
static void nesl_test_uninitialize(void)
{
    nesl_mapper_30_uninitialize(&g_test.mapper);
    g_test.mapper.extension.interrupt = NULL;
    g_test.mapper.extension.read_ram = NULL;
    g_test.mapper.extension.read_rom = NULL;
    g_test.mapper.extension.reset = NULL;
    g_test.mapper.extension.write_ram = NULL;
    g_test.mapper.extension.write_rom = NULL;
}

/*!
 * @brief Initialize test context.
 * @param header Pointer to cartridge header
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_initialize(const nesl_cartridge_header_t *header)
{
    nesl_test_uninitialize();
    memset(&g_test, 0, sizeof(g_test));
    g_test.mapper.cartridge.header = header;
    g_test.mapper.extension.interrupt = &nesl_mapper_30_interrupt;
    g_test.mapper.extension.read_ram = &nesl_mapper_30_read_ram;
    g_test.mapper.extension.read_rom = &nesl_mapper_30_read_rom;
    g_test.mapper.extension.reset = &nesl_mapper_30_reset;
    g_test.mapper.extension.write_ram = &nesl_mapper_30_write_ram;
    g_test.mapper.extension.write_rom = &nesl_mapper_30_write_rom;

    return nesl_mapper_30_initialize(&g_test.mapper);
}

/*!
 * @brief Test mapper-30 extension initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_30_initialize(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    header.rom.program = 2;

    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((g_test.mapper.ram.program == 0)
            && (g_test.mapper.rom.character[0] == 0)
            && (g_test.mapper.rom.program[0] == 0)
            && (g_test.mapper.rom.program[1] == 16 * 1024)
            && (g_test.mapper.mirror == MIRROR_HORIZONTAL)
            && (g_test.mapper.context != NULL)
            && (g_test.mapper.extension.interrupt == &nesl_mapper_30_interrupt)
            && (g_test.mapper.extension.read_ram == &nesl_mapper_30_read_ram)
            && (g_test.mapper.extension.read_rom == &nesl_mapper_30_read_rom)
            && (g_test.mapper.extension.reset == &nesl_mapper_30_reset)
            && (g_test.mapper.extension.write_ram == &nesl_mapper_30_write_ram)
            && (g_test.mapper.extension.write_rom == &nesl_mapper_30_write_rom))) {
        result = NESL_FAILURE;
        goto exit;
    }

    header.rom.program = 4;

    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((g_test.mapper.ram.program == 0)
            && (g_test.mapper.rom.character[0] == 0)
            && (g_test.mapper.rom.program[0] == 0)
            && (g_test.mapper.rom.program[1] == 3 * 16 * 1024)
            && (g_test.mapper.mirror == MIRROR_HORIZONTAL)
            && (g_test.mapper.context != NULL)
            && (g_test.mapper.extension.interrupt == &nesl_mapper_30_interrupt)
            && (g_test.mapper.extension.read_ram == &nesl_mapper_30_read_ram)
            && (g_test.mapper.extension.read_rom == &nesl_mapper_30_read_rom)
            && (g_test.mapper.extension.reset == &nesl_mapper_30_reset)
            && (g_test.mapper.extension.write_ram == &nesl_mapper_30_write_ram)
            && (g_test.mapper.extension.write_rom == &nesl_mapper_30_write_rom))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper-30 extension interrupt.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_30_interrupt(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_mapper_30_interrupt(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper-30 extension RAM read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_30_read_ram(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x6000 ... 0x7FFF:

                for(nesl_bank_e type = 0; type < BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 1, .rom.character = 2 };

                    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    if(type == BANK_PROGRAM_RAM) {
                        g_test.cartridge.ram.program[address & 0x1FFF] = data;

                        if(ASSERT((nesl_mapper_30_read_ram(&g_test.mapper, type, address) == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    } else if(ASSERT((g_test.data == 0)
                            && (g_test.type == 0)
                            && (g_test.address == 0))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;
            default:

                if(ASSERT((g_test.data == 0)
                        && (g_test.type == 0)
                        && (g_test.address == 0))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper-30 extension ROM read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_30_read_rom(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x0000 ... 0x1FFF:

                for(nesl_bank_e type = 0; type < BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 1 };

                    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    if(type == BANK_CHARACTER_ROM) {
                        g_test.cartridge.rom.character[address & 0x1FFF] = data;

                        if(ASSERT((nesl_mapper_30_read_rom(&g_test.mapper, type, address) == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    } else if(ASSERT((g_test.data == 0)
                            && (g_test.type == 0)
                            && (g_test.address == 0))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;
            case 0x8000 ... 0xFFFF:

                for(nesl_bank_e type = 0; type < BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 1 };

                    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    if(type == BANK_PROGRAM_ROM) {
                        g_test.cartridge.rom.program[address & 0x7FFF] = data;

                        if(ASSERT((nesl_mapper_30_read_rom(&g_test.mapper, type, address) == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x7FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    } else if(ASSERT((g_test.data == 0)
                            && (g_test.type == 0)
                            && (g_test.address == 0))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;
            default:

                if(ASSERT((g_test.data == 0)
                        && (g_test.type == 0)
                        && (g_test.address == 0))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper-30 extension reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_30_reset(void)
{
    nesl_mapper_30_t *context;
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_mapper_30_reset(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    context = (nesl_mapper_30_t *)g_test.mapper.context;

    if(ASSERT((g_test.mapper.rom.program[0] == context->bank.program * 16 * 1024)
            && (g_test.mapper.rom.program[1] == (g_test.mapper.cartridge.header->rom.program * 16 * 1024) - (1 * 16 * 1024))
            && (g_test.mapper.rom.character[0] == context->bank.character * 8 * 1024)
            && (g_test.mapper.mirror == (context->bank.one_screen ? MIRROR_ONE_LOW : g_test.mapper.cartridge.header->flag_6.mirror)))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper-30 extension uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_30_uninitialize(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
        goto exit;
    }

    nesl_mapper_30_uninitialize(&g_test.mapper);

    if(ASSERT(g_test.mapper.context == NULL)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper-30 extension RAM write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_30_write_ram(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x6000 ... 0x7FFF:

                for(nesl_bank_e type = 0; type < BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 1, .rom.character = 2 };

                    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    nesl_mapper_30_write_ram(&g_test.mapper, type, address, data);

                    if(type == BANK_PROGRAM_RAM) {

                        if(ASSERT((g_test.data == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    } else if(ASSERT((g_test.data == 0)
                            && (g_test.type == 0)
                            && (g_test.address == 0))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;
            default:

                if(ASSERT((g_test.data == 0)
                        && (g_test.type == 0)
                        && (g_test.address == 0))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper-30 extension ROM write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_30_write_rom(void)
{
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address) {
        nesl_cartridge_header_t header = { .rom.program = 4, .rom.character = 2 };

        switch(address) {
            case 0x8000 ... 0xFFFF:


                if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
                    goto exit;
                }

                for(uint8_t one_screen = 0; one_screen < 1; ++one_screen) {

                    for(uint8_t program = 0; program < 4; ++program) {

                        for(uint8_t character = 0; character< 2; ++character) {
                            nesl_mapper_30_bank_t bank = { .character = character, .one_screen = one_screen, .program = program };

                            nesl_mapper_30_write_rom(&g_test.mapper, BANK_PROGRAM_ROM, address, bank.raw);

                            if(ASSERT((g_test.mapper.rom.character[0] == (character * 8 * 1024))
                                    && (g_test.mapper.rom.program[1] == (3 * 16 * 1024))
                                    && (g_test.mapper.rom.program[0] == (program * 16 * 1024))
                                    && (g_test.mapper.mirror == (one_screen ? MIRROR_ONE_LOW : MIRROR_HORIZONTAL)))) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                        }
                    }
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

int main(void)
{
    const test TEST[] = {
        nesl_test_mapper_30_initialize, nesl_test_mapper_30_interrupt, nesl_test_mapper_30_read_ram, nesl_test_mapper_30_read_rom,
        nesl_test_mapper_30_reset, nesl_test_mapper_30_uninitialize, nesl_test_mapper_30_write_ram, nesl_test_mapper_30_write_rom,
        };

    int result = NESL_SUCCESS;

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
