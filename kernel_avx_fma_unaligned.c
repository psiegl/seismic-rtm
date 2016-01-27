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
#include <immintrin.h>

// function that implements the kernel of the seismic modeling algorithm
void seismic_exec_avx_fma_unaligned( void * v )
{
    stack_t * data = (stack_t*) v;
    int i, j, t;
    // sse vars... just like the OpenCL kernel
    __m256 s_two, s_sixteen, s_sixty;
    __m256 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m256 s_above2, s_under2, s_left2, s_right2;

    // preload register with const. values.
    float two = 2.0f;
    float sixteen = 16.0f;
    float sixty = 60.0f;

    s_two = _mm256_broadcast_ss( (const float*) &two );
    s_sixteen = _mm256_broadcast_ss( (const float*) &sixteen );
    s_sixty = _mm256_broadcast_ss( (const float*) &sixty );

    int teiler = 10;
    if(data->timesteps < teiler)
       teiler = data->timesteps;

    printf("processing...\n");

// measure performance from here

    // time loop
    for (t = 0; t < data->timesteps; t++)
    {
        // spatial loop in x
        for (i=2; i<data->width - 2; i++){
            // spatial loop in y
            for (j=2; j<data->height - 2; j+=8) {
                unsigned r = i * data->height + j;
                unsigned r_min1 = r - data->height;
                unsigned r_min2 = r - (data->height * 2);
                unsigned r_plus1 = r + data->height;
                unsigned r_plus2 = r + (data->height * 2);

                // calculates the pressure field t+1
                s_ppf_aligned = _mm256_loadu_ps( &(data->nppf[ r ]) ); // align it to get _load_ps
                s_vel_aligned= _mm256_loadu_ps( &(data->vel[ r ]) );
                s_actual = _mm256_loadu_ps( &(data->apf[ r ]) );

                s_left1 = _mm256_loadu_ps( &(data->apf[ r_min1 ]) );
                s_left2 = _mm256_loadu_ps( &(data->apf[ r_min2 ]) );
                s_right2 = _mm256_loadu_ps( &(data->apf[ r_plus2 ]) );
                s_right1 = _mm256_loadu_ps( &(data->apf[ r_plus1 ]) );
                s_above1 = _mm256_loadu_ps( &(data->apf[ r -1]) );
                s_under1 = _mm256_loadu_ps( &(data->apf[ r +1]) );

                s_above2 = _mm256_loadu_ps( &(data->apf[ r -2]) );

                s_under2 = _mm256_loadu_ps( &(data->apf[ r +2]) );

                // sum up
                s_sum1 = _mm256_add_ps( s_under1, _mm256_add_ps( s_above1, _mm256_add_ps( s_left1, s_right1)));
                s_above2 = _mm256_add_ps( s_left2, _mm256_add_ps( s_right2, _mm256_add_ps( s_under2, s_above2)));

                s_sum1 = _mm256_fmsub_ps( s_sixteen, s_sum1,  s_above2);

                s_sum1 = _mm256_fnmadd_ps( s_sixty, s_actual, s_sum1 );

                s_sum1 = _mm256_fmadd_ps( s_vel_aligned, s_sum1, _mm256_fmsub_ps(s_two, s_actual, s_ppf_aligned) );

                _mm256_storeu_ps( &(data->nppf[ r ]), s_sum1);
            }
        }

        // switch pointers instead of copying data
        float * tmp = data->nppf;
        data->nppf = data->apf;
        data->apf = tmp;

        // inserts the seismic pulse value in the desired position
        data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t+1];

        // shows one # at each 10% of the total processing time
        if ( ! (t % (data->timesteps/teiler)) )
        {
            printf("#");
            fflush(stdout);
        }
    }

// until here

    printf("\nend process!\n");
    fflush(stdout);
}


