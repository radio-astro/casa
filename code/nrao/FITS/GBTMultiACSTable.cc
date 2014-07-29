//# GBTMultiACSTable.cc: GBT multi-bank backend table for the ACS.
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

#include <nrao/FITS/GBTMultiACSTable.h>
#include <nrao/FITS/GBTStateTable.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordDesc.h>
#include <casa/Exceptions/Error.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Utilities/Assert.h>

GBTMultiACSTable::GBTMultiACSTable(const Vector<String> &fileNames,
				   GBTACSTable::VanVleckCorr vanVleckCorr,
				   GBTACSTable::Smoothing smoothing,
				   Int vvsize, Bool useDCBias,
				   Double dcbias, Int minbiasfactor,
				   Bool fixlags, String fixlagslog,
				   Double /*sigmaFactor*/, Int spikeStart)

    : itsValid(False), itsVVCorr(vanVleckCorr), itsSmoothing(smoothing),
      itsVVSize(vvsize), itsMinbiasfactor(minbiasfactor), 
      itsSpikeStart(spikeStart), itsUseDCBias(useDCBias), itsFixlags(fixlags),
      itsDCBias(dcbias), itsSigmaFactor(6.0), itsFiles(fileNames),
      itsBanks(4,0), itsSampler0(4,0), itsNbanks(0), itsBankNames(""),
      itsBankMap(0,4), itsFixLagsLog(fixlagslog)
{
    itsValid = init();
}

GBTMultiACSTable::~GBTMultiACSTable()
{
    for (uInt i=0;i<itsBanks.nelements();i++) {
	delete itsBanks[i];
	itsBanks[i] = 0;
    }
}


Bool GBTMultiACSTable::reopen(const Vector<String> &fileNames,
			       Bool resync)
{
    itsFiles.resize(fileNames.nelements());
    itsFiles = fileNames;
    itsValid = init(resync);
    return itsValid;
}

uInt GBTMultiACSTable::nchan(uInt whichSampler) const
{
    uInt whichBank = itsBankMap(whichSampler);
    return itsBanks[whichBank]->nchan(whichSampler-itsSampler0[whichBank]);
}

Double GBTMultiACSTable::bw(uInt whichSampler) const
{
    uInt whichBank = itsBankMap(whichSampler);
    return itsBanks[whichBank]->bw(whichSampler-itsSampler0[whichBank]);
}

Double GBTMultiACSTable::centerIF(uInt whichSampler) const
{
    uInt whichBank = itsBankMap(whichSampler);
    return itsBanks[whichBank]->centerIF(whichSampler-itsSampler0[whichBank]);
}

Bool GBTMultiACSTable::increases(uInt whichSampler) const
{
    uInt whichBank = itsBankMap(whichSampler);
    return itsBanks[whichBank]->increases(whichSampler-itsSampler0[whichBank]);
}

void GBTMultiACSTable::next() 
{
    for (uInt i=0;i<itsNbanks;i++) {
	itsBanks[i]->next();
    }
}

const Array<Float> GBTMultiACSTable::rawdata(uInt whichSampler) {
    uInt whichBank = itsBankMap(whichSampler);
    return itsBanks[whichBank]->rawdata(whichSampler-itsSampler0[whichBank]);
}

const Array<Float> GBTMultiACSTable::data(uInt whichSampler) {
    uInt whichBank = itsBankMap(whichSampler);
    return itsBanks[whichBank]->data(whichSampler-itsSampler0[whichBank]);
}

const Array<Float> GBTMultiACSTable::zeroChannel(uInt whichSampler) {
    uInt whichBank = itsBankMap(whichSampler);
    return itsBanks[whichBank]->zeroChannel(whichSampler-itsSampler0[whichBank]);
}

const Array<Bool> GBTMultiACSTable::badData(uInt whichSampler) {
    uInt whichBank = itsBankMap(whichSampler);
    return itsBanks[whichBank]->badData(whichSampler-itsSampler0[whichBank]);
}

Int GBTMultiACSTable::nlevels(uInt whichSampler) const
{
    uInt whichBank = itsBankMap(whichSampler);
    return itsBanks[whichBank]->nlevels();
}

Float GBTMultiACSTable::integrat(uInt whichSampler, uInt whichState) const
{
    uInt whichBank = itsBankMap(whichSampler);
    return itsBanks[whichBank]->integrat(whichSampler-itsSampler0[whichBank],whichState);
}

Bool GBTMultiACSTable::init(Bool resync)
{
    Bool result = True;
    itsNbanks = itsFiles.nelements();
    itsBankMap.clear();
    itsBankNames = "";
    if (!itsSampler.isNull()) {
	Vector<uInt> rows(itsSampler.nrow());
	indgen(rows);
	itsSampler.removeRow(rows);
    }
    if (!itsPort.isNull()) {
	Vector<uInt> rows(itsPort.nrow());
	indgen(rows);
	itsPort.removeRow(rows);
    }
    uInt nsamplers = 0;

    for (uInt i=0;i<itsNbanks;i++) {
	String thisFile = itsFiles[i];
	if (itsBanks[i]) {
	    // reopen
	    itsBanks[i]->reopen(thisFile, resync);
	} else {
	    // need to create it
	    itsBanks[i] = new GBTACSTable(thisFile,
					  itsVVCorr, itsSmoothing,
					  itsVVSize, itsUseDCBias,
					  itsDCBias, itsMinbiasfactor,
					  itsFixlags, itsFixLagsLog,
					  itsSigmaFactor, itsSpikeStart);
	    AlwaysAssert(itsBanks[i], AipsError);
	}
	if (itsBanks[i]->isValid()) {
	    itsBankNames = itsBankNames + itsBanks[i]->bank();
	    // samplers
	    {
		itsSampler0[i] = nsamplers;
		nsamplers += itsBanks[i]->nsamp();
		// add these samplers in
		if (itsSampler.isNull()) {
		    // need to create the table based on this banks sampler table
		    SetupNewTable newSamp("",itsBanks[i]->sampler().actualTableDesc(),
					  Table::Scratch);
		    itsSampler = Table(newSamp, itsBanks[i]->nsamp());
		    itsSamplerRow = TableRow(itsSampler);
		} else {
		    itsSampler.addRow(itsBanks[i]->nsamp());
		}
		ROTableRow newSampRow(itsBanks[i]->sampler());
		Int sampCount = itsSampler0[i];
		for (uInt j=0;j<newSampRow.table().nrow();j++) {
		    newSampRow.get(j);
		    itsSamplerRow.putMatchingFields(sampCount, newSampRow.record());
		    itsBankMap(sampCount) = i;
		    sampCount++;
		}
	    }
	    // ports
	    {
		// add these ports in
		Int portCount = 0;
		if (itsPort.isNull()) {
		    // need to create the table based on this banks sampler table
		    SetupNewTable newPort("",itsBanks[i]->port().actualTableDesc(),
					  Table::Scratch);
		    itsPort = Table(newPort, itsBanks[i]->port().nrow());
		    itsPortRow = TableRow(itsPort);
		} else {
		    portCount = itsPort.nrow();
		    itsPort.addRow(itsBanks[i]->port().nrow());
		}
		ROTableRow newPortRow(itsBanks[i]->port());
		for (uInt j=0;j<newPortRow.table().nrow();j++) {
		    newPortRow.get(j);
		    itsPortRow.putMatchingFields(portCount, newPortRow.record());
		    portCount++;
		}
	    }
	} else {
	    result = False;
	}
    }

    return result;
}
