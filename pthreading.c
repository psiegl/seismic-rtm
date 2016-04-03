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
#include <pthread.h>
#include <sys/time.h>
#include "pthreading.h"
#include "kernel.h"
#include "barrier/barrier.h"

void pthreading( config_t *config, float *APF, float *VEL, float *NPPF, float *pulsevector, struct timeval * t1, struct timeval * t2 ) {
  gettimeofday( t1, NULL);

  BARRIER_TYPE barrier;
  BARRIER_INIT( &barrier, config->threads );

  unsigned t_id = 0;
  unsigned width_part = (config->width - 4) / config->threads;
  if( config->variant.alignment )
    width_part += (config->variant.alignment / sizeof(float)) - (width_part % config->variant.alignment); // round up to next alignment

  stack_t * data = (stack_t*) malloc ( sizeof(stack_t) * config->threads );
  for( t_id = 0; t_id < config->threads; t_id++ ) {
    data[t_id].id = t_id;
    data[t_id].apf = APF;
    data[t_id].nppf = NPPF;
    data[t_id].vel = VEL;
    data[t_id].pulsevector = pulsevector;
    data[t_id].width = config->width;
    data[t_id].height = config->height;
    data[t_id].timesteps = config->timesteps;
    data[t_id].x_pulse = config->pulseX;
    data[t_id].y_pulse = config->pulseY;
    data[t_id].barrier = &barrier;

    data[t_id].x_start = 2 + t_id * width_part;
    if( t_id + 1 == config->threads ) 
      data[t_id].x_end = config->width - 2;
    else
      data[t_id].x_end = data[t_id].x_start + width_part;

    data[t_id].y_start = 2;
    data[t_id].y_end = config->height - 2;

    data[t_id].set_pulse = (data[t_id].x_start <= data[t_id].x_pulse && data[t_id].x_pulse < data[t_id].x_end);
  }

  void (* func)(void *) = config->variant.f_sequential;
  if( config->threads != 1 )
    func = config->variant.f_parallel;

  if( func == NULL ) {
    printf("no function ptr. found!\n");
    exit( EXIT_FAILURE );
  }


//  pthread_attr_t attr;
//  pthread_attr_init( &attr );
//  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );

  printf("processing...\n");

  pthread_t * threads = (pthread_t*) malloc ( sizeof(pthread_t) * (config->threads - 1) );

  unsigned i;
  for( i = 0; i < config->threads - 1; i++ ) {
    if( pthread_create( &threads[i], NULL, (void * (*)(void *))func, (void*) &data[i + 1] ) ) {
      printf("ERROR: Couldn't create thread %d of %d threads!!\nExiting...\n", i+1, config->threads);
      exit( EXIT_FAILURE );
    }

//  if( pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset) ) {
//    printf("WARNING: Couldn't pin thread %d to a single core! "
//           "Performance may suck...\n", i+1);
//  }
  }

  // execute code with this thread here ...
  func( &data[0] );

  for( i = 0; i < config->threads - 1; i++ ) {
    if( pthread_join( threads[i], NULL ) ) {
      printf("ERROR: Couldn't join thread %d of %d threads!!\nExiting...\n", i+1, config->threads );
      exit( EXIT_FAILURE );
    }
  }

  gettimeofday( t2, NULL);

  printf("\nend process!\n");
  fflush(stdout);

  free( data );
  free( threads );
}
