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

#include "kernel_avx2.h"
#include <inttypes.h>



/*
  AVX2 required!

  combines vectors: a) 0.f 1.f 2.f 3.f 4.f 5.f 6.f 7.f
                    b)         2.f 3.f 4.f 5.f 6.f 7.f 8.f 9.f

  to vector       res)     1.f 2.f 3.f 4.f 5.f 6.f 7.f 8.f
*/
inline __attribute__((always_inline)) __m256 avx2_combine( __m256 a, __m256 b, __m256i s_shl, __m256i s_shr ) {
  __m256 a_l = _mm256_permutevar8x32_ps( a, s_shl );
  __m256 b_r = _mm256_permutevar8x32_ps( b, s_shr );

  __m256 res = _mm256_permute2f128_ps( a_l, b_r, 0x34 );
  return res;
}


inline __attribute__((always_inline)) void kernel_avx2_unaligned( stack_t * data, __m256 s_two, __m256 s_sixteen, __m256 s_sixty, __m256i s_shl, __m256i s_shr )
{
    unsigned i, j;
    __m256 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m256 s_above2, s_under2, s_left2, s_right2;

    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++){
        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=8) {
            unsigned r = i * data->height + j;
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height * 2);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height * 2);

            // calculates the pressure field t+1
            s_ppf_aligned = _mm256_loadu_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
            s_vel_aligned= _mm256_loadu_ps( &(data->vel[ r ]) );
            s_actual = _mm256_loadu_ps( &(data->apf[ r ]) );

// eigentlich für SSE nur links und rechts nötig ... und dann kann durch combine mittlere erhalten werden

            s_left1 = _mm256_loadu_ps( &(data->apf[ r_min1 ]) );
            s_left2 = _mm256_loadu_ps( &(data->apf[ r_min2 ]) );
            s_right2 = _mm256_loadu_ps( &(data->apf[ r_plus2 ]) );
            s_right1 = _mm256_loadu_ps( &(data->apf[ r_plus1 ]) );

//            s_above1 = _mm256_loadu_ps( &(data->apf[ r -1]) );
//            s_under1 = _mm256_loadu_ps( &(data->apf[ r +1]) );

            s_above2 = _mm256_loadu_ps( &(data->apf[ r -2]) );
            s_under2 = _mm256_loadu_ps( &(data->apf[ r +2]) );

            s_above1 = avx2_combine( s_above2, s_actual, s_shl, s_shr );
            s_under1 = avx2_combine( s_actual, s_under2, s_shl, s_shr );

            // sum up
            s_sum1 = _mm256_add_ps( s_under1, _mm256_add_ps( s_above1, _mm256_add_ps( s_left1, s_right1)));
            s_above2 = _mm256_add_ps( s_left2, _mm256_add_ps( s_right2, _mm256_add_ps( s_under2, s_above2)));
            s_sum1 = _mm256_mul_ps( s_sixteen, s_sum1 );
            s_sum1 = _mm256_sub_ps( _mm256_sub_ps( s_sum1,  s_above2), _mm256_mul_ps( s_sixty, s_actual ) );
            s_sum1 = _mm256_add_ps( _mm256_mul_ps( s_vel_aligned, s_sum1), _mm256_sub_ps(_mm256_mul_ps( s_two, s_actual ), s_ppf_aligned) );

            _mm256_storeu_ps( &(data->nppf[ r ]), s_sum1);
        }
    }
}

SEISMIC_EXEC_AVX2_FCT( unaligned )
