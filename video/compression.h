#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <cstring>
#include <esp_heap_caps.h>
#include <stdint.h>
#include <esp32-hal-psram.h>

extern void debug_log(const char * format, ...);		// Debug log function

// This implementation uses a window size of 256 bytes, and a code size of 10 bits.
// The maximum compressed byte string size is 16 bytes.
//
// Code bits:
// 0000000000
// 9876543210
// ----------
// 00xxxxxxxx   New original data byte with value of xxxxxxxx
// 01iiiiiiii   String of 4 bytes starting at window index iiiiiiii
// 10iiiiiiii   String of 8 bytes starting at window index iiiiiiii
// 11iiiiiiii   String of 16 bytes starting at window index iiiiiiii
//
// Note: Worst case, the output can be 25% LARGER than the input!

#define COMPRESSION_WINDOW_SIZE 256     // power of 2
#define COMPRESSION_STRING_SIZE 16      // power of 2
#define COMPRESSION_TYPE_TURBO  'T'     // TurboVega-style compression
#define TEMP_BUFFER_SIZE        256

#define COMPRESSION_OUTPUT_CHUNK_SIZE	1024 // used to extend temporary buffer

#pragma pack(push, 1)
typedef struct {
    uint8_t     marker[3];
    uint8_t     type;
    uint32_t    orig_size;
} CompressionFileHeader;
#pragma pack(pop)


typedef void (*WriteCompressedByte)(void*, uint8_t);
typedef bool (*WriteDecompressedByte)(void*, uint8_t);

typedef struct {
    void*               context;
    WriteCompressedByte write_fcn;
    uint32_t            window_size;
    uint32_t            window_write_index;
    uint32_t            string_size;
    uint32_t            string_read_index;
    uint32_t            string_write_index;
    uint32_t            input_count;
    uint32_t            output_count;
    uint8_t             window_data[COMPRESSION_WINDOW_SIZE];
    uint8_t             string_data[COMPRESSION_STRING_SIZE];
    uint8_t             temp_buffer[TEMP_BUFFER_SIZE];
    uint8_t             out_byte;
    uint8_t             out_bits;
} CompressionData;

typedef struct {
    void*               context;
    WriteDecompressedByte write_fcn;
    uint32_t            window_size;
    uint32_t            window_write_index;
    uint32_t            input_count;
    uint32_t            output_count;
    uint32_t            orig_size;
    uint8_t             window_data[COMPRESSION_WINDOW_SIZE];
    uint8_t             temp_buffer[TEMP_BUFFER_SIZE];
    uint16_t            code;
    uint8_t             code_bits;
} DecompressionData;

void agon_init_compression(CompressionData* cd, void* context, WriteCompressedByte write_fcn) {
    memset(cd, 0, sizeof(CompressionData));
    cd->context = context;
    cd->write_fcn = write_fcn;
}

void agon_write_compressed_bit(CompressionData* cd, uint8_t comp_bit) {
    cd->out_byte = (cd->out_byte << 1) | comp_bit;
    if (++(cd->out_bits) >= 8) {
        (*cd->write_fcn)(cd, cd->out_byte);
        cd->out_byte = 0;
        cd->out_bits = 0;
    }
}

void agon_write_compressed_byte(CompressionData* cd, uint8_t comp_byte) {
    for (uint8_t bit = 0; bit < 8; bit++) {
        agon_write_compressed_bit(cd, (comp_byte & 0x80) ? 1 : 0);
        comp_byte <<= 1;
    }
}

// Write compressed output to a temporary buffer
//
static void local_write_compressed_byte(void* p_cd, uint8_t comp_byte) {
	CompressionData* cd = (CompressionData*) p_cd;
    uint8_t** pp_temp = (uint8_t**) cd->context;
	uint8_t* p_temp = *pp_temp;
	p_temp[cd->output_count++] = comp_byte;
	if (!(cd->output_count & (COMPRESSION_OUTPUT_CHUNK_SIZE-1))) {
		// extend the temporary buffer to make room for more output
		auto new_size = cd->output_count + COMPRESSION_OUTPUT_CHUNK_SIZE;
		uint8_t* p_temp2 = (uint8_t*) ps_malloc(new_size);
		if (p_temp2) {
			memcpy(p_temp2, p_temp, cd->output_count);
			*pp_temp = p_temp2; // use new buffer pointer now
		} else {
			debug_log("bufferCompress: cannot allocate temporary buffer of %d bytes\n\r", new_size);
			*pp_temp = NULL; // indicate failure
		}
		heap_caps_free(p_temp);
	}
}

