// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#include "kernel_sse.h"
#include <immintrin.h>

inline __attribute__((always_inline)) void kernel_sse_avx_fma_partial_aligned( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
{
    __m128 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m128 s_above2, s_under2, s_left2, s_right2;

    unsigned len_x = data->x_end - data->x_start;
    unsigned len_y = (data->y_end - data->y_start) / 4;

    unsigned r = data->x_start * data->height + data->y_start;
    float * NPPF = &data->nppf[ r ];
    float * VEL = &data->vel[ r ];
    float * APF = &data->apf[ r ];
    float * APF_pl1 = APF + data->height;
    float * APF_pl2 = APF_pl1 + data->height;
    float * APF_min1 = APF - data->height;
    float * APF_min2 = APF_min1 - data->height;
    APF -= 2;

    unsigned i, j;
    // spatial loop in x
    for (i=0; i<len_x; i++) {
        // spatial loop in y
        unsigned j = len_y;
        do {
            
            // calculates the pressure field t+1
            s_ppf_aligned = _mm_load_ps( NPPF ); // align it to get _load_ps
            s_vel_aligned = _mm_load_ps( VEL );

            s_left1 = _mm_load_ps( APF_min1 );
            s_left2 = _mm_load_ps( APF_min2 );
            s_right2 = _mm_load_ps( APF_pl2 );
            s_right1 = _mm_load_ps( APF_pl1 );

            __m256 v_in = _mm256_loadu_ps( APF ); // APF - 2
            s_above2 = _mm256_castps256_ps128( v_in ); //_mm_loadu_ps( APF ); // APF - 2
            s_under2 = _mm256_extractf128_ps( v_in, 1 ); //_mm_loadu_ps( APF + 4 ); // APF + 2

            s_actual = _mm_shuffle_ps( s_above2, s_under2, _MM_SHUFFLE( 1, 0, 3, 2 ) ); // 3 4 5 6
            s_above1 = _mm_shuffle_ps( s_above2, s_actual, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 2 3 4 5
            s_under1 = _mm_shuffle_ps( s_actual, s_under2, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 4 5 6 7

            // sum up
            s_sum1 = _mm_add_ps( _mm_add_ps( s_above1, s_under1),
                                 _mm_add_ps( s_left1, s_right1));

            s_above2 = _mm_add_ps( _mm_add_ps( s_right2, s_left2),
                                   _mm_add_ps( s_under2, s_above2));

            s_sum1 = _mm_fmsub_ps( s_sixteen, s_sum1,  s_above2);
            s_sum1 = _mm_fnmadd_ps( s_sixty, s_actual, s_sum1 );
            s_sum1 = _mm_fmadd_ps( s_vel_aligned, s_sum1, _mm_fmsub_ps(s_two, s_actual, s_ppf_aligned) );

            _mm_store_ps( NPPF, s_sum1);
            NPPF+=4;
            VEL+=4;
            APF+=4;
            APF_pl1+=4;
            APF_pl2+=4;
            APF_min1+=4;
            APF_min2+=4;
        } while ( (--j) );
        APF+=4;
        NPPF+=4;
        VEL+=4;
        APF_min1+=4;
        APF_min2+=4;
        APF_pl1+=4;
        APF_pl2+=4;
    }
}

SEISMIC_EXEC_SSE_FCT( avx_fma_partial_aligned );
#define SYM_KERNEL_CAP { .avx = 1, .fma3 = 1, .sse = 1 }
SYM_KERNEL( sse_avx_fma_partial_aligned,  SYM_KERNEL_CAP, 4 * sizeof(float), 4 * sizeof(float) );


inline __attribute__((always_inline)) void kernel_sse_avx_fma_partial_aligned_opt( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
{
    __m128 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m128 s_above2, s_under2, s_left2, s_right2;

    unsigned len_x = data->x_end - data->x_start;
    unsigned len_y = (data->y_end - data->y_start) / 4;

    unsigned r = data->x_start * data->height + data->y_start;
    float * NPPF = &data->nppf[ r ];
    float * VEL = &data->vel[ r ];
    float * APF = &data->apf[ r ];
    float * APF_pl1 = APF + data->height;
    float * APF_pl2 = APF_pl1 + data->height;
    float * APF_min1 = APF - data->height;
    float * APF_min2 = APF_min1 - data->height;
    APF -= 2;

    unsigned i, j;
    // spatial loop in x
    for (i=0; i<len_x; i++) {
        // spatial loop in y
        unsigned j = len_y;

        __m256 v_in = _mm256_loadu_ps( APF ); // APF - 2
        s_above2 = _mm256_castps256_ps128( v_in ); //_mm_loadu_ps( APF ); // APF - 2
        s_under2 = _mm256_extractf128_ps( v_in, 1 ); //_mm_loadu_ps( APF + 4 ); // APF + 2

        do {
            
            // calculates the pressure field t+1
            s_ppf_aligned = _mm_load_ps( NPPF ); // align it to get _load_ps
            s_vel_aligned = _mm_load_ps( VEL );

            s_left1 = _mm_load_ps( APF_min1 );
            s_left2 = _mm_load_ps( APF_min2 );
            s_right2 = _mm_load_ps( APF_pl2 );
            s_right1 = _mm_load_ps( APF_pl1 );


            s_actual = _mm_shuffle_ps( s_above2, s_under2, _MM_SHUFFLE( 1, 0, 3, 2 ) ); // 3 4 5 6
            s_above1 = _mm_shuffle_ps( s_above2, s_actual, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 2 3 4 5
            s_under1 = _mm_shuffle_ps( s_actual, s_under2, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 4 5 6 7

            // sum up
            s_sum1 = _mm_add_ps( _mm_add_ps( s_above1, s_under1),
                                 _mm_add_ps( s_left1, s_right1));

            s_above2 = _mm_add_ps( _mm_add_ps( s_right2, s_left2),
                                   _mm_add_ps( s_under2, s_above2));

            s_sum1 = _mm_fmsub_ps( s_sixteen, s_sum1,  s_above2);
            s_sum1 = _mm_fnmadd_ps( s_sixty, s_actual, s_sum1 );
            s_sum1 = _mm_fmadd_ps( s_vel_aligned, s_sum1, _mm_fmsub_ps(s_two, s_actual, s_ppf_aligned) );

            _mm_store_ps( NPPF, s_sum1);
            s_above2 = s_under2;
            s_under2 = _mm_loadu_ps( APF + 8 );
            
            NPPF+=4;
            VEL+=4;
            APF+=4;
            APF_pl1+=4;
            APF_pl2+=4;
            APF_min1+=4;
            APF_min2+=4;
        } while ( (--j) );
        APF+=4;
        NPPF+=4;
        VEL+=4;
        APF_min1+=4;
        APF_min2+=4;
        APF_pl1+=4;
        APF_pl2+=4;
    }
}

SEISMIC_EXEC_SSE_FCT( avx_fma_partial_aligned_opt );
#define SYM_KERNEL_CAP { .avx = 1, .fma3 = 1, .sse = 1 }
SYM_KERNEL( sse_avx_fma_partial_aligned_opt, SYM_KERNEL_CAP, 4 * sizeof(float), 4 * sizeof(float) );
