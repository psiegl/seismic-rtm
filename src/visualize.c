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
