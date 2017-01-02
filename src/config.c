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


variant_t variants[] = {
  { "plain_naiiv",                  0,                            seismic_exec_plain_naiiv,                 seismic_exec_plain_naiiv_pthread,                 0,                 1 * sizeof(float) },
  { "plain_opt",                    0,                            seismic_exec_plain_opt,                   seismic_exec_plain_opt_pthread,                   0,                 1 * sizeof(float) },

#ifdef __x86_64__
  { "sse_std",                      HAS_SSE,                      seismic_exec_sse_std,                     seismic_exec_sse_std_pthread,                     4 * sizeof(float), 4 * sizeof(float) },
  { "sse_unaligned",                HAS_SSE,                      seismic_exec_sse_unaligned,               seismic_exec_sse_unaligned_pthread,               0,                 4 * sizeof(float) },
  { "sse_aligned",                  HAS_SSE,                      seismic_exec_sse_aligned,                 seismic_exec_sse_aligned_pthread,                 4 * sizeof(float), 4 * sizeof(float) },
  { "sse_aligned_not_grouped",      HAS_SSE,                      seismic_exec_sse_aligned_not_grouped,     seismic_exec_sse_aligned_not_grouped_pthread,     4 * sizeof(float), 4 * sizeof(float) },
  { "sse_partial_aligned",          HAS_SSE,                      seismic_exec_sse_partial_aligned,         seismic_exec_sse_partial_aligned_pthread,         4 * sizeof(float), 4 * sizeof(float) },
  { "sse_avx_partial_aligned",      HAS_SSE | HAS_AVX,            seismic_exec_sse_avx_partial_aligned,     seismic_exec_sse_avx_partial_aligned_pthread,     4 * sizeof(float), 4 * sizeof(float) },
  { "sse_avx_partial_aligned_opt",  HAS_SSE | HAS_AVX,            seismic_exec_sse_avx_partial_aligned_opt, seismic_exec_sse_avx_partial_aligned_opt_pthread, 4 * sizeof(float), 4 * sizeof(float) },
  { "avx_unaligned",                HAS_AVX,                      seismic_exec_avx_unaligned,               seismic_exec_avx_unaligned_pthread,               0,                 8 * sizeof(float) },
  { "avx2_unaligned",               HAS_AVX | HAS_AVX2,           seismic_exec_avx2_unaligned,              seismic_exec_avx2_unaligned_pthread,              0,                 8 * sizeof(float) },
  { "fma_sse_std",                  HAS_SSE | HAS_FMA,            seismic_exec_sse_fma_std,                 seismic_exec_sse_fma_std_pthread,                 4 * sizeof(float), 4 * sizeof(float) },
  { "fma_sse_unaligned",            HAS_SSE | HAS_FMA,            seismic_exec_sse_fma_unaligned,           seismic_exec_sse_fma_unaligned_pthread,           0,                 4 * sizeof(float) },
  { "fma_sse_aligned",              HAS_SSE | HAS_FMA,            seismic_exec_sse_fma_aligned,             seismic_exec_sse_fma_aligned_pthread,             4 * sizeof(float), 4 * sizeof(float) },
  { "fma_sse_aligned_not_grouped",  HAS_SSE | HAS_FMA,            seismic_exec_sse_fma_aligned_not_grouped, seismic_exec_sse_fma_aligned_not_grouped_pthread, 4 * sizeof(float), 4 * sizeof(float) },
  { "fma_sse_partial_aligned",      HAS_SSE | HAS_FMA,            seismic_exec_sse_fma_partial_aligned,     seismic_exec_sse_fma_partial_aligned_pthread,     4 * sizeof(float), 4 * sizeof(float) },
  { "fma_sse_avx_partial_aligned",  HAS_SSE | HAS_AVX | HAS_FMA,  seismic_exec_sse_avx_fma_partial_aligned, seismic_exec_sse_avx_fma_partial_aligned_pthread, 4 * sizeof(float), 4 * sizeof(float) },
  { "fma_sse_avx_partial_aligned_opt", HAS_SSE | HAS_AVX | HAS_FMA,  seismic_exec_sse_avx_fma_partial_aligned_opt, seismic_exec_sse_avx_fma_partial_aligned_opt_pthread, 4 * sizeof(float), 4 * sizeof(float) },
  { "fma_avx_unaligned",            HAS_AVX | HAS_FMA,            seismic_exec_avx_fma_unaligned,           seismic_exec_avx_fma_unaligned_pthread,           0,                 8 * sizeof(float) },
  { "fma_avx2_unaligned",           HAS_AVX | HAS_AVX2 | HAS_FMA, seismic_exec_avx2_fma_unaligned,          seismic_exec_avx2_fma_unaligned_pthread,          0,                 8 * sizeof(float) },
#endif /* #ifdef __x86_64__ */

#ifdef __ALTIVEC__
  { "vmx",                          HAS_VMX,                      seismic_exec_vmx_aligned,                 seismic_exec_vmx_aligned_pthread,                 4 * sizeof(float), 4 * sizeof(float) },
#ifdef __VSX__
  { "vsx_aligned",                  HAS_VMX | HAS_VSX,            seismic_exec_vsx_aligned,                 seismic_exec_vsx_aligned_pthread,                 4 * sizeof(float), 4 * sizeof(float) },
  { "vsx_unaligned",                HAS_VMX | HAS_VSX,            seismic_exec_vsx_unaligned,               seismic_exec_vsx_unaligned_pthread,               0,                 4 * sizeof(float) },
#endif /* #ifdef __VSX__ */
#endif /* #ifdef __ALTIVEC__ */

#if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP )
  { "neon_aligned",                 HAS_NEON,                     seismic_exec_arm_neon_aligned,            seismic_exec_arm_neon_aligned_pthread,            4 * sizeof(float), 4 * sizeof(float) },
#endif /* #if defined( __ARM_NEON ) || defined ( __ARM_NEON_FP ) */
};


