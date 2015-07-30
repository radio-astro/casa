//# --------------------------------------------------------------------
//# LineFinder.h: this defines utility functions of line finding
//# --------------------------------------------------------------------
//# Copyright (C) 2015
//# National Astronomical Observatory of Japan
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
#ifndef _CASA_LINEFINDER_H_
#define _CASA_LINEFINDER_H_

#include <list>

#include <casa/aipstype.h>
#include <casa/Arrays/Vector.h>

#include <casa_sakura/SakuraAlignedArray.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  namespace linefinder {
    std::list<std::pair<size_t,size_t>> MADLineFinder(size_t const num_data, SakuraAlignedArray<float> const& data, SakuraAlignedArray<bool> const& mask, float const threshold, uint8_t max_iteration, size_t const minwidth, size_t const maxwidth, size_t const avg_limit, std::pair<size_t,size_t> edge);
    size_t countTrue(size_t num_data, bool data[/*num_data*/]);
float masked_median(size_t num_data, float const* data,
		    SakuraAlignedArray<bool> const& mask, float fraction=1.0);

  }
} //# NAMESPACE CASA - END
  
#endif /* _CASA_LINEFINDER_H_ */
