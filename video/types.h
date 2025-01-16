//+--------------------------------------------------------------------------
//
// File:        types.h
//
// Original Source info for psram_allocator and associated code:
// NightDriverStrip - (c) 2023 Plummer's Software LLC.  All Rights Reserved.
//
// This file is part of the NightDriver software project.
//
//    NightDriver is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NightDriver is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Nightdriver.  It is normally found in copying.txt
//    If not, see <https://www.gnu.org/licenses/>.
//
// Description:
//
//    Types of a somewhat general use
//
// History:     May-23-2023         Rbergen      Created
//
//---------------------------------------------------------------------------


#pragma once

#include <Arduino.h>

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <optional>
#include <memory>

// PreferPSRAMAlloc
//
// Will return PSRAM if it's available, regular ram otherwise

inline void * PreferPSRAMAlloc(size_t s)
{
	if (psramInit())
	{
		debug_log("PSRAM Array Request for %u bytes\n", s);
		return ps_malloc(s);
	}
	else
	{
		return malloc(s);
	}
}

// psram_allocator
//
// A C++ allocator that allocates from PSRAM instead of the regular heap. Initially
// I had just overloaded new for the classes I wanted in PSRAM, but that doesn't work
// with make_shared<> so I had to provide this allocator instead.
//
// When enabled, this puts all of the LEDBuffers in PSRAM.  The table that keeps track
// of them is still in base ram.
//
// (Davepl - I opted to make this *prefer* psram but return regular ram otherwise. It
//           avoids a lot of ifdef USE_PSRAM in the code.  But I've only proved it
//           correct, not tried it on a chip without yet.

template <typename T>
class psram_allocator
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	psram_allocator(){}
	~psram_allocator(){}

	template <class U> struct rebind { typedef psram_allocator<U> other; };
	template <class U> psram_allocator(const psram_allocator<U>&){}

	pointer address(reference x) const {return &x;}
	const_pointer address(const_reference x) const {return &x;}
	size_type max_size() const throw() {return size_t(-1) / sizeof(value_type);}

	pointer allocate(size_type n, const void * hint = 0)
	{
		void * pmem = PreferPSRAMAlloc(n*sizeof(T));
		return static_cast<pointer>(pmem) ;
	}

	void deallocate(pointer p, size_type n)
	{
		free(p);
	}

	template< class U, class... Args >
	void construct( U* p, Args&&... args )
	{
		::new((void *) p ) U(std::forward<Args>(args)...);
	}

	void destroy(pointer p)
	{
		p->~T();
	}
};

template <typename T, typename U>
bool operator==(const psram_allocator<T>&, const psram_allocator<U>&) { return true; }

template <typename T, typename U>
bool operator!=(const psram_allocator<T>&a, const psram_allocator<U>&b) { return !(a == b); }

// Typically we do not need a deleter because the regular one can handle PSRAM deallocations just fine,
// but for completeness, here it is.

template<typename T>
struct psram_deleter
{
	void operator()(T* ptr)
	{
		psram_allocator<T> allocator;
		allocator.destroy(ptr);
		allocator.deallocate(ptr, 1);
	}
};

// make_unique_psram
//
// Like std::make_unique, but returns PSRAM instead of base RAM.  We cheat a little here by not providing
// a deleter, because we know that PSRAM can be freed with the regular free() call and does not require
// special handling.

template<typename T, typename... Args>
std::unique_ptr<T> make_unique_psram(Args&&... args)
{
	psram_allocator<T> allocator;
	T* ptr = allocator.allocate(1);
	allocator.construct(ptr, std::forward<Args>(args)...);
	return std::unique_ptr<T>(ptr);
}

template<typename T>
std::unique_ptr<T[]> make_unique_psram_array(size_t size)
{
	psram_allocator<T> allocator;
	T* ptr = allocator.allocate(size);
	// No need to call construct since arrays don't have constructors
	return std::unique_ptr<T[]>(ptr);
}

// make_shared_psram
//
// Same as std::make_shared except allocates preferentially from the PSRAM pool

template<typename T, typename... Args>
std::shared_ptr<T> make_shared_psram(Args&&... args)
{
	psram_allocator<T> allocator;
	return std::allocate_shared<T>(allocator, std::forward<Args>(args)...);
}

template<typename T>
std::shared_ptr<T> make_shared_psram_array(size_t size)
{
	psram_allocator<T> allocator;
	return std::allocate_shared<T>(allocator, size);
}


// Data type conversion functions

