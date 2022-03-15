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
 * @file mapper_0.h
 * @brief Mapper 0 (NROM) extension.
 */

#ifndef NESL_MAPPER_0_H_
#define NESL_MAPPER_0_H_

#include <mapper.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Initialize mapper-0 extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_0_initialize(nesl_mapper_t *mapper);

/*!
 * @brief Send mapper-0 extension interrupt.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_0_interrupt(nesl_mapper_t *mapper);

/*!
 * @brief Read byte from mapper-0 RAM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @return mapper data
 */
uint8_t nesl_mapper_0_read_ram(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/*!
 * @brief Read byte from mapper-0 ROM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @return mapper data
 */
uint8_t nesl_mapper_0_read_rom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/*!
 * @brief Reset mapper-0 extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_0_reset(nesl_mapper_t *mapper);

/*!
 * @brief Uninitialize mapper-0 extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 */
void nesl_mapper_0_uninitialize(nesl_mapper_t *mapper);

/*!
 * @brief Write byte to mapper-0 RAM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @param[in] data mapper data
 */
void nesl_mapper_0_write_ram(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

/*!
 * @brief Write byte to mapper-0 ROM extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @param[in] data mapper data
 */
void nesl_mapper_0_write_rom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_0_H_ */
