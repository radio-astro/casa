//# AsdmIndex.cc: Index for the ASDM Storage Manager
//# Copyright (C) 2012
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

//# Includes
#include <asdmstman/AsdmIndex.h>
#include <casa/IO/AipsIO.h>

namespace casa {

  AipsIO& operator<< (AipsIO& os, const AsdmIndex& ix)
  {
    // This is version 1 of the index.
    os << Short(1) << ix.dataType << ix.fileNr
       << ix.nBl << ix.crossNspw << ix.crossNchan << ix.crossNpol
       << ix.nAnt << ix.autoNspw << ix.autoNchan << ix.autoNpol
       << ix.crossStepBl << ix.crossStepSpw
       << ix.autoStepBl << ix.autoStepSpw
       << ix.row << ix.crossOffset << ix.autoOffset;
    os.put (ix.scaleFactors);
    ///<< ix.scaleFactors[0] << ix.scaleFactors[1]
    ///<< ix.scaleFactors[2] << ix.scaleFactors[3] << 
    ///<< ix.scaleFactors[0] << ix.scaleFactors[1]
    ///<< ix.scaleFactors[2] << ix.scaleFactors[3];
    return os;
  }

  AipsIO& operator>> (AipsIO& os, AsdmIndex& ix)
  {
    Short version;
    os >> version >> ix.dataType >> ix.fileNr;
    os >> ix.nBl >> ix.crossNspw >> ix.crossNchan >> ix.crossNpol
       >> ix.nAnt >> ix.autoNspw >> ix.autoNchan >> ix.autoNpol
       >> ix.crossStepBl >> ix.crossStepSpw
       >> ix.autoStepBl >> ix.autoStepSpw
       >> ix.row >> ix.crossOffset >> ix.autoOffset;
    os.get (ix.scaleFactors);
    ///>> ix.scaleFactors[0] >> ix.scaleFactors[1]
    ///>> ix.scaleFactors[2] >> ix.scaleFactors[3];
    return os;
  }

} //# end namespace
