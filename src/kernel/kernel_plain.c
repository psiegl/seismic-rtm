// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#include "kernel.h"

inline __attribute__((always_inline)) void kernel_plain_naiiv( stack_t * data )
{
  unsigned x, z;
  for (x=data->x_start; x<data->x_end; x++){
    // spatial loop in z
    for (z=data->y_start; z<data->y_end; z+=data->y_offset) {
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
    unsigned t, p;
    for (t = 0, p = 0; t < data->timesteps; t++)
    {
        // inserts the seismic pulse value in the desired position
        data->apf[data->x_pulse * data->height + data->y_pulse] += data->pulsevector[t];

        kernel_plain_naiiv( data );

        // switch pointers instead of copying data
        float * tmp = data->nppf;
        data->nppf = data->apf;
        data->apf = tmp;
        
        // shows one # at each 10% of the total processing time
        if( ! data->id && t == p )
        {
            p += data->timesteps / 10;
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
    unsigned t, p;
    for (t = 0, p = 0; t < data->timesteps; t++)
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
        if( ! data->id && t == p )
        {
            p += data->timesteps / 10;
            printf("#");
            fflush(stdout);
        }
    }

    gettimeofday(&data->e, NULL);

    if( data->id )
        pthread_exit( NULL );
}

#define SYM_KERNEL_CAP {}
SYM_KERNEL( plain_naiiv, SYM_KERNEL_CAP, 0, 1 * sizeof(float) );






#define KERNEL_PLAIN_OPT_NO_PULSE( ITER_I ) \
{ \
  /* spatial loop in x */ \
  unsigned i = (ITER_I); \
  do \
  { \
    /* spatial loop in y */ \
    unsigned j = len_y; \
    do \
    { \
      /* calculates the pressure field t+1 */ \
      float v_OUT = *(APF-2); \
      v_OUT += *(APF+2); \
      float v_IN = *(APF-1); \
      v_IN += *(APF+1); \
      float v_APF = *APF; \
      v_IN += *APF_min1; \
      v_OUT += *APF_min2; \
      v_IN += *APF_pl1; \
      v_OUT += *APF_pl2; \
      float v_SUM = coeff_middle2 * v_APF; \
      v_SUM += coeff_inner * v_IN; \
      v_SUM += coeff_outer * v_OUT; \
 \
/*      (*NPPF) = coeff_middle * v_APF - (*NPPF) + (*VEL) * v_SUM; */ \
      (*NPPF) *= -1.0f; \
      (*NPPF) += coeff_middle * v_APF; \
      (*NPPF) += (*VEL) * v_SUM; \
 \
      APF += offset; \
      APF_min1 += offset; \
      APF_min2 += offset; \
      APF_pl1 += offset; \
      APF_pl2 += offset; \
      VEL += offset; \
      NPPF += offset; \
      j -= offset; \
    } \
    while( j > 0 ); \
    APF+=4; \
    NPPF+=4; \
    VEL+=4; \
    APF_min1+=4; \
    APF_min2+=4; \
    APF_pl1+=4; \
    APF_pl2+=4; \
    i--; \
  } \
  while( i > 0 ); \
}


inline __attribute__((always_inline)) void kernel_plain_opt_no_pulse( stack_t * data )
{
  float coeff_middle = 2.0f;
  float coeff_inner = 16.0f;
  float coeff_middle2 = -60.0f;
  float coeff_outer = -1.0f;

  unsigned r = data->x_start * data->height + data->y_start;
  unsigned offset = data->y_offset;
  float * NPPF = &data->nppf[ r ];
  float * VEL = &data->vel[ r ];
  float * APF = &data->apf[ r ];
  float * APF_pl1 = APF + data->height;
  float * APF_pl2 = APF_pl1 + data->height;
  float * APF_min1 = APF - data->height;
  float * APF_min2 = APF_min1 - data->height;
  unsigned len_x = data->x_end - data->x_start;
  unsigned len_y = data->height - 4;

//  if( ! len_y || ! len_x ) // checked in main!
//    return;

  KERNEL_PLAIN_OPT_NO_PULSE( len_x );
}




inline __attribute__((always_inline)) void kernel_plain_opt( stack_t * data, unsigned APF_offset, float** pulsevec )
{
  float coeff_middle = 2.0f;
  float coeff_inner = 16.0f;
  float coeff_middle2 = -60.0f;
  float coeff_outer = -1.0f;

  float * NPPF_pulse = &data->nppf[APF_offset];
  unsigned r = data->x_start * data->height + data->y_start;
  unsigned offset = data->y_offset;
  float * NPPF = &data->nppf[ r ];
  float * VEL = &data->vel[ r ];
  float * APF = &data->apf[ r ];
  float * APF_pl1 = APF + data->height;
  float * APF_pl2 = APF_pl1 + data->height;
  float * APF_min1 = APF - data->height;
  float * APF_min2 = APF_min1 - data->height;
  unsigned len_x = data->x_end - data->x_start;
  unsigned len_y = data->height - 4;

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
      float v_OUT = *(APF-2);
      v_OUT += *(APF+2);
      float v_IN = *(APF-1);
      v_IN += *(APF+1);
      float v_APF = *APF;
      v_IN += *APF_min1;
      v_OUT += *APF_min2;
      v_IN += *APF_pl1;
      v_OUT += *APF_pl2;
      float v_SUM = coeff_middle2 * v_APF;
      v_SUM += coeff_inner * v_IN;
      v_SUM += coeff_outer * v_OUT;

//      (*NPPF) = coeff_middle * v_APF - (*NPPF) + (*VEL) * v_SUM;
      (*NPPF) *= -1.0f;
      (*NPPF) += coeff_middle * v_APF;
      (*NPPF) += (*VEL) * v_SUM;

      if( NPPF == NPPF_pulse ) {
          *NPPF += *(*pulsevec);
          *pulsevec += 1;
      }

      APF += offset;
      APF_min1 += offset;
      APF_min2 += offset;
      APF_pl1 += offset;
      APF_pl2 += offset;
      VEL += offset;
      NPPF += offset;
      j -= offset;
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

inline __attribute__((always_inline)) void kernel_plain_opt_clopt( stack_t * data, unsigned APF_offset, float** pulsevec )
{
  float coeff_middle = 2.0f;
  float coeff_inner = 16.0f;
  float coeff_middle2 = -60.0f;
  float coeff_outer = -1.0f;

  float * NPPF_pulse = &data->nppf[APF_offset];
  unsigned r = data->x_start * data->height + data->y_start;
  unsigned offset = data->y_offset;
  float * NPPF = &data->nppf[ r ];
  float * VEL = &data->vel[ r ];
  float * APF = &data->apf[ r ];
  float * APF_pl1 = APF + data->height;
  float * APF_pl2 = APF_pl1 + data->height;
  float * APF_min1 = APF - data->height;
  float * APF_min2 = APF_min1 - data->height;
  unsigned len_y = data->height - 4;

//  if( ! len_y || ! len_x ) // checked in main!
//    return;

  KERNEL_PLAIN_OPT_NO_PULSE( data->x_pulse - data->x_start );

  {
    // spatial loop in y
    unsigned j = len_y;
    do
    {
      // calculates the pressure field t+1
      float v_OUT = *(APF-2);
      v_OUT += *(APF+2);
      float v_IN = *(APF-1);
      v_IN += *(APF+1);
      float v_APF = *APF;
      v_IN += *APF_min1;
      v_OUT += *APF_min2;
      v_IN += *APF_pl1;
      v_OUT += *APF_pl2;
      float v_SUM = coeff_middle2 * v_APF;
      v_SUM += coeff_inner * v_IN;
      v_SUM += coeff_outer * v_OUT;

//      (*NPPF) = coeff_middle * v_APF - (*NPPF) + (*VEL) * v_SUM;
      (*NPPF) *= -1.0f;
      (*NPPF) += coeff_middle * v_APF;
      (*NPPF) += (*VEL) * v_SUM;

      if( NPPF == NPPF_pulse ) {
          *NPPF += *(*pulsevec);
          *pulsevec += 1;
      }

      APF += offset;
      APF_min1 += offset;
      APF_min2 += offset;
      APF_pl1 += offset;
      APF_pl2 += offset;
      VEL += offset;
      NPPF += offset;
      j -= offset;
    }
    while( j > 0 );
    APF+=4;
    NPPF+=4;
    VEL+=4;
    APF_min1+=4;
    APF_min2+=4;
    APF_pl1+=4;
    APF_pl2+=4;
  }

  KERNEL_PLAIN_OPT_NO_PULSE( data->x_end - (data->x_pulse + 1) );
}





// function that implements the kernel of the seismic modeling algorithm
void seismic_exec_plain_opt( void * v )
{
    stack_t * data = (stack_t*) v;

    float* pulsevec = &data->pulsevector[0];
    unsigned APF_offset = data->x_pulse * data->height + data->y_pulse;

    data->apf[APF_offset] += *(pulsevec++);

    gettimeofday(&data->s, NULL);

    // time loop
    unsigned t, r, t_tmp = 0;
    unsigned p = data->timesteps / 10;
    for( r = 0; r < 10; r++ ) {
        for (t = 0; t < p; t++, t_tmp++)
        {
            kernel_plain_opt( data, APF_offset, &pulsevec );

            // switch pointers instead of copying data
            float * tmp = data->nppf;
            data->nppf = data->apf;
            data->apf = tmp;
        }

        // shows one # at each 10% of the total processing time
        {
            printf("#");
            fflush(stdout);
        }
    }
    for (t = t_tmp; t < data->timesteps; t++)
    {
        kernel_plain_opt( data, APF_offset, &pulsevec );

        // switch pointers instead of copying data
        float * tmp = data->nppf;
        data->nppf = data->apf;
        data->apf = tmp;
    }

    gettimeofday(&data->e, NULL);
}

// function that implements the kernel of the seismic modeling algorithm
void seismic_exec_plain_opt_pthread( void * v )
{
    stack_t * data = (stack_t*) v;

    float* pulsevec = &data->pulsevector[0];
    unsigned APF_offset = data->x_pulse * data->height + data->y_pulse;

    if( data->set_pulse )
        data->apf[APF_offset] += *(pulsevec++);

    // start everything in parallel
    BARRIER( data->barrier, data->id );

    gettimeofday(&data->s, NULL);

    // time loop
    unsigned t = 0;
#if 1
    if( data->set_pulse )
    {
        unsigned r, t_tmp = 0;
        unsigned p = data->timesteps / 10;
        for( r = 0; r < 10; r++ ) {
            for (t = 0; t < p; t++, t_tmp++)
            {
                kernel_plain_opt_clopt( data, APF_offset, &pulsevec );

                // switch pointers instead of copying data
                float * tmp = data->nppf;
                data->nppf = data->apf;
                data->apf = tmp;

                BARRIER( data->barrier, data->id );
            }

            // shows one # at each 10% of the total processing time
            {
                printf("#");
                fflush(stdout);
            }
        }
        for (t = t_tmp; t < data->timesteps; t++)
        {
            kernel_plain_opt_clopt( data, APF_offset, &pulsevec );

            // switch pointers instead of copying data
            float * tmp = data->nppf;
            data->nppf = data->apf;
            data->apf = tmp;

            BARRIER( data->barrier, data->id );
        }
    }
#else
    if( data->set_pulse )
    {
        unsigned r, t_tmp = 0;
        unsigned p = data->timesteps / 10;
        for( r = 0; r < 10; r++ ) {
            for (t = 0; t < p; t++, t_tmp++)
            {
                kernel_plain_opt( data, APF_offset, &pulsevec );

                // switch pointers instead of copying data
                float * tmp = data->nppf;
                data->nppf = data->apf;
                data->apf = tmp;

                BARRIER( data->barrier, data->id );
            }

            // shows one # at each 10% of the total processing time
            {
                printf("#");
                fflush(stdout);
            }
        }
        for (t = t_tmp; t < data->timesteps; t++)
        {
            kernel_plain_opt( data, APF_offset, &pulsevec );

            // switch pointers instead of copying data
            float * tmp = data->nppf;
            data->nppf = data->apf;
            data->apf = tmp;

            BARRIER( data->barrier, data->id );
        }
    }
#endif
    else {
        for (; t < data->timesteps; t++)
        {
            kernel_plain_opt_no_pulse( data );

            // switch pointers instead of copying data
            float * tmp = data->nppf;
            data->nppf = data->apf;
            data->apf = tmp;

            BARRIER( data->barrier, data->id );
        }
    }

    gettimeofday(&data->e, NULL);

    if( data->id )
        pthread_exit( NULL );
}

SYM_KERNEL( plain_opt, SYM_KERNEL_CAP, 0, 1 * sizeof(float) );
