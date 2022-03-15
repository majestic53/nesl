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
 * @file mapper.c
 * @brief Mapper subsystem.
 */

#include <mapper_0.h>
#include <mapper_1.h>
#include <mapper_2.h>
#include <mapper_3.h>
#include <mapper_4.h>
#include <mapper_30.h>
#include <mapper_66.h>

/*!
 * @struct nesl_mapper_extension_t
 * @brief Mapper extension context.
 */
typedef struct {
    nesl_mapper_e type;                                                     /*!< Mapper type */
    nesl_error_e (*initialize)(nesl_mapper_t *mapper);                      /*!< Mapper extension initialization */
    void (*uninitialize)(nesl_mapper_t *mapper);                            /*!< Mapper extension uninitialization */
} nesl_mapper_extension_t;

/*!
 * @brief Supported mapper extensions array.
 * @note If a new mapper extension is added, it must be added into this array
 */
static const nesl_mapper_extension_t CONTEXT[] = {
    { MAPPER_0, nesl_mapper_0_initialize, nesl_mapper_0_uninitialize, },    /*!< Mapper 0 (NROM) */
    { MAPPER_1, nesl_mapper_1_initialize, nesl_mapper_1_uninitialize, },    /*!< Mapper 1 (MMC1) */
    { MAPPER_2, nesl_mapper_2_initialize, nesl_mapper_2_uninitialize, },    /*!< Mapper 2 (UxROM) */
    { MAPPER_3, nesl_mapper_3_initialize, nesl_mapper_3_uninitialize, },    /*!< Mapper 3 (CNROM) */
    { MAPPER_4, nesl_mapper_4_initialize, nesl_mapper_4_uninitialize, },    /*!< Mapper 4 (MMC3) */
    { MAPPER_30, nesl_mapper_30_initialize, nesl_mapper_30_uninitialize, }, /*!< Mapper 30 (UNROM) */
    { MAPPER_66, nesl_mapper_66_initialize, nesl_mapper_66_uninitialize, }, /*!< Mapper 66 (GxROM) */
    };

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Initialize mapper extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_mapper_extension_initialize(nesl_mapper_t *mapper)
{
    nesl_error_e result = NESL_FAILURE;
    const nesl_mapper_extension_t *extension = NULL;
    int count = sizeof(CONTEXT) / sizeof(*(CONTEXT)), index;

    for(index = 0; index < count; ++index) {
        extension = &CONTEXT[index];

        if(extension->type == mapper->type) {

            if((result = extension->initialize(mapper)) == NESL_FAILURE) {
                goto exit;
            }
            break;
        }
    }

    if(index == count) {
        result = SET_ERROR("Unsupported mapper type -- %u", mapper->type);
        goto exit;
    }

exit:
    return result;
}

/*!
 * @brief Uninitialize mapper extension.
 * @param[in,out] mapper Pointer to mapper subsystem context
 */
static void nesl_mapper_extension_uninitialize(nesl_mapper_t *mapper)
{

    for(int index = 0; index < (sizeof(CONTEXT) / sizeof(*(CONTEXT))); ++index) {
        const nesl_mapper_extension_t *extension = &CONTEXT[index];

        if(extension->type == mapper->type) {
            extension->uninitialize(mapper);
            break;
        }
    }
}

nesl_error_e nesl_mapper_initialize(nesl_mapper_t *mapper, const void *data, int length)
{
    nesl_error_e result;

    if((result = nesl_cartridge_initialize(&mapper->cartridge, data, length)) == NESL_FAILURE) {
        goto exit;
    }

    mapper->mirror = nesl_cartridge_get_mirror(&mapper->cartridge);
    mapper->type = nesl_cartridge_get_mapper(&mapper->cartridge);

    if((result = nesl_mapper_extension_initialize(mapper)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_mapper_reset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

nesl_error_e nesl_mapper_interrupt(nesl_mapper_t *mapper)
{
    return mapper->extension.interrupt(mapper);
}

uint8_t nesl_mapper_read(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case BANK_CHARACTER_RAM:
        case BANK_PROGRAM_RAM:
            result = mapper->extension.read_ram(mapper, type, address);
            break;
        case BANK_CHARACTER_ROM:
        case BANK_PROGRAM_ROM:
            result = mapper->extension.read_rom(mapper, type, address);
            break;
        default:
            break;
    }

    return result;
}

nesl_error_e nesl_mapper_reset(nesl_mapper_t *mapper)
{
    return mapper->extension.reset(mapper);
}

void nesl_mapper_uninitialize(nesl_mapper_t *mapper)
{
    nesl_mapper_extension_uninitialize(mapper);
    nesl_cartridge_uninitialize(&mapper->cartridge);
    memset(mapper, 0, sizeof(*mapper));
}

void nesl_mapper_write(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{

    switch(type) {
        case BANK_CHARACTER_RAM:
        case BANK_PROGRAM_RAM:
            mapper->extension.write_ram(mapper, type, address, data);
            break;
        case BANK_CHARACTER_ROM:
        case BANK_PROGRAM_ROM:
            mapper->extension.write_rom(mapper, type, address, data);
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