float float16ToFloat32(uint16_t h) {
    uint32_t sign = ((h >> 15) & 1) << 31;
    uint32_t exponent = ((h >> 10) & 0x1f);
    uint32_t fraction = h & 0x3ff;

    if (exponent == 0) {
        if (fraction == 0) {
            // Zero
            return sign == 0 ? +0.0f : -0.0f;
        } else {
            // Subnormal number
            while ((fraction & 0x400) == 0) {
                fraction <<= 1;
                exponent--;
            }
            exponent++;
            fraction &= 0x3ff;
        }
    } else if (exponent == 31) {
        if (fraction == 0) {
            // Infinity
            return sign == 0 ? +INFINITY : -INFINITY;
        } else {
            // NaN
            return NAN;
        }
    }

    exponent = (exponent + 127 - 15) << 23;
    fraction <<= 13;

    uint32_t f = sign | exponent | fraction;
    return reinterpret_cast<float&>(f);
}

uint16_t float32ToFloat16(float h) {
	uint32_t f = reinterpret_cast<uint32_t&>(h);
	uint32_t sign = (f >> 31) & 0x8000;
	uint32_t exponent = (f >> 23) & 0xff;
	uint32_t fraction = f & 0x7fffff;

	if (exponent == 0) {
		if (fraction == 0) {
			// Zero
			return sign >> 16;
		} else {
			// Subnormal number
			while ((fraction & 0x400000) == 0) {
				fraction <<= 1;
				exponent--;
			}
			exponent++;
			fraction &= 0x7fffff;
		}
	} else if (exponent == 0xff) {
		if (fraction == 0) {
			// Infinity
			return (sign >> 16) | 0x7c00;
		} else {
			// NaN
			return (sign >> 16) | 0x7c00 | (fraction >> 13);
		}
	}

	exponent = (exponent - 127 + 15) << 10;
	fraction >>= 13;

	return (sign >> 16) | exponent | fraction;
}

float convertValueToFloat(uint32_t rawValue, bool is16Bit, bool isFixed, int8_t shift) {
	if (isFixed) {
		// fixed point value
		// we will scale our rawValue by a factor to create our float
		// this version assumes base value is -1 to +1, multiplied by 2^shift
		// so binary point starts at bit 31 and is moved right by shift
		// auto scale = (1u << shift) / (float)(1u << 31);
		// if (is16Bit) {
		// 	// ensure 16-bit values are 32-bit values with their buttom 16 bits empty
		// 	rawValue <<= 16;
		// }
		// in this version the binary point starts after bit 0 and is moved left by shift
		// which is arguably a bit more intuitive in use, and matches the xtensa fixed point instruction support
		// we need to use our shift value to determine how far to scale
		auto scale = shift < 0 ? 1 << -shift : 1.0f / (1 << shift);
		if (is16Bit) {
			return (float)(int16_t)rawValue * scale;
		}
		return (float)(int32_t)rawValue * scale;
	} else {
		// floating point value - shift ignored
		// if we're reading a 16-bit value, we need to convert to 32-bit float
		if (is16Bit) {
			return float16ToFloat32(rawValue);
		}
		// take our raw value and interpret its bits as a float
		return *(float*)&rawValue;
	}
};

uint32_t convertFloatToValue(float rawValue, bool is16Bit, bool isFixed, int8_t shift) {
	if (isFixed) {
		// fixed point value
		// we will scale our rawValue by a factor to create our float
		// this version assumes base value is -1 to +1, multiplied by 2^shift
		// so binary point starts at bit 31 and is moved right by shift
		// auto scale = (1u << shift) / (float)(1u << 31);
		// in this version the binary point starts after bit 0 and is moved left by shift
		// which is arguably a bit more intuitive in use, and matches the xtensa fixed point instruction support
		// we need to use our shift value to determine how far to scale
		auto scale = shift < 0 ? 1 << -shift : 1.0f / (1 << shift);
		if (is16Bit) {
			return (uint16_t)(rawValue / scale);
		}
		return (uint32_t)(rawValue / scale);
	} else {
		// floating point value - shift ignored
		// if we're writing a 16-bit value, we need to convert from 32-bit float
		if (is16Bit) {
			return float32ToFloat16(rawValue);
		}
		// take our raw value and interpret its bits as a float
		return *(uint32_t*)&rawValue;
	}
};

// This returns an INT32 because it will return either the value
// in the range 0-65535 or -1 if the string is invalid.
int32_t textToWord(const char * text) {
	int32_t val = 0;
	int8_t digit = 0;

	if (!isdigit(text[0])) {
		debug_log("convert to ASCII text %s invalid\n\r", text);
		return -1;
	};
	while (isdigit(text[digit]) && digit<6) {
		val = (val * 10) + text[digit] - '0';
		digit++;
	};
	debug_log("converted text %s is %u\n\r", text,val);
	return (val < 65536 ? val : -1);
};
