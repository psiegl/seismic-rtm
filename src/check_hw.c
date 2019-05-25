// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#ifdef __x86_64__
# include <cpuid.h>
#endif /* #ifdef __x86_64__ */
#if defined( __ALTIVEC__ ) && defined( __VSX__ )
# include <stdio.h>
#endif /* #if defined( __ALTIVEC__ ) && defined( __VSX__ ) */

#if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP )
# include <sys/auxv.h>
# include <asm/hwcap.h>
#endif /* #if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP ) */
#include "check_hw.h"

/* option to parse lscpu? */
uint32_t check_hw_capabilites( void )
{
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

#if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP ) // __ARM_ARCH
#ifdef HWCAP_NEON
  if(getauxval(AT_HWCAP) & HWCAP_NEON)
    cap |= HAS_NEON;  
#endif /* #ifdef HWCAP_NEON */
#ifdef HWCAP_ASIMD
  if(getauxval(AT_HWCAP) & HWCAP_ASIMD)
    cap |= HAS_NEON | HAS_ASIMD;
#endif /* #ifdef HWCAP_ASIMD */
#endif /* #if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP ) */
  return cap;
}
