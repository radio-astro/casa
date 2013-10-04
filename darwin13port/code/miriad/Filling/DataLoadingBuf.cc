//# ClassFileName.cc:  this defines ClassName, which ...
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
//# $Id: DataLoadingBuf.cc,v 1.2 2009/09/03 23:28:32 pteuben Exp $

//# Includes
#include <miriad/Filling/DataLoadingBuf.h>

void DataLoadingBuf::resizeForNspect(Int nspect) {
    uInt n = vislist.nelements();
    nspect++;
    vislist.resize(nspect);
    flgslist.resize(nspect);
    for(Int i=n; i < nspect; i++) {
        vislist[i] = NULL;
        flgslist[i] = NULL;
    }
}

DataLoadingBuf::DataLoadingBuf() : vislist(), flgslist(), wt(1), rms(1) { }

DataLoadingBuf::~DataLoadingBuf() {
    for(uInt i=0; i < vislist.nelements(); i++) {
        if (vislist[i] != NULL) { delete vislist[i]; vislist[i] = NULL; }
        if (flgslist[i] != NULL) { delete flgslist[i]; flgslist[i] = NULL; }
    }
}
