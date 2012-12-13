//# GBTSampler.cc:  defines GBTSampler, holds SAMPLER table information
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

#include <nrao/FITS/GBTSampler.h>

#include <nrao/FITS/GBTBackendTable.h>

#include <casa/Arrays/ArrayMath.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/Table.h>

GBTSampler::GBTSampler(const GBTBackendTable &backendTab, Int nIFrows)
    : itsHasCross(False)
{
    if (backendTab.sampler().nrow() > 0) {
	uInt nrow = backendTab.sampler().nrow();
	itsBankA.resize(nrow);
	itsBankB.resize(nrow);
	itsPortA.resize(nrow);
	itsPortB.resize(nrow);
	itsNchan.resize(nrow);

	// the simple case, it exists as it should, should
	// be sufficient to just check for one of the 4 columns
	if (backendTab.sampler().tableDesc().isColumn("BANK_A")) {
	    // must be ACS
	    // these columns really should all exist.  Don't check here and let
	    // table exception happen if they don't - its a really severe error
	    // that probably means other bad things have happened and so don't
	    // attempt to recover.
	    ROScalarColumn<String> bankACol(backendTab.sampler(), "BANK_A");
	    bankACol.getColumn(itsBankA);
	    ROScalarColumn<String> bankBCol(backendTab.sampler(), "BANK_B");
	    bankBCol.getColumn(itsBankB);
	    TableColumn portACol(backendTab.sampler(), "PORT_A");
	    TableColumn portBCol(backendTab.sampler(), "PORT_B");
	    for (uInt i=0;i<nrow;i++) {
		itsPortA[i] = portACol.asInt(i);
		itsPortB[i] = portBCol.asInt(i);
	    }

	    String polarize = "AUTO";
	    if (backendTab.sampler().keywordSet().fieldNumber("POLARIZE") >= 0) {
		polarize = backendTab.sampler().keywordSet().asString("POLARIZE");
	    }
	    itsHasCross = polarize == "CROSS";
	} else {
	    // must be either DCR or SpectralProcessor
	    if (backendTab.sampler().tableDesc().isColumn("CHANNELID")) {
		// DCR data
		itsBankA = backendTab.keywords().asString("INPBNK");
		TableColumn channelidCol(backendTab.sampler(),"CHANNELID");
		for (uInt i=0;i<nrow;i++) {
		    itsPortA[i] = channelidCol.asInt(i) + 1;
		}
	    } else {
		// SpectralProcessor 
		String multMode("Square");
		if (backendTab.keywords().fieldNumber("MULTMODE") >= 0) {
		    multMode = backendTab.keywords().asString("MULTMODE");
		} else {
		    // use nIFrows to watch for older SqrCross data
		    if (Int(nrow) == 2*nIFrows) multMode = "SqrCross";
		}
		if (multMode == "Square") {
		    // defaults to this if MULTMODE keyword does not exist
		    // these vectors are short enough (<=16 I think) that its not
		    // worth the effort to construct references and do this using
		    // cute masking techniques, I think.
		    // This does assume that there are an even number of rows, which
		    // should also be safe.
		    uInt nhalf = nrow / 2;
		    for (uInt i=0;i<nhalf;i++) {
			itsBankA[i] = "A";
			itsBankA[i+nhalf] = "B";
			itsPortA[i] = i+1;
			itsPortA[i+nhalf] = i+1;
		    } 	
		    itsBankB = itsBankA;
		    itsPortB = itsPortA;
		} else {
		    // must be SqrCross
		    itsHasCross = True;
		    uInt nhalf = nrow / 2;
		    uInt nquarter = nrow / 4;
		    for (uInt i=0;i<nquarter;i++) {
			uInt first = i*2;
			itsBankA[first] = "A";
			itsBankB[first] = "A";
			itsBankA[first+1] = "A";
			itsBankB[first+1] = "B";
			itsBankA[first+nhalf] = "B";
			itsBankB[first+nhalf] = "B";
			itsBankA[first+nhalf+1] = "B";
			itsBankB[first+nhalf+1] = "A";
			Int portNr = i+1;
			itsPortA[first] = itsPortB[first] = portNr;
			itsPortA[first+1] = itsPortB[first+1] = portNr;
			itsPortA[first+nhalf] = itsPortB[first+nhalf] = portNr;
			itsPortA[first+nhalf+1] = itsPortB[first+nhalf+1] = portNr;
		    }
		}		    
	    }
	}
	// get the nchan from the backend
	for (uInt i=0;i<nrow;i++) {
	    itsNchan[i] = backendTab.nchan(i);
	}
    }
}
