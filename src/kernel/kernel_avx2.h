// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Dr.-Ing. Patrick Siegl <patrick@siegl.it>

#ifndef _KERNEL_AVX2_H_
#define _KERNEL_AVX2_H_
#ifdef __x86_64__
#include "kernel.h"
#include <immintrin.h>
#include <stdint.h>

/*
  AVX2 required!

  combines vectors: a) 0.f 1.f 2.f 3.f 4.f 5.f 6.f 7.f
                    b)         2.f 3.f 4.f 5.f 6.f 7.f 8.f 9.f

  to vector       res)     1.f 2.f 3.f 4.f 5.f 6.f 7.f 8.f
*/
#define AVX2_CENTER( a, b, s_shl, s_shr ) \
({ \
  __m256 a_l = _mm256_permutevar8x32_ps( (a), (s_shl) ); \
  __m256 b_r = _mm256_permutevar8x32_ps( (b), (s_shr) ); \
  \
  _mm256_permute2f128_ps( (a_l), (b_r), 0x34 ); \
})

static inline __attribute__((always_inline)) void init_shuffle( __m256i * s_shl, __m256i * s_shr ) {
  uint32_t shl[8] = { 1, 2, 3, 4, 5, 6, 7, 7 };
  uint32_t shr[8] = { 0, 0, 1, 2, 3, 4, 5, 6 };

  *s_shl =  _mm256_lddqu_si256( (__m256i const *) &shl[0] );
  *s_shr =  _mm256_lddqu_si256( (__m256i const *) &shr[0] );
}

// function that implements the kernel of the seismic modeling algorithm
#define SEISMIC_EXEC_AVX2_FCT( NAME ) \
void seismic_exec_avx2_##NAME( void * v ) \
{ \
    stack_t * data = (stack_t*) v; \
 \
    /* preload register with const. values. */ \
    float two = 2.0f; \
    float sixteen = 16.0f; \
    float min_sixty = -60.0f; \
 \
    __m256 s_two = _mm256_broadcast_ss( (const float*) &two ); \
    __m256 s_sixteen = _mm256_broadcast_ss( (const float*) &sixteen ); \
    __m256 s_min_sixty = _mm256_broadcast_ss( (const float*) &min_sixty ); \
 \
    __m256i s_shl, s_shr; \
    init_shuffle( &s_shl, &s_shr ); \
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
            kernel_avx2_##NAME( data, s_two, s_sixteen, s_min_sixty, s_shl, s_shr ); \
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
        kernel_avx2_##NAME( data, s_two, s_sixteen, s_min_sixty, s_shl, s_shr ); \
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
void seismic_exec_avx2_##NAME##_pthread(void * v ) \
{ \
    stack_t * data = (stack_t*) v; \
 \
    /* preload register with const. values. */ \
    float two = 2.0f; \
    float sixteen = 16.0f; \
    float min_sixty = -60.0f; \
 \
    __m256 s_two = _mm256_broadcast_ss( (const float*) &two ); \
    __m256 s_sixteen = _mm256_broadcast_ss( (const float*) &sixteen ); \
    __m256 s_min_sixty = _mm256_broadcast_ss( (const float*) &min_sixty ); \
 \
    __m256i s_shl, s_shr; \
    init_shuffle( &s_shl, &s_shr ); \
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
                kernel_avx2_##NAME( data, s_two, s_sixteen, s_min_sixty, s_shl, s_shr ); \
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
            kernel_avx2_##NAME( data, s_two, s_sixteen, s_min_sixty, s_shl, s_shr ); \
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
            kernel_avx2_##NAME( data, s_two, s_sixteen, s_min_sixty, s_shl, s_shr ); \
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
#endif /* #ifndef _KERNEL_AVX2_H_ */
