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

#include "kernel_sse.h"
#include <immintrin.h>

inline __attribute__((always_inline)) void kernel_sse_fma_std( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
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
            s_sum1 = _mm_add_ps( s_under1,
                                 _mm_add_ps( s_above1,
                                             _mm_add_ps( s_left1, s_right1)));

            s_above2 = _mm_add_ps( s_left2, _mm_add_ps( s_right2, _mm_add_ps( s_under2, s_above2)));

            s_sum1 = _mm_fmsub_ps( s_sixteen, s_sum1,  s_above2);
            s_sum1 = _mm_fnmadd_ps( s_sixty, s_actual, s_sum1 );
            s_sum1 = _mm_fmadd_ps( s_vel_aligned, s_sum1, _mm_fmsub_ps(s_two, s_actual, s_ppf_aligned) );

            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( fma_std );



inline __attribute__((always_inline)) void kernel_sse_fma_aligned( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
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
            s_actual = _mm_load_ps( &(data->apf[ r ]) );

            s_left1 = _mm_load_ps( &(data->apf[ r_min1 ]) );
            s_left2 = _mm_load_ps( &(data->apf[ r_min2 ]) );
            s_right2 = _mm_load_ps( &(data->apf[ r_plus2 ]) );
            s_right1 = _mm_load_ps( &(data->apf[ r_plus1 ]) );
            s_above1 = _mm_load_ps( &(data->apf[ r -4]) );
            s_under1 = _mm_load_ps( &(data->apf[ r +4]) );

            // see _mm_shuffle_ps and _mm_move_ss in intel's manual for explanation
            s_under2 = _mm_movelh_ps( _mm_shuffle_ps(s_actual, s_actual, _MM_SHUFFLE(0, 0, 3, 2)),
                                      s_under1);
            s_under1 = _mm_move_ss( s_actual, s_under1 );
            s_under1 = _mm_shuffle_ps( s_under1, s_under1, _MM_SHUFFLE(0, 3, 2, 1) );

            s_above2 = _mm_movelh_ps( _mm_movehl_ps( s_above1, s_above1), s_actual );
            s_above1 = _mm_move_ss( _mm_shuffle_ps( s_actual, s_actual, _MM_SHUFFLE(2, 1, 0, 3)),
                                    _mm_shuffle_ps( s_above1, s_above1, _MM_SHUFFLE(0, 1, 2, 3)) );

            // sum up
            s_sum1 = _mm_add_ps( s_under1,
                                 _mm_add_ps( s_above1,
                                             _mm_add_ps( s_left1, s_right1)));

            s_above2 = _mm_add_ps( s_left2, _mm_add_ps( s_right2, _mm_add_ps( s_under2, s_above2)));

            s_sum1 = _mm_fmsub_ps( s_sixteen, s_sum1,  s_above2);
            s_sum1 = _mm_fnmadd_ps( s_sixty, s_actual, s_sum1 );
            s_sum1 = _mm_fmadd_ps( s_vel_aligned, s_sum1, _mm_fmsub_ps(s_two, s_actual, s_ppf_aligned) );

            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( fma_aligned );



inline __attribute__((always_inline)) void kernel_sse_fma_aligned_not_grouped( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
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
//   _mm_prefetch( (const char*) &(data->apf[ r -4])  , _MM_HINT_T2); 
            s_actual = _mm_load_ps( &(data->apf[ r ]) );
            s_under1 = _mm_load_ps( &(data->apf[ r +4]) );
            s_above1 = _mm_load_ps( &(data->apf[ r -4]) );
            s_left2 = _mm_load_ps( &(data->apf[ r_min2 ]) );
            s_right2 = _mm_load_ps( &(data->apf[ r_plus2 ]) );

            s_under2 = _mm_movelh_ps( _mm_shuffle_ps(s_actual, s_actual, _MM_SHUFFLE(0, 0, 3, 2)),
                                      s_under1);
            s_above2 = _mm_movelh_ps( _mm_movehl_ps( s_above1, s_above1), s_actual );

            s_above2 = _mm_add_ps( s_left2, _mm_add_ps( s_right2, _mm_add_ps( s_under2, s_above2)));

            s_under1 = _mm_move_ss( s_actual, s_under1 );
            s_above1 = _mm_move_ss( _mm_shuffle_ps( s_actual, s_actual, _MM_SHUFFLE(2, 1, 0, 3)),
                                    _mm_shuffle_ps( s_above1, s_above1, _MM_SHUFFLE(0, 1, 2, 3)) );

            s_left1 = _mm_load_ps( &(data->apf[ r_min1 ]) );
            s_right1 = _mm_load_ps( &(data->apf[ r_plus1 ]) );

            s_under1 = _mm_shuffle_ps( s_under1, s_under1, _MM_SHUFFLE(0, 3, 2, 1) );
            // sum up
            s_sum1 = _mm_add_ps( s_under1,
                                 _mm_add_ps( s_above1,
                                             _mm_add_ps( s_left1, s_right1)));

            s_ppf_aligned = _mm_load_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
            s_vel_aligned = _mm_load_ps( &(data->vel[ r ]) );

            s_sum1 = _mm_fmsub_ps( s_sixteen, s_sum1,  s_above2);
            s_sum1 = _mm_fnmadd_ps( s_sixty, s_actual, s_sum1 );
            s_sum1 = _mm_fmadd_ps( s_vel_aligned, s_sum1, _mm_fmsub_ps(s_two, s_actual, s_ppf_aligned) );

            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( fma_aligned_not_grouped );



inline __attribute__((always_inline)) void kernel_sse_fma_unaligned( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
{
    __m128 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m128 s_above2, s_under2, s_left2, s_right2;
    
    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {

        unsigned r = i * data->height + data->y_start;
        s_above2 = _mm_loadu_ps( &(data->apf[ r -2]) );

        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=4) {
            unsigned r = i * data->height + j;
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height * 2);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height * 2);
            
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
            s_sum1 = _mm_add_ps( s_under1,
                                 _mm_add_ps( s_above1,
                                             _mm_add_ps( s_left1, s_right1)));

            s_above2 = _mm_add_ps( s_left2, _mm_add_ps( s_right2, _mm_add_ps( s_under2, s_above2)));

            s_sum1 = _mm_fmsub_ps( s_sixteen, s_sum1,  s_above2);
            s_sum1 = _mm_fnmadd_ps( s_sixty, s_actual, s_sum1 );
            s_sum1 = _mm_fmadd_ps( s_vel_aligned, s_sum1, _mm_fmsub_ps(s_two, s_actual, s_ppf_aligned) );

            _mm_storeu_ps( &(data->nppf[ r ]), s_sum1);
            
            s_above2 = s_under2;
        }
    }
}

SEISMIC_EXEC_SSE_FCT( fma_unaligned );



inline __attribute__((always_inline)) void kernel_sse_fma_partial_aligned( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty )
{
    __m128 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m128 s_above2, s_under2, s_left2, s_right2;

    unsigned len_x = data->x_end - data->x_start;
    unsigned len_y = (data->y_end - data->y_start)/4;

    unsigned r = data->x_start * data->height + data->y_start;
    float * NPPF = &data->nppf[ r ];
    float * VEL = &data->vel[ r ];
    float * APF = &data->apf[ r ];
    float * APF_pl1 = APF + data->height;
    float * APF_pl2 = APF_pl1 + data->height;
    float * APF_min1 = APF - data->height;
    float * APF_min2 = APF_min1 - data->height;
    APF += 2;

//  if( ! len_y || ! len_x ) // checked in main!
//    return;

    // spatial loop in x
    unsigned i = len_x;
    do
    {
        s_above2 = _mm_loadu_ps( APF - 4 ); // APF - 2

        // spatial loop in y
        unsigned j = len_y;
        do
        {

            // calculates the pressure field t+1
            s_ppf_aligned = _mm_load_ps( NPPF ); // align it to get _load_ps
            s_vel_aligned = _mm_load_ps( VEL );

            s_left1 = _mm_load_ps( APF_min1 );
            s_left2 = _mm_load_ps( APF_min2 );
            s_right2 = _mm_load_ps( APF_pl2 );
            s_right1 = _mm_load_ps( APF_pl1 );

            s_under2 = _mm_loadu_ps( APF ); // APF + 2

            s_actual = _mm_shuffle_ps( s_above2, s_under2, _MM_SHUFFLE( 1, 0, 3, 2 ) ); // 3 4 5 6
            s_above1 = _mm_shuffle_ps( s_above2, s_actual, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 2 3 4 5
            s_under1 = _mm_shuffle_ps( s_actual, s_under2, _MM_SHUFFLE( 2, 1, 2, 1 ) ); // 4 5 6 7

            // sum up
            s_sum1 = _mm_add_ps( s_under1,
                                 _mm_add_ps( s_above1,
                                             _mm_add_ps( s_left1, s_right1)));

            s_above2 = _mm_add_ps( s_left2, _mm_add_ps( s_right2, _mm_add_ps( s_under2, s_above2)));

            s_sum1 = _mm_fmsub_ps( s_sixteen, s_sum1,  s_above2);
            s_sum1 = _mm_fnmadd_ps( s_sixty, s_actual, s_sum1 );
            s_sum1 = _mm_fmadd_ps( s_vel_aligned, s_sum1, _mm_fmsub_ps(s_two, s_actual, s_ppf_aligned) );

            _mm_store_ps( NPPF, s_sum1);

            s_above2 = s_under2;
            NPPF+=4;
            VEL+=4;
            APF+=4;
            APF_pl1+=4;
            APF_pl2+=4;
            APF_min1+=4;
            APF_min2+=4;
            j--;
        }
        while( j > 0 );
        APF+=4;
        NPPF+=4;
        VEL+=4;
        APF_min1+=4;
        APF_min2+=4;
        APF_pl1+=4;
        APF_pl2+=4;
        i--;
    }
    while( i > 0 );
}

SEISMIC_EXEC_SSE_FCT( fma_partial_aligned );
