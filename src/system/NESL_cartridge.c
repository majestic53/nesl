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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static int NESL_CartridgeValidate(const void *data, int length)
{
    int expected = sizeof(nesl_cartridge_header_t), result = NESL_SUCCESS;
    const nesl_cartridge_header_t *header = (const nesl_cartridge_header_t *)data;

    if(!data) {
        result = NESL_SET_ERROR("Invalid data -- %p", data);
        goto exit;
    }

    if(length < expected) {
        result = NESL_SET_ERROR("Invalid length -- %.02f KB (%i bytes), expecting %.02f KB (%i bytes)", length / (float)1024, length, expected / (float)1024, expected);
        goto exit;
    }

    if(strncmp(header->magic, "NES\x1A", 4)) {
        result = NESL_SET_ERROR("Malformed header -- %s", "String mismatch");
        goto exit;
    }

    if(header->flag_7.version) {
        result = NESL_SET_ERROR("Unsupported header version -- %u", header->flag_7.version);
        goto exit;
    }

    if(header->flag_7.vs_unisystem || header->flag_7.playchoice_10) {
        result = NESL_SET_ERROR("Unsupported header type -- %s", header->flag_7.vs_unisystem ? "VS-Unisystem" : "PlayChoice-10");
        goto exit;
    }

    if(header->flag_6.trainer) {
        expected += 512;
    }

    expected += (header->rom.program * 16 * 1024) + (header->rom.character * 8 * 1024);

    if(length != expected) {
        result = NESL_SET_ERROR("Invalid length -- %.02f KB (%i bytes), expecting %.02f KB (%i bytes)", length / (float)1024, length, expected / (float)1024, expected);
        goto exit;
    }

exit:
    return result;
}

uint8_t NESL_CartridgeGetBankCount(nesl_cartridge_t *cartridge, int type)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_PROGRAM_RAM:
            result = cartridge->header->ram.program;
            break;
        case NESL_BANK_CHARACTER_ROM:
            result = cartridge->header->rom.character;
            break;
        case NESL_BANK_PROGRAM_ROM:
            result = cartridge->header->rom.program;
            break;
        default:
            break;
    }

    return result;
}

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
    int result;
    const uint8_t *ptr = data;

    if((result = NESL_CartridgeValidate(data, length)) == NESL_FAILURE) {
        goto exit;
    }

    cartridge->header = (const nesl_cartridge_header_t *)ptr;
    ptr += sizeof(*cartridge->header);

    if(cartridge->header->flag_6.trainer) {
        ptr += 512;
    }

    if(cartridge->header->rom.program) {
        cartridge->rom.program = ptr;
        ptr += (cartridge->header->rom.program * 16 * 1024);
    }

    if(cartridge->header->rom.character) {
        cartridge->rom.character = ptr;
        ptr += (cartridge->header->rom.character * 8 * 1024 );
    } else {

        if(!(cartridge->ram.character = calloc(32 * 8 * 1024, sizeof(uint8_t)))) {
            result = NESL_SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", 8, 8 * 1024);
            goto exit;
        }

        cartridge->rom.character = cartridge->ram.character;
    }

    if(!(cartridge->ram.program = calloc((cartridge->header->ram.program ? cartridge->header->ram.program : 1) * 8 * 1024, sizeof(uint8_t)))) {
        result = NESL_SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", (cartridge->header->ram.program ? cartridge->header->ram.program : 1) * 8,
            (cartridge->header->ram.program ? cartridge->header->ram.program : 1) * 8 * 1024);
        goto exit;
    }

    memset(cartridge->ram.program, 0, (cartridge->header->ram.program ? cartridge->header->ram.program : 1) * 8 * 1024);

exit:
    return result;
}

uint8_t NESL_CartridgeReadRam(nesl_cartridge_t *cartridge, int type, uint32_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_PROGRAM_RAM:
            result = cartridge->ram.program[address];
            break;
        default:
            break;
    }

    return result;
}

uint8_t NESL_CartridgeReadRom(nesl_cartridge_t *cartridge, int type, uint32_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_CHARACTER_ROM:
            result = cartridge->rom.character[address];
            break;
        case NESL_BANK_PROGRAM_ROM:
            result = cartridge->rom.program[address];
            break;
        default:
            break;
    }

    return result;
}

void NESL_CartridgeUninit(nesl_cartridge_t *cartridge)
{

    if(cartridge->ram.character) {
        free(cartridge->ram.character);
        cartridge->ram.character = NULL;
    }

    if(cartridge->ram.program) {
        free(cartridge->ram.program);
        cartridge->ram.program = NULL;
    }

    memset(cartridge, 0, sizeof(*cartridge));
}

void NESL_CartridgeWriteRam(nesl_cartridge_t *cartridge, int type, uint32_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_CHARACTER_RAM:
            cartridge->ram.character[address] = data;
            break;
        case NESL_BANK_PROGRAM_RAM:
            cartridge->ram.program[address] = data;
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
