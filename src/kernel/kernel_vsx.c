//  This file is part of seismic-rtm.
//
//  Copyright (c) 2017, Dipl.-Inf. Patrick Siegl
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "kernel_ppc.h"

inline __attribute__((always_inline)) void kernel_vsx_unaligned( stack_t * data, vector float s_two, vector float s_sixteen, vector float s_min_sixty, vector unsigned char mergeOneHighThreeLow, vector unsigned char mergeThreeHighOneLow, vector unsigned char mergeHighLow  )
{
    vector float s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    vector float s_above2, s_under2, s_left2, s_right2;
    
    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {
    
        unsigned r = i * data->height + data->y_start;
        s_above1 = vec_vsx_ld(0, &(data->apf[ r -4]) );
        s_actual = vec_vsx_ld(0, &(data->apf[ r ]) );
        s_above2 = vec_perm(s_above1, s_actual, mergeHighLow);
        
        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=4, r+=4) {
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height << 1);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height << 1);
            
            // calculates the pressure field t+1
            s_under1 = vec_vsx_ld(0, &(data->apf[ r +4]) );
            vector float s_under1_orig = s_under1;
            s_left1 = vec_vsx_ld(0, &(data->apf[r_min1]) );
            s_left2 = vec_vsx_ld(0, &(data->apf[r_min2]) );
            s_right1 = vec_vsx_ld(0, &(data->apf[r_plus1]) );
            s_right2 = vec_vsx_ld(0, &(data->apf[r_plus2]) );
            s_ppf_aligned = vec_vsx_ld(0, &(data->nppf[ r ]) );
            s_vel_aligned = vec_vsx_ld(0, &(data->vel[ r ]) );

            s_under2 = vec_perm(s_actual, s_under1, mergeHighLow);
            
            s_above2 = vec_add( vec_add( s_right2, s_left2),  vec_add( s_above2, s_under2));
            
            s_above1 = vec_perm(s_above1, s_actual, mergeOneHighThreeLow);
            s_under1 = vec_perm(s_actual, s_under1, mergeThreeHighOneLow);

            // sum up
            s_sum1 = vec_add( vec_add(s_under1, s_above1), vec_add( s_left1, s_right1));

            s_sum1 = vec_msub( s_sixteen, s_sum1, s_above2); /* VSX */
            s_sum1 = vec_madd( s_min_sixty, s_actual, s_sum1 );
            s_ppf_aligned = vec_msub( s_two, s_actual, s_ppf_aligned ); /* VSX */
            s_sum1 = vec_madd( s_vel_aligned, s_sum1, s_ppf_aligned );

            // store result
            s_above1 = s_actual;
            s_actual = s_under1_orig;
            s_above2 = s_under2;
            vec_vsx_st(s_sum1, 0, &(data->nppf[ r ]));
        }
    }
}

SEISMIC_EXEC_PPC_FCT( vsx_unaligned );


inline __attribute__((always_inline)) void kernel_vsx_aligned( stack_t * data, vector float s_two, vector float s_sixteen, vector float s_min_sixty, vector unsigned char mergeOneHighThreeLow, vector unsigned char mergeThreeHighOneLow, vector unsigned char mergeHighLow  )
{
    vector float s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    vector float s_above2, s_under2, s_left2, s_right2;
    
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
            s_left1 = vec_ld(0, &(data->apf[r_min1]) );
            s_left2 = vec_ld(0, &(data->apf[r_min2]) );
            s_right1 = vec_ld(0, &(data->apf[r_plus1]) );
            s_right2 = vec_ld(0, &(data->apf[r_plus2]) );
            s_ppf_aligned = vec_ld(0, &(data->nppf[ r ]) );
            s_vel_aligned = vec_ld(0, &(data->vel[ r ]) );

            s_under2 = vec_perm(s_actual, s_under1, mergeHighLow);
            
            s_above2 = vec_add( vec_add( s_right2, s_left2),  vec_add( s_above2, s_under2));
            
            s_above1 = vec_perm(s_above1, s_actual, mergeOneHighThreeLow);
            s_under1 = vec_perm(s_actual, s_under1, mergeThreeHighOneLow);

            // sum up
            s_sum1 = vec_add( vec_add(s_under1, s_above1), vec_add( s_left1, s_right1));

            s_sum1 = vec_msub( s_sixteen, s_sum1, s_above2); /* VSX */
            s_sum1 = vec_madd( s_min_sixty, s_actual, s_sum1 );
            s_ppf_aligned = vec_msub( s_two, s_actual, s_ppf_aligned ); /* VSX */
            s_sum1 = vec_madd( s_vel_aligned, s_sum1, s_ppf_aligned );

            // store result
            s_above1 = s_actual;
            s_actual = s_under1_orig;
            s_above2 = s_under2;
            vec_st(s_sum1, 0, &(data->nppf[ r ]));
        }
    }
}

SEISMIC_EXEC_PPC_FCT( vsx_aligned );
