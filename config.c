//  This file is part of seismic-rtm.
//
//  seismic-rtm is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  seismic is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seismic.  If not, see <http://www.gnu.org/licenses/>.

#include <sys/utsname.h>    /* for uname */
#include <unistd.h>         /* for getopt */
#include <getopt.h>         /* for getopt_long; standard getopt is in unistd.h */
#include <stdio.h>          /* for printf */
#include <stdlib.h>         /* for atoi */
#include <string.h>         /* for strcmp */
#include "config.h"
#include "check_hw.h"

unsigned int ggT(unsigned int a, unsigned int b){
  if(b == 0)
    return a;
  else return ggT(b, a % b);
}

unsigned int kgV(unsigned int a, unsigned int b){
  return (a * b) / ggT(a, b);
}

void default_values( config_t * config ) {
  config->height    = 2300;
  config->width     = 748;
  config->timesteps = 100;
  config->pulseY    = config->height / 2;
  config->pulseX    = config->width / 2;
  config->kernel    = KERNEL__PLAIN_C;
  config->alignment = 0;
  config->vectorwidth = 1 * sizeof(float);
  config->threads   = 1;

  config->output    = 0;
  config->ascii     = 0; // will also be used for scale!
}

void print_usage( const char * argv0 ) {
  config_t c;
  default_values( &c );

  printf("\n"
         "usage: %s [options] \n", argv0 );
  printf("\n"
         "  --height \t( -x )\n"
         "  \t Define horizontal matrix size.   Default: %d\n", c.height );
  printf("\n"
         "  --width \t( -y )\n"
         "  \t Define vertical matrix size.     Default: %d\n", c.width );
  printf("\n"
         "  --pulseY \t( -i )\n"
         "  \t y coordinate of pulse offset.    Default: %d\n", c.pulseY );
  printf("\n"
         "  --pulseX \t( -j )\n"
         "  \t x coordinate of pulse offset.    Default: %d\n", c.pulseX );
  printf("\n"
         "  --timesteps \t( -t )\n"
         "  \t Determine number of timesteps.   Default: %d\n", c.timesteps );
  printf("\n"
         "  --kernel \t( -k )\n"
         "  \t plain_c,                         Default: plain_c\n");

  uint32_t cap = check_hw_capabilites();
  if( cap & HAS_SSE )
    printf("  \t sse_std,\n"
         "  \t sse_unaligned,\n"
         "  \t sse_aligned,\n"
         "  \t sse_aligned_not_grouped\n" );
  if( cap & HAS_AVX )
    printf("  \t avx_unaligned,\n"
           "  \t avx2_unaligned\n");

  printf("\n"
         "  --threads \t( -p )\n"
         "  \t Number of threads.               Default: %d\n", c.threads);
  printf("\n"
         "  --output \t( -o )\n"
         "  \t Write output to file 'file'.     Default: \"output.bin\"\n"
         "\n"
         "  --ascii\t( -a ) <scale>\n"
         "  \t Print an ascii image.            Default: %d\n", c.ascii);
  printf("  \t Parameter will be used as scale.\n"
         "\n"
         "  --help \t( -h )\n"
         "  \t Show this help page.\n"
         "\n");
}

