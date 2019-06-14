// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#include "kernel_sse.h"

inline __attribute__((always_inline)) void kernel_sse_std( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
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
            unsigned r_min2 = r - (data->height << 1);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height << 1);
            
            // calculates the pressure field t+1
            s_ppf_aligned = _mm_load_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
            s_vel_aligned = _mm_load_ps( &(data->vel[ r ]) );
            s_actual = _mm_load_ps( &(data->apf[ r ]) );

            s_left1 = _mm_load_ps( &(data->apf[r_min1]) );
            s_left2 = _mm_load_ps( &(data->apf[r_min2]) );
            s_right2 = _mm_load_ps( &(data->apf[r_plus2]) );
            s_right1 = _mm_load_ps( &(data->apf[r_plus1]) );
            s_above1 = _mm_loadu_ps( &(data->apf[ r -1]) );
            s_under1 = _mm_loadu_ps( &(data->apf[ r +1]) );

            s_above2 = _mm_loadl_pi( _mm_shuffle_ps(s_actual, s_actual, _MM_SHUFFLE(1, 0, 0, 0)),
                                     (__m64 const*)&(data->apf[ r -2]));

            s_under2 = _mm_loadh_pi( _mm_shuffle_ps(s_actual, s_actual, _MM_SHUFFLE(0, 0, 3, 2)),
                                     (__m64 const*)&(data->apf[ r +4]));

            // sum up
            s_sum1 = _mm_add_ps( _mm_add_ps(s_left1, s_right1),
                                 _mm_add_ps(s_under1, s_above1));

            s_above2 = _mm_add_ps( _mm_add_ps( s_right2, s_left2),
                                   _mm_add_ps( s_under2, s_above2));

            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( std );
#define SYM_KERNEL_CAP { .sse = 1 }
SYM_KERNEL( sse_std, SYM_KERNEL_CAP, 4 * sizeof(float), 4 * sizeof(float) );