// Write decompressed output to a temporary buffer
//
static bool local_write_decompressed_byte(void* p_dd, uint8_t orig_data) {
	DecompressionData* dd = (DecompressionData*) p_dd;
    uint8_t** pp_temp = (uint8_t**) dd->context;
	uint8_t* p_temp = *pp_temp;
    if (dd->output_count >= dd->orig_size) {
        return false;
    }
	p_temp[dd->output_count++] = orig_data;
    return true;
}

void agon_compress_byte(CompressionData* cd, uint8_t orig_byte) {
    // Add the new original byte to the string
    cd->string_data[cd->string_write_index++] = orig_byte;
    cd->string_write_index &= (COMPRESSION_STRING_SIZE - 1);
    if (cd->string_size < COMPRESSION_STRING_SIZE) {
        (cd->string_size)++;
    } else {
        cd->string_read_index = (cd->string_read_index + 1) & (COMPRESSION_STRING_SIZE - 1);
    }

    if (cd->string_size >= 16) {
        if (cd->window_size >= 16) {
            // Determine whether the full string of 16 is in the window
            for (uint32_t start = 0; start <= cd->window_size - 16; start++) {
                uint32_t wi = start;
                uint32_t si = cd->string_read_index;
                uint8_t match = 1;
                for (uint8_t i = 0; i < 16; i++) {
                    if (cd->window_data[wi++] != cd->string_data[si++]) {
                        match = 0;
                        break;
                    }
                    wi &= (COMPRESSION_WINDOW_SIZE - 1);
                    si &= (COMPRESSION_STRING_SIZE - 1);
                }
                if (match) {
                    agon_write_compressed_bit(cd, 1); // Output first '1' of '11iiiiiiii'.
                    agon_write_compressed_bit(cd, 1); // Output second '1' of '11iiiiiiii'.
                    agon_write_compressed_byte(cd, (uint8_t) (start)); // Output window index
                    cd->string_size = 0;
                    return;
                }
            }
        }

        if (cd->window_size >= 8) {
            // Determine whether the partial string of 8 is in the window
            for (uint32_t start = 0; start <= cd->window_size - 8; start++) {
                uint32_t wi = start;
                uint32_t si = cd->string_read_index;
                uint8_t match = 1;
                for (uint8_t i = 0; i < 8; i++) {
                    if (cd->window_data[wi++] != cd->string_data[si++]) {
                        match = 0;
                        break;
                    }
                    wi &= (COMPRESSION_WINDOW_SIZE - 1);
                    si &= (COMPRESSION_STRING_SIZE - 1);
                }
                if (match) {
                    agon_write_compressed_bit(cd, 1); // Output '1' of '10iiiiiiii'.
                    agon_write_compressed_bit(cd, 0); // Output '0' of '10iiiiiiii'.
                    agon_write_compressed_byte(cd, (uint8_t) (start)); // Output window index
                    cd->string_size -= 8;
                    cd->string_read_index = (cd->string_read_index + 8) & (COMPRESSION_STRING_SIZE - 1);
                    return;
                }
            }
        }
            
        if (cd->window_size >= 4) {
            // Determine whether the partial string of 4 is in the window
            for (uint32_t start = 0; start <= cd->window_size - 4; start++) {
                uint32_t wi = start;
                uint32_t si = cd->string_read_index;
                uint8_t match = 1;
                for (uint8_t i = 0; i < 4; i++) {
                    if (cd->window_data[wi++] != cd->string_data[si++]) {
                        match = 0;
                        break;
                    }
                    wi &= (COMPRESSION_WINDOW_SIZE - 1);
                    si &= (COMPRESSION_STRING_SIZE - 1);
                }
                if (match) {
                    agon_write_compressed_bit(cd, 0); // Output '0' of '01iiiiiiii'.
                    agon_write_compressed_bit(cd, 1); // Output '1' of '01iiiiiiii'.
                    agon_write_compressed_byte(cd, (uint8_t) (start)); // Output window index
                    cd->string_size -= 4;
                    cd->string_read_index = (cd->string_read_index + 4) & (COMPRESSION_STRING_SIZE - 1);
                    return;
                }
            }
        }

        // Need to make room in the string for the next original byte
        uint8_t old_byte = cd->string_data[cd->string_read_index++];
        agon_write_compressed_bit(cd, 0); // Output '0' of '00xxxxxxxx'.
        agon_write_compressed_bit(cd, 0); // Output '0' of '00xxxxxxxx'.
        agon_write_compressed_byte(cd, (uint8_t) (old_byte)); // Output old original byte
        cd->string_size -= 1;
        cd->string_read_index &= (COMPRESSION_STRING_SIZE - 1);

        // Add the old original byte to the window
        cd->window_data[cd->window_write_index++] = old_byte;
        cd->window_write_index &= (COMPRESSION_WINDOW_SIZE - 1);
        if (cd->window_size < COMPRESSION_WINDOW_SIZE) {
            (cd->window_size)++;
        }
    }
}

