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

#ifndef _KERNEL_AVX_H_
#define _KERNEL_AVX_H_

#include "kernel.h"
#include <immintrin.h>

// function that implements the kernel of the seismic modeling algorithm
#define SEISMIC_EXEC_AVX_FCT( NAME ) \
void seismic_exec_avx_##NAME( void * v ) \
{ \
    stack_t * data = (stack_t*) v; \
 \
    /* preload register with const. values. */ \
    float two = 2.0f; \
    float sixteen = 16.0f; \
    float sixty = 60.0f; \
 \
    __m256 s_two = _mm256_broadcast_ss( (const float*) &two ); \
    __m256 s_sixteen = _mm256_broadcast_ss( (const float*) &sixteen ); \
    __m256 s_sixty = _mm256_broadcast_ss( (const float*) &sixty ); \
 \
    data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[0]; \
 \
    unsigned num_div = data->timesteps / 10; \
    unsigned num_mod = data->timesteps - (num_div * 10); \
 \
    gettimeofday(&data->s, NULL); \
 \
    /* time loop */ \
    unsigned t, r, t_tmp = 0; \
    for( r = 0; r < 10; r++ ) { \
        for (t = 0; t < num_div; t++, t_tmp++) \
        { \
            kernel_avx_##NAME( data, s_two, s_sixteen, s_sixty ); \
 \
            /* switch pointers instead of copying data */ \
            float * tmp = data->nppf; \
            data->nppf = data->apf; \
            data->apf = tmp; \
 \
            /* + 1 because we add the pulse for the _next_ time step */ \
            /* inserts the seismic pulse value in the desired position */ \
            data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t_tmp+1]; \
        } \
 \
        /* shows one # at each 10% of the total processing time */ \
        { \
            printf("#"); \
            fflush(stdout); \
        } \
    } \
    for (t = 0; t < num_mod; t++) \
    { \
        kernel_avx_##NAME( data, s_two, s_sixteen, s_sixty ); \
 \
        /* switch pointers instead of copying data */ \
        float * tmp = data->nppf; \
        data->nppf = data->apf; \
        data->apf = tmp; \
 \
        /* + 1 because we add the pulse for the _next_ time step */ \
        /* inserts the seismic pulse value in the desired position */ \
        data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t_tmp+t+1]; \
    } \
 \
    gettimeofday(&data->e, NULL); \
} \
 \
 \
void seismic_exec_avx_##NAME##_pthread(void * v ) \
{ \
    stack_t * data = (stack_t*) v; \
 \
    /* preload register with const. values. */ \
    float two = 2.0f; \
    float sixteen = 16.0f; \
    float sixty = 60.0f; \
 \
    __m256 s_two = _mm256_broadcast_ss( (const float*) &two ); \
    __m256 s_sixteen = _mm256_broadcast_ss( (const float*) &sixteen ); \
    __m256 s_sixty = _mm256_broadcast_ss( (const float*) &sixty ); \
 \
    if( data->set_pulse ) \
        data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[0]; \
 \
    unsigned num_div = data->timesteps / 10; \
    unsigned num_mod = data->timesteps - (num_div * 10); \
 \
    /* start everything in parallel */ \
    BARRIER( data->barrier, data->id ); \
 \
    gettimeofday(&data->s, NULL); \
 \
    /* time loop */ \
    unsigned t; \
    if( data->set_pulse ) \
    { \
        unsigned r, t_tmp = 0; \
        for( r = 0; r < 10; r++ ) { \
            for (t = 0; t < num_div; t++, t_tmp++) \
            { \
                kernel_avx_##NAME( data, s_two, s_sixteen, s_sixty ); \
 \
                /* switch pointers instead of copying data */ \
                float * tmp = data->nppf; \
                data->nppf = data->apf; \
                data->apf = tmp; \
 \
                /* + 1 because we add the pulse for the _next_ time step */ \
                /* inserts the seismic pulse value in the desired position */ \
                data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t_tmp+1]; \
 \
                BARRIER( data->barrier, data->id ); \
            } \
 \
            /* shows one # at each 10% of the total processing time */ \
            { \
                printf("#"); \
                fflush(stdout); \
            } \
        } \
        for (t = 0; t < num_mod; t++) \
        { \
            kernel_avx_##NAME( data, s_two, s_sixteen, s_sixty ); \
 \
            /* switch pointers instead of copying data */ \
            float * tmp = data->nppf; \
            data->nppf = data->apf; \
            data->apf = tmp; \
 \
            /* + 1 because we add the pulse for the _next_ time step */ \
            /* inserts the seismic pulse value in the desired position */ \
            data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t_tmp+t+1]; \
 \
            BARRIER( data->barrier, data->id ); \
        } \
    } \
    else \
        for (t = 0; t < data->timesteps; t++) \
        { \
            kernel_avx_##NAME( data, s_two, s_sixteen, s_sixty ); \
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

#endif /* #ifndef _KERNEL_AVX_H_ */