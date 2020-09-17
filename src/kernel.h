// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Dr.-Ing. Patrick Siegl <patrick@siegl.it>

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "barrier/barrier.h"
#include "check_hw.h"

typedef struct _stack_t stack_t;
struct _stack_t {
  unsigned id;
  BARRIER_TYPE* barrier;

  float* apf;
  float* nppf;
  float* vel;
  float* pulsevector;

  unsigned width;
  unsigned x_start;
  unsigned x_end;

  unsigned height;
  unsigned y_start;
  unsigned y_end;
  unsigned y_offset;
  unsigned timesteps;

  unsigned x_pulse;
  unsigned y_pulse;

  unsigned set_pulse;
  unsigned clopt;

  struct timeval s;
  struct timeval e;
};

#define SYM_KERNEL( NAME, CAP, ALIGNMENT, VECTORWIDTH ) \
sym_kernel_t sym_##NAME = { \
  .name = #NAME, \
  .cap.in = CAP, \
  .fnc_sgl = seismic_exec_##NAME, \
  .fnc_par = seismic_exec_##NAME##_pthread, \
  .alignment = ALIGNMENT, \
  .vectorwidth = VECTORWIDTH \
}; \
extern unsigned sym_kern_c; \
extern sym_kernel_t* sym_kern[]; \
__attribute__((constructor)) void sym_##NAME##_init(void) { \
  sym_kern[ sym_kern_c++ ] = &sym_##NAME; \
}

typedef struct _sym_kernel_t sym_kernel_t;
struct _sym_kernel_t {
  const char *name;
  archfeatures cap;
  void (*fnc_sgl)( void * v );
  void (*fnc_par)( void * v );
  unsigned int alignment;
  unsigned int vectorwidth;
};

#endif /* #ifndef _KERNEL_H_ */
