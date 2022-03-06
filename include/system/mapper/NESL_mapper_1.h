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
 * @file NESL_mapper_1.h
 * @brief Mapper 1 (MMC1) extension.
 */

#ifndef NESL_MAPPER_1_H_
#define NESL_MAPPER_1_H_

#include "../NESL_mapper.h"

/**
 * @union nesl_mapper_1_character_t
 * @brief Mapper-1 character register.
 */
typedef union {

    struct {
        uint8_t bank : 5;                   /*< Character ROM bank */
    };

    uint8_t raw;                            /*< Raw byte */
} nesl_mapper_1_character_t;

/**
 * @union nesl_mapper_1_control_t
 * @brief Mapper-1 control register.
 */
typedef union {

    struct {
        uint8_t mirror : 2;                 /*< Mirror type */
        uint8_t program : 2;                /*< Program bank mode */
        uint8_t character : 1;              /*< Character bank mode */
    };

    uint8_t raw;                            /*< Raw byte */
} nesl_mapper_1_control_t;

/**
 * @union nesl_mapper_1_program_t
 * @brief Mapper-1 program register.
 */
typedef union {

    struct {
        uint8_t bank : 4;                   /*< Program ROM bank */
        uint8_t ram_disable : 1;            /*< Program RAM disable */
    };

    uint8_t raw;                            /*< Raw byte */
} nesl_mapper_1_program_t;

/**
 * @struct nesl_mapper_1_context_t
 * @brief Mapper-1 extension context.
 */
typedef struct {

    struct {
        uint8_t data;                       /*< Serial data */
        uint8_t position;                   /*< Serial position */
    } shift;

    nesl_mapper_1_control_t control;        /*< Control register */
    nesl_mapper_1_character_t character[2]; /*< Character register */
    nesl_mapper_1_program_t program;        /*< Program register */
} nesl_mapper_1_context_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initialize mapper-1 extension.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper1Init(nesl_mapper_t *mapper);

/**
 * @brief Send mapper-1 extension interrupt.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper1Interrupt(nesl_mapper_t *mapper);

/**
 * @brief Read byte from mapper-1 RAM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @return mapper data
 */
uint8_t NESL_Mapper1ReadRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/**
 * @brief Read byte from mapper-1 ROM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @return mapper data
 */
uint8_t NESL_Mapper1ReadRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/**
 * @brief Reset mapper-1 extension.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper1Reset(nesl_mapper_t *mapper);

/**
 * @brief Write byte to mapper-1 RAM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @param data mapper data
 */
void NESL_Mapper1WriteRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

/**
 * @brief Write byte to mapper-1 ROM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @param data mapper data
 */
void NESL_Mapper1WriteRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

/**
 * @brief Uninitialize mapper-1 extension.
 * @param mapper Pointer to mapper subsystem context
 */
void NESL_Mapper1Uninit(nesl_mapper_t *mapper);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_1_H_ */
