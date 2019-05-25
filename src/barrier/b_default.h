// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#ifndef _B_DEFAULT_H_
#define _B_DEFAULT_H_

#include <pthread.h>

#define BARRIER_TYPE                      pthread_barrier_t
#define BARRIER_INIT( bar, c_num )        pthread_barrier_init( bar, NULL, c_num );
#define BARRIER( bar, c_id )              pthread_barrier_wait( bar );

#endif /* #ifndef _B_DEFAULT_H_ */
