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
  *VEL = (float*)malloc_aligned( size_matrice, alignment );
  *NPPF = (float*)malloc_aligned( size_matrice, alignment );
  *pulsevector = (float*)malloc( (timesteps + 1) * sizeof(float) );
  return ( *APF == NULL
           || *VEL == NULL
           || *NPPF == NULL
           || *pulsevector == NULL ) * -1;
  return 0;
}

#endif /* #ifndef _SEISMIC_H_ */
