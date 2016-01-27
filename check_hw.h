//  This file is part of seismic-rtm.
//
//  seismic-rtm is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  seismic is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seismic.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _CHECK_HW_H_
#define _CHECK_HW_H_

#include <stdint.h>

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

uint32_t check_hw_capabilites( void ) {
  uint32_t cap = 0;

  if(__builtin_cpu_supports("sse"))
    cap |= HAS_SSE;
  if(__builtin_cpu_supports("avx"))
    cap |= HAS_AVX;
  if(__builtin_cpu_supports("avx2"))
    cap |= HAS_AVX2;
  if(__builtin_cpu_supports("fma"))
    cap |= HAS_FMA;

  return cap;
}


#endif /* #ifndef _CHECK_HW_H_ */
