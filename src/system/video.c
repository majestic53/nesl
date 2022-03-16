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
 * @file video.c
 * @brief Video subsystem.
 */

#include <mapper.h>
#include <video.h>

/*!
 * @brief Video port getter function.
 * @param[in,out] video  Pointer to video context
 * @return Byte read from video port
 */
typedef uint8_t (*nesl_video_get_port)(nesl_video_t *video);

/*!
 * @brief Video port setter function.
 * @param[in,out] video  Pointer to video context
 * @param[in] data Byte to write to video port
 */
typedef void (*nesl_video_set_port)(nesl_video_t *video, uint8_t data);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Load background patterns into shift registers.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_background_load(nesl_video_t *video)
{

    switch((video->cycle - 1) % 8) {
        case 0:
            video->background.attribute.lsb.low = (video->background.attribute.data & 1) ? 0xFF : 0;
            video->background.attribute.msb.low = (video->background.attribute.data & 2) ? 0xFF : 0;
            video->background.pattern.lsb.low = video->background.pattern.data.low;
            video->background.pattern.msb.low = video->background.pattern.data.high;
            video->background.type = nesl_bus_read(BUS_VIDEO, 0x2000 + (video->address.v.word & 0x0FFF));
            break;
        case 2:
            video->background.attribute.data = nesl_bus_read(BUS_VIDEO, 0x23C0 + (video->address.v.nametable_y << 11)
                + (video->address.v.nametable_x << 10) + ((video->address.v.coarse_y >> 2) << 3) + (video->address.v.coarse_x >> 2));

            if(video->address.v.coarse_y & 2) {
                video->background.attribute.data >>= 4;
            }

            if(video->address.v.coarse_x & 2) {
                video->background.attribute.data >>= 2;
            }

            video->background.attribute.data &= 3;
            break;
        case 4:
            video->background.pattern.data.low = nesl_bus_read(BUS_VIDEO, (video->port.control.background_pattern << 12)
                + ((uint16_t)video->background.type << 4) + video->address.v.fine_y);
            break;
        case 6:
            video->background.pattern.data.high = nesl_bus_read(BUS_VIDEO, (video->port.control.background_pattern << 12)
                + ((uint16_t)video->background.type << 4) + video->address.v.fine_y + 8);
            break;
        case 7:

            if(video->port.mask.background_show || video->port.mask.sprite_show) {

                if(video->address.v.coarse_x == 31) {
                    video->address.v.coarse_x = 0;
                    video->address.v.nametable_x = ~video->address.v.nametable_x;
                } else {
                    ++video->address.v.coarse_x;
                }
            }
            break;
        default:
            break;
    }
}

/*!
 * @brief Shift background shift registers.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_background_shift(nesl_video_t *video)
{

    if(video->port.mask.background_show) {
        video->background.attribute.lsb.word <<= 1;
        video->background.attribute.msb.word <<= 1;
        video->background.pattern.lsb.word <<= 1;
        video->background.pattern.msb.word <<= 1;
    }
}

/*!
 * @brief Flip sprite data.
 * @param[in] data Sprite data
 * @return Flipped sprite data
 */
static uint8_t nesl_video_flip(uint8_t data)
{
    data = ((data & 0x0F) << 4) | ((data & 0xF0) >> 4);
    data = ((data & 0x33) << 2) | ((data & 0xCC) >> 2);
    data = ((data & 0x55) << 1) | ((data & 0xAA) >> 1);

    return data;
}

/*!
 * @brief Get data port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @return Data port register data
 */
static uint8_t nesl_video_get_port_data(nesl_video_t *video)
{
    uint8_t result = video->port.data.low;

    video->port.data.low = nesl_bus_read(BUS_VIDEO, video->address.v.word);

    if(video->address.v.word >= 0x3F00) {
        result = video->port.data.low;
    }

    video->address.v.word += (video->port.control.increment ? 32 : 1);

    return result;
}

/*!
 * @brief Get OAM-data port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @return OAM-data port register data
 */
static uint8_t nesl_video_get_port_oam_data(nesl_video_t *video)
{
    return ((uint8_t *)video->ram.oam)[video->port.oam_address.low];
}

/*!
 * @brief Get video status port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @return Status port register data
 */
static uint8_t nesl_video_get_port_status(nesl_video_t *video)
{
    uint8_t result;
    nesl_video_status_t status = {};

    status.raw = video->port.status.raw;
    status.unused = video->port.data.low;
    result = status.raw;
    video->port.status.vertical_blank = false;
    video->port.latch = false;

    return result;
}

