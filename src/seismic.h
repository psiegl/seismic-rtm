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

#define FC          125.0f
#define ALPHA         9.0f
#define BETA          5.0f

// http://subsurfwiki.org/wiki/Ricker_wavelet
// http://wiki.seg.org/wiki/Dictionary:Ricker_wavelet
float ricker_wavelet(float t,float fc) {
  float x   = M_PI*fc*t;
  float aux = x*x;
  return (1.0 - 2.0*aux)*exp((double) -aux);
}

#define init_seismic_pulsevector( pulsevector, timesteps, dt, tf, fpeak ) \
  { \
    unsigned i; \
    for( i = 0; i < timesteps; i++ ) { \
      float time = ( i * dt ) - tf; \
      pulsevector[ i ] = ricker_wavelet( time, fpeak ); \
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
    float vmin = sqrt(.5) * 40.0; \
    float vmax = BETA * ALPHA; \
    \
    float h = 2.0 / ( FC ); \
    float fat = 1.0f / ( BETA * BETA * 12.0f ); \
    float tf = ( 2.0f * sqrt( M_PI ) ) / FC; \
    \
    /* determine time sampling interval to ensure stability */ \
    float dt = h / (2.0 * vmax); \
    \
    /* determine maximum temporal frequency to avoid dispersion */ \
    float fmax = vmin/(10.0*h); \
    \
    /* compute or set peak frequency for ricker wavelet */ \
    float fpeak = 0.5 * fmax;  \
    \
    init_seismic_pulsevector( (pulsevector), (timesteps), dt, tf, fpeak ); \
    init_seismic_matrices( (width), (height), (VEL), (APF), (NPPF), fat ); \
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
