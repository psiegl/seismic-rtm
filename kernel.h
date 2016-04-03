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

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "barrier/barrier.h"

typedef struct _stack_t stack_t;
struct _stack_t {
  unsigned id;
  BARRIER_TYPE * barrier;

  float * apf;
  float * nppf;
  float * vel;
  float * pulsevector;

  unsigned width;
  unsigned x_start;
  unsigned x_end;

  unsigned height;
  unsigned y_start;
  unsigned y_end;
  unsigned timesteps;

  unsigned x_pulse;
  unsigned y_pulse;

  unsigned set_pulse;

  struct timeval s;
  struct timeval e;
};

void seismic_exec_plain_naiiv( void * v );
void seismic_exec_plain_naiiv_pthread( void * v );

void seismic_exec_plain_opt( void * v );
void seismic_exec_plain_opt_pthread( void * v );

void seismic_exec_sse_std( void * v );
void seismic_exec_sse_std_pthread( void * v );

void seismic_exec_sse_fma_std( void * v );
void seismic_exec_sse_fma_std_pthread( void * v );

void seismic_exec_sse_unaligned( void * v );
void seismic_exec_sse_unaligned_pthread( void * v );

void seismic_exec_sse_fma_unaligned( void * v );
void seismic_exec_sse_fma_unaligned_pthread( void * v );

void seismic_exec_sse_aligned( void * v );
void seismic_exec_sse_aligned_pthread( void * v );

void seismic_exec_sse_fma_aligned( void * v );
void seismic_exec_sse_fma_aligned_pthread( void * v );

void seismic_exec_sse_partial_aligned( void * v );
void seismic_exec_sse_partial_aligned_pthread( void * v );

void seismic_exec_sse_fma_partial_aligned( void * v );
void seismic_exec_sse_fma_partial_aligned_pthread( void * v );

void seismic_exec_sse_avx_partial_aligned( void * v );
void seismic_exec_sse_avx_partial_aligned_pthread( void * v );

void seismic_exec_sse_avx_fma_partial_aligned( void * v );
void seismic_exec_sse_avx_fma_partial_aligned_pthread( void * v );

void seismic_exec_sse_aligned_not_grouped( void * v );
void seismic_exec_sse_aligned_not_grouped_pthread( void * v );

void seismic_exec_sse_fma_aligned_not_grouped( void * v );
void seismic_exec_sse_fma_aligned_not_grouped_pthread( void * v );

void seismic_exec_avx_unaligned( void * v );
void seismic_exec_avx_unaligned_pthread( void * v );

void seismic_exec_avx_fma_unaligned( void * v );
void seismic_exec_avx_fma_unaligned_pthread( void * v );

void seismic_exec_avx2_unaligned( void * v );
void seismic_exec_avx2_unaligned_pthread( void * v );

void seismic_exec_avx2_fma_unaligned( void * v );
void seismic_exec_avx2_fma_unaligned_pthread( void * v );

#endif /* #ifndef _KERNEL_H_ */
