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

#include "../../include/system/NESL_mapper.h"
#include "../include/NESL_common.h"

typedef struct {
    nesl_mapper_t mapper;
    int type;
    uint16_t address;
    uint8_t data;
    bool interrupt;
    bool reset;

    struct {
        const nesl_cartridge_header_t *header;
        const void *data;
        int length;
        bool initialized;
        int status;
    } cartridge;

    struct {
        bool initialized;
        int status;
    } state;
} nesl_test_t;

static nesl_test_t g_test = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


uint8_t NESL_CartridgeGetMapper(nesl_cartridge_t *cartridge)
{
    return (cartridge->header->flag_7.type_high << 4) | cartridge->header->flag_6.type_low;
}

uint8_t NESL_CartridgeGetMirror(nesl_cartridge_t *cartridge)
{
    return cartridge->header->flag_6.mirror;
}

int NESL_CartridgeInit(nesl_cartridge_t *cartridge, const void *data, int length)
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


int NESL_Mapper0Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper0Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

int NESL_Mapper1Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper1Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

int NESL_Mapper2Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper2Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

int NESL_Mapper3Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper3Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

int NESL_Mapper30Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper30Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

int NESL_Mapper4Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper4Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

int NESL_Mapper66Init(nesl_mapper_t *mapper)
{
    g_test.state.initialized = (g_test.state.status == NESL_SUCCESS);

    return g_test.state.status;
}

void NESL_Mapper66Uninit(nesl_mapper_t *mapper)
{
    g_test.state.initialized = false;
}

int NESL_SetError(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

static int NESL_TestInterruptHandler(nesl_mapper_t *mapper)
{
    g_test.interrupt = true;

    return NESL_SUCCESS;
}

static uint8_t NESL_TestReadHandler(nesl_mapper_t *mapper, int type, uint16_t address)
{
    g_test.type = type;
    g_test.address = address;

    return g_test.data;
}

static int NESL_TestResetHandler(nesl_mapper_t *mapper)
{
    g_test.reset = true;

    return NESL_SUCCESS;
}

static void NESL_TestWriteHandler(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    g_test.type = type;
    g_test.address = address;
    g_test.data = data;
}

static void NESL_TestInit(nesl_cartridge_header_t *header, int type)
{
    memset(&g_test, 0, sizeof(g_test));
    header->flag_6.type_low = type & 0x0F;
    header->flag_7.type_high = (type & 0xF0) >> 4;
    g_test.cartridge.header = header;
    g_test.mapper.cartridge.header = g_test.cartridge.header;
    g_test.mapper.callback.interrupt = &NESL_TestInterruptHandler;
    g_test.mapper.callback.ram_read = &NESL_TestReadHandler;
    g_test.mapper.callback.ram_write = &NESL_TestWriteHandler;
    g_test.mapper.callback.reset = &NESL_TestResetHandler;
    g_test.mapper.callback.rom_read = &NESL_TestReadHandler;
    g_test.mapper.callback.rom_write = &NESL_TestWriteHandler;
}

static int NESL_TestMapperInit(void)
{
    int result = NESL_SUCCESS;
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

static int NESL_TestMapperInterrupt(void)
{
    int result = NESL_SUCCESS;
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

static int NESL_TestMapperRead(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        for(int type = 0; type < NESL_BANK_MAX; ++type) {
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

static int NESL_TestMapperReset(void)
{
    int result = NESL_SUCCESS;
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

static int NESL_TestMapperUninit(void)
{
    nesl_cartridge_header_t header = {};
    int result = NESL_SUCCESS;

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

static int NESL_TestMapperWrite(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address) {

        for(int type = 0; type < NESL_BANK_MAX; ++type, ++data) {
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

static const NESL_Test TEST[] = {
    NESL_TestMapperInit,
    NESL_TestMapperInterrupt,
    NESL_TestMapperRead,
    NESL_TestMapperReset,
    NESL_TestMapperUninit,
    NESL_TestMapperWrite,
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
