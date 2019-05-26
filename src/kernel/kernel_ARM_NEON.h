// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#ifndef _KERNEL_ARM_NEON_H_
#define _KERNEL_ARM_NEON_H_
#if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP )
#include "kernel.h"
#include <arm_neon.h>

// function that implements the kernel of the seismic modeling algorithm
#define SEISMIC_EXEC_ARM_NEON_FCT( NAME ) \
void seismic_exec_arm_neon_##NAME( void * v ) \
{ \
    stack_t * data = (stack_t*) v; \
 \
    /* preload register with const. values. */ \
    float32_t two = 2.0f; \
    float32_t sixteen = 16.0f; \
    float32_t min_sixty = -60.0f; \
 \
    float32x4_t neon_two     = vld1q_dup_f32( (const float32_t *) &two ); \
    float32x4_t neon_sixteen = vld1q_dup_f32( (const float32_t *) &sixteen ); \
    float32x4_t neon_minus_sixty = vld1q_dup_f32( (const float32_t *) &min_sixty ); \
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
            kernel_arm_neon_##NAME( data, neon_two, neon_sixteen, neon_minus_sixty ); \
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
        kernel_arm_neon_##NAME( data, neon_two, neon_sixteen, neon_minus_sixty ); \
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
void seismic_exec_arm_neon_##NAME##_pthread( void * v ) \
{ \
    stack_t * data = (stack_t*) v; \
 \
    /* preload register with const. values. */ \
    float32_t two = 2.0f; \
    float32_t sixteen = 16.0f; \
    float32_t min_sixty = -60.0f; \
 \
    float32x4_t neon_two     = vld1q_dup_f32( (const float32_t *) &two ); \
    float32x4_t neon_sixteen = vld1q_dup_f32( (const float32_t *) &sixteen ); \
    float32x4_t neon_minus_sixty = vld1q_dup_f32( (const float32_t *) &min_sixty ); \
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
                kernel_arm_neon_##NAME( data, neon_two, neon_sixteen, neon_minus_sixty ); \
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
            kernel_arm_neon_##NAME( data, neon_two, neon_sixteen, neon_minus_sixty ); \
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
            kernel_arm_neon_##NAME( data, neon_two, neon_sixteen, neon_minus_sixty ); \
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

#endif /* #if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP ) */
#endif /* #ifndef _KERNEL_ARM_NEON_H_ */
