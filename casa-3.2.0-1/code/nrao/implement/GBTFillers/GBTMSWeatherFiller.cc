//# GBTMSWeatherFiller.cc: GBTMSWeatherFiller fills the MSWeather table for GBT fillers
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

//# Includes

#include <nrao/GBTFillers/GBTMSWeatherCopier.h>
#include <nrao/GBTFillers/GBTMSWeatherFiller.h>

#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSWeather.h>
#include <ms/MeasurementSets/MSWeatherColumns.h>
#include <casa/Quanta/MVTime.h>
#include <tables/Tables/IncrementalStMan.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ScaColDesc.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

GBTMSWeatherFiller::GBTMSWeatherFiller()
    : device_p("WEATHER"), msWeather_p(0), copiers_p(1),
      ndap_p(0), antennaId_p(-1)
{;}


GBTMSWeatherFiller::GBTMSWeatherFiller(MSWeather &msWeather)
    : device_p("WEATHER"), msWeather_p(0), copiers_p(1),
      ndap_p(0), antennaId_p(-1)
{init(msWeather);}

GBTMSWeatherFiller::~GBTMSWeatherFiller()
{
    delete msWeather_p;
    msWeather_p = 0;

    for (Int i=0;i<ndap_p;i++) {
	delete copiers_p[i];
	copiers_p[i] = 0;
    }
}

Int GBTMSWeatherFiller::prepare(const String &fileName,
				const String &manager, const String &sampler,
				const MVTime &startTime)
{
    Int result = GBTDAPFillerBase::prepare(fileName, manager, sampler, startTime);
    if (result >= ndap_p) {
	// try and make a new copier
	if (Int(copiers_p.nelements()) <= result) {
	    copiers_p.resize(copiers_p.nelements()*2);
	}
	// we can finally create a copier here
	copiers_p[result] = new GBTMSWeatherCopier(*msWeather_p, dapRecord(result),
						   tableDesc(result), result);
	AlwaysAssert(copiers_p[result], AipsError);
	ndap_p = result + 1;
    } else if (result>=0) {
	// the dapRecord has been re-initialized for this - reinit the copier
	copiers_p[result]->reinit(dapRecord(result), tableDesc(result), result);
    }

    // if result < 0, then there was a problem and we should report that
    return result;
}

void GBTMSWeatherFiller::attach(MSWeather &msWeather)
{init(msWeather);}

void GBTMSWeatherFiller::fill()
{
    // add in the number of rows to fill
    if (rowsToFill() > 0) {
	// add the rows, remember where we start from
	uInt thisrow = msWeather_p->nrow();
	uInt newRows = rowsToFill();
	uInt endrow = thisrow + newRows;;
	msWeather_p->addRow(newRows, True);
	while (thisrow < endrow) {
	    // everything is done in the copier
	    copiers_p[currentId()]->copy(thisrow, currentSampler(),
					 currentManager(), currentTime(),
					 currentInterval(), antennaId_p);
	    next();
	    thisrow++;
	}
    } 
}

void GBTMSWeatherFiller::init(MSWeather &msWeather)
{
    msWeather_p = new MSWeather(msWeather);
    AlwaysAssert(msWeather_p, AipsError);
}
