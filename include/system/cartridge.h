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
 * @file cartridge.h
 * @brief Cartridge subsystem.
 */

#ifndef NESL_CARTRIDGE_H_
#define NESL_CARTRIDGE_H_

#include <common.h>

/**
 * @enum nesl_bank_e
 * @brief Bank types.
 */
typedef enum {
    BANK_CHARACTER_RAM = 0,                 /*< Character RAM bank (CHR) */
    BANK_CHARACTER_ROM,                     /*< Character ROM bank (CHR) */
    BANK_PROGRAM_RAM,                       /*< Program RAM bank (PRG) */
    BANK_PROGRAM_ROM,                       /*< Program ROM bank (PRG) */
    BANK_MAX,                               /*< Maximum bank */
} nesl_bank_e;

/**
 * @enum nesl_mapper_e
 * @brief Mapper types.
 */
typedef enum {
    MAPPER_0 = 0,                           /*< Mapper 0 (NROM) */
    MAPPER_1,                               /*< Mapper 1 (MMC1) */
    MAPPER_2,                               /*< Mapper 2 (UxROM) */
    MAPPER_3,                               /*< Mapper 3 (CNROM) */
    MAPPER_4 = 4,                           /*< Mapper 4 (MMC3) */
    MAPPER_30 = 30,                         /*< Mapper 30 (UNROM) */
    MAPPER_66 = 66,                         /*< Mapper 66 (GxROM) */
} nesl_mapper_e;

/**
 * @enum nesl_mirror_e
 * @brief Mirroring types.
 */
typedef enum {
    MIRROR_HORIZONTAL = 0,                  /*< Horizontal mirroring */
    MIRROR_VERTICAL,                        /*< Vertical mirroring */
    MIRROR_ONE_LOW,                         /*< One-Low mirroring */
    MIRROR_ONE_HIGH,                        /*< One-High mirroring */
    MIRROR_MAX,
} nesl_mirror_e;

/**
 * @struct nesl_cartridge_header_t
 * @brief Cartridge header context.
 */
typedef struct {
    char magic[4];                          /*< Magic number */

    struct {
        uint8_t program;                    /*< Program ROM bank count */
        uint8_t character;                  /*< Character ROM bank count */
    } rom;

    union {

        struct {
            uint8_t mirror : 1;             /*< Mirror type (0:Horizontal, 1:Vertical) */
            uint8_t ram : 1;                /*< Program RAM present */
            uint8_t trainer : 1;            /*< Trainer present */
            uint8_t four_screen : 1;        /*< Four-screen flag */
            uint8_t type_low : 4;           /*< Mapper type (low-nibble) */
        };

        uint8_t raw;                        /*< Raw byte */
    } flag_6;

    union {

        struct {
            uint8_t vs_unisystem : 1;       /*< VS-Unisystem flag */
            uint8_t playchoice_10 : 1;      /*< PlayChoice10 flag */
            uint8_t version : 2;            /*< Version (0:iNES1, >=2:iNES2) */
            uint8_t type_high : 4;          /*< Mapper type (high-nibble) */
        };

        uint8_t raw;                        /*< Raw byte */
    } flag_7;

    struct {
        uint8_t program;                    /*< Program RAM bank count */
    } ram;

    uint8_t unused[7];                      /*< Unused bytes */
} nesl_cartridge_header_t;

/**
 * @struct nesl_cartridge_t
 * @brief Cartridge context.
 */
typedef struct {
    const nesl_cartridge_header_t *header;  /*< Constant pointer to cartridge header */

    struct {
        uint8_t *character;                 /*< Pointer to character RAM banks */
        uint8_t *program;                   /*< Pointer to program RAM banks */
    } ram;

    struct {
        const uint8_t *character;           /*< Pointer to character ROM banks */
        const uint8_t *program;             /*< Pointer to program ROM banks */
    } rom;
} nesl_cartridge_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Get cartridge bank count
 * @param cartridge Pointer to cartridge subsystem context
 * @param type Bank type
 * @return Cartridge bank count
 */
uint8_t nesl_cartridge_get_banks(nesl_cartridge_t *cartridge, nesl_bank_e type);

/**
 * @brief Get cartridge mapper type
 * @param cartridge Pointer to cartridge subsystem context
 * @return Cartridge mapper type
 */
nesl_mapper_e nesl_cartridge_get_mapper(nesl_cartridge_t *cartridge);

/**
 * @brief Get cartridge mirror type
 * @param cartridge Pointer to cartridge subsystem context
 * @return Cartridge mirror type
 */
nesl_mirror_e nesl_cartridge_get_mirror(nesl_cartridge_t *cartridge);

/**
 * @brief Initialize cartridge subsystem.
 * @param cartridge Pointer to cartridge subsystem context
 * @param data Constant pointer to cartridge data
 * @param length Cartridge data length in bytes
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_cartridge_initialize(nesl_cartridge_t *cartridge, const void *data, int length);

/**
 * @brief Read byte from cartridge subsystem RAM bank.
 * @param cartridge Pointer to cartridge subsystem context
 * @param type Bank type
 * @param address cartridge address
 * @return cartridge data
 */
uint8_t nesl_cartridge_read_ram(nesl_cartridge_t *cartridge, nesl_bank_e type, uint32_t address);

/**
 * @brief Read byte from cartridge subsystem ROM bank.
 * @param cartridge Pointer to cartridge subsystem context
 * @param type Bank type
 * @param address cartridge address
 * @return cartridge data
 */
uint8_t nesl_cartridge_read_rom(nesl_cartridge_t *cartridge, nesl_bank_e type, uint32_t address);

/**
 * @brief Uninitialize cartridge subsystem.
 * @param cartridge Pointer to cartridge subsystem context
 */
void nesl_cartridge_uninitialize(nesl_cartridge_t *cartridge);

/**
 * @brief Write byte to cartridge subsystem RAM bank
 * @param cartridge Pointer to cartridge subsystem context
 * @param type Bank type
 * @param address cartridge address
 * @param data cartridge data
 */
void nesl_cartridge_write_ram(nesl_cartridge_t *cartridge, nesl_bank_e type, uint32_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_CARTRIDGE_H_ */
