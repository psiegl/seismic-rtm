// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#ifndef _CHECK_HW_H_
#define _CHECK_HW_H_

#include <stdint.h>
#include <unistd.h>         /* for sysconf / _SC_NPROCESSORS_ONLN */
#include "cpu_features_macros.h"

#ifdef CPU_FEATURES_ARCH_X86_64
# include "cpuinfo_x86.h"
typedef union {
  X86Features in;
  uint64_t bits;
} archfeatures;
#endif /* #ifdef CPU_FEATURES_ARCH_X86_64 */

#ifdef CPU_FEATURES_ARCH_PPC
# include "cpuinfo_ppc.h"
typedef union {
  PPCFeatures in;
  uint64_t bits;
} archfeatures;
#endif /* #ifdef CPU_FEATURES_ARCH_PPC */

#ifdef CPU_FEATURES_ARCH_ARM
# include "cpuinfo_arm.h"
typedef union {
  ArmFeatures in;
  uint64_t bits;
} archfeatures;
#endif /* #ifdef CPU_FEATURES_ARCH_ARM */

#ifdef CPU_FEATURES_ARCH_AARCH64
# include "cpuinfo_aarch64.h"
typedef union {
  Aarch64Features in;
  uint64_t bits;
} archfeatures;
#endif /* #ifdef CPU_FEATURES_ARCH_AARCH64 */

static archfeatures check_hw_capabilites( void )
{
#ifdef CPU_FEATURES_ARCH_X86_64
  archfeatures cap = { .in = GetX86Info().features };
#endif /* #ifdef CPU_FEATURES_ARCH_X86_64 */

#ifdef CPU_FEATURES_ARCH_PPC
  archfeatures cap = { .in = GetPPCInfo().features };
#endif /* #ifdef CPU_FEATURES_ARCH_PPC */

#ifdef CPU_FEATURES_ARCH_ARM
  archfeatures cap = { .in = GetArmInfo().features };
#endif /* #ifdef CPU_FEATURES_ARCH_ARM */

#ifdef CPU_FEATURES_ARCH_AARCH64
  archfeatures cap = { .in = GetAarch64Info().features };
#endif /* #ifdef CPU_FEATURES_ARCH_AARCH64 */
  return cap;
}

static void print_hw_capabilites( archfeatures cap )
{
  if( cap.bits ) {
#ifdef CPU_FEATURES_ARCH_X86_64
//    if( cap & HAS_SSE )
    printf(" sse");
    if( cap.in.avx )
      printf(" avx");
    if( cap.in.avx2 )
      printf(" avx2");
    if( cap.in.fma3 )
      printf(" fma");
#endif

#ifdef CPU_FEATURES_ARCH_PPC
    if( cap.altivec )
      printf(" altivec");
    if( cap.in.vsx )
      printf(" vsx");
#endif /* CPU_FEATURES_ARCH_PPC */

#ifdef CPU_FEATURES_ARCH_ARM
    if( cap.in.neon )
      printf(" neon");
#endif /* CPU_FEATURES_ARCH_ARM */

#ifdef CPU_FEATURES_ARCH_AARCH64
    if( cap.in.asimd )
      printf(" asimd");
#endif /* CPU_FEATURES_ARCH_AARCH64 */
  }
}

static unsigned get_num_cores( void ) {
#if MACOS
    unsigned nm[2];
    size_t len = 4;
    uint32_t count;

    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if(count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
        if(count < 1) { count = 1; }
    }
    return count;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

#endif /* #ifndef _CHECK_HW_H_ */
