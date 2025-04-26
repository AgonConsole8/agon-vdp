#ifndef VDU_FONTS_H
#define VDU_FONTS_H

#include "agon.h"
#include "agon_fonts.h"
#include "types.h"
#include "vdu_stream_processor.h"


// VDU 23, 0, &95, command, [<args>]: Font management command support
//
void VDUStreamProcessor::vdu_sys_font() {
	auto command = readByte_t(); if (command == -1) return;
    
    switch (command) {
        case FONT_SELECT: {
            // VDU 23, 0, &95, 0, bufferId; flags  - Select font by bufferId
            auto bufferId = readWord_t(); if (bufferId == -1) return;
            auto flags = readByte_t(); if (flags == -1) return;
            context->changeFont(bufferId, flags);
            sendModeInformation();
        } break;
        case FONT_FROM_BUFFER: {
            // VDU 23, 0, &95, 1, bufferId; width, height, ascent, flags  - Load font from buffer
            // maybe add on chptrbuffer at the end? for var-width fonts??
            auto bufferId = readWord_t(); if (bufferId == -1) return;
            auto width = readByte_t(); if (width == -1) return;
            auto height = readByte_t(); if (height == -1) return;
            auto ascent = readByte_t(); if (ascent == -1) return;
            auto flags = readByte_t(); if (flags == -1) return;
            createFontFromBuffer(bufferId, width, height, ascent, flags);
        } break;
        case FONT_SET_INFO: {
            // VDU 23, 0, &95, 2, bufferId; field, value;  - Set font info
            auto bufferId = readWord_t(); if (bufferId == -1) return;
            auto field = readByte_t(); if (field == -1) return;
            auto value = readWord_t(); if (value == -1) return;
            setFontInfo(bufferId, field, value);
            sendModeInformation();
        } break;
        case FONT_SET_NAME: {
            // either it will be: VDU 23, 0, &95, 3, bufferId; <ZeroTerminatedString>  - Set font name
            // or VDU 23, 0, &95, bufferId; 3, namefield, <ZeroTerminatedString>  - Set font name
            // use of a field identifier allows for future expansion ??? not sure what for tho
        	auto bufferId = readWord_t(); if (bufferId == -1) return;
            debug_log("fontSetName: not yet implemented\n\r");
        } break;
        case FONT_CLEAR: {
            // VDU 23, 0, &95, 4, bufferId;  - Clear font
            auto bufferId = readWord_t(); if (bufferId == -1) return;
            if (bufferId == 65535) {
                // clear all fonts
                resetFonts();
            } else {
                clearFont(bufferId);
            }
            sendModeInformation();
        } break;
        case FONT_COPY_SYSTEM: {
            // VDU 23, 0, &95, 5, bufferId;  - Copy system font
            auto bufferId = readWord_t(); if (bufferId == -1) return;
            bufferClear(bufferId);
            auto size = 256 * (((FONT_AGON.width + 7) >> 3) * FONT_AGON.height);
            auto buff = bufferCreate(bufferId, size);
            if (buff == nullptr) {
                debug_log("fontCopySystem: failed to create buffer %d\n\r", bufferId);
                return;
            }
            memcpy(buff->getBuffer(), FONT_AGON.data, size);
            auto fontCopy = createFontFromBuffer(bufferId, FONT_AGON.width, FONT_AGON.height, FONT_AGON.ascent, FONT_AGON.flags);
            if (fontCopy == nullptr) {
                debug_log("fontCopySystem: failed to create font %d\n\r", bufferId);
                return;
            }
            fontCopy->pointSize = FONT_AGON.pointSize;
            sendModeInformation();
        } break;
        case FONT_SELECT_BY_NAME: {
            // VDU 23, 0, &95, &10, <ZeroTerminatedString>  - Select font by name
            debug_log("fontSelectByName: not yet implemented\n\r");
        } break;
        case FONT_DEBUG_INFO: {
            // VDU 23, 0, &95, &20, bufferId;  - Get font debug info
            auto bufferId = readWord_t(); if (bufferId == -1) return;
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

#endif // VDU_FONTS_H
