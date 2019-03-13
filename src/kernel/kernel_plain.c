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

#include "kernel.h"

inline __attribute__((always_inline)) void kernel_plain_naiiv( stack_t * data )
{
  unsigned x, z;
  for (x=data->x_start; x<data->x_end; x++){
    // spatial loop in z
    for (z=data->y_start; z<data->y_end; z++) {
      // calculates the pressure field t+1
      unsigned off = x * data->height + z;
      data->nppf[ off ] = 2.0f*data->apf[ off ] - data->nppf[ off ] + data->vel[ off ] 
          *(-60.0f*data->apf[ off ]
            +16.0f*(data->apf[ off - 1 ]+data->apf[ off + 1 ]+data->apf[ off - data->height ]+data->apf[ off + data->height ] )
            -(data->apf[ off - 2 ]+data->apf[ off + 2 ]+data->apf[ off - (data->height * 2) ]+data->apf[ off + (data->height * 2) ] ));
    }
  }
}

// function that implements the kernel of the seismic modeling algorithm
void seismic_exec_plain_naiiv( void * v )
{
    stack_t * data = (stack_t*) v;

    gettimeofday(&data->s, NULL);

    // time loop
    unsigned t;
    for (t = 0; t < data->timesteps; t++)
    {
        // inserts the seismic pulse value in the desired position
        data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t];

        kernel_plain_naiiv( data );

        // switch pointers instead of copying data
        float * tmp = data->nppf;
        data->nppf = data->apf;
        data->apf = tmp;
        
        // shows one # at each 10% of the total processing time
        if( ! (t % (data->timesteps/10)) )
        {
            printf("#");
            fflush(stdout);
        }
    }

    gettimeofday(&data->e, NULL);
}

// function that implements the kernel of the seismic modeling algorithm
void seismic_exec_plain_naiiv_pthread( void * v )
{
    stack_t * data = (stack_t*) v;

    gettimeofday(&data->s, NULL);

    // time loop
    unsigned t;
    for (t = 0; t < data->timesteps; t++)
    {
        BARRIER( data->barrier, data->id );

        // inserts the seismic pulse value in the desired position
        if( data->set_pulse )
          data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t];

        BARRIER( data->barrier, data->id );

        kernel_plain_naiiv( data );

        // switch pointers instead of copying data
        float * tmp = data->nppf;
        data->nppf = data->apf;
        data->apf = tmp;
        
        // shows one # at each 10% of the total processing time
        if( ! data->id && ! (t % (data->timesteps/10)) )
        {
            printf("#");
            fflush(stdout);
        }
    }

    gettimeofday(&data->e, NULL);

    if( data->id )
        pthread_exit( NULL );
}


inline __attribute__((always_inline)) void kernel_plain_opt( stack_t * data )
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
void seismic_exec_plain_opt( void * v )
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
            kernel_plain_opt( data );

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
        kernel_plain_opt( data );

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
void seismic_exec_plain_opt_pthread( void * v )
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
                kernel_plain_opt( data );

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
            kernel_plain_opt( data );

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
            kernel_plain_opt( data );

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


