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



#include <gavl.h>
#include <config.h>
#include <accel.h>

/*
 *   Test for available CPU extensions
 *   Shamelessly stolen from ffmpeg (ffmpeg.sf.net)
 */

#define ACCEL_GENERIC 0

/*
 *  Wrappers for foreign flag constants
 */

#define MM_MMX      GAVL_ACCEL_MMX
#define MM_MMXEXT   GAVL_ACCEL_MMXEXT
#define MM_SSE      GAVL_ACCEL_SSE
#define MM_SSE2     GAVL_ACCEL_SSE2
#define MM_SSE3     GAVL_ACCEL_SSE3
#define MM_SSSE3    GAVL_ACCEL_SSSE3
#define MM_3DNOW    GAVL_ACCEL_3DNOW
#define MM_3DNOWEXT GAVL_ACCEL_3DNOWEXT

#ifdef ARCH_X86_64
#  define REG_b "rbx"
#  define REG_S "rsi"
#else
#  define REG_b "ebx"
#  define REG_S "esi"
#endif

/* ebx saving is necessary for PIC. gcc seems unable to see it alone */
#define cpuid(index,eax,ebx,ecx,edx)\
    __asm __volatile\
        ("mov %%"REG_b", %%"REG_S"\n\t"\
         "cpuid\n\t"\
         "xchg %%"REG_b", %%"REG_S\
         : "=a" (eax), "=S" (ebx),\
           "=c" (ecx), "=d" (edx)\
         : "0" (index));

/* Function to test if multimedia instructions are supported...  */

int gavl_accel_supported()
  {
#ifdef ARCH_X86
     int rval = 0;
    int eax, ebx, ecx, edx;
    int max_std_level, max_ext_level, std_caps=0, ext_caps=0;

#ifndef ARCH_X86_64
    long a, c;

    __asm__ __volatile__ (
                          /* See if CPUID instruction is supported ... */
                          /* ... Get copies of EFLAGS into eax and ecx */
                          "pushf\n\t"
                          "pop %0\n\t"
                          "mov %0, %1\n\t"

                          /* ... Toggle the ID bit in one copy and store */
                          /*     to the EFLAGS reg */
                          "xor $0x200000, %0\n\t"
                          "push %0\n\t"
                          "popf\n\t"

                          /* ... Get the (hopefully modified) EFLAGS */
                          "pushf\n\t"
                          "pop %0\n\t"
                          : "=a" (a), "=c" (c)
                          :
                          : "cc"
                          );

    if (a == c)
        return 0; /* CPUID not supported */
#endif // !ARCH_X86_64

    cpuid(0, max_std_level, ebx, ecx, edx);

    if(max_std_level >= 1){
        cpuid(1, eax, ebx, ecx, std_caps);
        if (std_caps & (1<<23))
            rval |= MM_MMX;
        if (std_caps & (1<<25))
            rval |= MM_MMXEXT | MM_SSE;
        if (std_caps & (1<<26))
            rval |= MM_SSE2;
        if (ecx & 1)
            rval |= MM_SSE3;
        if (ecx & 0x00000200 )
          rval |= MM_SSSE3;

    }

    cpuid(0x80000000, max_ext_level, ebx, ecx, edx);

    if(max_ext_level >= 0x80000001){
        cpuid(0x80000001, eax, ebx, ecx, ext_caps);
        if (ext_caps & (1<<31))
            rval |= MM_3DNOW;
        if (ext_caps & (1<<30))
            rval |= MM_3DNOWEXT;
        if (ext_caps & (1<<23))
            rval |= MM_MMX;
        if (ext_caps & (1<<22))
            rval |= MM_MMXEXT;
    }

    return rval|ACCEL_GENERIC;
#else
  return ACCEL_GENERIC;
#endif
  }
