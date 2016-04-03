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

#ifndef _B_DEFAULT_H_
#define _B_DEFAULT_H_

#include <pthread.h>

#define BARRIER_TYPE                      pthread_barrier_t
#define BARRIER_INIT( bar, c_num )        pthread_barrier_init( bar, NULL, c_num );
#define BARRIER( bar, c_id )              pthread_barrier_wait( bar );

#endif /* #ifndef _B_DEFAULT_H_ */
