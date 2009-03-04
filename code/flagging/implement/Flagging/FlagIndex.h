//# RFASelector.h: this defines RFASelector
//# Copyright (C) 2000,2001
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
//# $Id$
#ifndef FLAGGING_FLAGINDEX_H
#define FLAGGING_FLAGINDEX_H

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Utilities/Regex.h>
#include <casa/OS/HostInfo.h>
#include <casa/System/PGPlotter.h>
#include <casa/System/ProgressMeter.h>
#include <casa/stdio.h>
#include <casa/math.h>
#include <stdarg.h>

#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVEpoch.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  class FlagIndex {

  public:
    FlagIndex();
    FlagIndex(const String &time,
	      uInt ant1,
	      uInt ant2,
	      uInt spw,
	      const String &chan,
	      const String &corr);

    FlagIndex &operator=(const FlagIndex & other);
    
  private:  
    String time;
    uInt ant1;
    uInt ant2;
    
    uInt spw;
    String chan;
    
    String corr;
    
    //uInt field;
    //uInt scan;
    //time_interval
    
    //uInt polid;

    friend ostream &operator<<(ostream &os, const FlagIndex &fi);
  };

} //# NAMESPACE CASA - END

#endif
