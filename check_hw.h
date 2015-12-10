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

#include <cpuid.h>
#include <stdint.h>

#define HAS_SSE   (1 << 0)
#define HAS_SSE2  (1 << 1)
#define HAS_SSE3  (1 << 2)
#define HAS_SSSE3 (1 << 3)
#define HAS_SSE41 (1 << 4)
#define HAS_SSE42 (1 << 5)
#define HAS_SSE4a (1 << 6)
#define HAS_FMA3  (1 << 7)
#define HAS_FMA4  (1 << 8)
#define HAS_AVX   (1 << 9)
#define HAS_AVX2  (1 << 10)

void cpuid(int info[4], int InfoType){
    __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
}

uint32_t check_hw_capabilites( void ) {
  uint32_t cap = 0;

  int info[4];
  cpuid(info, 0);
  int nIds = info[0];

  cpuid(info, 0x80000000);
  unsigned nExIds = info[0];

  if (nIds >= 0x00000001){
    cpuid(info,0x00000001);
    cap |= (info[3] & ((int)1 << 25)) ? HAS_SSE : 0;
    cap |= (info[3] & ((int)1 << 26)) ? HAS_SSE2 : 0;
    cap |= (info[2] & ((int)1 <<  0)) ? HAS_SSE3 : 0;

    cap |= (info[2] & ((int)1 <<  9)) ? HAS_SSSE3 : 0;
    cap |= (info[2] & ((int)1 << 19)) ? HAS_SSE41 : 0;
    cap |= (info[2] & ((int)1 << 20)) ? HAS_SSE42 : 0;

    cap |= (info[2] & ((int)1 << 28)) ? HAS_AVX : 0;
    cap |= (info[2] & ((int)1 << 12)) ? HAS_FMA3 : 0;
  }
  if (nIds >= 0x00000007){
    cpuid(info,0x00000007);
    cap |= (info[1] & ((int)1 <<  5)) ? HAS_AVX2 : 0;
  }
  if (nExIds >= 0x80000001){
    cpuid(info,0x80000001);
    cap |= (info[2] & ((int)1 <<  6)) ? HAS_SSE4a : 0;
    cap |= (info[2] & ((int)1 << 16)) ? HAS_FMA4 : 0;
  }

  return cap;
}


#endif /* #ifndef _CHECK_HW_H_ */
