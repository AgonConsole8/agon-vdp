#ifndef VDU_FONTS_H
#define VDU_FONTS_H

#include <memory>
#include <unordered_map>

#include <fabgl.h>

#include "agon.h"
#include "graphics.h"
#include "types.h"
#include "vdu_stream_processor.h"

std::unordered_map<uint16_t, std::shared_ptr<fabgl::FontInfo>> fonts;	// Storage for our fonts

// VDU 23, 0, &93, bufferId; command, [<args>]: Font management command support
//
void VDUStreamProcessor::vdu_sys_font() {
	auto bufferId = readWord_t(); if (bufferId == -1) return;
	auto command = readByte_t(); if (command == -1) return;
    
    switch (command) {
        case FONT_SELECT: {
            // VDU 23, 0, &93, bufferId; 0  - Select font by bufferId
            if (bufferId == 65535) {
                // reset to default font
                changeFont(&fabgl::FONT_AGON);
            } else {
                if (fonts.find(bufferId) == fonts.end()) {
                    debug_log("fontSelect: font %d not found\n\r", bufferId);
                    return;
                }
                changeFont(fonts[bufferId].get());
            }
        } break;
        case FONT_FROM_BUFFER: {
            // VDU 23, 0, &93, bufferId; 1, width, height, ascent, flags  - Load font from buffer
            // maybe add on chptrbuffer at the end? for var-width fonts??
            auto width = readByte_t(); if (width == -1) return;
            auto height = readByte_t(); if (height == -1) return;
            auto ascent = readByte_t(); if (ascent == -1) return;
            auto flags = readByte_t(); if (flags == -1) return;
            createFontFromBuffer(bufferId, width, height, ascent, flags);
        } break;
        case FONT_SET_INFO: {
            // VDU 23, 0, &93, bufferId; 2, field, value;  - Set font info
            auto field = readByte_t(); if (field == -1) return;
            auto value = readWord_t(); if (value == -1) return;
            setFontInfo(bufferId, field, value);
        } break;
        case FONT_SET_NAME: {
            // either it will be: VDU 23, 0, &93, bufferId; 3, <ZeroTerminatedString>  - Set font name
            // or VDU 23, 0, &93, bufferId; 3, namefield, <ZeroTerminatedString>  - Set font name
            // use of a field identifier allows for future expansion ??? not sure what for tho
            debug_log("fontSetName: not yet implemented\n\r");
        } break;
        case FONT_SELECT_BY_NAME: {
            // VDU 23, 0, &93, bufferId; &10, <ZeroTerminatedString>  - Select font by name
            debug_log("fontSelectByName: not yet implemented\n\r");
        } break;
        case FONT_DEBUG_INFO: {
            // VDU 23, 0, &93, bufferId; &20  - Get font debug info
            if (fonts.find(bufferId) == fonts.end()) {
                debug_log("fontDebugInfo: font %d not found\n\r", bufferId);
                return;
            }
            auto font = fonts[bufferId];
            debug_log("Font %d: %dx%d, ascent %d, flags %d, point size %d, inleading %d, exleading %d, weight %d, charset %d, codepage %d\n\r",
                bufferId, font->width, font->height, font->ascent, font->flags, font->pointSize, font->inleading, font->exleading, font->weight, font->charset, font->codepage);
        } break;
    }
}

void VDUStreamProcessor::createFontFromBuffer(uint16_t bufferId, uint8_t width, uint8_t height, uint8_t ascent, uint8_t flags) {
	if (bufferId == 65535 || (buffers.find(bufferId) == buffers.end())) {
		debug_log("createFontFromBuffer: buffer %d not found\n\r", bufferId);
		return;
	}
	if (buffers[bufferId].size() != 1) {
		debug_log("createFontFromBuffer: buffer %d is not a singular buffer and cannot be used for a font source\n\r", bufferId);
		return;
	}

    if (~flags & FONTINFOFLAGS_VARWIDTH) {
        // Font is fixed width, so we can calculate the size that our font data should be
        auto size = ((width + 7) >> 3) * height * 256;
        if (buffers[bufferId][0]->size() != size) {
            debug_log("createFontFromBuffer: buffer %d is not the correct size for a fixed width font\n\r", bufferId);
            return;
        }
    } else {
        // Variable width fonts not yet supported - will be in the future
        debug_log("createFontFromBuffer: variable width fonts not yet supported\n\r");
        return;
    }

	auto data = buffers[bufferId][0]->getBuffer();

    auto font = std::make_shared<fabgl::FontInfo>();
    font->width = width;
    font->height = height;
    font->ascent = ascent;
    font->flags = flags;
    font->data = data;

    // Fill in default/empty values for the rest of the fields
    font->chptr = nullptr;
    font->pointSize = 0;
    font->inleading = 0;
    font->exleading = 0;
    font->weight = 400;
    font->charset = 255;
    font->codepage = 1215;

    fonts[bufferId] = font;
}

void VDUStreamProcessor::setFontInfo(uint16_t bufferId, uint8_t field, uint16_t value) {
    if (fonts.find(bufferId) == fonts.end()) {
        debug_log("setFontInfo: font %d not found\n\r", bufferId);
        return;
    }

    auto font = fonts[bufferId];
    switch (field) {
        case FONT_INFO_WIDTH: {
            font->width = (uint8_t) value;
        } break;
        case FONT_INFO_HEIGHT: {
            font->height = (uint8_t) value;
        } break;
        case FONT_INFO_ASCENT: {
            font->ascent = (uint8_t) value;
        } break;
        case FONT_INFO_FLAGS: {
            font->flags = (uint8_t) value;
        } break;
        case FONT_INFO_CHARPTRS_BUFFER: {
            if (buffers.find(value) == buffers.end()) {
                debug_log("setFontInfo: buffer %d for character pointers not found\n\r", value);
                return;
            }
            if (buffers[value].size() != 1) {
                debug_log("setFontInfo: buffer %d is not a singular buffer and cannot be used for a font character pointer source\n\r", value);
                return;
            }
            font->chptr = (const uint32_t*) (buffers[value][0]->getBuffer());
        } break;
        case FONT_INFO_POINTSIZE: {
            font->pointSize = (uint8_t) value;
        } break;
        case FONT_INFO_INLEADING: {
            font->inleading = (uint8_t) value;
        } break;
        case FONT_INFO_EXLEADING: {
            font->exleading = (uint8_t) value;
        } break;
        case FONT_INFO_WEIGHT: {
            font->weight = value;
        } break;
        case FONT_INFO_CHARSET: {
            font->charset = value;
        } break;
        case FONT_INFO_CODEPAGE: {
            font->codepage = value;
        } break;
    }
}

#endif // VDU_FONTS_H
