// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Dr.-Ing. Patrick Siegl patrick@siegl.it

#ifndef _CHECK_HW_H_
#define _CHECK_HW_H_

#include <stdint.h>
#include <unistd.h>         /* for sysconf / _SC_NPROCESSORS_ONLN */
#include "cpu_features_macros.h"

#ifdef CPU_FEATURES_ARCH_X86_64
# include "cpuinfo_x86.h"
#elif defined(CPU_FEATURES_ARCH_PPC)
# include "cpuinfo_ppc.h"
#elif defined(CPU_FEATURES_ARCH_ARM)
# include "cpuinfo_arm.h"
#elif defined(CPU_FEATURES_ARCH_AARCH64)
# include "cpuinfo_aarch64.h"
#endif

typedef union {
#ifdef CPU_FEATURES_ARCH_X86_64
  X86Features in;
#elif defined(CPU_FEATURES_ARCH_PPC)
  PPCFeatures in;
#elif defined(CPU_FEATURES_ARCH_ARM)
  ArmFeatures in;
#elif defined(CPU_FEATURES_ARCH_AARCH64)
  Aarch64Features in;
#endif
  uint64_t bits;
} archfeatures;

static archfeatures check_hw_capabilites( void )
{
  archfeatures cap = {
#ifdef CPU_FEATURES_ARCH_X86_64
    .in = GetX86Info().features
#elif defined(CPU_FEATURES_ARCH_PPC)
    .in = GetPPCInfo().features
#elif defined(CPU_FEATURES_ARCH_ARM)
    .in = GetArmInfo().features
#elif defined(CPU_FEATURES_ARCH_AARCH64)
    .in = GetAarch64Info().features
#endif
  };
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
#endif /* #ifdef CPU_FEATURES_ARCH_X86_64 */

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
