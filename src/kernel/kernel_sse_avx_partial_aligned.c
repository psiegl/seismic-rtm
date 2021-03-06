// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Dr.-Ing. Patrick Siegl <patrick@siegl.it>

#include "kernel_sse.h"
#include <immintrin.h>

inline __attribute__((always_inline)) void kernel_sse_avx_partial_aligned( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
{
    __m128 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m128 s_above2, s_under2, s_left2, s_right2;

    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {
        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=4) {
            unsigned r = i * data->height + j;
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height * 2);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height * 2);
            
            // calculates the pressure field t+1
            s_ppf_aligned = _mm_load_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
            s_vel_aligned = _mm_load_ps( &(data->vel[ r ]) );

            s_left1 = _mm_load_ps( &(data->apf[ r_min1 ]) );
            s_left2 = _mm_load_ps( &(data->apf[ r_min2 ]) );
            s_right2 = _mm_load_ps( &(data->apf[ r_plus2 ]) );
            s_right1 = _mm_load_ps( &(data->apf[ r_plus1 ]) );

            __m256 v_in = _mm256_loadu_ps( &data->apf[ r - 2 ] );
            s_above2 = _mm256_castps256_ps128( v_in ); //_mm_loadu_ps( &(data->apf[ r -2]) );
            s_under2 = _mm256_extractf128_ps( v_in, 1 ); //_mm_loadu_ps( &(data->apf[ r +2]) );

            s_actual = _mm_shuffle_ps( s_above2, s_under2, _MM_SHUFFLE( 1, 0, 3, 2 ) ); // 3 4 5 6
            s_above1 = _mm_shuffle_ps( s_above2, s_actual, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 2 3 4 5
            s_under1 = _mm_shuffle_ps( s_actual, s_under2, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 4 5 6 7

            // sum up
            s_sum1 = _mm_add_ps( _mm_add_ps( s_left1, s_right1),
                                 _mm_add_ps( s_above1, s_under1 ));

            s_above2 = _mm_add_ps( _mm_add_ps( s_right2, s_left2), 
                                   _mm_add_ps( s_under2, s_above2));

            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( avx_partial_aligned );
#define SYM_KERNEL_CAP { .avx = 1, .sse = 1 }
SYM_KERNEL( sse_avx_partial_aligned, SYM_KERNEL_CAP, 4 * sizeof(float), 4 * sizeof(float) );


inline __attribute__((always_inline)) void kernel_sse_avx_partial_aligned_opt( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
{
    __m128 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m128 s_above2, s_under2, s_left2, s_right2;

    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {
        unsigned r = i * data->height + data->y_start;

        __m256 v_in = _mm256_loadu_ps( &data->apf[ r - 2 ] );
        s_above2 = _mm256_castps256_ps128( v_in ); //_mm_loadu_ps( &(data->apf[ r -2]) );
        s_under2 = _mm256_extractf128_ps( v_in, 1 ); //_mm_loadu_ps( &(data->apf[ r +2]) );

        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=4, r+=4) {
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height << 1);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height << 1);
            
            // calculates the pressure field t+1
            s_ppf_aligned = _mm_load_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
            s_vel_aligned = _mm_load_ps( &(data->vel[ r ]) );

            s_left1 = _mm_load_ps( &(data->apf[ r_min1 ]) );
            s_left2 = _mm_load_ps( &(data->apf[ r_min2 ]) );
            s_right2 = _mm_load_ps( &(data->apf[ r_plus2 ]) );
            s_right1 = _mm_load_ps( &(data->apf[ r_plus1 ]) );
            
            

            s_actual = _mm_shuffle_ps( s_above2, s_under2, _MM_SHUFFLE( 1, 0, 3, 2 ) ); // 3 4 5 6
            s_above1 = _mm_shuffle_ps( s_above2, s_actual, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 2 3 4 5
            s_under1 = _mm_shuffle_ps( s_actual, s_under2, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 4 5 6 7

            // sum up
            s_sum1 = _mm_add_ps( _mm_add_ps( s_left1, s_right1),
                                 _mm_add_ps( s_above1, s_under1 ));

            s_above2 = _mm_add_ps( _mm_add_ps( s_right2, s_left2), 
                                   _mm_add_ps( s_under2, s_above2));

            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
            s_above2 = s_under2;
            s_under2 = _mm_loadu_ps( &(data->apf[ r + 6 ]) );
        }
    }
}

SEISMIC_EXEC_SSE_FCT( avx_partial_aligned_opt );
#define SYM_KERNEL_CAP { .avx = 1, .sse = 1 }
SYM_KERNEL( sse_avx_partial_aligned_opt, SYM_KERNEL_CAP, 4 * sizeof(float), 4 * sizeof(float) );
