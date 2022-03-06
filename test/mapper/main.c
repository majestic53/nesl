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
 * @brief Test application for mapper subsystem.
 */

#include "../../include/system/NESL_mapper.h"
#include "../include/NESL_common.h"

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_mapper_t mapper;   /*< Mapper context */
    nesl_bank_e type;       /*< Bank type */
    uint16_t address;       /*< Bank address */
    uint8_t data;           /*< Bank data */
    bool interrupt;         /*< Interrupt state */
    bool reset;             /*< Reset state */

    struct {
        const nesl_cartridge_header_t *header;  /*< Cartridge header */
        const void *data;                       /*< Cartridge data */
        int length;                             /*< Cartridge data length in bytes */
        bool initialized;                       /*< Cartridge initialized state */
        nesl_error_e status;                    /*< Cartridge error state */
    } cartridge;

    struct {
        bool initialized;       /*< Mapper extension initialized state */
        nesl_error_e status;    /*< Mapper extension error state */
    } state;
} nesl_test_t;

static nesl_test_t g_test = {}; /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_mapper_e NESL_CartridgeGetMapper(nesl_cartridge_t *cartridge)
{
    return (nesl_mapper_e)((cartridge->header->flag_7.type_high << 4) | cartridge->header->flag_6.type_low);
}

nesl_mirror_e NESL_CartridgeGetMirror(nesl_cartridge_t *cartridge)
{
    return (nesl_mirror_e)cartridge->header->flag_6.mirror;
}

nesl_error_e NESL_CartridgeInit(nesl_cartridge_t *cartridge, const void *data, int length)
{
    g_test.cartridge.data = data;
    g_test.cartridge.length = length;
    g_test.cartridge.initialized = (g_test.cartridge.status == NESL_SUCCESS);

    return g_test.cartridge.status;
}

void NESL_CartridgeUninit(nesl_cartridge_t *cartridge)
{
    g_test.cartridge.initialized = false;
}


nesl_error_e NESL_Mapper0Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper0Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e NESL_Mapper1Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper1Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e NESL_Mapper2Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper2Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e NESL_Mapper3Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper3Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e NESL_Mapper30Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper30Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e NESL_Mapper4Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper4Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e NESL_Mapper66Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper66Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

nesl_error_e NESL_SetError(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

static nesl_error_e NESL_TestInterruptHandler(nesl_mapper_t *mapper)
{
    g_test.interrupt = true;

    return NESL_SUCCESS;
}

static uint8_t NESL_TestReadHandler(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    g_test.type = type;
    g_test.address = address;

    return g_test.data;
}

static nesl_error_e NESL_TestResetHandler(nesl_mapper_t *mapper)
{
    g_test.reset = true;

    return NESL_SUCCESS;
}

static void NESL_TestWriteHandler(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{
    g_test.type = type;
    g_test.address = address;
    g_test.data = data;
}

static void NESL_TestInit(nesl_cartridge_header_t *header, nesl_mapper_e type)
{
    memset(&g_test, 0, sizeof(g_test));
    header->flag_6.type_low = type & 0x0F;
    header->flag_7.type_high = (type & 0xF0) >> 4;
    g_test.cartridge.header = header;
    g_test.mapper.cartridge.header = g_test.cartridge.header;
    g_test.mapper.callback.interrupt = &NESL_TestInterruptHandler;
    g_test.mapper.callback.read_ram = &NESL_TestReadHandler;
    g_test.mapper.callback.read_rom = &NESL_TestReadHandler;
    g_test.mapper.callback.reset = &NESL_TestResetHandler;
    g_test.mapper.callback.write_ram = &NESL_TestWriteHandler;
    g_test.mapper.callback.write_rom = &NESL_TestWriteHandler;
}

static nesl_error_e NESL_TestMapperInit(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    NESL_TestInit(&header, 0);
    g_test.cartridge.status = NESL_FAILURE;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header, 0);
    g_test.state.status = NESL_FAILURE;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header, 0xFF);

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    memset(&header, 0, sizeof(header));
    NESL_TestInit(&header, NESL_MAPPER_0);

    if(NESL_ASSERT((NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.state.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_0)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestMapperInterrupt(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    NESL_TestInit(&header, 0);
    NESL_MapperInterrupt(&g_test.mapper);

    if(NESL_ASSERT(g_test.interrupt == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header, NESL_MAPPER_0);
    NESL_MapperInterrupt(&g_test.mapper);

    if(NESL_ASSERT(g_test.interrupt == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestMapperRead(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        for(nesl_bank_e type = 0; type < NESL_BANK_MAX; ++type) {
            nesl_cartridge_header_t header = {};

            NESL_TestInit(&header, 0);
            g_test.data = data;

            if(NESL_ASSERT((NESL_MapperRead(&g_test.mapper, type, address) == data)
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

static nesl_error_e NESL_TestMapperReset(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    NESL_TestInit(&header, 0);
    NESL_MapperReset(&g_test.mapper);

    if(NESL_ASSERT(g_test.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header, NESL_MAPPER_0);
    NESL_MapperReset(&g_test.mapper);

    if(NESL_ASSERT(g_test.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestMapperUninit(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    NESL_TestInit(&header, NESL_MAPPER_0);
    NESL_MapperUninit(&g_test.mapper);

    if(NESL_ASSERT(g_test.state.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestMapperWrite(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address) {

        for(nesl_bank_e type = 0; type < NESL_BANK_MAX; ++type, ++data) {
            nesl_cartridge_header_t header = {};

            NESL_TestInit(&header, 0);
            NESL_MapperWrite(&g_test.mapper, type, address, data);

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

int main(void)
{
    static const NESL_Test TEST[] = {
        NESL_TestMapperInit, NESL_TestMapperInterrupt, NESL_TestMapperRead, NESL_TestMapperReset,
        NESL_TestMapperUninit, NESL_TestMapperWrite,
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