/*!
 * @brief Get video unused port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @return Unused port register data
 */
static uint8_t nesl_video_get_port_unused(nesl_video_t *video)
{
    return video->port.data.low;
}

/*!
 * @brief Update internal address x-coordinates.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_horizontal_set(nesl_video_t *video)
{
    video->background.attribute.lsb.low = (video->background.attribute.data & 1) ? 0xFF : 0;
    video->background.attribute.msb.low = (video->background.attribute.data & 2) ? 0xFF : 0;
    video->background.pattern.lsb.low = video->background.pattern.data.low;
    video->background.pattern.msb.low = video->background.pattern.data.high;

    if(video->port.mask.background_show || video->port.mask.sprite_show) {
        video->address.v.coarse_x = video->address.t.coarse_x;
        video->address.v.nametable_x = video->address.t.nametable_x;
    }
}

/*!
 * @brief Send mapper interrupt on A12.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_mapper_interrupt(nesl_video_t *video)
{

    if(video->port.mask.background_show || video->port.mask.sprite_show) {
        nesl_bus_interrupt(INTERRUPT_MAPPER);
    }
}

/*!
 * @brief Calculate video nametable address.
 * @param[in] address Desired address
 * @param[in] mirror Mapper mirror type
 * @param[out] bank Calcuated bank
 * @return Calculated address
 */
static uint16_t nesl_video_nametable_address(uint16_t address, nesl_mirror_e mirror, int *bank)
{

    switch(mirror) {
        case MIRROR_HORIZONTAL:

            switch(address) {
                case 0x2000 ... 0x23FF:
                case 0x2400 ... 0x27FF:
                    *bank = 0;
                    break;
                case 0x2800 ... 0x2BFF:
                case 0x2C00 ... 0x2FFF:
                    *bank = 1;
                    break;
                default:
                    break;
            }
            break;
        case MIRROR_VERTICAL:

            switch(address) {
                case 0x2000 ... 0x23FF:
                case 0x2800 ... 0x2BFF:
                    *bank = 0;
                    break;
                case 0x2400 ... 0x27FF:
                case 0x2C00 ... 0x2FFF:
                    *bank = 1;
                    break;
                default:
                    break;
            }
            break;
        case MIRROR_ONE_LOW:
            *bank = 0;
            break;
        case MIRROR_ONE_HIGH:
            *bank = 1;
            break;
        default:
            break;
    }

    return address & 0x03FF;
}

/*!
 * @brief Calcluate video palette address.
 * @param[in] address Desired address
 * @return Calculated address
 */
static uint16_t nesl_video_palette_address(uint16_t address)
{

    switch((address &= 0x1F)) {
        case 0x10:
        case 0x14:
        case 0x18:
        case 0x1C:
            address -= 0x10;
            break;
        default:
            break;
    }

    return address;
}

/*!
 * @brief Render video pixel to service.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_render(nesl_video_t *video)
{
    bool priority;
    uint8_t color[3] = {}, palette[3] = {};

    if(video->port.mask.background_show) {
        uint16_t mask = 0x8000 >> video->address.fine_x;

        color[1] = (((video->background.pattern.msb.word & mask) > 0) << 1) | ((video->background.pattern.lsb.word & mask) > 0);
        palette[1] = (((video->background.attribute.msb.word & mask) > 0) << 1) | ((video->background.attribute.lsb.word & mask) > 0);
    }

    if(video->port.mask.sprite_show) {
        video->sprite.sprite_0_render = false;

        for(uint8_t index = 0; index < video->sprite.count; ++index) {
            const nesl_video_object_t *object = &video->sprite.object[index];

            if(!object->x) {
                color[2] = (((video->sprite.pattern[index].msb & 0x80) > 0) << 1) | ((video->sprite.pattern[index].lsb & 0x80) > 0);
                palette[2] = object->attribute.palette + 4;
                priority = (object->attribute.priority == 0);

                if(color[2]) {

                    if(!index) {
                        video->sprite.sprite_0_render = true;
                    }
                    break;
                }
            }
        }
    }

    if(!color[1] && !color[2]) {
        color[0] = 0;
        palette[0] = 0;
    } else if(color[1] && !color[2]) {
        color[0] = color[1];
        palette[0] = palette[1];
    } else if(!color[1] && color[2]) {
        color[0] = color[2];
        palette[0] = palette[2];
    } else {
        color[0] = color[priority ? 2 : 1];
        palette[0] = palette[priority ? 2 : 1];

        if(video->sprite.sprite_0_found && video->sprite.sprite_0_render
                && video->port.mask.background_show && video->port.mask.sprite_show) {

            if(video->port.mask.background_left_show || video->port.mask.sprite_left_show) {

                if((video->cycle > 0) && (video->cycle <= 257)) {
                    video->port.status.sprite_0_hit = true;
                }
            } else if((video->cycle > 8) && (video->cycle <= 257)) {
                video->port.status.sprite_0_hit = true;
            }
        }
    }

    if((video->scanline >= 0) && (video->scanline < 240) && ((video->cycle - 1) < 256)) {
        nesl_service_set_pixel(nesl_bus_read(BUS_VIDEO, 0x3F00 + (4 * palette[0]) + color[0]),
            video->port.mask.red_emphasis, video->port.mask.green_emphasis, video->port.mask.blue_emphasis,
            video->cycle - 1, video->scanline);
    }
}

/*!
 * @brief Set video address port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @param[in] data Address port register data
 */
