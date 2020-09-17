// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Dr.-Ing. Patrick Siegl <patrick@siegl.it>

#include "kernel_avx2.h"

inline __attribute__((always_inline)) void kernel_avx2_unaligned( stack_t * data, __m256 s_two, __m256 s_sixteen, __m256 s_min_sixty, __m256i s_shl, __m256i s_shr )
{
    unsigned i, j;
    __m256 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum;
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

            s_left1 = _mm256_loadu_ps( &(data->apf[ r_min1 ]) );
            s_left2 = _mm256_loadu_ps( &(data->apf[ r_min2 ]) );
            s_right2 = _mm256_loadu_ps( &(data->apf[ r_plus2 ]) );
            s_right1 = _mm256_loadu_ps( &(data->apf[ r_plus1 ]) );

            s_above2 = _mm256_loadu_ps( &(data->apf[ r - 2]) );

#if 1
//                                  |08 09 10 11|
            __m128 s_under2l = _mm_loadu_ps( &(data->apf[ r + 2 + 4]) );

//          |00 01 02 03 04 05 06 07|
//                      |04 05 06 07 04 05 06 07|
            s_under2 = _mm256_permute2f128_ps( s_above2, s_above2, 0x11);

//                                  |08 09 10 11|
//                      |04 05 06 07 04 05 06 07|
//                      |04 05 06 07|08 09 10 11|
            s_under2 = _mm256_insertf128_ps( s_under2, s_under2l, 1 );
#else
// loads 4 floats that we have already in register
            s_under2 = _mm256_loadu_ps( &(data->apf[ r + 2 ]) );
#endif

//          |00 01(02 03)04 05(06 07)|
//                     |(04 05)06 07(08 09)10 11|
//                |02 03 04 05 06 07 08 09|
            s_actual = _mm256_shuffle_ps( s_above2, s_under2, _MM_SHUFFLE( 1, 0, 3, 2 ) );


            s_above1 = AVX2_CENTER( s_above2, s_actual, s_shl, s_shr );
            s_under1 = AVX2_CENTER( s_actual, s_under2, s_shl, s_shr );


            s_sum = _mm256_add_ps( _mm256_sub_ps( _mm256_mul_ps( s_two,
                                                                 s_actual ),
                                                  s_ppf_aligned ),
                                   _mm256_mul_ps( s_vel_aligned,
                                                  _mm256_sub_ps( _mm256_add_ps( _mm256_mul_ps( s_min_sixty,
                                                                                               s_actual ),
                                                                                _mm256_mul_ps( s_sixteen,
                                                                                               _mm256_add_ps( _mm256_add_ps( _mm256_add_ps( s_above1,
                                                                                                                                            s_under1 ),
                                                                                                                             s_left1 ),
                                                                                                              s_right1 ) ) ),
                                                                 _mm256_add_ps( _mm256_add_ps( _mm256_add_ps( s_above2,
                                                                                                              s_under2 ),
                                                                                               s_left2 ),
                                                                                s_right2 ) ) ) );

            _mm256_storeu_ps( &(data->nppf[ r ]), s_sum);
        }
    }
}

SEISMIC_EXEC_AVX2_FCT( unaligned );
#define SYM_KERNEL_CAP { .avx = 1, .avx2 = 1 }
SYM_KERNEL( avx2_unaligned, SYM_KERNEL_CAP, 0, 8 * sizeof(float) );
