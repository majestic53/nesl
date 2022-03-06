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
 * @file NESL_mapper_66.h
 * @brief Mapper 66 (GxROM) extension.
 */

#ifndef NESL_MAPPER_66_H_
#define NESL_MAPPER_66_H_

#include "../NESL_mapper.h"

/**
 * @union nesl_mapper_66_bank_t
 * @brief Mapper-66 bank register.
 */
typedef union {

    struct {
        uint8_t character : 2;  /*< Character bank */
        uint8_t unused : 2;     /*< Unused */
        uint8_t program : 2;    /*< Program bank */
    };

    uint8_t raw;                /*< Raw byte */
} nesl_mapper_66_bank_t;

/**
 * @struct nesl_mapper_66_context_t
 * @brief Mapper-66 extension context.
 */
typedef struct {
    nesl_mapper_66_bank_t bank; /*< Bank register */
} nesl_mapper_66_context_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initialize mapper-66 extension.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper66Init(nesl_mapper_t *mapper);

/**
 * @brief Send mapper-66 extension interrupt.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper66Interrupt(nesl_mapper_t *mapper);

/**
 * @brief Read byte from mapper-66 RAM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @return mapper data
 */
uint8_t NESL_Mapper66ReadRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/**
 * @brief Read byte from mapper-66 ROM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @return mapper data
 */
uint8_t NESL_Mapper66ReadRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/**
 * @brief Reset mapper-66 extension.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper66Reset(nesl_mapper_t *mapper);

/**
 * @brief Write byte to mapper-66 RAM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @param data mapper data
 */
void NESL_Mapper66WriteRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

/**
 * @brief Write byte to mapper-66 ROM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @param data mapper data
 */
void NESL_Mapper66WriteRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

/**
 * @brief Uninitialize mapper-66 extension.
 * @param mapper Pointer to mapper subsystem context
 */
void NESL_Mapper66Uninit(nesl_mapper_t *mapper);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_66_H_ */