unsigned long round_and_get_unit( unsigned long mem, const char ** type ) {
  const char * types[] = { "B", "KB", "MB", "GB", "TB", "PB" };

  unsigned i;
  for( i = 0; i < sizeof(types)/sizeof(types[0]); i++ ) {
    if( mem < 1024 ) {
      *type = types[i];
      return mem;
    }
    mem /= 1024;
  }
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

    {NULL,          0,                  NULL,            0 }
  };

  uint32_t cap = check_hw_capabilites();
  while( 1 ) {
    int option_index = 0;
    int opt = getopt_long( argc, argv, "x:y:i:j:t:k:p:h", long_options, &option_index );
    if( opt == -1 )
      break;

    switch (opt) {
      case 'x':
	      config->height = atoi( optarg );
        break;

      case 'y':
	      config->width = atoi( optarg );
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
        if( ! strcmp( optarg, "sse_std" ) && (cap & HAS_SSE) ) {
          config->kernel = KERNEL__SIMD_SSE_STD;
          config->alignment = 4 * sizeof(float);
          config->vectorwidth = 4 * sizeof(float);
        }
        else if( ! strcmp( optarg, "sse_unaligned" ) && (cap & HAS_SSE) ) {
          config->kernel = KERNEL__SIMD_SSE_UNALIGNED;
          config->alignment = 0;
          config->vectorwidth = 4 * sizeof(float);
        }
        else if( ! strcmp( optarg, "sse_aligned" ) && (cap & HAS_SSE) ) {
          config->kernel = KERNEL__SIMD_SSE_ALIGNED;
          config->alignment = 4 * sizeof(float);
          config->vectorwidth = 4 * sizeof(float);
        }
        else if( ! strcmp( optarg, "sse_aligned_not_grouped" ) && (cap & HAS_SSE) ) {
          config->kernel = KERNEL__SIMD_SSE_ALIGNED;
          config->alignment = 4 * sizeof(float);
          config->vectorwidth = 4 * sizeof(float);
        }
        else if( ! strcmp( optarg, "avx_unaligned" ) && (cap & HAS_AVX) ) {
          config->kernel = KERNEL__SIMD_AVX_UNALIGNED;
          config->alignment = 0;
          config->vectorwidth = 8 * sizeof(float);
        }
        else if( ! strcmp( optarg, "avx2_unaligned" ) && (cap & HAS_AVX) ) {
          config->kernel = KERNEL__SIMD_AVX2_UNALIGNED;
          config->alignment = 0;
          config->vectorwidth = 8 * sizeof(float);
        }
        else if( ! strcmp( optarg, "plain_c" ) ) {
        }
        else {
          printf("\n\tNo supported version given! '%s' \n\n", optarg );
          print_usage( argv[0] );
          exit(EXIT_FAILURE);
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

      case 'h':
        print_usage( argv[0] );
        exit(EXIT_SUCCESS);
        break;

      case '?':
        printf("Try '%s --help' for more information.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (optind != argc) {
    printf("%s: unrecognized option '%s'.\n", argv[0], argv[optind]);
    printf("Try '%s --help' for more information.\n", argv[0]);
    exit(EXIT_FAILURE);
  }

// validation checks!
  if( (config->vectorwidth || config->threads) &&
      ((config->height - 4) * sizeof(float)) % (config->vectorwidth * config->threads) ) {
    printf("The height needs to be: (X * simd * threads) + 4!\n");
    exit(EXIT_FAILURE);
  }

  if( config->pulseX > config->width )
    config->pulseX = config->width / 2;
  if( config->pulseY > config->height )
    config->pulseY = config->height / 2;
  if( config->threads < 1 )
    config->threads = 1;
}

void print_config( config_t * config ) {
  struct { kernel_t k; const char * n; } kernels[] = {
    { KERNEL__PLAIN_C, "plain_c" },
    { KERNEL__SIMD_SSE_STD, "sse_std" },
    { KERNEL__SIMD_SSE_UNALIGNED, "sse_unaligned" },
    { KERNEL__SIMD_SSE_ALIGNED, "sse_aligned" },
    { KERNEL__SIMD_SSE_ALIGNED_NOT_GROUPED, "sse_aligned_not_grouped" },
    { KERNEL__SIMD_AVX_UNALIGNED, "avx_unaligned" },
    { KERNEL__SIMD_AVX2_UNALIGNED, "avx2_unaligned" }
  };

  unsigned i;
  const char * name = "?!?";
  for( i = 0; i < sizeof(kernels)/sizeof(kernels[0]); i++ ) {
    if( kernels[i].k == config->kernel ) {
      name = kernels[i].n;
      break;
    }
  }
    
  printf("-=-=-=-=-=-\n"
         "=== Running configuration:\n"
         "(ID=0Z): res    = %dx%d\n"
         "(ID=0Z): time   = %d\n"
         "(ID=0Z): pulse  = %d,%d\n"
         "(ID=0Z): kernel = %s\n"
         "(ID=0Z): thrds  = %d\n",
         config->width, config->height,
         config->timesteps,
         config->pulseX, config->pulseY,
         name,
         config->threads );

  unsigned long mem = config->height * (config->width + config->alignment) * sizeof(float) * 3 /* APF, NPPF, VEL */
                      + config->timesteps * sizeof(float) /* pulsevector */;
  const char * type;
  mem = round_and_get_unit( mem, &type );
  printf("(ID=0Z): mem    = %ld %s\n", mem, type );

  printf("=== Running environment:\n");

  struct utsname myuts;
  if( ! uname( &myuts ) ) {
    printf("(ID=0Z): HOST   = %s\n"
           "(ID=0Z): MACH   = %s\n",
           myuts.nodename, myuts.machine );
  }

  unsigned char buffer[10 + 1];
  FILE * pipe = popen("grep -ci 'processor' /proc/cpuinfo", "r");
  fgets(buffer, 10, pipe);
  pclose(pipe);

  printf( "(ID=0Z): CORES  = %d", atoi(buffer) );

  uint32_t cap = check_hw_capabilites();
  if( cap & (HAS_SSE | HAS_AVX) )
    printf(" inc.");
  if( cap & HAS_SSE )
    printf(" SSE");
  if( cap & HAS_AVX )
    printf(" AVX");
  // AVX2?

  printf("\n\n");
}

