//# AsdmIndex.h: Index for the ASDM Storage Manager
//# Copyright (C) 2012
//# Associated Universities, Inc. Washington DC, USA.
//# (c) European Southern Observatory, 2012
//# Copyright by ESO (in the framework of the ALMA collaboration)
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
//# You should have receied a copy of the GNU Library General Public License
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
//# $Id: AsdmColumn.h 19324 2011-11-21 07:29:55Z diepen $

#ifndef ASDM_ASDMINDEX_H
#define ASDM_ASDMINDEX_H


//# Includes
#include <casa/aips.h>
#include <casa/vector.h>
#include <ostream>

namespace casa {

//# Forward declaration.
class AipsIO;


// <summary>
// An index entry for the ASDM Storage Manager.
// </summary>

// <use visibility=local>

// <reviewed reviewer="UNKNOWN" date="before2004/08/25" tests="tAsdmStMan.cc">
// </reviewed>

// <prerequisite>
//# Classes you should understand before using this one.
//   <li> <linkto class=AsdmStMan>AsdmStMan</linkto>
// </prerequisite>

// <synopsis>
// ASDM stores all data of a time slot in a data array for the cross-correlation
// data and a data array for the auto-correlation data.
// An object of this class defines the shapes and locations of the data for
// each time slot.
// </synopsis>

struct AsdmIndex
{
  // Get nr of complex values in cross-correlation data block.
  uInt dataSize() const;

  // Get number of rows spanned by the data blocks.
  uInt nrow() const
  //{ return nBl*nSpw; } 
  { return nBl; } 

  //# Data members.
  uInt  fileNr;
  uInt  nBl, nSpw, nChan, nPol;
  uInt  stepBl, stepSpw;
  Int64 row;
  vector<double> scaleFactors;    //# crossdata scale factor per spw
  Int64 fileOffset;
  uInt  blockOffset;
  Short dataType;       //# data type: 0=short 1=int32 3=float cross data
  //                                   10=auto 11=wvr? 12=zerolags
};

  // Read/write object into AipsIO.
  // <group>
  AipsIO& operator<< (AipsIO&, const AsdmIndex&);
  AipsIO& operator>> (AipsIO&, AsdmIndex&);

  std::ostream& operator<<(std::ostream& os, const AsdmIndex& ix);


} //# end namespace

#endif
