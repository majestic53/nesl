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

#include "../../include/system/NESL_mapper.h"
#include "../../include/system/NESL_video.h"
#include "../../include/NESL_service.h"

typedef uint8_t (*NESL_VideoPortGet)(nesl_video_t *video);
typedef void (*NESL_VideoPortSet)(nesl_video_t *video, uint8_t data);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void NESL_VideoBackgroundLoad(nesl_video_t *video)
{

    switch((video->cycle - 1) % 8) {
        case 0:
            video->background.attribute.lsb.low = (video->background.attribute.data & 1) ? 0xFF : 0;
            video->background.attribute.msb.low = (video->background.attribute.data & 2) ? 0xFF : 0;
            video->background.pattern.lsb.low = video->background.pattern.data.low;
            video->background.pattern.msb.low = video->background.pattern.data.high;
            video->background.type = NESL_BusRead(NESL_BUS_VIDEO, 0x2000 + (video->address.v.word & 0x0FFF));
            break;
        case 2:
            video->background.attribute.data = NESL_BusRead(NESL_BUS_VIDEO, 0x23C0 + (video->address.v.nametable_y << 11)
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
            video->background.pattern.data.low = NESL_BusRead(NESL_BUS_VIDEO, (video->port.control.background_pattern << 12)
                + ((uint16_t)video->background.type << 4) + video->address.v.fine_y);
            break;
        case 6:
            video->background.pattern.data.high = NESL_BusRead(NESL_BUS_VIDEO, (video->port.control.background_pattern << 12)
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

static void NESL_VideoBackgroundShift(nesl_video_t *video)
{

    if(video->port.mask.background_show) {
        video->background.attribute.lsb.word <<= 1;
        video->background.attribute.msb.word <<= 1;
        video->background.pattern.lsb.word <<= 1;
        video->background.pattern.msb.word <<= 1;
    }
}

static uint8_t NESL_VideoFlip(uint8_t data)
{
    uint8_t result = 0;

    for(uint8_t index = 0; index < 8; ++index) {
        result >>= 1;

        if(data & 0x80) {
            result |= 0x80;
        }

        data <<= 1;
    }

    return result;
}

static void NESL_VideoHorizontalSet(nesl_video_t *video)
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

static void NESL_VideoMapperInterrupt(nesl_video_t *video)
{

    if(video->port.mask.background_show || video->port.mask.sprite_show) {
        NESL_BusInterrupt(NESL_INTERRUPT_MAPPER);
    }
}

static uint16_t NESL_VideoNametableAddress(uint16_t address, int mirror, int *bank)
{

    switch(mirror) {
        case NESL_MIRROR_HORIZONTAL:

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
        case NESL_MIRROR_VERTICAL:

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
        case NESL_MIRROR_ONE_LOW:
            *bank = 0;
            break;
        case NESL_MIRROR_ONE_HIGH:
            *bank = 1;
            break;
        default:
            break;
    }

    return address & 0x03FF;
}

static uint16_t NESL_VideoPaletteAddress(uint16_t address)
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

static void NESL_VideoRender(nesl_video_t *video)
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
        NESL_ServiceSetPixel(NESL_BusRead(NESL_BUS_VIDEO, 0x3F00 + (4 * palette[0]) + color[0]),
            video->port.mask.red_emphasis, video->port.mask.green_emphasis, video->port.mask.blue_emphasis,
            video->cycle - 1, video->scanline);
    }
}

static void NESL_VideoSpriteEvaluate(nesl_video_t *video)
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

static void NESL_VideoSpriteLoad(nesl_video_t *video)
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

        video->sprite.pattern[index].lsb = NESL_BusRead(NESL_BUS_VIDEO, address.word);
        video->sprite.pattern[index].msb = NESL_BusRead(NESL_BUS_VIDEO, address.word + 8);

        if(object->attribute.flip_horizontal) {
            video->sprite.pattern[index].lsb = NESL_VideoFlip(video->sprite.pattern[index].lsb);
            video->sprite.pattern[index].msb = NESL_VideoFlip(video->sprite.pattern[index].msb);
        }
    }
}