void agon_finish_compression(CompressionData* cd) {
    while (cd->string_size) {
        agon_write_compressed_bit(cd, 0); // Output '0' of '00xxxxxxxx'.
        agon_write_compressed_bit(cd, 0); // Output '0' of '00xxxxxxxx'.
        agon_write_compressed_byte(cd, (uint8_t) (cd->string_data[cd->string_read_index++])); // Output orig byte
        cd->string_size -= 1;
        cd->string_read_index &= (COMPRESSION_STRING_SIZE - 1);
    }
    if (cd->out_bits) {
        (*cd->write_fcn)(cd, (cd->out_byte << (8 - cd->out_bits))); // Output final bits
    }
}

void agon_init_decompression(DecompressionData* dd, void* context, WriteDecompressedByte write_fcn, uint32_t orig_size) {
    memset(dd, 0, sizeof(DecompressionData));
    dd->context = context;
    dd->write_fcn = write_fcn;
    dd->orig_size = orig_size;
}

void agon_decompress_byte(DecompressionData* dd, uint8_t comp_byte) {
    for (uint8_t bit = 0; bit < 8; bit++) {
        dd->code = (dd->code << 1) | ((comp_byte & 0x80) ? 1 : 0);
        comp_byte <<= 1;
        if (++(dd->code_bits) >= 10) {
            // Interpret the incoming code
            uint16_t command = (dd->code >> 8);
            uint8_t value = (uint8_t)dd->code;
            dd->code = 0;
            dd->code_bits = 0;
            uint8_t size;

            switch (command)
            {
            case 0: // value is copy of original byte
                // Add the new decompressed byte to the window
                dd->window_data[dd->window_write_index++] = value;
                dd->window_write_index &= (COMPRESSION_WINDOW_SIZE - 1);
                if (dd->window_size < COMPRESSION_WINDOW_SIZE) {
                    (dd->window_size)++;
                }
                (*(dd->write_fcn))(dd, value);
                continue;

            case 1: // value is index to string of 4 bytes
                size = 4;
                break;

            case 2: // value is index to string of 8 bytes
                size = 8;
                break;

            case 3: // value is index to string of 16 bytes
                size = 16;
                break;
            }

            // Extract a byte string from the window
            uint8_t string_data[COMPRESSION_STRING_SIZE];
            uint32_t wi = value;
            for (uint8_t si = 0; si < size; si++) {
                uint8_t out_byte = dd->window_data[wi++];
                wi &= (COMPRESSION_WINDOW_SIZE - 1);
                string_data[si] = out_byte;
                if (!(*(dd->write_fcn))(dd, out_byte)) {
                    // decompression has overflowed the output buffer
                    debug_log("Decompression overflow\n\r");
                    return;
                };
            }
        }
    }
}

#endif // COMPRESSION_H
