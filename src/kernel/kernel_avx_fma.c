// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#include "kernel_avx.h"

inline __attribute__((always_inline)) void kernel_avx_fma_unaligned( stack_t * data, __m256 s_two, __m256 s_sixteen, __m256 s_sixty )
{
    __m256 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m256 s_above2, s_under2, s_left2, s_right2;

    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {
        unsigned r = i * data->height;
        unsigned r_min1 = r - data->height;
        unsigned r_min2 = r - (data->height * 2);
        unsigned r_plus1 = r + data->height;
        unsigned r_plus2 = r + (data->height * 2);
        s_above1 = _mm256_loadu_ps( &(data->apf[ r -1]) );
        s_under1 = _mm256_loadu_ps( &(data->apf[ r +1]) );
        s_left1 = _mm256_loadu_ps( &(data->apf[ r_min1 ]) );
        s_right1 = _mm256_loadu_ps( &(data->apf[ r_plus1 ]) );
        s_left2 = _mm256_loadu_ps( &(data->apf[ r_min2 ]) );
        s_above2 = _mm256_loadu_ps( &(data->apf[ r -2]) );
        s_under2 = _mm256_loadu_ps( &(data->apf[ r +2]) );
        s_actual = _mm256_loadu_ps( &(data->apf[ r ]) );
        s_ppf_aligned = _mm256_loadu_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
        s_vel_aligned= _mm256_loadu_ps( &(data->vel[ r ]) );
        r += 8;
        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=8, r+=8,
                                             r_plus2+=8) {

            // calculates the pressure field t+1
            s_right2 = _mm256_loadu_ps( &(data->apf[ r_plus2 ]) );

            // sum up
            s_under1 = _mm256_add_ps( s_above1, s_under1);
            s_above1 = _mm256_loadu_ps( &(data->apf[ r -1]) );
            s_left1 = _mm256_add_ps( s_left1, s_right1);
            s_right1 = _mm256_loadu_ps( &(data->apf[ r_plus1 += 8 ]) );
            s_sum1 = _mm256_add_ps( s_under1, s_left1);
            s_under1 = _mm256_loadu_ps( &(data->apf[ r +1]) );
            s_right2 = _mm256_add_ps( s_right2, s_left2);
            s_left1 = _mm256_loadu_ps( &(data->apf[ r_min1+=8 ]) );
            s_under2 = _mm256_add_ps( s_under2, s_above2);
            s_left2 = _mm256_loadu_ps( &(data->apf[ r_min2+=8 ]) );
            s_above2 = _mm256_add_ps( s_right2, s_under2);

            s_under2 = _mm256_loadu_ps( &(data->apf[ r +2]) );
            s_sum1 = _mm256_fmsub_ps( s_sixteen, s_sum1,  s_above2);
            s_above2 = _mm256_loadu_ps( &(data->apf[ r -2]) );

            __m256 s_sum2 = _mm256_fmsub_ps(s_two, s_actual, s_ppf_aligned);
            s_ppf_aligned = _mm256_loadu_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
            s_sum1 = _mm256_fnmadd_ps( s_sixty, s_actual, s_sum1 );

            s_actual = _mm256_loadu_ps( &(data->apf[ r ]) );
            s_sum1 = _mm256_fmadd_ps( s_vel_aligned, s_sum1, s_sum2 );
            s_vel_aligned= _mm256_loadu_ps( &(data->vel[ r ]) );

            _mm256_storeu_ps( &(data->nppf[ r - 8 ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_AVX_FCT( fma_unaligned )



