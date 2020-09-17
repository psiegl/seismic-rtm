// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Dr.-Ing. Patrick Siegl <patrick@siegl.it>

#ifndef _SEISMIC_H_
#define _SEISMIC_H_

#include <math.h> // sqrt, exp
#include <stdlib.h> // posix_memalign, malloc


// http://subsurfwiki.org/wiki/Ricker_wavelet
// http://wiki.seg.org/wiki/Dictionary:Ricker_wavelet
float ricker_wavelet(float t,float fc) {
  double x   = ((double)M_PI)*fc*t;
  double aux = x*x;
  return (1.0 - 2.0*aux)*exp(-aux);
}

#define init_seismic_pulsevector( pulsevector, timesteps, fpeak ) \
  { \
    float tdelay = 1.0/fpeak; \
    unsigned t; \
    for( t = 0; t < timesteps; t++ ) { \
      pulsevector[ t ] = ricker_wavelet( t - tdelay, fpeak ); \
    } \
    pulsevector[ timesteps ] = 0.0f; /* performance optimisation */ \
  }

#define init_seismic_matrices( width, height, VEL, APF, NPPF, fat ) \
  { \
    unsigned i; \
    for( i = 0; i < (height) * (width); i++ ) { \
      (APF)[ i ] = (NPPF)[ i ] = 0.0f; \
      (VEL)[ i ] = fat; \
    } \
  }

#define init_seismic_buffers( width, height, timesteps, VEL, APF, NPPF, pulsevector ) \
  { \
    float c_max  = 2000     ; \
    float c_min  =    0.002 ; \
    float h      =    2     ; \
    float fmax = h*c_min*5; \
    float dt = 0.606*h/c_max; /* this is the max value. otherwise it needs to be lower */ \
    printf("fmax %f, c_min %f, c_max %f, h %f, dt %f\n", fmax, c_min, c_max, h, dt); \
    init_seismic_pulsevector( (pulsevector), (timesteps), fmax ); \
    float c_avg = (c_max - c_min)/2 + c_min; /* loaded velocity */ \
    printf("courant val: %.12f\n", c_max * dt / h); \
    init_seismic_matrices( (width), (height), (VEL), (APF), (NPPF), (c_avg*c_avg*dt*dt)/( h * h * 12.0f ) ); \
  }


void * malloc_aligned( size_t len, unsigned alignment ) {
  if( ! alignment ) {
    return malloc( len );
  }
  else {
    /*
      we add aligned * sizeof(float), as we always want to start with (for i = 2; i < ...).
      below, we set the exact ptr. to this address, so that APF[2] starts at an aligned address ...
    */
    void * h;
    int ret = posix_memalign( &h, (size_t)(alignment * sizeof(float)), len + alignment ); // SSE: aligned = 4, AVX: aligned = 8
    if( ret != 0 || h == NULL )
      return NULL;
    return (void*)((char*)h + alignment - 2 * sizeof(float));
  }
}

int alloc_seismic_buffers( unsigned width, unsigned height, unsigned timesteps, unsigned alignment, float **VEL, float **APF, float **NPPF, float **pulsevector ) {
  unsigned long size_matrice = (width * height) * sizeof(float);
  *APF = (float*)malloc_aligned( size_matrice, alignment );
  *VEL = (float*)malloc_aligned( size_matrice, alignment );
  *NPPF = (float*)malloc_aligned( size_matrice, alignment );
  *pulsevector = (float*)malloc( (timesteps + 1) * sizeof(float) );
  return ( *APF == NULL
           || *VEL == NULL
           || *NPPF == NULL
           || *pulsevector == NULL ) * -1;
}

#endif /* #ifndef _SEISMIC_H_ */