static void NESL_VideoSpriteShift(nesl_video_t *video)
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

static void NESL_VideoVerticalBlank(nesl_video_t *video)
{

    video->port.status.vertical_blank = true;

    if(video->port.control.interrupt) {
        NESL_BusInterrupt(NESL_INTERRUPT_NON_MASKABLE);
    }
}

static void NESL_VideoVerticalBlankExit(nesl_video_t *video)
{
    video->port.status.sprite_overflow = false;
    video->port.status.sprite_0_hit = false;
    video->port.status.vertical_blank = false;

    for(uint8_t index = 0; index < 8; ++index) {
        video->sprite.pattern[index].lsb = 0;
        video->sprite.pattern[index].msb = 0;
    }
}

static void NESL_VideoVerticalIncrement(nesl_video_t *video)
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

static void NESL_VideoVerticalSet(nesl_video_t *video)
{

    if(video->port.mask.background_show || video->port.mask.sprite_show) {
        video->address.v.coarse_y = video->address.t.coarse_y;
        video->address.v.nametable_y = video->address.t.nametable_y;
        video->address.v.fine_y = video->address.t.fine_y;
    }
}

bool NESL_VideoCycle(nesl_video_t *video)
{
    bool result = false;

    switch(video->scanline) {
        case (-1):

            if(video->cycle == 1) {
                NESL_VideoVerticalBlankExit(video);
            }
        case 0 ... 239:

            switch(video->cycle) {
                case 1 ... 256:
                case 321 ... 337:

                    if(video->cycle > 1) {
                        NESL_VideoBackgroundShift(video);

                        if(video->cycle <= 256) {
                            NESL_VideoSpriteShift(video);
                        }
                    }

                    NESL_VideoBackgroundLoad(video);

                    if(video->cycle == 256) {
                        NESL_VideoVerticalIncrement(video);
                    }
                    break;
                case 257:
                    NESL_VideoHorizontalSet(video);

                    if(video->scanline >= 0) {
                        NESL_VideoSpriteEvaluate(video);
                        NESL_VideoSpriteLoad(video);
                    }
                    break;
                case 260:
                    NESL_VideoMapperInterrupt(video);
                    break;
                case 280 ... 304:

                    if(video->scanline == -1) {
                        NESL_VideoVerticalSet(video);
                    }
                    break;
                default:
                    break;
            }
            break;
        case 241:

            if(video->cycle == 1) {
                NESL_VideoVerticalBlank(video);
            }
            break;
        default:
            break;
    }

    NESL_VideoRender(video);

    if(++video->cycle > 340) {
        video->cycle = 0;

        if((result = (++video->scanline > 260))) {
            video->scanline = (-1);
        }
    }

    return result;
}

