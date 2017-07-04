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
    return (h + alignment - 2 * sizeof(float));
  }
}

int alloc_seismic_buffers( unsigned width, unsigned height, unsigned timesteps, unsigned alignment, float **VEL, float **APF, float **NPPF, float **pulsevector ) {
  unsigned long size_matrice = (width * height) * sizeof(float);
  *APF = (float*)malloc_aligned( size_matrice, alignment );
  if( *APF == NULL )
    return -1;

  *VEL = (float*)malloc_aligned( size_matrice, alignment );
  if( *VEL == NULL )
    return -1;

  *NPPF = (float*)malloc_aligned( size_matrice, alignment );
  if( *NPPF == NULL )
    return -1;

  *pulsevector = (float*)malloc( (timesteps + 1) * sizeof(float) );
  if( *pulsevector == NULL )
    return -1;

  return 0;
}

#endif /* #ifndef _SEISMIC_H_ */
