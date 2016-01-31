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

#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct _variant_t variant_t;
struct _variant_t {
  const char * type;
  unsigned cap;
  void (* f_sequential)(void *);
  void (* f_parallel)(void *);
  unsigned alignment;
  unsigned vectorwidth;
};

typedef struct _config_t config_t;
struct _config_t {
  unsigned width; // num. of floats
  unsigned height; // num. of floats
  unsigned timesteps;
  unsigned pulseY;
  unsigned pulseX;

  variant_t variant;

  unsigned threads;

  unsigned output;
  unsigned ascii;

  double GFLOP;
};

void get_config( int argc, char * argv[], config_t * config );
void print_config( config_t * config );

#endif /* #ifndef _CONFIG_H_ */
