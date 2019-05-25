// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#ifndef _VISUALIZE_H_
#define _VISUALIZE_H_

#include "config.h"

void write_matrice( config_t * config, float * apf, float * nppf  );
void show_ascii( config_t * config, unsigned scale, float * apf, float * nppf  );

#endif /* #ifndef _VISUALIZE_H_ */
