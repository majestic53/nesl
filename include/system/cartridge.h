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

#ifndef NESL_CARTRIDGE_H_
#define NESL_CARTRIDGE_H_

#include "../common.h"

enum {
    NESL_RAM_CHARACTER = 0,
    NESL_RAM_PROGRAM,
    NESL_ROM_CHARACTER,
    NESL_ROM_PROGRAM,
    NESL_ROM_MAX,
};

typedef struct {
    char magic[4];

    struct {
        uint8_t program;
        uint8_t character;
    } rom;

    union {

        struct {
            uint8_t mirror : 1;
            uint8_t ram : 1;
            uint8_t trainer : 1;
            uint8_t four_screen : 1;
            uint8_t type_low : 4;
        };

        uint8_t raw;
    } flag_6;

    union {

        struct {
            uint8_t vs_unisystem : 1;
            uint8_t playchoice_10 : 1;
            uint8_t version : 2;
            uint8_t type_high : 4;
        };

        uint8_t raw;
    } flag_7;

    struct {
        uint8_t program;
    } ram;

    uint8_t unused[7];
} nesl_header_t;

typedef struct {
    const nesl_header_t *header;

    struct {
        uint8_t *character;
        uint8_t *program;
    } ram;

    struct {
        const uint8_t *character;
        const uint8_t *program;
    } rom;
} nesl_cartridge_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int nesl_cartridge_initialize(nesl_cartridge_t *cartridge, const void *data, int length);

uint8_t nesl_cartridge_ram_read(nesl_cartridge_t *cartridge, int type, uint32_t address);

void nesl_cartridge_ram_write(nesl_cartridge_t *cartridge, int type, uint32_t address, uint8_t data);

uint8_t nesl_cartridge_rom_read(nesl_cartridge_t *cartridge, int type, uint32_t address);

void nesl_cartridge_uninitialize(nesl_cartridge_t *cartridge);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_CARTRIDGE_H_ */