int NESL_VideoInit(nesl_video_t *video, const int *mirror)
{
    int result;

    if((result = NESL_VideoReset(video, mirror)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

uint8_t NESL_VideoOamRead(nesl_video_t *video, uint8_t address)
{
    return ((uint8_t *)video->ram.oam)[address];
}

void NESL_VideoOamWrite(nesl_video_t *video, uint8_t address, uint8_t data)
{
    ((uint8_t *)video->ram.oam)[address] = data;
}

static uint8_t NESL_VideoPortGetData(nesl_video_t *video)
{
    uint8_t result = video->port.data.low;

    video->port.data.low = NESL_BusRead(NESL_BUS_VIDEO, video->address.v.word);

    if(video->address.v.word >= 0x3F00) {
        result = video->port.data.low;
    }

    video->address.v.word += (video->port.control.increment ? 32 : 1);

    return result;
}

static uint8_t NESL_VideoPortGetOamData(nesl_video_t *video)
{
    return ((uint8_t *)video->ram.oam)[video->port.oam_address.low];
}

static uint8_t NESL_VideoPortGetStatus(nesl_video_t *video)
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

static uint8_t NESL_VideoPortGetUnused(nesl_video_t *video)
{
    return video->port.data.low;
}

static const NESL_VideoPortGet PORT_GET[] = {
    NESL_VideoPortGetUnused,
    NESL_VideoPortGetUnused,
    NESL_VideoPortGetStatus,
    NESL_VideoPortGetUnused,
    NESL_VideoPortGetOamData,
    NESL_VideoPortGetUnused,
    NESL_VideoPortGetUnused,
    NESL_VideoPortGetData,
    };

uint8_t NESL_VideoPortRead(nesl_video_t *video, uint16_t address)
{
    return PORT_GET[address & 7](video);
}

static void NESL_VideoPortSetAddress(nesl_video_t *video, uint8_t data)
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

static void NESL_VideoPortSetControl(nesl_video_t *video, uint8_t data)
{
    video->port.control.raw = data;
    video->address.t.nametable_x = video->port.control.nametable_x;
    video->address.t.nametable_y = video->port.control.nametable_y;
}

static void NESL_VideoPortSetData(nesl_video_t *video, uint8_t data)
{
    NESL_BusWrite(NESL_BUS_VIDEO, video->address.v.word, data);
    video->address.v.word += (video->port.control.increment ? 32 : 1);
}

static void NESL_VideoPortSetMask(nesl_video_t *video, uint8_t data)
{
    video->port.mask.raw = data;
}

static void NESL_VideoPortSetOamAddress(nesl_video_t *video, uint8_t data)
{
    video->port.oam_address.low = data;
}

static void NESL_VideoPortSetOamData(nesl_video_t *video, uint8_t data)
{
    ((uint8_t *)video->ram.oam)[video->port.oam_address.low] = data;

    if(!video->port.status.vertical_blank) {
        ++video->port.oam_address.low;
    }
}

static void NESL_VideoPortSetScroll(nesl_video_t *video, uint8_t data)
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

static void NESL_VideoPortSetUnused(nesl_video_t *video, uint8_t data)
{
    video->port.data.low = data;
}

static const NESL_VideoPortSet PORT_SET[] = {
    NESL_VideoPortSetControl,
    NESL_VideoPortSetMask,
    NESL_VideoPortSetUnused,
    NESL_VideoPortSetOamAddress,
    NESL_VideoPortSetOamData,
    NESL_VideoPortSetScroll,
    NESL_VideoPortSetAddress,
    NESL_VideoPortSetData,
    };

void NESL_VideoPortWrite(nesl_video_t *video, uint16_t address, uint8_t data)
{
    PORT_SET[address & 7](video, data);
}

uint8_t NESL_VideoRead(nesl_video_t *video, uint16_t address)
{
    int bank = 0;
    uint8_t result = 0;

    switch(address) {
        case 0x3000 ... 0x3EFF:
            address -= 0x1000;
        case 0x2000 ... 0x2FFF:
            address = NESL_VideoNametableAddress(address, *video->mirror, &bank);
            result = video->ram.nametable[bank][address];
            break;
        case 0x3F00 ... 0x3FFF:
            result = video->ram.palette[NESL_VideoPaletteAddress(address)];
            break;
        default:
            break;
    }

    return result;
}

int NESL_VideoReset(nesl_video_t *video, const int *mirror)
{
    memset(video, 0, sizeof(*video));
    video->scanline = -1;
    video->mirror = mirror;

    return NESL_SUCCESS;
}

void NESL_VideoUninit(nesl_video_t *video)
{
    memset(video, 0, sizeof(*video));
}

void NESL_VideoWrite(nesl_video_t *video, uint16_t address, uint8_t data)
{
    int bank = 0;

    switch(address) {
        case 0x3000 ... 0x3EFF:
            address -= 0x1000;
        case 0x2000 ... 0x2FFF:
            address = NESL_VideoNametableAddress(address, *video->mirror, &bank);
            video->ram.nametable[bank][address] = data;
            break;
        case 0x3F00 ... 0x3FFF:
            video->ram.palette[NESL_VideoPaletteAddress(address)] = data;
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
