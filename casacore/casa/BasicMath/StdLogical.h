//# Copyright (C) 1995,1996,1999-2001
//# Associated Universities, Inc. Washington DC, USA.
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


#ifndef STDLOGICAL_H_
#define STDLOGICAL_H_

#include <casa/aips.h>

namespace casa {

// define logical operations on STL containers

template<class T, class U> Bool allNearAbs (const T &l, const T &r, const U tolerance);



} // end namespace casa

#ifndef CASACORE_NO_AUTO_TEMPLATES
#include <casa/BasicMath/StdLogical.tcc>
#endif //# CASACORE_NO_AUTO_TEMPLATES



#endif /* STDLOGICAL_H_ */
