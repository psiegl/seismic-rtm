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

inline __attribute__((always_inline)) void kernel_plain( stack_t * data )
{
  float coeff_middle = 2.0f;
  float coeff_inner = 16.0f;
  float coeff_middle2 = 60.0f;
  float coeff_outer = 1.0f;

  unsigned r = data->x_start * data->height + data->y_start;
  float * NPPF = &data->nppf[ r ];
  float * VEL = &data->vel[ r ];
  float * APF = &data->apf[ r ];
  float * APF_pl1 = APF + data->height;
  float * APF_pl2 = APF_pl1 + data->height;
  float * APF_min1 = APF - data->height;
  float * APF_min2 = APF_min1 - data->height;
  unsigned len_x = data->x_end - data->x_start;
  unsigned len_y = data->y_end - data->y_start;
  coeff_middle2 *= -1;
  coeff_outer *= -1;

//  if( ! len_y || ! len_x ) // checked in main!
//    return;

  // spatial loop in x
  unsigned i = len_x;
  do
  {
    // spatial loop in y
    unsigned j = len_y;
    do
    {
      // calculates the pressure field t+1
      //float v_IN = 0;
      //float v_OUT = 0;
      float v_OUT = *(APF-2);
      v_OUT += *(APF+2);
      float v_IN = *(APF-1);
      v_IN += *(APF+1);
      float v_APF = *(APF++);
      v_IN += *(APF_min1++);
      v_OUT += *(APF_min2++);
      v_IN += *(APF_pl1++);
      v_OUT += *(APF_pl2++);
      float v_SUM = coeff_middle2 * v_APF;
      v_SUM += coeff_inner * v_IN;
      v_SUM += coeff_outer * v_OUT;

//      (*NPPF) = coeff_middle * v_APF - (*NPPF) + (*VEL) * v_SUM;
      (*NPPF) *= -1;
      (*NPPF) += coeff_middle * v_APF;
      (*NPPF) += (*(VEL++)) * v_SUM;

      NPPF++;
      j--;
    }
    while( j > 0 );
    APF+=4;
    NPPF+=4;
    VEL+=4;
    APF_min1+=4;
    APF_min2+=4;
    APF_pl1+=4;
    APF_pl2+=4;
    i--;
  }
  while( i > 0 );
}



// function that implements the kernel of the seismic modeling algorithm
void seismic_exec_plain( void * v )
{
    stack_t * data = (stack_t*) v;

    data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[0];

    unsigned num_div = data->timesteps / 10;
    unsigned num_mod = data->timesteps - (num_div * 10);

    gettimeofday(&data->s, NULL);

    // time loop
    unsigned t, r, t_tmp = 0;
    for( r = 0; r < 10; r++ ) {
        for (t = 0; t < num_div; t++, t_tmp++)
        {
            kernel_plain( data );

            // switch pointers instead of copying data
            float * tmp = data->nppf;
            data->nppf = data->apf;
            data->apf = tmp;

            // + 1 because we add the pulse for the _next_ time step
            // inserts the seismic pulse value in the desired position
            data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t_tmp+1];
        }

        // shows one # at each 10% of the total processing time
        {
            printf("#");
            fflush(stdout);
        }
    }
    for (t = 0; t < num_mod; t++)
    {
        kernel_plain( data );

        // switch pointers instead of copying data
        float * tmp = data->nppf;
        data->nppf = data->apf;
        data->apf = tmp;

        // + 1 because we add the pulse for the _next_ time step
        // inserts the seismic pulse value in the desired position
        data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t_tmp+t+1];
    }

    gettimeofday(&data->e, NULL);
}






// function that implements the kernel of the seismic modeling algorithm
void seismic_exec_pthread( void * v )
{
    stack_t * data = (stack_t*) v;

    if( data->set_pulse )
        data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[0];

    unsigned num_div = data->timesteps / 10;
    unsigned num_mod = data->timesteps - (num_div * 10);

    // start everything in parallel
    BARRIER( data->barrier, data->id );

    gettimeofday(&data->s, NULL);

    // time loop
    unsigned t;
    if( data->set_pulse )
    {
        unsigned r, t_tmp = 0;
        for( r = 0; r < 10; r++ ) {
            for (t = 0; t < num_div; t++, t_tmp++)
            {
                kernel_plain( data );

                // switch pointers instead of copying data
                float * tmp = data->nppf;
                data->nppf = data->apf;
                data->apf = tmp;

                // + 1 because we add the pulse for the _next_ time step
                // inserts the seismic pulse value in the desired position
                data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t_tmp+1];

                BARRIER( data->barrier, data->id );
            }

            // shows one # at each 10% of the total processing time
            {
                printf("#");
                fflush(stdout);
            }
        }
        for (t = 0; t < num_mod; t++)
        {
            kernel_plain( data );

            // switch pointers instead of copying data
            float * tmp = data->nppf;
            data->nppf = data->apf;
            data->apf = tmp;

            // + 1 because we add the pulse for the _next_ time step
            // inserts the seismic pulse value in the desired position
            data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t_tmp+t+1];

            BARRIER( data->barrier, data->id );
        }
    }
    else
        for (t = 0; t < data->timesteps; t++)
        {
            kernel_plain( data );

            // switch pointers instead of copying data
            float * tmp = data->nppf;
            data->nppf = data->apf;
            data->apf = tmp;

            BARRIER( data->barrier, data->id );
        }

    gettimeofday(&data->e, NULL);

    if( data->id )
        pthread_exit( NULL );
}
