/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2024 Members of the Gmerlin project
 * http://github.com/bplaum
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/



/* bswap.h taken from ffmpeg and adjusted to gavl,
 * original copyright below
 */

/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file bswap.h
 * byte swap.
 */

#ifndef BSWAP_H_INCLUDED
#define BSWAP_H_INCLUDED

#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#else

#ifdef ARCH_X86_64
#  define LEGACY_REGS "=Q"
#else
#  define LEGACY_REGS "=q"
#endif

static inline uint16_t bswap_16(uint16_t x)
{
#if defined(ARCH_X86)
  __asm("rorw $8, %0"   :
        LEGACY_REGS (x) :
        "0" (x));
#elif defined(ARCH_SH4)
        __asm__("swap.b %0,%0":"=r"(x):"0"(x));
#else
    x= (x>>8) | (x<<8);
#endif
    return x;
}

static inline uint32_t bswap_32(uint32_t x)
{
#if defined(ARCH_X86)
#if __CPU__ != 386
 __asm("bswap   %0":
      "=r" (x)     :
#else
 __asm("xchgb   %b0,%h0\n"
      "         rorl    $16,%0\n"
      "         xchgb   %b0,%h0":
      LEGACY_REGS (x)                :
#endif
      "0" (x));
#elif defined(ARCH_SH4)
        __asm__(
        "swap.b %0,%0\n"
        "swap.w %0,%0\n"
        "swap.b %0,%0\n"
        :"=r"(x):"0"(x));
#elif defined(ARCH_ARM)
    uint32_t t;
    __asm__ (
      "eor %1, %0, %0, ror #16 \n\t"
      "bic %1, %1, #0xFF0000   \n\t"
      "mov %0, %0, ror #8      \n\t"
      "eor %0, %0, %1, lsr #8  \n\t"
      : "+r"(x), "+r"(t));
#elif defined(ARCH_BFIN)
    unsigned tmp;
    asm("%1 = %0 >> 8 (V);\n\t"
        "%0 = %0 << 8 (V);\n\t"
        "%0 = %0 | %1;\n\t"
        "%0 = PACK(%0.L, %0.H);\n\t"
        : "+d"(x), "=&d"(tmp));
#else
    x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
    x= (x>>16) | (x<<16);
#endif
    return x;
}

static inline uint64_t bswap_64(uint64_t x)
{
#if 0
    x= ((x<< 8)&0xFF00FF00FF00FF00ULL) | ((x>> 8)&0x00FF00FF00FF00FFULL);
    x= ((x<<16)&0xFFFF0000FFFF0000ULL) | ((x>>16)&0x0000FFFF0000FFFFULL);
    return (x>>32) | (x<<32);
#elif defined(ARCH_X86_64)
  __asm("bswap  %0":
        "=r" (x)   :
        "0" (x));
  return x;
#else
    union {
        uint64_t ll;
        uint32_t l[2];
    } w, r;
    w.ll = x;
    r.l[0] = bswap_32 (w.l[1]);
    r.l[1] = bswap_32 (w.l[0]);
    return r.ll;
#endif
}

#endif  /* !HAVE_BYTESWAP_H */

#endif /* BSWAP_H_INCLUDED */
