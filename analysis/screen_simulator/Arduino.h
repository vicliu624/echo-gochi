#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))
#endif

#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#endif

#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const uint32_t*)(addr))
#endif

#ifndef memcpy_P
#define memcpy_P(dest, src, len) memcpy((dest), (src), (len))
#endif

#ifndef F
#define F(value) value
#endif

using byte = uint8_t;
