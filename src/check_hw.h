// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#ifndef _CHECK_HW_H_
#define _CHECK_HW_H_

#include <stdint.h>

#define HAS_SSE   (1 << 0)
//#define HAS_SSE2  (1 << 1)
//#define HAS_SSE3  (1 << 2)
//#define HAS_SSSE3 (1 << 3)
//#define HAS_SSE41 (1 << 4)
//#define HAS_SSE42 (1 << 5)
//#define HAS_SSE4a (1 << 6)
#define HAS_FMA   (1 << 7)
//#define HAS_FMA4  (1 << 8)
#define HAS_AVX   (1 << 9)
#define HAS_AVX2  (1 << 10)

#define HAS_VMX   (1 << 11)
#define HAS_VSX   (1 << 12)

#define HAS_NEON  (1 << 13)
#define HAS_ASIMD (1 << 14)

/* option to parse lscpu? */
uint32_t check_hw_capabilites( void );

#endif /* #ifndef _CHECK_HW_H_ */
