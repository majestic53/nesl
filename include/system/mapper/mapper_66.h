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
 * @file mapper_66.h
 * @brief Mapper 66 (GxROM) extension.
 */

#ifndef NESL_MAPPER_66_H_
#define NESL_MAPPER_66_H_

#include <mapper.h>

/*!
 * @union nesl_mapper_66_bank_t
 * @brief Mapper-66 bank register.
 */
typedef union {

    struct {
        uint8_t character : 2;  /*!< Character bank */
        uint8_t unused : 2;     /*!< Unused */
        uint8_t program : 2;    /*!< Program bank */
    };

    uint8_t raw;                /*!< Raw byte */
} nesl_mapper_66_bank_t;

/*!
 * @struct nesl_mapper_66_t
 * @brief Mapper-66 context.
 */
typedef struct {
    nesl_mapper_66_bank_t bank; /*!< Bank register */
} nesl_mapper_66_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Initialize mapper-66 extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_66_initialize(nesl_mapper_t *mapper);

/*!
 * @brief Send mapper-66 extension interrupt.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_66_interrupt(nesl_mapper_t *mapper);

/*!
 * @brief Read byte from mapper-66 RAM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @return mapper data
 */
uint8_t nesl_mapper_66_read_ram(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/*!
 * @brief Read byte from mapper-66 ROM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @return mapper data
 */
uint8_t nesl_mapper_66_read_rom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/*!
 * @brief Reset mapper-66 extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_66_reset(nesl_mapper_t *mapper);

/*!
 * @brief Uninitialize mapper-66 extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 */
void nesl_mapper_66_uninitialize(nesl_mapper_t *mapper);

/*!
 * @brief Write byte to mapper-66 RAM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @param[in] data mapper data
 */
void nesl_mapper_66_write_ram(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

/*!
 * @brief Write byte to mapper-66 ROM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @param[in] data mapper data
 */
void nesl_mapper_66_write_rom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_66_H_ */
