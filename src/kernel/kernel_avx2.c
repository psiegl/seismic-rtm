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

#include "kernel_avx2.h"

inline __attribute__((always_inline)) void kernel_avx2_unaligned( stack_t * data, __m256 s_two, __m256 s_sixteen, __m256 s_sixty, __m256i s_shl, __m256i s_shr )
{
    unsigned i, j;
    __m256 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m256 s_above2, s_under2, s_left2, s_right2;

    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {
        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=8) {
            unsigned r = i * data->height + j;
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height * 2);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height * 2);

            // calculates the pressure field t+1
            s_ppf_aligned = _mm256_loadu_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
            s_vel_aligned= _mm256_loadu_ps( &(data->vel[ r ]) );
            s_actual = _mm256_loadu_ps( &(data->apf[ r ]) );

// eigentlich für SSE nur links und rechts nötig ... und dann kann durch combine mittlere erhalten werden

            s_left1 = _mm256_loadu_ps( &(data->apf[ r_min1 ]) );
            s_left2 = _mm256_loadu_ps( &(data->apf[ r_min2 ]) );
            s_right2 = _mm256_loadu_ps( &(data->apf[ r_plus2 ]) );
            s_right1 = _mm256_loadu_ps( &(data->apf[ r_plus1 ]) );

//            s_above1 = _mm256_loadu_ps( &(data->apf[ r -1]) );
//            s_under1 = _mm256_loadu_ps( &(data->apf[ r +1]) );

            s_above2 = _mm256_loadu_ps( &(data->apf[ r -2]) );
            s_under2 = _mm256_loadu_ps( &(data->apf[ r +2]) );

            s_above1 = avx2_combine( s_above2, s_actual, s_shl, s_shr );
            s_under1 = avx2_combine( s_actual, s_under2, s_shl, s_shr );

            // sum up
            s_sum1 = _mm256_add_ps( _mm256_add_ps( s_above1, s_under1),
                                    _mm256_add_ps( s_left1, s_right1));
            s_above2 = _mm256_add_ps( _mm256_add_ps( s_right2, s_left2),
                                      _mm256_add_ps( s_under2, s_above2));
            s_sum1 = _mm256_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm256_sub_ps( _mm256_sub_ps( s_sum1,  s_above2), _mm256_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm256_add_ps( _mm256_mul_ps( s_vel_aligned, s_sum1), _mm256_sub_ps(_mm256_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            _mm256_storeu_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_AVX2_FCT( unaligned )
