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
#include <stdlib.h>
#include <omp.h>
#include "config.h"
#include "seismic.h"
#include "pthreading.h"
#include "openmp.h"
#include "visualize.h"

int main( int argc, char * argv[] ) {
  
  config_t config;
  get_config( argc, argv, &config );
  print_config( &config );

  printf("allocate and initialize seismic data\n");
  float *APF, *VEL, *NPPF, *pulsevector;
  if( alloc_seismic_buffers( config.width, config.height, config.timesteps, config.variant.alignment, &VEL, &APF, &NPPF, &pulsevector ) ) {
    printf("allocation failure\n");
    exit(EXIT_FAILURE);
  }
  init_seismic_buffers( config.width, config.height, config.timesteps, VEL, APF, NPPF, pulsevector );

  struct timeval t1, t2;
  if( config.openmp )
    openmp_threading( &config, APF, VEL, NPPF, pulsevector, &t1, &t2 );
  else
    pthreading( &config, APF, VEL, NPPF, pulsevector, &t1, &t2 );

  double elapsedTimeOuter = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0; // ms
//  double elapsedTimeInner = (data[0].e.tv_sec - data[0].s.tv_sec) * 1000.0 + (data[0].e.tv_usec - data[0].s.tv_usec) / 1000.0; // ms

  printf("\n");
  printf("(ID=0Z): OUTER  = %.2f ms (GFLOPS: %.2f)\n", elapsedTimeOuter, config.GFLOP/elapsedTimeOuter );
//  printf("(ID=0Z): INNER  = %.2f ms (GFLOPS: %.2f)\n", elapsedTimeInner, config.GFLOP/elapsedTimeInner );

  if( config.ascii ) {
    show_ascii( &config, config.ascii, APF, NPPF );
  }
  if( config.output ) {
    write_matrice( &config, APF, NPPF );
  }

  // aligned version!
  unsigned alignment = config.variant.alignment ? (config.variant.alignment - 2 * sizeof(float)) : 0;
  free( ((void*)APF) - alignment );
  free( ((void*)NPPF) - alignment );
  free( ((void*)VEL) - alignment );

  free( pulsevector );

  return 0;
}

