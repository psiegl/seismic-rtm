// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#include "kernel_ppc.h"

inline __attribute__((always_inline)) void kernel_vmx_aligned( stack_t * data, vector float s_two, vector float s_sixteen, vector float s_min_sixty, vector unsigned char mergeOneHighThreeLow, vector unsigned char mergeThreeHighOneLow, vector unsigned char mergeHighLow  )
{
    vector float s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    vector float s_above2, s_under2, s_left2, s_right2;
    
    vector float s_zero = (vector float) {0.0f, 0.0f, 0.0f, 0.0f};
    vector float s_one = (vector float) {1.0f, 1.0f, 1.0f, 1.0f};
    
    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {
    
        unsigned r = i * data->height + data->y_start;
        s_above1 = vec_ld(0, &(data->apf[ r -4]) );
        s_actual = vec_ld(0, &(data->apf[ r ]) );
        s_above2 = vec_perm(s_above1, s_actual, mergeHighLow);
        
        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=4, r+=4) {
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height << 1);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height << 1);
            
            // calculates the pressure field t+1
            s_under1 = vec_ld(0, &(data->apf[ r +4]) );
            vector float s_under1_orig = s_under1;
            s_left2 = vec_ld(0, &(data->apf[r_min2]) );
            s_right2 = vec_ld(0, &(data->apf[r_plus2]) );

            s_under2 = vec_perm(s_actual, s_under1, mergeHighLow);
            
            s_above2 = vec_add( vec_add( s_right2, s_left2),  vec_add( s_above2, s_under2));
            
            s_above1 = vec_perm(s_above1, s_actual, mergeOneHighThreeLow);
            s_under1 = vec_perm(s_actual, s_under1, mergeThreeHighOneLow);

            s_left1 = vec_ld(0, &(data->apf[r_min1]) );
            s_right1 = vec_ld(0, &(data->apf[r_plus1]) );

            // sum up
            s_sum1 = vec_add( vec_add(s_under1, s_above1), vec_add( s_left1, s_right1));

            s_ppf_aligned = vec_ld(0, &(data->nppf[ r ]) );
            s_vel_aligned = vec_ld(0, &(data->vel[ r ]) );

            s_ppf_aligned = vec_nmsub( vec_nmsub( s_two, s_actual, s_ppf_aligned ), s_one, s_zero);
            s_sum1 = vec_nmsub( s_one,
                                  vec_nmsub( s_sixteen, s_sum1, s_above2),
                                  s_zero);

            s_sum1 = vec_madd( s_min_sixty, s_actual, s_sum1 );
            s_sum1 = vec_madd( s_ppf_aligned, s_one, vec_madd( s_vel_aligned, s_sum1, s_zero ) );

            // store result
            s_above1 = s_actual;
            s_actual = s_under1_orig;
            s_above2 = s_under2;
            vec_st(s_sum1, 0, &(data->nppf[ r ]));
        }
    }
}

SEISMIC_EXEC_PPC_FCT( vmx_aligned );
SYM_KERNEL( vmx, HAS_VMX, 4 * sizeof(float), 4 * sizeof(float) );