// function that implements the kernel of the seismic modeling algorithm
void seismic_exec_avx_fma_unaligned_pthread(void * v )
{
    stack_t * data = (stack_t*) v;

    // sse vars... just like the OpenCL kernel
    __m256 s_two, s_sixteen, s_sixty;
    __m256 s_ppf_aligned, s_vel_aligned, s_actual, s_above1, s_left1, s_under1, s_right1, s_sum1;
    __m256 s_above2, s_under2, s_left2, s_right2;

    // preload register with const. values.
    float two = 2.0f;
    float sixteen = 16.0f;
    float sixty = 60.0f;

    s_two = _mm256_broadcast_ss( (const float*) &two );
    s_sixteen = _mm256_broadcast_ss( (const float*) &sixteen );
    s_sixty = _mm256_broadcast_ss( (const float*) &sixty );

    int teiler = 10;
    int isSeismicPrivileg = data->set_pulse;
    if( isSeismicPrivileg ) {
      if(data->timesteps < teiler)
         teiler = data->timesteps;

      printf("processing...\n");
    }

    // start everything in parallel
    BARRIER( data->barrier, data->id );
/*    unsigned ret = pthread_barrier_wait( data->barrier );
    if( ret && ret != PTHREAD_BARRIER_SERIAL_THREAD ) {
        fprintf(stderr, "ERROR: Couldn't sync on barrier!!\nExiting...\n");
        exit( EXIT_FAILURE );
    }
*/
// measure performance from here
  
    // time loop
    unsigned i, j, t;
    for (t = 0; t < data->timesteps; t++)
    {
        // spatial loop in x
        for (i=data->x_start; i<data->x_end; i++) {
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

                s_left1 = _mm256_loadu_ps( &(data->apf[ r_min1 ]) );
                s_left2 = _mm256_loadu_ps( &(data->apf[ r_min2 ]) );
                s_right2 = _mm256_loadu_ps( &(data->apf[ r_plus2 ]) );
                s_right1 = _mm256_loadu_ps( &(data->apf[ r_plus1 ]) );
                s_above1 = _mm256_loadu_ps( &(data->apf[ r -1]) );
                s_under1 = _mm256_loadu_ps( &(data->apf[ r +1]) );

                s_above2 = _mm256_loadu_ps( &(data->apf[ r -2]) );

                s_under2 = _mm256_loadu_ps( &(data->apf[ r +2]) );

                // sum up
                s_sum1 = _mm256_add_ps( s_under1, _mm256_add_ps( s_above1, _mm256_add_ps( s_left1, s_right1)));
                s_above2 = _mm256_add_ps( s_left2, _mm256_add_ps( s_right2, _mm256_add_ps( s_under2, s_above2)));

                s_sum1 = _mm256_fmsub_ps( s_sixteen, s_sum1,  s_above2);

                s_sum1 = _mm256_fnmadd_ps( s_sixty, s_actual, s_sum1 );

                s_sum1 = _mm256_fmadd_ps( s_vel_aligned, s_sum1, _mm256_fmsub_ps(s_two, s_actual, s_ppf_aligned) );

                _mm256_storeu_ps( &(data->nppf[ r ]), s_sum1);
            }
        }

        // switch pointers instead of copying data
        float * tmp = data->nppf;
        data->nppf = data->apf;
        data->apf = tmp;

        // + 1 because we add the pulse for the _next_ time step
        if( isSeismicPrivileg )
        {
            // inserts the seismic pulse value in the desired position
            data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t+1];

            // shows one # at each 10% of the total processing time
            if ( ! (t % (data->timesteps/teiler)) )
            {
                printf("#");
                fflush(stdout);
            }
        }

        BARRIER( data->barrier, data->id );
        /*ret = pthread_barrier_wait( data->barrier );
        if( ret && ret != PTHREAD_BARRIER_SERIAL_THREAD ) {
            fprintf(stderr, "ERROR: Couldn't sync on barrier!!\nExiting...\n");
            exit( EXIT_FAILURE );
        }*/
    }

// until here

    if( data->x_start != 2 )
      pthread_exit( NULL );
    else {
      printf("\nend process!\n");
      fflush(stdout);
    }
}