//# GBTMSAntennaFiller.cc: GBTMSAntennaFiller fills the MSAntenna table for GBT fillers
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

#include <nrao/GBTFillers/GBTMSAntennaFiller.h>

#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <nrao/FITS/GBTAntennaFile.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Utilities/Assert.h>

GBTMSAntennaFiller::GBTMSAntennaFiller()
    : msAntenna_p(0), msAntCols_p(0), antId_p(-1)
{;}


GBTMSAntennaFiller::GBTMSAntennaFiller(MSAntenna &msAntenna)
    : msAntenna_p(0), msAntCols_p(0), antId_p(-1)
{init(msAntenna);}

GBTMSAntennaFiller::~GBTMSAntennaFiller()
{
    delete msAntenna_p;
    msAntenna_p = 0;

    delete msAntCols_p;
    msAntCols_p = 0;
}

void GBTMSAntennaFiller::attach(MSAntenna &msAntenna)
{init(msAntenna);}

void GBTMSAntennaFiller::fill(const GBTAntennaFile &antennaFile)
{
    if (msAntenna_p && 
	(msAntenna_p->nrow() == 0 || 
	 antennaFile.telescop() != name_p ||
	 !(antennaFile.position().getRef().getType() == position_p.getRef().getType()) ||
	 !(antennaFile.position().getValue() == position_p.getValue()))) {
	// add a row to the table or re-use an existing row - the look-back should be cheap
	// and infrequent
	antId_p = 0;
	while (antId_p < Int(msAntenna_p->nrow())) {
	    if (antennaFile.telescop() == msAntCols_p->name()(antId_p)) {
		MPosition pos(msAntCols_p->positionMeas().convert(antId_p, antennaFile.position().getRef()));
		if (pos.getRef().getType() == antennaFile.position().getRef().getType() &&
		    pos.getValue() == antennaFile.position().getValue()) break;
	    }
	    antId_p++;
	}
	if (antId_p >= Int(msAntenna_p->nrow())) {
	    // add a row	
	    antId_p = msAntenna_p->nrow();
	    msAntenna_p->addRow();
	    // even if not attached, these have the appropriate defaults
	    name_p = antennaFile.telescop();
	    position_p = antennaFile.position();
	    msAntCols_p->name().put(antId_p, name_p);
	    msAntCols_p->station().put(antId_p, "GREENBANK");
	    msAntCols_p->type().put(antId_p, "GROUND-BASED");
	    msAntCols_p->mount().put(antId_p, "ALT-AZ");
	    msAntCols_p->positionMeas().put(antId_p, position_p);
	    // eventually we'll need an accurate value for the offset
	    Vector<Double> offset(3);
	    offset = 0.0;
	    msAntCols_p->offset().put(antId_p, offset);
	    // eventually we'll need to get an accurate value from M&C
	    msAntCols_p->dishDiameterQuant().put(antId_p, Quantity(110.0,"m"));
	    msAntCols_p->flagRow().put(antId_p, False);
	} else {
	    // reuse the existing row
	    name_p = antennaFile.telescop();
	    position_p = antennaFile.position();
	}
    } 
}

void GBTMSAntennaFiller::init(MSAntenna &msAntenna)
{
    msAntenna_p = new MSAntenna(msAntenna);
    AlwaysAssert(msAntenna_p, AipsError);

    msAntCols_p = new MSAntennaColumns(msAntenna);
    AlwaysAssert(msAntCols_p, AipsError);
}
