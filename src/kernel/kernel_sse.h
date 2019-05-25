// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#ifndef _KERNEL_SSE_H_
#define _KERNEL_SSE_H_
#ifdef __x86_64__
#include "kernel.h"
#include <xmmintrin.h>

// function that implements the kernel of the seismic modeling algorithm
#define SEISMIC_EXEC_SSE_FCT( NAME ) \
void seismic_exec_sse_##NAME( void * v ) \
{ \
    stack_t * data = (stack_t*) v; \
 \
    /* preload register with const. values. */ \
    float two[4] = {2.0f, 2.0f, 2.0f, 2.0f}; \
    float sixteen[4] = {16.0f,16.0f,16.0f,16.0f}; \
    float sixty[4] = {60.0f,60.0f,60.0f,60.0f}; \
 \
    __m128 s_two = _mm_loadu_ps( (const float *) &two ); \
    __m128 s_sixteen = _mm_loadu_ps( (const float *) &sixteen ); \
    __m128 s_sixty = _mm_loadu_ps( (const float *) &sixty ); \
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
            kernel_sse_##NAME( data, s_two, s_sixteen, s_sixty ); \
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
        kernel_sse_##NAME( data, s_two, s_sixteen, s_sixty ); \
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
void seismic_exec_sse_##NAME##_pthread(void * v ) \
{ \
    stack_t * data = (stack_t*) v; \
 \
    /* preload register with const. values. */ \
    float two[4] = {2.0f, 2.0f, 2.0f, 2.0f}; \
    float sixteen[4] = {16.0f,16.0f,16.0f,16.0f}; \
    float sixty[4] = {60.0f,60.0f,60.f,60.0f}; \
 \
    __m128 s_two = _mm_loadu_ps( (const float *) &two ); \
    __m128 s_sixteen = _mm_loadu_ps( (const float *) &sixteen ); \
    __m128 s_sixty = _mm_loadu_ps( (const float *) &sixty ); \
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
                kernel_sse_##NAME( data, s_two, s_sixteen, s_sixty ); \
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
            kernel_sse_##NAME( data, s_two, s_sixteen, s_sixty ); \
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

#endif /* #ifdef __x86_64__ */
#endif /* #ifndef _KERNEL_SSE_H_ */
