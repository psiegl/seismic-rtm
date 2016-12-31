//  This file is part of seismic-rtm.
//
//  seismic-rtm is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  seismic-rtm is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seismic.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _CHECK_HW_H_
#define _CHECK_HW_H_

#include <stdint.h>
#ifdef __x86_64__
#include <cpuid.h>
#endif /* #ifdef __x86_64__ */
#if defined( __ALTIVEC__ ) && defined( __VSX__ )
#include <stdio.h>
#endif /* #if defined( __ALTIVEC__ ) && defined( __VSX__ ) */

#define HAS_SSE   (1 << 0)
#define HAS_SSE2  (1 << 1)
#define HAS_SSE3  (1 << 2)
#define HAS_SSSE3 (1 << 3)
#define HAS_SSE41 (1 << 4)
#define HAS_SSE42 (1 << 5)
#define HAS_SSE4a (1 << 6)
#define HAS_FMA   (1 << 7)
#define HAS_FMA4  (1 << 8)
#define HAS_AVX   (1 << 9)
#define HAS_AVX2  (1 << 10)
#define HAS_VMX   (1 << 11)
#define HAS_VSX   (1 << 12)

uint32_t check_hw_capabilites( void ) {
  uint32_t cap = 0;
#ifdef __x86_64__
  if( __builtin_cpu_supports("sse") )
    cap |= HAS_SSE;
  if( __builtin_cpu_supports("avx") )
    cap |= HAS_AVX;
  if( __builtin_cpu_supports("avx2") )
    cap |= HAS_AVX2;
//  if(__builtin_cpu_supports("fma")) // not always supported
//    cap |= HAS_FMA;
  int info[4];
  __cpuid_count(0x00000001, 0, info[0], info[1], info[2], info[3]);
  if((info[2] & ((int)1 << 12)) == ((int)1 << 12))
    cap |= HAS_FMA;
#endif /* #ifdef __x86_64__ */
#ifdef __ALTIVEC__
  cap |= HAS_VMX;
#ifdef __VSX__
  // needs at least Power ISA v2.06 (e.g. POWER7)
  FILE *f = fopen("/proc/cpuinfo", "r");
  char b[256];
  while(!feof(f)) {
    fgets(b, 256, f);
    if(!memcmp(b, "cpu", 3)) {
      int version;
      cap |= HAS_VSX * (sscanf(b, "%*[^0123456789]%d", &version) && version >= 7);
      break;
    }
  }
  fclose(f);
#endif /* #ifdef __VSX__ */
#endif /* #ifdef __ALTIVEC__ */

  return cap;
}


#endif /* #ifndef _CHECK_HW_H_ */
