// Copyright 2017 - , Dr.-Ing. Patrick Siegl
// SPDX-License-Identifier: BSD-2-Clause

#ifndef _BARRIER_H_
#define _BARRIER_H_

#include "barrier_config.h"

#if defined(BUTTERFLY) || defined(DISSEMINATION)
#  include "b_butterfly.h"
#else
#  include "b_default.h"
#endif

#endif /* #ifndef _BARRIER_H_ */
