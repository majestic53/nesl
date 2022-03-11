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
 * @file NESL_mapper_4.h
 * @brief Mapper 4 (MMC3) extension.
 */

#ifndef NESL_MAPPER_4_H_
#define NESL_MAPPER_4_H_

#include <NESL_mapper.h>

/**
 * @union nesl_mapper_4_mirror_t
 * @brief Mapper-4 mirror register.
 */
typedef union {

    struct {
        uint8_t mode : 1;               /*< Mirror mode (0:Vertical, 1:Horizontal) */
    };

    uint8_t raw;                        /*< Raw byte */
} nesl_mapper_4_mirror_t;

/**
 * @union nesl_mapper_4_protect_t
 * @brief Mapper-4 RAM protection register.
 */
typedef union {

    struct {
        uint8_t unused : 6;             /*< Unused */
        uint8_t ram_read_only : 1;      /*< Program RAM read-only */
        uint8_t ram_enable : 1;         /*< Program RAM enable */
    };

    uint8_t raw;                        /*< Raw byte */
} nesl_mapper_4_protect_t;

/**
 * @union nesl_mapper_4_select_t
 * @brief Mapper-4 select register.
 */
typedef union {

    struct {
        uint8_t bank : 3;               /*< Bank mode select */
        uint8_t unused : 3;             /*< Unused */
        uint8_t program : 1;            /*< Program bank */
        uint8_t character : 1;          /*< Character bank */
    };

    uint8_t raw;                        /*< Raw byte */
} nesl_mapper_4_select_t;

/**
 * @struct nesl_mapper_4_t
 * @brief Mapper-4 context.
 */
typedef struct {

    struct {
        uint8_t index[8];               /*< Bank indicies */
        uint8_t data;                   /*< Bank data */
    } bank;

    struct {
        bool enable;                    /*< Enable A12 interrupt */
        uint8_t count;                  /*< Interrupt counter */
        uint8_t latch;                  /*< Interrupt latch */
    } interrupt;

    nesl_mapper_4_select_t select;      /*< Select register */
    nesl_mapper_4_mirror_t mirror;      /*< Mirror register */
    nesl_mapper_4_protect_t protect;    /*< RAM protection register */
} nesl_mapper_4_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initialize mapper-4 extension.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper4Initialize(nesl_mapper_t *mapper);

/**
 * @brief Send mapper-4 extension interrupt.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper4Interrupt(nesl_mapper_t *mapper);

/**
 * @brief Read byte from mapper-4 RAM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @return mapper data
 */
uint8_t NESL_Mapper4ReadRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/**
 * @brief Read byte from mapper-4 ROM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @return mapper data
 */
uint8_t NESL_Mapper4ReadRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/**
 * @brief Reset mapper-4 extension.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper4Reset(nesl_mapper_t *mapper);

/**
 * @brief Uninitialize mapper-4 extension.
 * @param mapper Pointer to mapper subsystem context
 */
void NESL_Mapper4Uninitialize(nesl_mapper_t *mapper);

/**
 * @brief Write byte to mapper-4 RAM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @param data mapper data
 */
void NESL_Mapper4WriteRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

/**
 * @brief Write byte to mapper-4 ROM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @param data mapper data
 */
void NESL_Mapper4WriteRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_4_H_ */
