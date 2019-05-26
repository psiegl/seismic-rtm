// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#if MACOS
#  include <sys/param.h>
#  include <sys/sysctl.h>
#endif
#include <sys/utsname.h>    /* for uname */
#include <unistd.h>         /* for getopt */
#include <getopt.h>         /* for getopt_long; standard getopt is in unistd.h */
#include <stdio.h>          /* for printf */
#include <stdlib.h>         /* for atoi */
#include <string.h>         /* for strcmp */
#include "config.h"
#include "check_hw.h"
#include "kernel.h"

#define elemsof( x )        (sizeof( (x) ) / sizeof( (x)[0] ))

#define VARTMPL( NAME, CAP, ALIGNMENT, VECTORWIDTH ) { #NAME, CAP, seismic_exec_##NAME, seismic_exec_##NAME##_pthread, ALIGNMENT, VECTORWIDTH }

variant_t variants[] = {
  VARTMPL( plain_naiiv,                  0,                            0,                 1 * sizeof(float) ),
  VARTMPL( plain_opt,                    0,                            0,                 1 * sizeof(float) ),

#ifdef __x86_64__
  VARTMPL( sse_std,                      HAS_SSE,                      4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( sse_unaligned,                HAS_SSE,                      0,                 4 * sizeof(float) ),
  VARTMPL( sse_aligned,                  HAS_SSE,                      4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( sse_aligned_not_grouped,      HAS_SSE,                      4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( sse_partial_aligned,          HAS_SSE,                      4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( sse_avx_partial_aligned,      HAS_SSE | HAS_AVX,            4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( sse_avx_partial_aligned_opt,  HAS_SSE | HAS_AVX,            4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( avx_unaligned,                HAS_AVX,                      0,                 8 * sizeof(float) ),
  VARTMPL( avx2_unaligned,               HAS_AVX | HAS_AVX2,           0,                 8 * sizeof(float) ),
  VARTMPL( sse_fma_std,                  HAS_SSE | HAS_FMA,            4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( sse_fma_unaligned,            HAS_SSE | HAS_FMA,            0,                 4 * sizeof(float) ),
  VARTMPL( sse_fma_aligned,              HAS_SSE | HAS_FMA,            4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( sse_fma_aligned_not_grouped,  HAS_SSE | HAS_FMA,            4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( sse_fma_partial_aligned,      HAS_SSE | HAS_FMA,            4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( sse_avx_fma_partial_aligned,  HAS_SSE | HAS_AVX | HAS_FMA,  4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( sse_avx_fma_partial_aligned_opt, HAS_SSE | HAS_AVX | HAS_FMA, 4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( avx_fma_unaligned,            HAS_AVX | HAS_FMA,            0,                 8 * sizeof(float) ),
  VARTMPL( avx2_fma_unaligned,           HAS_AVX | HAS_AVX2 | HAS_FMA, 0,                 8 * sizeof(float) ),
#endif /* #ifdef __x86_64__ */

#ifdef __ALTIVEC__
  VARTMPL( vmx,                          HAS_VMX,                      4 * sizeof(float), 4 * sizeof(float) ),
#ifdef __VSX__
  VARTMPL( vsx_aligned,                  HAS_VMX | HAS_VSX,            4 * sizeof(float), 4 * sizeof(float) ),
  VARTMPL( vsx_unaligned,                HAS_VMX | HAS_VSX,            0,                 4 * sizeof(float) ),
#endif /* #ifdef __VSX__ */
#endif /* #ifdef __ALTIVEC__ */

#if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP )
  VARTMPL( neon_aligned,                 HAS_NEON,                     4 * sizeof(float), 4 * sizeof(float) ),
#endif /* #if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP ) */
};

void default_values( config_t * config ) {
  config->width     = 2300;
  config->height    = 748;
  config->timesteps = 100;
  config->pulseY    = config->height / 2;
  config->pulseX    = config->width / 2;
  config->variant   = variants[0];
  config->threads   = 1;

  config->output    = 0;
  config->ascii     = 0; // will also be used for scale!
  config->verbose   = 1;
}

void print_usage( const char * argv0 ) {
  config_t c;
  default_values( &c );

  printf("\n"
         "usage: %s [options] \n"
         "\n"
         "  --width \t( -y )                    Default: %u\n"
         "  \t Define vertical matrix size.\n"
         "  --height \t( -x )                    Default: %u\n"
         "  \t Define horizontal matrix size.\n"
         "  --pulseY \t( -i )                    Default: %u\n"
         "  \t y coordinate of pulse offset.\n"
         "  --pulseX \t( -j )                    Default: %u\n"
         "  \t x coordinate of pulse offset.\n"
         "  --timesteps \t( -t )                    Default: %u\n"
         "  \t Determine number of timesteps.\n"
         "  --kernel \t( -k )                    Default: %s\n",
          argv0, c.height, c.width, c.pulseY, c.pulseX, c.timesteps, variants[0].type );

  uint32_t cap = check_hw_capabilites();
  unsigned i;
  for( i = 0; i < elemsof(variants); i++ )
    if( ! (variants[i].cap & ~cap) )
      printf("  \t %s\n", variants[i].type );

  printf("  --threads \t( -p )                    Default: %u\n"
         "  \t Number of threads.\n"
         "  --output \t( -o )                    Default: \"output.bin\"\n"
         "  \t Write output to file 'file'.\n"
         "  --ascii\t( -a ) <scale>            Default: %u\n"
         "  \t Print an ascii image.\n"
         "  \t Parameter will be used as scale.\n"
         "  --quite\t( -q)\n"
         "  \t Run without verbose output.\n"
         "  --help \t( -h )\n"
         "  \t Show this help page.\n", c.threads, c.ascii );
}

unsigned long round_and_get_unit( unsigned long mem, char * type ) {
  char types[] = { ' ', 'K', 'M', 'G', 'T' };

  unsigned i;
  for( i = 0; i < elemsof(types); i++ ) {
    if( mem < (1<<10) ) {
      *type = types[i];
      return mem;
    }
    mem >>= 10;
  }

  *type = 'P';
  return mem;
}

void get_config( int argc, char * argv[], config_t * config ) {
  default_values( config );

/*** Getopt ***/
  struct option long_options[] = {
//  name            has_arg             flag            var
    {"height",      required_argument,  NULL,           'x'},
    {"width",       required_argument,  NULL,           'y'},
    {"pulseY",      required_argument,  NULL,           'i'},
    {"pulseX",      required_argument,  NULL,           'j'},
    {"timesteps",   required_argument,  NULL,           't'},
    {"kernel",      required_argument,  NULL,           'k'},
    {"threads",     required_argument,  NULL,           'p'},
    {"output",      no_argument,        NULL,           'o'},
    {"ascii",       required_argument,  NULL,           'a'},
    {"help",        no_argument,        NULL,           'h'},
    {"quite",       no_argument,        NULL,           'q'},

    {NULL,          0,                  NULL,            0 }
  };

  uint32_t cap = check_hw_capabilites();
  while( 1 ) {
    int option_index = 0;
    int opt = getopt_long( argc, argv, "x:y:i:j:t:k:p:hq", long_options, &option_index );
    if( opt == -1 )
      break;

    switch (opt) {
      case 'y':
        config->width = atoi( optarg );
        break;

      case 'x':
        config->height = atoi( optarg );
        break;

      case 'i':
        config->pulseY = atoi( optarg );
        break;

      case 'j':
        config->pulseX = atoi( optarg );
        break;

      case 't':
        config->timesteps = atoi( optarg );
        break;

      case 'k':
        {
          unsigned i, found = 0;
          for( i = 0; i < sizeof(variants)/sizeof(variants[0]); i++ ) {
            if( ! strcmp( optarg, variants[i].type )
                && (cap & variants[i].cap) == variants[i].cap ) {
              config->variant = variants[i];
              found = 1;
              break;
            }
          }

          if( ! found ) {
            fprintf(stderr, "ERROR:\n\tNo supported version given! '%s' \n\n", optarg );
            print_usage( argv[0] );
            exit(EXIT_FAILURE);
          }
        }
        break;

      case 'p':
        config->threads = atoi( optarg );
        break;

      case 'o':
        config->output = 1;
        break;

      case 'a':
        config->ascii = atoi(optarg);
        break;

      case 'q':
        config->verbose = 0;
        break;

      case 'h':
        print_usage( argv[0] );
        exit(EXIT_SUCCESS);
        break;

      case '?':
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (optind != argc) {
    fprintf(stderr, "%s: unrecognized option '%s'.\n", argv[0], argv[optind]);
    fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if( config->height <= 4 || config->width <= 4 ) {
    fprintf(stderr, "ERROR: height and width need to be larger than 4\n");
    exit(EXIT_FAILURE);
  }

// validation checks!
  if( (config->variant.vectorwidth || config->threads)
      && ((config->height - 4) * sizeof(float)) % (config->variant.vectorwidth * config->threads) ) {
    fprintf(stderr, "ERROR: the height needs to be: (X * simd * threads) + 4!\n");
    exit(EXIT_FAILURE);
  }

  if( config->pulseX > config->width ) {
    fprintf(stderr, "ERROR: pulseX (%u) is larger then width (%u)!\n", config->pulseX, config->width);
    exit(EXIT_FAILURE);
  }
  
  if( config->pulseY > config->height ) {
    fprintf(stderr, "ERROR: pulseY (%u) is larger then height (%u)!\n", config->pulseY, config->height);
    exit(EXIT_FAILURE);
  }
  
  if( ! config->threads )
    config->threads = 1;

  config->GFLOP = (((double)(config->width - 4) * (double)(config->height - 4) * 15.0 + 1.0) * (double)config->timesteps)/1000000.0;
}

unsigned getNumCores( void ) {
#if MACOS
    unsigned nm[2];
    size_t len = 4;
    uint32_t count;

    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if(count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
        if(count < 1) { count = 1; }
    }
    return count;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

void print_config( config_t * config ) {

  if(!config->verbose)
    return;

  unsigned long mem = (unsigned long)config->height
                      * (unsigned long)(config->width + config->variant.alignment)
                      * sizeof(float) * 3 /* APF, NPPF, VEL */
                      + (config->timesteps /* +1? */) * sizeof(float) /* pulsevector */;
  char type;
  mem = round_and_get_unit( mem, &type );

  printf("-=-=-=-=-=-\n"
         "=== Running configuration:\n"
         "(rank0): res    = %ux%u\n"
         "(rank0): time   = %u\n"
         "(rank0): pulse  = %ux%u\n"
         "(rank0): kernel = %s\n"
         "(rank0): thrds  = %u\n"
         "(rank0): mem    = %ld %cB\n"
         "(rank0): GFLOP  = %.2f\n"
         "=== Running environment:\n",
         config->width, config->height,
         config->timesteps,
         config->pulseX, config->pulseY,
         config->variant.type,
         config->threads,
         mem, type, config->GFLOP );

  struct utsname myuts;
  if( ! uname( &myuts ) ) {
    printf("(rank0): HOST   = %s\n"
           "(rank0): MACH   = %s\n",
           myuts.nodename, myuts.machine );
  }

  printf( "(rank0): CORES  = %u", getNumCores() );

  uint32_t cap = check_hw_capabilites();
  if( cap ) {
    printf(" inc.");
    if( cap & HAS_SSE )
      printf(" SSE");
    if( cap & HAS_AVX ) {
      printf(" AVX");
      if( cap & HAS_AVX2 )
        printf(" AVX2");
    }
    if( cap & HAS_FMA )
      printf(" FMA");
    if( cap & HAS_VMX ) {
      printf(" VMX");
      if( cap & HAS_VSX )
        printf(" VSX");
    }
    if( cap & HAS_NEON )
      printf(" NEON");
    if( cap & HAS_ASIMD )
      printf(" ASIMD");
  }

  printf("\n\n");
}

