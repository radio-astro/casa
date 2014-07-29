//# MirVisReader.h: reads data from a Miriad Visibility dataset 
//# Copyright (C) 2000,2001,2002
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
//#
//# $Id: MirVarHandler.h,v 1.2 2009/09/03 23:28:32 pteuben Exp $

#ifndef BIMA_MIRVARHANDLER_H
#define BIMA_MIRVARHANDLER_H

#include <casa/namespace.h>
//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
  //PJT class FillMetadata;
} //# NAMESPACE CASA - END

class FillMetadata;

class MirVarHandler {
protected:
    MirVarHandler() { }   // can't instantiate this directly
    MirVarHandler(MirVarHandler&) { }  

public:
    // handle an update to the integration time
    virtual void updateIntTime(FillMetadata &fm, Double time) = 0;

    // handle an update to the observatory
    virtual void updateObservatory(FillMetadata &fm, Double time) = 0;

    // handle an update to the array configuration
    virtual void updateArray(FillMetadata &fm, Double time) = 0;

    // handle an update to the array configuration
    virtual void updatePolSetup(FillMetadata &fm, Double time) = 0;

    // handle an update to the source
    virtual void updateSource(FillMetadata &fm, Double time) = 0;

	// handle an update to the source model -- dgoscha
	virtual void updateSourceModel(FillMetadata &fm, Double time) = 0;

    // handle an update to the observing field
    virtual void updateField(FillMetadata &fm, Double time) = 0;

    // handle an update to the frequency setup
    virtual void updateFreqSetup(FillMetadata &fm, Double time) = 0;

    // handle an update to system temperatures
    virtual void updateTsys(FillMetadata &fm, Double time) = 0;
};

#endif
