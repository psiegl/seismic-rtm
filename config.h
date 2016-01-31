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

#ifndef _CONFIG_H_
#define _CONFIG_H_

enum _kernel_t {
  KERNEL__PLAIN_C,
  KERNEL__SIMD_SSE_STD,
  KERNEL__SIMD_SSE_UNALIGNED,
  KERNEL__SIMD_SSE_ALIGNED,
  KERNEL__SIMD_SSE_PARTIAL_ALIGNED,
  KERNEL__SIMD_SSE_ALIGNED_NOT_GROUPED,
  KERNEL__SIMD_AVX_UNALIGNED,
  KERNEL__SIMD_AVX2_UNALIGNED,
  KERNEL__SIMD_FMA_SSE_STD,
  KERNEL__SIMD_FMA_SSE_UNALIGNED,
  KERNEL__SIMD_FMA_SSE_ALIGNED,
  KERNEL__SIMD_FMA_SSE_PARTIAL_ALIGNED,
  KERNEL__SIMD_FMA_SSE_ALIGNED_NOT_GROUPED,
  KERNEL__SIMD_FMA_AVX_UNALIGNED,
  KERNEL__SIMD_FMA_AVX2_UNALIGNED
};
typedef enum _kernel_t kernel_t;


typedef struct _config_t config_t;
struct _config_t {
  unsigned height; // num. of floats
  unsigned width; // num. of floats
  unsigned timesteps;
  unsigned pulseY;
  unsigned pulseX;

  kernel_t kernel;
  void (* f_sequential)(void *);
  void (* f_parallel)(void *);
  unsigned alignment; // in Byte
  unsigned vectorwidth;
  unsigned threads;

  unsigned output;
  unsigned ascii;

  double GFLOP;
};

void get_config( int argc, char * argv[], config_t * config );
void print_config( config_t * config );

#endif /* #ifndef _CONFIG_H_ */
