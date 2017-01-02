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
