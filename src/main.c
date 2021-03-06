// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Dr.-Ing. Patrick Siegl <patrick@siegl.it>

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <string.h>
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
  if( alloc_seismic_buffers( config.width, config.height, config.timesteps, config.variant->alignment, &VEL, &APF, &NPPF, &pulsevector ) ) {
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
  if( config.variant->alignment )
    width_part += (config.variant->alignment / sizeof(float)) - (width_part % config.variant->alignment); // round up to next alignment

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
    data[t_id].y_offset = (!config.variant->alignment) ? 1 : (config.variant->alignment / sizeof(float));

    data[t_id].x_start = 2 + t_id * width_part;
    if( t_id + 1 == config.threads ) 
      data[t_id].x_end = config.width - 2;
    else
      data[t_id].x_end = data[t_id].x_start + width_part;

    data[t_id].y_start = 2;
    data[t_id].y_end = config.height - 2;

    data[t_id].set_pulse = (data[t_id].x_start <= data[t_id].x_pulse && data[t_id].x_pulse < data[t_id].x_end);
    data[t_id].clopt = config.clopt;

    // Cacheline optimized
    if( config.clopt
        && ( ! strcmp( "plain_naiiv", config.variant->name )
             || ! strcmp( "plain_opt", config.variant->name )
          /* || ! strcmp( "sse_std", config.variant->name ) */ ) ) {
      data[t_id].x_start = 2;
      data[t_id].x_end = config.width - 2;
      data[t_id].y_start = 2 + t_id * ( config.variant->alignment ? (config.variant->alignment / sizeof(float)) : 1 );
      data[t_id].y_end = config.height - 2;
      data[t_id].y_offset *= config.threads;
    }
  }

  void (* func)(void *) = config.variant->fnc_sgl;
  if( config.threads != 1 )
    func = config.variant->fnc_par;

  if( func == NULL ) {
    printf("no function ptr. found!\n");
    exit( EXIT_FAILURE );
  }

  unsigned cores = get_num_cores();
  if(config.verbose
     && config.threads > cores)
    printf("WARNING: amount of chosen threads is higher\n"
           "         then cores available (%u vs %u).\n"
           "         performance may suffer.\n", config.threads, cores);

  if(config.verbose)
    printf("processing...\n");

  pthread_attr_t attr;
  pthread_attr_init( &attr );
  if( pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE ) ) {
    printf("WARNING: could not set PTHREAD_CREATE_JOINABLE");
  }

  if( pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) ) {
    printf("WARNING: could not set PTHREAD_EXPLICIT_SCHED");
  }

  pthread_t * threads = (pthread_t*) malloc ( sizeof(pthread_t) * (config.threads - 1) );
  cpu_set_t cpuset;
  unsigned i;
  for( i = 0; i < config.threads - 1; i++ ) {
    CPU_ZERO(&cpuset); // first zero
    CPU_SET((i+1) % cores, &cpuset); // set only the specific one

    if( pthread_create( &threads[i], &attr, (void * (*)(void *))func, (void*) &data[i + 1] ) ) {
      printf("ERROR: Couldn't create thread %u of %u threads!!\nExiting...\n", i+1, config.threads);
      exit( EXIT_FAILURE );
    }

    if( pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset)
        && config.verbose ) {
      printf("WARNING: Couldn't pin thread %u to a single core! "
             "Performance may suck...\n", i+1);
    }
  }

  // execute code with this thread here ...
  CPU_ZERO(&cpuset); // first zero
  CPU_SET(0, &cpuset); // set only the specific one
  if( pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset)
      && config.verbose ) {
    printf("WARNING: Couldn't pin thread %u to a single core! "
           "Performance may suck...\n", i+1);
  }
  func( &data[0] );

  for( i = 0; i < config.threads - 1; i++ ) {
    if( pthread_join( threads[i], NULL ) ) {
      printf("ERROR: Couldn't join thread %u of %u threads!!\nExiting...\n", i+1, config.threads );
      exit( EXIT_FAILURE );
    }
  }

  gettimeofday(&t2, NULL);

  pthread_attr_destroy( &attr );

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
  unsigned alignment = config.variant->alignment ? (config.variant->alignment - 2 * sizeof(float)) : 0;
  free( ((char*)APF) - alignment );
  free( ((char*)NPPF) - alignment );
  free( ((char*)VEL) - alignment );

  free( pulsevector );
  free( data );
  free( threads );

  return 0;
}

