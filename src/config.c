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

unsigned sym_kern_c = 0;
sym_kernel_t* sym_kern[32];

void default_values( config_t * config ) {
  config->width     = 2300;
  config->height    = 748;
  config->timesteps = 100;
  config->pulseY    = config->height / 2;
  config->pulseX    = config->width / 2;
  config->variant   = sym_kern[0];
  config->threads   = 1;

  config->output    = 0;
  config->ofile     = "output.bin";
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
          argv0, c.height, c.width, c.pulseY, c.pulseX, c.timesteps, sym_kern[0]->name );

  archfeatures cap = check_hw_capabilites();
  unsigned i;
  for( i = 0; i < sym_kern_c; i++ )
    if( ! (sym_kern[i]->cap.bits & ~cap.bits) )
      printf("  \t %s\n", sym_kern[i]->name );

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

  if( ! sym_kern_c ) {
    fprintf(stderr, "ERROR: no kernels compiled into!\n");
    exit(EXIT_FAILURE);
  }

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
    {"output",      optional_argument,  NULL,           'o'},
    {"ascii",       required_argument,  NULL,           'a'},
    {"help",        no_argument,        NULL,           'h'},
    {"quite",       no_argument,        NULL,           'q'},

    {NULL,          0,                  NULL,            0 }
  };

  archfeatures cap = check_hw_capabilites();
  while( 1 ) {
    int option_index = 0;
    int opt = getopt_long( argc, argv, "x:y:i:j:t:k:p:o::a:hq", long_options, &option_index );
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
          for( i = 0; i < sym_kern_c; i++ ) {
            if( ! strcmp( optarg, sym_kern[i]->name )
                && (cap.bits & sym_kern[i]->cap.bits) == sym_kern[i]->cap.bits ) {
              config->variant = sym_kern[i];
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
        if (optarg)
          config->ofile = optarg;
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
  if( (config->variant->vectorwidth || config->threads)
      && ((config->height - 4) * sizeof(float)) % (config->variant->vectorwidth * config->threads) ) {
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

void print_config( config_t * config ) {

  if(!config->verbose)
    return;

  unsigned long mem = (unsigned long)config->height
                      * (unsigned long)(config->width + config->variant->alignment)
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
         config->variant->name,
         config->threads,
         mem, type, config->GFLOP );

  struct utsname myuts;
  if( ! uname( &myuts ) ) {
    printf("(rank0): HOST   = %s\n"
           "(rank0): MACH   = %s\n",
           myuts.nodename, myuts.machine );
  }

  printf( "(rank0): CORES  = %u\n", get_num_cores() );

  archfeatures cap = check_hw_capabilites();
  printf( "(rank0): FLAGS  =" );
  print_hw_capabilites( cap );

  printf("\n\n");
}

