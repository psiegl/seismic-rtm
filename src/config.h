// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "kernel.h"

typedef struct _config_t config_t;
struct _config_t {
  unsigned width; // num. of floats
  unsigned height; // num. of floats
  unsigned timesteps;
  unsigned pulseY;
  unsigned pulseX;

  sym_kernel_t* variant;

  unsigned threads;

  unsigned output;
  const char *ofile;
  unsigned ascii;
  unsigned verbose;

  double GFLOP;
};

void get_config( int argc, char * argv[], config_t * config );
void print_config( config_t * config );

#endif /* #ifndef _CONFIG_H_ */
