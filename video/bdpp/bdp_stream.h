//
// Title:			Agon Video BIOS
// Author:			Dean Belfield
// Subtitle:		Bidirectional Packet Protocol
// Author:			Curtis Whitley
// Contributors:	
// Created:			13/02/2024
// Last Updated:	13/02/2024
//

#pragma once
#include <Stream.h>
#include "bdpp/packet.h"

// This class represents a stream of data coming from BDPP.
//
class BdppStream : public Stream {
    public:
    BdppStream(uint8_t stream_index) {
        this->stream_index = stream_index;
    }

    virtual int available() {
        
    }

    virtual int read() {

    }

    virtual int peek() {

    }

    virtual size_t readBytes(char *buffer, size_t length);

    virtual size_t readBytes(uint8_t *buffer, size_t length)
    {
        return readBytes((char *) buffer, length);
    }

    virtual String readString();

    protected:
    uint8_t stream_index; // index of this string (0..BDPP_MAX_STREAMS-1)
};
