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

#include <stdio.h>
#include <pthread.h>
#include "config.h"
#include "kernel.h"
#include "seismic.h"
#include "visualize.h"
#include "barrier/barrier.h"

int main( int argc, char * argv[] ) {

  config_t config;
  get_config( argc, argv, &config );
  print_config( &config );

  if(config.verbose)
    printf("allocate and initialize seismic data\n");
  float *APF, *VEL, *NPPF, *pulsevector;
  if( alloc_seismic_buffers( config.width, config.height, config.timesteps, config.variant.alignment, &VEL, &APF, &NPPF, &pulsevector ) ) {
    printf("allocation failure\n");
    exit(EXIT_FAILURE);
  }
  init_seismic_buffers( config.width, config.height, config.timesteps, VEL, APF, NPPF, pulsevector );


  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

  BARRIER_TYPE barrier;
  BARRIER_INIT( &barrier, config.threads );

  unsigned t_id = 0;
  unsigned width_part = (config.width - 4) / config.threads;
  if( config.variant.alignment )
    width_part += (config.variant.alignment / sizeof(float)) - (width_part % config.variant.alignment); // round up to next alignment

  stack_t * data = (stack_t*) malloc ( sizeof(stack_t) * config.threads );
  for( t_id = 0; t_id < config.threads; t_id++ ) {
    data[t_id].id = t_id;
    data[t_id].apf = APF;
    data[t_id].nppf = NPPF;
    data[t_id].vel = VEL;
    data[t_id].pulsevector = pulsevector;
    data[t_id].width = config.width;
    data[t_id].height = config.height;
    data[t_id].timesteps = config.timesteps;
    data[t_id].x_pulse = config.pulseX;
    data[t_id].y_pulse = config.pulseY;
    data[t_id].barrier = &barrier;

    data[t_id].x_start = 2 + t_id * width_part;
    if( t_id + 1 == config.threads ) 
      data[t_id].x_end = config.width - 2;
    else
      data[t_id].x_end = data[t_id].x_start + width_part;

    data[t_id].y_start = 2;
    data[t_id].y_end = config.height - 2;

    data[t_id].set_pulse = (data[t_id].x_start <= data[t_id].x_pulse && data[t_id].x_pulse < data[t_id].x_end);
  }

  void (* func)(void *) = config.variant.f_sequential;
  if( config.threads != 1 )
    func = config.variant.f_parallel;

  if( func == NULL ) {
    printf("no function ptr. found!\n");
    exit( EXIT_FAILURE );
  }


//  pthread_attr_t attr;
//  pthread_attr_init( &attr );
//  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );

  if(config.verbose)
    printf("processing...\n");

  pthread_t * threads = (pthread_t*) malloc ( sizeof(pthread_t) * (config.threads - 1) );

  unsigned i;
  for( i = 0; i < config.threads - 1; i++ ) {
    if( pthread_create( &threads[i], NULL, (void * (*)(void *))func, (void*) &data[i + 1] ) ) {
      printf("ERROR: Couldn't create thread %d of %d threads!!\nExiting...\n", i+1, config.threads);
      exit( EXIT_FAILURE );
    }

//  if( pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset) ) {
//    printf("WARNING: Couldn't pin thread %d to a single core! "
//           "Performance may suck...\n", i+1);
//  }
  }

  // execute code with this thread here ...
  func( &data[0] );

  for( i = 0; i < config.threads - 1; i++ ) {
    if( pthread_join( threads[i], NULL ) ) {
      printf("ERROR: Couldn't join thread %d of %d threads!!\nExiting...\n", i+1, config.threads );
      exit( EXIT_FAILURE );
    }
  }

  gettimeofday(&t2, NULL);

  if(config.verbose) {
    printf("\nend process!\n");
    fflush(stdout);

    double elapsedTimeOuter = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0; // ms
    double elapsedTimeInner = (data[0].e.tv_sec - data[0].s.tv_sec) * 1000.0 + (data[0].e.tv_usec - data[0].s.tv_usec) / 1000.0; // ms

    printf("\n");
    printf("(ID=0Z): OUTER  = %.2f ms (GFLOPS: %.2f)\n", elapsedTimeOuter, config.GFLOP/elapsedTimeOuter );
    printf("(ID=0Z): INNER  = %.2f ms (GFLOPS: %.2f)\n", elapsedTimeInner, config.GFLOP/elapsedTimeInner );
  }
  else
    printf("\n");

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
  free( data );
  free( threads );

  return 0;
}

