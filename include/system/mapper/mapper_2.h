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
 * @file mapper_2.h
 * @brief Mapper 2 (UxROM) extension.
 */

#ifndef NESL_MAPPER_2_H_
#define NESL_MAPPER_2_H_

#include <mapper.h>

/*!
 * @union nesl_mapper_2_program_t
 * @brief Mapper-2 program register.
 */
typedef union {

    struct {
        uint8_t bank : 4;               /*!< Program bank */
    };

    uint8_t raw;                        /*!< Raw byte */
} nesl_mapper_2_program_t;

/*!
 * @struct nesl_mapper_2_t
 * @brief Mapper-2 context.
 */
typedef struct {
    nesl_mapper_2_program_t program;    /*!< Program register */
} nesl_mapper_2_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Initialize mapper-2 extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_2_initialize(nesl_mapper_t *mapper);

/*!
 * @brief Send mapper-2 extension interrupt.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_2_interrupt(nesl_mapper_t *mapper);

/*!
 * @brief Read byte from mapper-2 RAM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @return mapper data
 */
uint8_t nesl_mapper_2_read_ram(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/*!
 * @brief Read byte from mapper-2 ROM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @return mapper data
 */
uint8_t nesl_mapper_2_read_rom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/*!
 * @brief Reset mapper-2 extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_2_reset(nesl_mapper_t *mapper);

/*!
 * @brief Uninitialize mapper-2 extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 */
void nesl_mapper_2_uninitialize(nesl_mapper_t *mapper);

/*!
 * @brief Write byte to mapper-2 RAM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @param[in] data mapper data
 */
void nesl_mapper_2_write_ram(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

/*!
 * @brief Write byte to mapper-2 ROM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @param[in] data mapper data
 */
void nesl_mapper_2_write_rom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_2_H_ */
