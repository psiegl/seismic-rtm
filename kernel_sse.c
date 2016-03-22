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

#include "kernel.h"
#include <xmmintrin.h>

// function that implements the kernel of the seismic modeling algorithm
#define SEISMIC_EXEC_SSE_FCT( NAME ) \
void seismic_exec_sse_##NAME( void * v ) \
{ \
    stack_t * data = (stack_t*) v; \
 \
    float two[4] = {2.0f, 2.0f, 2.0f, 2.0f}; \
    float sixteen[4] = {16.0f,16.0f,16.0f,16.0f}; \
    float sixty[4] = {60.0f,60.0f,60.0f,60.0f}; \
 \
    /* preload register with const. values. */ \
    __m128 s_two = _mm_loadu_ps( (const float *) &two ); \
    __m128 s_sixteen = _mm_loadu_ps( (const float *) &sixteen ); \
    __m128 s_sixty = _mm_loadu_ps( (const float *) &sixty ); \
 \
    data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[0]; \
 \
    gettimeofday(&data->s, NULL); \
 \
    /* time loop */  \
    unsigned t; \
    for (t = 0; t < data->timesteps; t++) \
    { \
        kernel_sse_##NAME( data, s_two, s_sixteen, s_sixty ); \
 \
        /* switch pointers instead of copying data */ \
        float * tmp = data->nppf; \
        data->nppf = data->apf; \
        data->apf = tmp; \
 \
        /* inserts the seismic pulse value in the desired position */ \
        data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t+1]; \
 \
        /* shows one # at each 10% of the total processing time */ \
        if ( ! (t % (data->timesteps/10 + 1)) ) \
        { \
            printf("#"); \
            fflush(stdout); \
        } \
    } \
 \
    gettimeofday(&data->e, NULL); \
} \
 \
 \
void seismic_exec_sse_##NAME##_pthread(void * v ) \
{ \
    stack_t * data = (stack_t*) v; \
 \
    float two[4] = {2.0f, 2.0f, 2.0f, 2.0f}; \
    float sixteen[4] = {16.0f,16.0f,16.0f,16.0f}; \
    float sixty[4] = {60.0f,60.0f,60.f,60.0f}; \
 \
    /* preload register with const. values. */ \
    __m128 s_two = _mm_loadu_ps( (const float *) &two ); \
    __m128 s_sixteen = _mm_loadu_ps( (const float *) &sixteen ); \
    __m128 s_sixty = _mm_loadu_ps( (const float *) &sixty ); \
 \
    if( data->set_pulse ) \
        data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[0]; \
 \
    /* start everything in parallel */ \
    BARRIER( data->barrier, data->id ); \
 \
    gettimeofday(&data->s, NULL); \
 \
    /* time loop */ \
    unsigned t; \
    if( data->set_pulse ) \
        for (t = 0; t < data->timesteps; t++) \
        { \
            kernel_sse_##NAME( data, s_two, s_sixteen, s_sixty ); \
 \
            /* switch pointers instead of copying data */ \
            float * tmp = data->nppf; \
            data->nppf = data->apf; \
            data->apf = tmp; \
 \
            /* + 1 because we add the pulse for the _next_ time step */ \
            /* inserts the seismic pulse value in the desired position */ \
            data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t+1]; \
 \
            /* shows one # at each 10% of the total processing time */ \
            if ( ! (t % (data->timesteps/10 + 1)) ) \
            { \
                printf("#"); \
                fflush(stdout); \
            } \
 \
            BARRIER( data->barrier, data->id ); \
        } \
    else \
        for (t = 0; t < data->timesteps; t++) \
        { \
            kernel_sse_##NAME( data, s_two, s_sixteen, s_sixty ); \
 \
            /* switch pointers instead of copying data */ \
            float * tmp = data->nppf; \
            data->nppf = data->apf; \
            data->apf = tmp; \
 \
            BARRIER( data->barrier, data->id ); \
        } \
 \
    gettimeofday(&data->e, NULL); \
 \
    if( data->id ) \
        pthread_exit( NULL ); \
}



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
            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( std );



inline __attribute__((always_inline)) void kernel_sse_aligned( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
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
            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( aligned );



inline __attribute__((always_inline)) void kernel_sse_aligned_not_grouped( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
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

            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            _mm_store_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( aligned_not_grouped );



inline __attribute__((always_inline)) void kernel_sse_unaligned( stack_t * data, __m128 s_two, __m128 s_sixteen, __m128 s_sixty  )
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
            s_ppf_aligned = _mm_loadu_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
            s_vel_aligned= _mm_loadu_ps( &(data->vel[ r ]) );

            s_above2 = _mm_loadu_ps( &(data->apf[ r - 2]) );
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
            s_sum1 = _mm_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm_sub_ps( _mm_sub_ps( s_sum1,  s_above2), _mm_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm_add_ps( _mm_mul_ps( s_vel_aligned, s_sum1), _mm_sub_ps(_mm_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            _mm_storeu_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_SSE_FCT( unaligned );