static void nesl_video_set_port_address(nesl_video_t *video, uint8_t data)
{

    if(video->port.latch) {
        video->address.t.low = data;
        video->address.v.word = video->address.t.word;
        video->port.latch = false;
    } else {
        video->address.t.high = (data & 0x3F);
        video->port.latch = true;
    }
}

/*!
 * @brief Set video control port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @param[in] data Control port register data
 */
static void nesl_video_set_port_control(nesl_video_t *video, uint8_t data)
{
    video->port.control.raw = data;
    video->address.t.nametable_x = video->port.control.nametable_x;
    video->address.t.nametable_y = video->port.control.nametable_y;
}

/*!
 * @brief Set video data port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @param[in] data Data port register data
 */
static void nesl_video_set_port_data(nesl_video_t *video, uint8_t data)
{
    nesl_bus_write(BUS_VIDEO, video->address.v.word, data);
    video->address.v.word += (video->port.control.increment ? 32 : 1);
}

/*!
 * @brief Set video mask port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @param[in] data Mask port register data
 */
static void nesl_video_set_port_mask(nesl_video_t *video, uint8_t data)
{
    video->port.mask.raw = data;
}

/*!
 * @brief Set video OAM-address port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @param[in] data OAM-address port register data
 */
static void nesl_video_set_port_oam_address(nesl_video_t *video, uint8_t data)
{
    video->port.oam_address.low = data;
}

/*!
 * @brief Set video OAM-data port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @param[in] data OAM-data port register data
 */
static void nesl_video_set_port_oam_data(nesl_video_t *video, uint8_t data)
{
    ((uint8_t *)video->ram.oam)[video->port.oam_address.low] = data;

    if(!video->port.status.vertical_blank) {
        ++video->port.oam_address.low;
    }
}

/*!
 * @brief Set video scroll port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @param[in] data Scroll port register data
 */
static void nesl_video_set_port_scroll(nesl_video_t *video, uint8_t data)
{

    if(video->port.latch) {
        video->address.t.coarse_y = data >> 3;
        video->address.t.fine_y = data & 7;
        video->port.latch = false;
    } else {
        video->address.t.coarse_x = data >> 3;
        video->address.fine_x = data & 7;
        video->port.latch = true;
    }
}

/*!
 * @brief Set video unused port register.
 * @param[in,out] video  Pointer to video subsystem context
 * @param[in] data Unused port register data
 */
static void nesl_video_set_port_unused(nesl_video_t *video, uint8_t data)
{
    video->port.data.low = data;
}

