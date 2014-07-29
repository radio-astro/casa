//# VBRemapper.h: Base classes for objects that process VisBuffGroups
//# as fed to them by GroupProcessor.
//# Copyright (C) 2011
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
//#

#ifndef MSVIS_VBREMAPPER_H
#define MSVIS_VBREMAPPER_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <synthesis/MSVis/VisBufferComponents.h>
#include <map>

namespace casa { //# NAMESPACE CASA - BEGIN

class VisBuffer;

//<summary>VBRemappers remap VisBuffers</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisBuffer">VisBuffer</linkto>
//   <li> <linkto class="SubMS">SubMS</linkto>
// </prerequisite>
//
// <etymology>
// VBRemappers remap VisBuffers.
// </etymology>
//
// <synopsis>
// When selected parts of an MS are written to a new MS, it may be necessary to
// remap metadata indices like Field ID, DDID, ObservationID, etc..
// </synopsis>
//
// <example>
// // DDIDs 1, 4, and 7, and field ID 3 is selected.  They should become 0, 1,
// // and 2, and 0, in the output.
// </example>
//<todo>
// <li> 
//</todo>
class VBRemapper
{
public:
  // Defaults to no remapping.
  VBRemapper();

  VBRemapper(const std::map<VisBufferComponents::EnumType,
             std::map<Int, Int> >& inToOutMaps);

  //// Copy construct
  //VBRemapper(const VBRemapper& gw) {}

  // Destructor
  virtual ~VBRemapper() {}

  //// Assignment
  //virtual VBRemapper& operator=(const VBRemapper& gw) {}

  const std::map<VisBufferComponents::EnumType, std::map<Int, Int> >& getMaps() const
  {
    return inToOutMaps_p;
  }

  void setMaps(const std::map<VisBufferComponents::EnumType, std::map<Int, Int> >& maps)
  {
    inToOutMaps_p = maps;
  }

  // Remaps vb and returns its success value.
  Bool remap(VisBuffer& vb, const Bool squawk=True) const;

protected:
  // Remaps col using mapper.
  // Returns False if col has a value that is not in mapper's keys (those
  // values are left unchanged), True otherwise.
  Bool remapScalar(Int& colref, const std::map<Int, Int>& mapper) const;
  Bool remapVector(Vector<Int>& col, const std::map<Int, Int>& mapper) const;

  // Set of maps from input ID to output ID, keyed by VisBufferComponent.
  std::map<VisBufferComponents::EnumType, std::map<Int, Int> > inToOutMaps_p;
};

} //# NAMESPACE CASA - END

#endif

