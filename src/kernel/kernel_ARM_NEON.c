// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#include "kernel_ARM_NEON.h"

inline __attribute__((always_inline)) void kernel_arm_neon_aligned( stack_t * data, float32x4_t neon_two, float32x4_t neon_sixteen, float32x4_t neon_minus_sixty )
{
    float32x4_t neon_ppf, neon_vel, neon_actual, neon_above1, neon_left1, neon_under1, neon_right1, neon_sum, neon_under, neon_above;
    float32x4_t neon_above2, neon_under2, neon_left2, neon_right2;

    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {
        unsigned r = i * data->height + data->y_start;
        float32x4_t neon_left_pre  = vld1q_f32( (const float32_t *) &data->apf[ r - 4 ] );
        float32x4_t neon_actual = vld1q_f32( (const float32_t *) &data->apf[ r ] );
        float32x4_t neon_left2 = vextq_f32( neon_left_pre, neon_actual, 2 );
        
        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=4, r+=4) {
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height << 1);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height << 1);

            float32x4_t neon_ppf = vld1q_f32( (const float32_t *) &data->nppf[ r ] );
            float32x4_t neon_vel = vld1q_f32( (const float32_t *) &data->vel[ r ] );
            float32x4_t neon_right_pre = vld1q_f32( (const float32_t *) &data->apf[ r + 4 ] );
            float32x4_t neon_under1  = vld1q_f32( (const float32_t *) &data->apf[ r_min1 ] );
            float32x4_t neon_under2  = vld1q_f32( (const float32_t *) &data->apf[ r_min2 ] );
            float32x4_t neon_above1  = vld1q_f32( (const float32_t *) &data->apf[ r_plus1 ] );
            float32x4_t neon_above2  = vld1q_f32( (const float32_t *) &data->apf[ r_plus2 ] );
            float32x4_t neon_right1 = vextq_f32( neon_actual, neon_right_pre, 1 );
            float32x4_t neon_right2 = vextq_f32( neon_actual, neon_right_pre, 2 );
            float32x4_t neon_left1 = vextq_f32( neon_left_pre, neon_actual, 3 );
            float32x4_t sum0 = vaddq_f32( neon_left1, neon_right1 );
            float32x4_t sum1 = vaddq_f32( neon_under1, neon_above1 );
            float32x4_t sum2 = vaddq_f32( neon_left2, neon_right2 );
            float32x4_t sum3 = vaddq_f32( neon_under2, neon_above2 );
            float32x4_t sum4 = vaddq_f32( sum0, sum1 );
            float32x4_t sum5 = vaddq_f32( sum2, sum3 );
            float32x4_t sum6 = vmlsq_f32( sum5, sum4, neon_sixteen ); // negative!
            sum6 = vnegq_f32( sum6 );
            float32x4_t sum7 = vmlaq_f32( sum6, neon_actual, neon_minus_sixty );
            float32x4_t sum8 = vmlsq_f32( neon_ppf, neon_actual, neon_two ); // negative!
            sum8 = vnegq_f32( sum8 );
            float32x4_t sum9 = vmlaq_f32( sum8, neon_vel, sum7 );
            neon_left_pre = neon_actual;
            neon_actual = neon_right_pre;
            neon_left2 = neon_right2;
            vst1q_f32( (float32_t *) &(data->nppf[r]), sum9);
        }
    }
}

SEISMIC_EXEC_ARM_NEON_FCT( aligned );
SYM_KERNEL( arm_neon_aligned, HAS_NEON, 4 * sizeof(float), 4 * sizeof(float) );