/*!
 * @brief Evaluate sprites present on current scanline.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_sprite_evaluate(nesl_video_t *video)
{
    video->port.status.sprite_overflow = false;
    memset(&video->sprite.object, 0xFF, sizeof(video->sprite.object));
    video->sprite.sprite_0_found = false;
    video->sprite.count = 0;

    for(uint8_t index = 0; index < 64; ++index) {
        const nesl_video_object_t *object = &video->ram.oam[index];
        int16_t offset = ((int16_t)video->scanline - (int16_t)object->y);

        if((offset >= 0) && (offset < (video->port.control.sprite_size ? 16 : 8))) {

            if(video->sprite.count < 8) {
                video->sprite.object[video->sprite.count++].raw = object->raw;

                if(!index) {
                    video->sprite.sprite_0_found = true;
                }
            } else {
                video->port.status.sprite_overflow = true;
                break;
            }
        }
    }
}

/*!
 * @brief Load sprite patterns into shift registers.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_sprite_load(nesl_video_t *video)
{

    for(uint8_t index = 0; index < video->sprite.count; ++index) {
        nesl_register_t address = {};
        const nesl_video_object_t *object = &video->sprite.object[index];

        if(video->port.control.sprite_size) {

            if(object->attribute.flip_vertical) {

                if((video->scanline - object->y) < 8) {
                    address.word = (object->type.bank << 12) + (((object->type.raw & 0xFE) + 1) << 4) + ((7 - (video->scanline - object->y)) & 7);
                } else {
                    address.word = (object->type.bank << 12) + ((object->type.raw & 0xFE) << 4) + ((7 - (video->scanline - object->y)) & 7);
                }
            } else {

                if((video->scanline - object->y) < 8) {
                    address.word = (object->type.bank << 12) + ((object->type.raw & 0xFE) << 4) + ((video->scanline - object->y) & 7);
                } else {
                    address.word = (object->type.bank << 12) + (((object->type.raw & 0xFE) + 1) << 4) + ((video->scanline - object->y) & 7);
                }
            }
        } else {

            if(object->attribute.flip_vertical) {
                address.word = (video->port.control.sprite_pattern << 12) + (object->type.raw << 4) + (7 - (video->scanline - object->y));
            } else {
                address.word = (video->port.control.sprite_pattern << 12) + (object->type.raw << 4) + (video->scanline - object->y);
            }
        }

        video->sprite.pattern[index].lsb = nesl_bus_read(BUS_VIDEO, address.word);
        video->sprite.pattern[index].msb = nesl_bus_read(BUS_VIDEO, address.word + 8);

        if(object->attribute.flip_horizontal) {
            video->sprite.pattern[index].lsb = nesl_video_flip(video->sprite.pattern[index].lsb);
            video->sprite.pattern[index].msb = nesl_video_flip(video->sprite.pattern[index].msb);
        }
    }
}

/*!
 * @brief Shift sprite shift registers.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_sprite_shift(nesl_video_t *video)
{

    if(video->port.mask.sprite_show) {

        for(uint8_t index = 0; index < 8; ++index) {
            nesl_video_object_t *object = &video->sprite.object[index];

            if(!object->x) {
                video->sprite.pattern[index].lsb <<= 1;
                video->sprite.pattern[index].msb <<= 1;
            } else {
                --object->x;
            }
        }
    }
}

/*!
 * @brief Enter vertical blank.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_vertical_blank(nesl_video_t *video)
{

    video->port.status.vertical_blank = true;

    if(video->port.control.interrupt) {
        nesl_bus_interrupt(INTERRUPT_NON_MASKABLE);
    }
}

/*!
 * @brief Exit vertical blank.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_vertical_blank_exit(nesl_video_t *video)
{
    video->port.status.sprite_overflow = false;
    video->port.status.sprite_0_hit = false;
    video->port.status.vertical_blank = false;

    for(uint8_t index = 0; index < 8; ++index) {
        video->sprite.pattern[index].lsb = 0;
        video->sprite.pattern[index].msb = 0;
    }
}

/*!
 * @brief Increment address y-coordinates.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_vertical_increment(nesl_video_t *video)
{

    if(video->port.mask.background_show || video->port.mask.sprite_show) {

        if(video->address.v.fine_y == 7) {
            video->address.v.fine_y = 0;

            switch(video->address.v.coarse_y) {
                case 29:
                    video->address.v.nametable_y = ~video->address.v.nametable_y;
                case 31:
                    video->address.v.coarse_y = 0;
                    break;
                default:
                    ++video->address.v.coarse_y;
                    break;
            }
        } else {
            ++video->address.v.fine_y;
        }
    }
}

/*!
 * @brief Update internal address y-coordinates.
 * @param[in,out] video  Pointer to video subsystem context
 */
static void nesl_video_vertical_set(nesl_video_t *video)
{

    if(video->port.mask.background_show || video->port.mask.sprite_show) {
        video->address.v.coarse_y = video->address.t.coarse_y;
        video->address.v.nametable_y = video->address.t.nametable_y;
        video->address.v.fine_y = video->address.t.fine_y;
    }
}

