//# GBTFeedInfo.cc: GBTFeedInfo is a simple class used by GBTMSFeedFiller
//# Copyright (C) 2003
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

//# Includes

#include <nrao/GBTFillers/GBTFeedInfo.h>

#include <casa/BasicSL/Constants.h>


GBTFeedInfo::GBTFeedInfo()
    : itsFeedNumber(-1), itsPhysFeed(-1), itsIFRows(0), itsPolz(0), itsOffsets(2,0.0),
      itsName(""), itsSRFeedNumber(-1)
{;}

GBTFeedInfo::GBTFeedInfo(const GBTFeedInfo &other) 
    : itsFeedNumber(-1), itsPhysFeed(-1), itsIFRows(0), itsPolz(0), itsOffsets(2,0.0),
      itsName(""), itsSRFeedNumber(-1)
{
    *this = other;
}

GBTFeedInfo &GBTFeedInfo::operator=(const GBTFeedInfo &other) 
{
    if (this != &other) {
	itsFeedNumber = other.itsFeedNumber;
	itsPhysFeed = other.itsPhysFeed;
	itsIFRows.resize(other.itsIFRows.nelements(), True, False);
	itsIFRows = other.itsIFRows;
	itsPolz.resize(other.itsPolz.nelements());
	itsPolz = other.itsPolz;
	itsOffsets = other.itsOffsets;
	itsName = other.itsName;
	itsSRFeedNumber = other.itsSRFeedNumber;
    }
    return *this;
}
void GBTFeedInfo::addRow(Int whichRow)
{
    uInt thisSize = itsIFRows.nelements();
    itsIFRows.resize(thisSize+1);
    itsIFRows[thisSize] = whichRow;
}

void GBTFeedInfo::setPol(const String &pol, Int whichRec,
			 Int nrec)
{
    if (nrec != Int(itsPolz.nelements())) {
	itsPolz.resize(nrec);
    }
    itsPolz[whichRec] = pol;
}

void GBTFeedInfo::setOffsets(Double xeloffset, Double eloffset) 
{
    itsOffsets[0] = xeloffset*C::pi/180.0;
    itsOffsets[1] = eloffset*C::pi/180.0;
}
