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

#ifndef _BARRIER_H_
#define _BARRIER_H_

#include "barrier_config.h"

#if defined(BUTTERFLY) || defined(DISSEMINATION)
#  include "b_butterfly.h"
#else
#  include "b_default.h"
#endif

#endif /* #ifndef _BARRIER_H_ */
