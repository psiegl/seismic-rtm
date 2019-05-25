// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

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
  unsigned verbose;

  double GFLOP;
};

void get_config( int argc, char * argv[], config_t * config );
void print_config( config_t * config );

#endif /* #ifndef _CONFIG_H_ */