unsigned int ggT(unsigned int a, unsigned int b){
  if( ! b )
    return a;
  else return ggT(b, a % b);
}

unsigned int kgV(unsigned int a, unsigned int b){
  return (a * b) / ggT(a, b);
}

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
}

void print_usage( const char * argv0 ) {
  config_t c;
  default_values( &c );

  printf("\n"
         "usage: %s [options] \n"
         "\n"
         "  --width \t( -y )                    Default: %d\n"
         "  \t Define vertical matrix size.\n"
         "  --height \t( -x )                    Default: %d\n"
         "  \t Define horizontal matrix size.\n"
         "  --pulseY \t( -i )                    Default: %d\n"
         "  \t y coordinate of pulse offset.\n"
         "  --pulseX \t( -j )                    Default: %d\n"
         "  \t x coordinate of pulse offset.\n"
         "  --timesteps \t( -t )                    Default: %d\n"
         "  \t Determine number of timesteps.\n"
         "  --kernel \t( -k )                    Default: %s\n",
          argv0, c.height, c.width, c.pulseY, c.pulseX, c.timesteps, variants[0].type );

  uint32_t cap = check_hw_capabilites();
  unsigned i;
  for( i = 0; i < elemsof(variants); i++ )
    if( ! (variants[i].cap & ~cap) )
      printf("  \t %s\n", variants[i].type );

  printf("  --threads \t( -p )                    Default: %d\n"
         "  \t Number of threads.\n"
         "  --output \t( -o )                    Default: \"output.bin\"\n"
         "  \t Write output to file 'file'.\n"
         "  --ascii\t( -a ) <scale>            Default: %d\n"
         "  \t Print an ascii image.\n"
         "  \t Parameter will be used as scale.\n"
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

    {NULL,          0,                  NULL,            0 }
  };

  uint32_t cap = check_hw_capabilites();
  while( 1 ) {
    int option_index = 0;
    int opt = getopt_long( argc, argv, "x:y:i:j:t:k:p:h", long_options, &option_index );
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
    fprintf(stderr, "ERROR: pulseX (%d) is larger then width (%d)!\n", config->pulseX, config->width);
    exit(EXIT_FAILURE);
  }
  
  if( config->pulseY > config->height ) {
    fprintf(stderr, "ERROR: pulseY (%d) is larger then height (%d)!\n", config->pulseY, config->height);
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

  unsigned long mem = config->height * (config->width + config->variant.alignment) * sizeof(float) * 3 /* APF, NPPF, VEL */
                      + (config->timesteps /* +1? */) * sizeof(float) /* pulsevector */;
  char type;
  mem = round_and_get_unit( mem, &type );

  printf("-=-=-=-=-=-\n"
         "=== Running configuration:\n"
         "(rank0): res    = %dx%d\n"
         "(rank0): time   = %d\n"
         "(rank0): pulse  = %dx%d\n"
         "(rank0): kernel = %s\n"
         "(rank0): thrds  = %d\n"
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

  printf( "(rank0): CORES  = %d", getNumCores() );

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
