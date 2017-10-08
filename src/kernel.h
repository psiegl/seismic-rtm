//  This file is part of seismic-rtm.
//
//  Copyright (c) 2017, Dipl.-Inf. Patrick Siegl
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "barrier/barrier.h"

typedef struct _stack_t stack_t;
struct _stack_t {
  unsigned id;
  BARRIER_TYPE* barrier;

  float* apf;
  float* nppf;
  float* vel;
  float* pulsevector;

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

#ifdef __x86_64__
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

void seismic_exec_sse_avx_partial_aligned_opt( void * v );
void seismic_exec_sse_avx_partial_aligned_opt_pthread( void * v );

void seismic_exec_sse_avx_fma_partial_aligned( void * v );
void seismic_exec_sse_avx_fma_partial_aligned_pthread( void * v );

void seismic_exec_sse_avx_fma_partial_aligned_opt( void * v );
void seismic_exec_sse_avx_fma_partial_aligned_opt_pthread( void * v );

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
#endif /* #ifdef __x86_64__ */

#ifdef __ALTIVEC__
void seismic_exec_vmx_aligned( void * v );
void seismic_exec_vmx_aligned_pthread( void * v );
#ifdef __VSX__
void seismic_exec_vsx_unaligned( void * v );
void seismic_exec_vsx_unaligned_pthread( void * v );
void seismic_exec_vsx_aligned( void * v );
void seismic_exec_vsx_aligned_pthread( void * v );
#endif /* #ifdef __VSX__ */
#endif /* #ifdef __ALTIVEC__ */

#if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP )
void seismic_exec_arm_neon_aligned( void * v );
void seismic_exec_arm_neon_aligned_pthread( void * v );
#endif /* #if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP ) */

#endif /* #ifndef _KERNEL_H_ */
