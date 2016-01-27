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

// function that implements the kernel of the seismic modeling algorithm
void seismic_exec_plain( void * v )
{
    stack_t * data = (stack_t*) v;

    int teiler = 10;
    if(data->timesteps < teiler)
       teiler = data->timesteps;

    printf("processing...\n");

// measure performance from here

    // time loop
    unsigned i, j, t;
    for (t = 0; t < data->timesteps; t++)
    {
        // spatial loop in x
        for (i=2; i<data->width - 2; i++){
            // spatial loop in y
            for (j=2; j<data->height - 2; j++) {
                unsigned r = i * data->height + j;
                unsigned r_min1 = r - data->height;
                unsigned r_min2 = r - (data->height * 2);
                unsigned r_plus1 = r + data->height;
                unsigned r_plus2 = r + (data->height * 2);
                
                // calculates the pressure field t+1
                data->nppf[ r ] = 2.0f*data->apf[ r ] - data->nppf[ r ] + data->vel[ r ] 
                    *(16.0f*(data->apf[ r -1]+data->apf[ r +1]+data->apf[ r_min1 ]+data->apf[ r_plus1 ] )
                      - (data->apf[ r -2]+data->apf[ r +2]+data->apf[ r_min2 ]+data->apf[ r_plus2 ] )
                      -60.0f*data->apf[ r ]);
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
}


// function that implements the kernel of the seismic modeling algorithm
void seismic_exec_pthread( void * v )
{
    stack_t * data = (stack_t*) v;

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
        for (i=data->x_start; i<data->x_end; i++){
          // spatial loop in y
          for (j=data->y_start; j<data->y_end; j++) {
                unsigned r = i * data->height + j;
                unsigned r_min1 = r - data->height;
                unsigned r_min2 = r - (data->height * 2);
                unsigned r_plus1 = r + data->height;
                unsigned r_plus2 = r + (data->height * 2);

                // calculates the pressure field t+1
                data->nppf[ r ] = 2.0f*data->apf[ r ] - data->nppf[ r ] + data->vel[ r ] 
                    *(16.0f*(data->apf[ r -1]+data->apf[ r +1]+data->apf[ r_min1 ]+data->apf[ r_plus1 ] )
                      - (data->apf[ r -2]+data->apf[ r +2]+data->apf[ r_min2 ]+data->apf[ r_plus2 ] )
                      -60.0f*data->apf[ r ]);
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
