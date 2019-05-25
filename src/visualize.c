// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#include "visualize.h"
#include <stdlib.h>
#include <stdio.h>

void write_matrice( config_t * config, float * apf, float * nppf ) {
  float * matrice;
  if( config->timesteps & 0x1 )
    matrice = nppf;
  else
    matrice = apf;

  FILE * f1 = fopen( "output.bin", "wb" );
  if( f1 == NULL )
    exit(EXIT_FAILURE);

  fwrite( matrice, sizeof(float), config->height * config->width, f1 );
  fclose(f1);
}

void show_ascii( config_t * config, unsigned scale, float * apf, float * nppf  ) {
  float * matrice;
  if( config->timesteps & 0x1 )
    matrice = nppf;
  else
    matrice = apf;

  unsigned i, j;
  for( j = 0; j < config->height; j+=scale ) {
    for( i = 0; i < config->width; i+=scale ) {
      unsigned offset = i * config->height + j;
      if( matrice[ offset ] == 0.0f )
        printf("0");
      else if( matrice[ offset ] > 0.0f )
        printf("+");
      else
        printf("-");
    }
    printf("\n");
  }
  printf("\n");
}
