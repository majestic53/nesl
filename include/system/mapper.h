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
 * @file mapper.h
 * @brief Mapper subsystem.
 */

#ifndef NESL_MAPPER_H_
#define NESL_MAPPER_H_

#include <bus.h>
#include <cartridge.h>

/*!
 * @struct nesl_mapper_s
 * @brief Mapper context.
 */

/*!
 * @struct nesl_mapper_t
 * @brief Mapper context.
 */
typedef struct nesl_mapper_s {
    nesl_cartridge_t cartridge;                                                                             /*!< Cartridge context */
    void *context;                                                                                          /*!< Pointer to extension context */
    nesl_mirror_e mirror;                                                                                   /*!< Mirror type */
    nesl_mapper_e type;                                                                                     /*!< Mapper type */

    struct {
        nesl_error_e (*interrupt)(struct nesl_mapper_s *mapper);                                            /*!< Mapper extension interrupt */
        uint8_t (*read_ram)(struct nesl_mapper_s *mapper, nesl_bank_e type, uint16_t address);              /*!< Mapper extension read RAM */
        uint8_t (*read_rom)(struct nesl_mapper_s *mapper, nesl_bank_e type, uint16_t address);              /*!< Mapper extension read ROM */
        nesl_error_e (*reset)(struct nesl_mapper_s *mapper);                                                /*!< Mapper extension reset */
        void (*write_ram)(struct nesl_mapper_s *mapper, nesl_bank_e type, uint16_t address, uint8_t data);  /*!< Mapper extension write RAM */
        void (*write_rom)(struct nesl_mapper_s *mapper, nesl_bank_e type, uint16_t address, uint8_t data);  /*!< Mapper extension write ROM */
    } extension;

    struct {
        uint32_t program;                                                                                   /*!< Program RAM index */
    } ram;

    struct {
        uint32_t character[8];                                                                              /*!< Character ROM indicies */
        uint32_t program[4];                                                                                /*!< Program ROM indicies */
    } rom;
} nesl_mapper_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Initialize mapper subsystem.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] data Constant pointer to cartridge data
 * @param[in] length Cartridge data length in bytes
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_initialize(nesl_mapper_t *mapper, const void *data, int length);

/*!
 * @brief Send mapper subsystem interrupt.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_interrupt(nesl_mapper_t *mapper);

/*!
 * @brief Read byte from mapper subsystem.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @return mapper data
 */
uint8_t nesl_mapper_read(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address);

/*!
 * @brief Reset mapper subsystem.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_mapper_reset(nesl_mapper_t *mapper);

/*!
 * @brief Uninitialize mapper subsystem.
 * @param[in,out] mapper Pointer to mapper subsystem context
 */
void nesl_mapper_uninitialize(nesl_mapper_t *mapper);

/*!
 * @brief Write byte to mapper subsystem.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] type Bank type
 * @param[in] address mapper address
 * @param[in] data mapper data
 */
void nesl_mapper_write(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_H_ */
