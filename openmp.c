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

#include <stdio.h>
#include <sys/time.h>
#include <omp.h>
#include "openmp.h"

/*
            #pragma vector aligned
            #pragma ivdep
//            #pragma unroll(8)
//            #pragma omp parallel for simd

*/

void openmp_threading( config_t *config, float *APF, float *VEL, float *NPPF, float *pulsevector, struct timeval * t1, struct timeval * t2 ) {

    omp_set_num_threads( config->threads );

    gettimeofday( t1, NULL);
    
    unsigned i, j, t;
    
    // time loop
    for (t = 0; t < config->timesteps; t++)
    {

        // inserts the seismic pulse value in the desired position
        APF[config->pulseX * config->height + config->pulseY] += pulsevector[t];

        #pragma omp parallel for
        for (i=2; i<config->width - 2; i++){

//          const int id = omp_get_thread_num();
//          printf("%d -> %d\n", id, omp_get_num_threads() );

          // spatial loop in y
          #pragma unroll(8)
          #pragma omp parallel for simd
          for (j=2; j<config->height - 2; j++) {
            // calculates the pressure field t+1
            NPPF[ (i * config->height + j) ] = 2.0f*APF[ (i * config->height + j) ] - NPPF[ (i * config->height + j) ] + VEL[ (i * config->height + j) ] 
                *(16.0f*(APF[ (i * config->height + j) -1]+APF[ (i * config->height + j) +1]+APF[ (i * config->height + j) - config->height ]+APF[ (i * config->height + j) + config->height ] )
                  - (APF[ (i * config->height + j) -2]+APF[ (i * config->height + j) +2]+APF[ (i * config->height + j) - (config->height * 2) ]+APF[ (i * config->height + j) + (config->height * 2) ] )
                  -60.0f*APF[ (i * config->height + j) ]);
          }
        }

        // switch pointers instead of copying data
        float * tmp = NPPF;
        NPPF = APF;
        APF = tmp;
        
        // shows one # at each 10% of the total processing time
        if( ! (t % (config->timesteps/10)) )
        {
            printf("#");
            fflush(stdout);
        }
    }

    gettimeofday( t2, NULL);
}
