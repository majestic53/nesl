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
 * @brief Test application for mapper subsystem.
 */

#include <mapper.h>
#include <test.h>

/*!
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_mapper_t mapper;                       /*!< Mapper context */
    nesl_bank_e type;                           /*!< Bank type */
    uint16_t address;                           /*!< Bank address */
    uint8_t data;                               /*!< Bank data */
    bool interrupt;                             /*!< Interrupt state */
    bool reset;                                 /*!< Reset state */

    struct {
        const nesl_cartridge_header_t *header;  /*!< Cartridge header */
        const void *data;                       /*!< Cartridge data */
        int length;                             /*!< Cartridge data length in bytes */
        bool initialized;                       /*!< Cartridge initialized state */
        nesl_error_e status;                    /*!< Cartridge error state */
    } cartridge;

    struct {
        bool initialized;                       /*!< Mapper extension initialized state */
        nesl_error_e status;                    /*!< Mapper extension error state */
    } state;
} nesl_test_t;

static nesl_test_t g_test = {};                 /*!< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_mapper_e nesl_cartridge_get_mapper(nesl_cartridge_t *cartridge)
{
    return (nesl_mapper_e)((cartridge->header->flag_7.type_high << 4) | cartridge->header->flag_6.type_low);
}

nesl_mirror_e nesl_cartridge_get_mirror(nesl_cartridge_t *cartridge)
{
    return (nesl_mirror_e)cartridge->header->flag_6.mirror;
}

nesl_error_e nesl_cartridge_initialize(nesl_cartridge_t *cartridge, const void *data, int length)
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


nesl_error_e nesl_mapper_0_initialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void nesl_mapper_0_uninitialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e nesl_mapper_1_initialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void nesl_mapper_1_uninitialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e nesl_mapper_2_initialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void nesl_mapper_2_uninitialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e nesl_mapper_3_initialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void nesl_mapper_3_uninitialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e nesl_mapper_30_initialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void nesl_mapper_30_uninitialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e nesl_mapper_4_initialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void nesl_mapper_4_uninitialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e nesl_mapper_66_initialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void nesl_mapper_66_uninitialize(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e nesl_set_error(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

/*!
 * @brief Test interrupt handler.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_interrupt_handler(nesl_mapper_t *mapper)
{
    g_test.interrupt = true;

    return NESL_SUCCESS;
}

/*!
 * @brief Test read handler.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Mapper type
 * @param[in] address Mapper address
 * @return Mapper data
 */
static uint8_t nesl_test_read_handler(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    g_test.type = type;
    g_test.address = address;

    return g_test.data;
}

/*!
 * @brief Test reset handler.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_reset_handler(nesl_mapper_t *mapper)
{
    g_test.reset = true;

    return NESL_SUCCESS;
}

/*!
 * @brief Test write handler.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Mapper type
 * @param[in] address Mapper address
 * @param[in] data Mapper data
 */
static void nesl_test_write_handler(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{
    g_test.type = type;
    g_test.address = address;
    g_test.data = data;
}

/*!
 * @brief Initialize test context.
 * @param[in] header Pointer to cartridge header
 * @param[in] type Mapper type
 */
static void nesl_test_initialize(nesl_cartridge_header_t *header, nesl_mapper_e type)
{
    memset(&g_test, 0, sizeof(g_test));
    header->flag_6.type_low = type & 0x0F;
    header->flag_7.type_high = (type & 0xF0) >> 4;
    g_test.cartridge.header = header;
    g_test.mapper.cartridge.header = g_test.cartridge.header;
    g_test.mapper.extension.interrupt = &nesl_test_interrupt_handler;
    g_test.mapper.extension.read_ram = &nesl_test_read_handler;
    g_test.mapper.extension.read_rom = &nesl_test_read_handler;
    g_test.mapper.extension.reset = &nesl_test_reset_handler;
    g_test.mapper.extension.write_ram = &nesl_test_write_handler;
    g_test.mapper.extension.write_rom = &nesl_test_write_handler;
}

/*!
 * @brief Test mapper subsystem initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_initialize(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    nesl_test_initialize(&header, 0);
    g_test.cartridge.status = NESL_FAILURE;

    if(ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header, 0);
    g_test.state.status = NESL_FAILURE;

    if(ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header, 0xFF);

    if(ASSERT(nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    memset(&header, 0, sizeof(header));
    nesl_test_initialize(&header, MAPPER_0);

    if(ASSERT((nesl_mapper_initialize(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.state.initialized == true)
            && (g_test.mapper.type == MAPPER_0)
            && (g_test.mapper.mirror == MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper subsystem interrupt.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_interrupt(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    nesl_test_initialize(&header, 0);
    nesl_mapper_interrupt(&g_test.mapper);

    if(ASSERT(g_test.interrupt == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header, MAPPER_0);
    nesl_mapper_interrupt(&g_test.mapper);

    if(ASSERT(g_test.interrupt == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper subsystem read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_read(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        for(nesl_bank_e type = 0; type < BANK_MAX; ++type) {
            nesl_cartridge_header_t header = {};

            nesl_test_initialize(&header, 0);
            g_test.data = data;

            if(ASSERT((nesl_mapper_read(&g_test.mapper, type, address) == data)
                    && (g_test.type == type)
                    && (g_test.address == address))) {
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
 * @brief Test mapper subsystem reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_reset(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    nesl_test_initialize(&header, 0);
    nesl_mapper_reset(&g_test.mapper);

    if(ASSERT(g_test.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_test_initialize(&header, MAPPER_0);
    nesl_mapper_reset(&g_test.mapper);

    if(ASSERT(g_test.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper subsystem uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_uninitialize(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    nesl_test_initialize(&header, MAPPER_0);
    nesl_mapper_uninitialize(&g_test.mapper);

    if(ASSERT(g_test.state.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test mapper subsystem write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_mapper_write(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address) {

        for(nesl_bank_e type = 0; type < BANK_MAX; ++type, ++data) {
            nesl_cartridge_header_t header = {};

            nesl_test_initialize(&header, 0);
            nesl_mapper_write(&g_test.mapper, type, address, data);

            if(ASSERT((g_test.data == data)
                    && (g_test.type == type)
                    && (g_test.address == address))) {
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
        nesl_test_mapper_initialize, nesl_test_mapper_interrupt, nesl_test_mapper_read, nesl_test_mapper_reset,
        nesl_test_mapper_uninitialize, nesl_test_mapper_write,
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