inline __attribute__((always_inline)) void kernel_sse_aligned( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
{
    __m128 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m128 s_above2, s_under2, s_left2, s_right2;
    
    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {
        unsigned r = i * data->height + data->y_start;
        s_above1 = _mm_load_ps( &(data->apf[ r -4]) );
        s_actual = _mm_load_ps( &(data->apf[ r ]) );
        s_above2 = _mm_movelh_ps( _mm_movehl_ps( s_above1, s_above1), s_actual );
        
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
            s_under1 = _mm_load_ps( &(data->apf[ r +4]) );

            // see _mm_shuffle_ps and _mm_move_ss in intel's manual for explanation
            s_under2 = _mm_movelh_ps( _mm_shuffle_ps(s_actual, s_actual, _MM_SHUFFLE(0, 0, 3, 2)), s_under1);
            
            __m128 s_under1t = _mm_move_ss( s_actual, s_under1 );
            s_under1t = _mm_shuffle_ps( s_under1t, s_under1t, _MM_SHUFFLE(0, 3, 2, 1) );

            s_above1 = _mm_move_ss( _mm_shuffle_ps( s_actual, s_actual, _MM_SHUFFLE(2, 1, 0, 3)),
                                    _mm_shuffle_ps( s_above1, s_above1, _MM_SHUFFLE(0, 1, 2, 3)) );

            // sum up
            s_sum1 = _mm_add_ps( _mm_add_ps( s_above1, s_under1t),
                                 _mm_add_ps( s_left1, s_right1));

            s_above2 = _mm_add_ps( _mm_add_ps( s_right2, s_left2),
                                   _mm_add_ps( s_under2, s_above2));

            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            s_above1 = s_actual;
            s_actual = s_under1;
            s_above2 = s_under2;
            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( aligned );
#define SYM_KERNEL_CAP { .sse = 1 }
SYM_KERNEL( sse_aligned, SYM_KERNEL_CAP, 4 * sizeof(float), 4 * sizeof(float) );


inline __attribute__((always_inline)) void kernel_sse_aligned_not_grouped( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
{
    __m128 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m128 s_above2, s_under2, s_left2, s_right2;
    
    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {
        unsigned r = i * data->height + data->y_start;
        s_above1 = _mm_load_ps( &(data->apf[ r -4]) );
        s_actual = _mm_load_ps( &(data->apf[ r ]) );
        s_above2 = _mm_movelh_ps( _mm_movehl_ps( s_above1, s_above1), s_actual );
        
        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=4, r+=4) {
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height << 1);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height << 1);
            
            // calculates the pressure field t+1
//   _mm_prefetch( (const char*) &(data->apf[ r -4])  , _MM_HINT_T2); 
            s_under1 = _mm_load_ps( &(data->apf[ r +4]) );
            s_left2 = _mm_load_ps( &(data->apf[ r_min2 ]) );
            s_right2 = _mm_load_ps( &(data->apf[ r_plus2 ]) );

            s_under2 = _mm_movelh_ps( _mm_shuffle_ps(s_actual, s_actual, _MM_SHUFFLE(0, 0, 3, 2)),
                                      s_under1);

            s_above2 = _mm_add_ps( _mm_add_ps( s_under2, s_above2),
                                   _mm_add_ps( s_right2, s_left2));

            __m128 s_under1t = _mm_move_ss( s_actual, s_under1 );
            s_above1 = _mm_move_ss( _mm_shuffle_ps( s_actual, s_actual, _MM_SHUFFLE(2, 1, 0, 3)),
                                    _mm_shuffle_ps( s_above1, s_above1, _MM_SHUFFLE(0, 1, 2, 3)) );

            s_left1 = _mm_load_ps( &(data->apf[ r_min1 ]) );
            s_right1 = _mm_load_ps( &(data->apf[ r_plus1 ]) );

            s_under1t = _mm_shuffle_ps( s_under1t, s_under1t, _MM_SHUFFLE(0, 3, 2, 1) );
            // sum up
            s_sum1 = _mm_add_ps( _mm_add_ps( s_above1, s_under1t),
                                 _mm_add_ps( s_left1, s_right1));

            s_ppf_aligned = _mm_load_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
            s_vel_aligned = _mm_load_ps( &(data->vel[ r ]) );

            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            s_above1 = s_actual;
            s_actual = s_under1;
            s_above2 = s_under2;
            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( aligned_not_grouped );
#define SYM_KERNEL_CAP { .sse = 1 }
SYM_KERNEL( sse_aligned_not_grouped, SYM_KERNEL_CAP, 4 * sizeof(float), 4 * sizeof(float) );


inline __attribute__((always_inline)) void kernel_sse_unaligned( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
{
    __m128 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m128 s_above2, s_under2, s_left2, s_right2;
    
    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {

        unsigned r = i * data->height + data->y_start;
        s_above2 = _mm_loadu_ps( &(data->apf[ r -2]) );

        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=4, r+=4) {
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height << 1);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height << 1);
            
            // calculates the pressure field t+1
            s_ppf_aligned = _mm_loadu_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
            s_vel_aligned= _mm_loadu_ps( &(data->vel[ r ]) );

//            s_above2 = _mm_loadu_ps( &(data->apf[ r - 2]) );
            s_under2 = _mm_loadu_ps( &(data->apf[ r + 2]) );

            s_left1 = _mm_loadu_ps( &(data->apf[ r_min1 ]) );
            s_left2 = _mm_loadu_ps( &(data->apf[ r_min2 ]) );
            s_right2 = _mm_loadu_ps( &(data->apf[ r_plus2 ]) );
            s_right1 = _mm_loadu_ps( &(data->apf[ r_plus1 ]) );

            s_actual = _mm_shuffle_ps( s_above2, s_under2, _MM_SHUFFLE(1,0,3,2) );

            s_above1 = _mm_shuffle_ps( s_above2, s_actual, _MM_SHUFFLE(2,1,2,1) );
            s_under1 = _mm_shuffle_ps( s_actual, s_under2, _MM_SHUFFLE(2,1,2,1) );

            // sum up
            s_sum1 = _mm_add_ps( _mm_add_ps( s_above1, s_under1 ),
                                 _mm_add_ps( s_left1, s_right1) );

            s_above2 = _mm_add_ps( _mm_add_ps( s_under2, s_above2),
                                   _mm_add_ps( s_right2, s_left2 ) );

            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            s_above2 = s_under2;
            _mm_storeu_ps( &(data->nppf[ r ]), s_sum1);            
        }
    }
}

SEISMIC_EXEC_SSE_FCT( unaligned );
#define SYM_KERNEL_CAP { .sse = 1 }
SYM_KERNEL( sse_unaligned, SYM_KERNEL_CAP, 0, 4 * sizeof(float) );


inline __attribute__((always_inline)) void kernel_sse_partial_aligned( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
{
    __m128 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m128 s_above2, s_under2, s_left2, s_right2;

    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {

        unsigned r = i * data->height + data->y_start;
        s_above2 = _mm_loadu_ps( &(data->apf[ r -2]) );

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

            s_under2 = _mm_loadu_ps( &(data->apf[ r +2]) );

            s_actual = _mm_shuffle_ps( s_above2, s_under2, _MM_SHUFFLE( 1, 0, 3, 2 ) ); // 3 4 5 6
            s_above1 = _mm_shuffle_ps( s_above2, s_actual, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 2 3 4 5
            s_under1 = _mm_shuffle_ps( s_actual, s_under2, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 4 5 6 7

            // sum up
            s_sum1 = _mm_add_ps( _mm_add_ps( s_above1, s_under1 ),
                                 _mm_add_ps( s_left1, s_right1) );

            s_above2 = _mm_add_ps( _mm_add_ps( s_right2, s_left2 ),
                                   _mm_add_ps( s_under2, s_above2) );

            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            s_above2 = s_under2;
            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( partial_aligned );
#define SYM_KERNEL_CAP { .sse = 1 }
SYM_KERNEL( sse_partial_aligned, SYM_KERNEL_CAP, 4 * sizeof(float), 4 * sizeof(float) );
