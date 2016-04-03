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

#ifndef _PTHREADING_H_
#define _PTHREADING_H_

#include <stdlib.h>
#include "config.h"

void pthreading( config_t * config, float *APF, float *VEL, float *NPPF, float *pulsevector, struct timeval * t1, struct timeval * t2 );

#endif /* #ifndef _PTHREADING_H_ */
