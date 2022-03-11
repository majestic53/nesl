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
 * @file NESL_mapper_3.h
 * @brief Mapper 3 (CNROM) extension.
 */

#ifndef NESL_MAPPER_3_H_
#define NESL_MAPPER_3_H_

#include <NESL_mapper.h>

/**
 * @union nesl_mapper_3_character_t
 * @brief Mapper-3 character register.
 */
typedef union {

    struct {
        uint8_t bank : 2;                   /*< Character bank */
    };

    uint8_t raw;                            /*< Raw byte */
} nesl_mapper_3_character_t;

/**
 * @struct nesl_mapper_3_t
 * @brief Mapper-3 context.
 */
typedef struct {
    nesl_mapper_3_character_t character;    /*< Character register */
} nesl_mapper_3_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initialize mapper-3 extension.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper3Initialize(nesl_mapper_t *mapper);

/**
 * @brief Send mapper-3 extension interrupt.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper3Interrupt(nesl_mapper_t *mapper);

/**
 * @brief Read byte from mapper-3 RAM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @return mapper data
 */
uint8_t NESL_Mapper3ReadRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/**
 * @brief Read byte from mapper-3 ROM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @return mapper data
 */
uint8_t NESL_Mapper3ReadRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/**
 * @brief Reset mapper-3 extension.
 * @param mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_Mapper3Reset(nesl_mapper_t *mapper);

/**
 * @brief Uninitialize mapper-3 extension.
 * @param mapper Pointer to mapper subsystem context
 */
void NESL_Mapper3Uninitialize(nesl_mapper_t *mapper);

/**
 * @brief Write byte to mapper-3 RAM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @param data mapper data
 */
void NESL_Mapper3WriteRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

/**
 * @brief Write byte to mapper-3 ROM extension.
 * @param mapper Pointer to mapper subsystem context
 * @param type Bank type
 * @param address mapper address
 * @param data mapper data
 */
void NESL_Mapper3WriteRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_3_H_ */