bool nesl_video_cycle(nesl_video_t *video)
{
    bool result = false;

    switch(video->scanline) {
        case (-1):

            if(video->cycle == 1) {
                nesl_video_vertical_blank_exit(video);
            }
        case 0 ... 239:

            switch(video->cycle) {
                case 1 ... 256:
                case 321 ... 337:

                    if(video->cycle > 1) {
                        nesl_video_background_shift(video);

                        if(video->cycle <= 256) {
                            nesl_video_sprite_shift(video);
                        }
                    }

                    nesl_video_background_load(video);

                    if(video->cycle == 256) {
                        nesl_video_vertical_increment(video);
                    }
                    break;
                case 257:
                    nesl_video_horizontal_set(video);

                    if(video->scanline >= 0) {
                        nesl_video_sprite_evaluate(video);
                        nesl_video_sprite_load(video);
                    }
                    break;
                case 260:
                    nesl_video_mapper_interrupt(video);
                    break;
                case 280 ... 304:

                    if(video->scanline == -1) {
                        nesl_video_vertical_set(video);
                    }
                    break;
                default:
                    break;
            }
            break;
        case 241:

            if(video->cycle == 1) {
                nesl_video_vertical_blank(video);
            }
            break;
        default:
            break;
    }

    nesl_video_render(video);

    if(++video->cycle > 340) {
        video->cycle = 0;

        if((result = (++video->scanline > 260))) {
            video->scanline = (-1);
        }
    }

    return result;
}

nesl_error_e nesl_video_initialize(nesl_video_t *video, const nesl_mirror_e *mirror)
{
    nesl_error_e result;

    if((result = nesl_video_reset(video, mirror)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

uint8_t nesl_video_read(nesl_video_t *video, uint16_t address)
{
    int bank = 0;
    uint8_t result = 0;

    switch(address) {
        case 0x3000 ... 0x3EFF:
            address -= 0x1000;
        case 0x2000 ... 0x2FFF:
            address = nesl_video_nametable_address(address, *video->mirror, &bank);
            result = video->ram.nametable[bank][address];
            break;
        case 0x3F00 ... 0x3FFF:
            result = video->ram.palette[nesl_video_palette_address(address)];
            break;
        default:
            break;
    }

    return result;
}

uint8_t nesl_video_read_oam(nesl_video_t *video, uint8_t address)
{
    return ((uint8_t *)video->ram.oam)[address];
}

uint8_t nesl_video_read_port(nesl_video_t *video, uint16_t address)
{
    static const nesl_video_get_port PORT[] = {
        nesl_video_get_port_unused, nesl_video_get_port_unused, nesl_video_get_port_status, nesl_video_get_port_unused,
        nesl_video_get_port_oam_data, nesl_video_get_port_unused, nesl_video_get_port_unused, nesl_video_get_port_data,
        };

    return PORT[address & 7](video);
}

nesl_error_e nesl_video_reset(nesl_video_t *video, const nesl_mirror_e *mirror)
{
    memset(video, 0, sizeof(*video));
    video->scanline = -1;
    video->mirror = mirror;

    return NESL_SUCCESS;
}

void nesl_video_uninitialize(nesl_video_t *video)
{
    memset(video, 0, sizeof(*video));
}

void nesl_video_write(nesl_video_t *video, uint16_t address, uint8_t data)
{
    int bank = 0;

    switch(address) {
        case 0x3000 ... 0x3EFF:
            address -= 0x1000;
        case 0x2000 ... 0x2FFF:
            address = nesl_video_nametable_address(address, *video->mirror, &bank);
            video->ram.nametable[bank][address] = data;
            break;
        case 0x3F00 ... 0x3FFF:
            video->ram.palette[nesl_video_palette_address(address)] = data;
            break;
        default:
            break;
    }
}

void nesl_video_write_oam(nesl_video_t *video, uint8_t address, uint8_t data)
{
    ((uint8_t *)video->ram.oam)[address] = data;
}

void nesl_video_write_port(nesl_video_t *video, uint16_t address, uint8_t data)
{
    static const nesl_video_set_port PORT[] = {
        nesl_video_set_port_control, nesl_video_set_port_mask, nesl_video_set_port_unused, nesl_video_set_port_oam_address,
        nesl_video_set_port_oam_data, nesl_video_set_port_scroll, nesl_video_set_port_address, nesl_video_set_port_data,
        };

    PORT[address & 7](video, data);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
