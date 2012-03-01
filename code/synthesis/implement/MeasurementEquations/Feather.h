//# Feather.h: Helper for feathering
//# Copyright (C) 1996-2012
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$
#ifndef SYNTHESIS_FEATHER_H
#define SYNTHESIS_FEATHER_H

#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  //Forward declaration
  template<class T> class ImageInterface;
  // <summary> Class that contains functions needed for feathering</summary>

  class Feather{

  public:
    static void feather(const String& image, const ImageInterface<Float>& high, const ImageInterface<Float>& low, const Float& sdScale=1.0, const String& lowPSF="", const Bool useDefault=True, const String& vpTable="" , const Bool doPlot=False);
    






  };



} //# NAMESPACE CASA - END

#endif
