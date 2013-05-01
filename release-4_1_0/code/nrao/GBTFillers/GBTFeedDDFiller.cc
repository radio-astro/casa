//# GBTFeedDDFiller.cc: This class fills the FEED and DATA_DESC tables
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

#include <nrao/GBTFillers/GBTFeedDDFiller.h>

#include <nrao/FITS/GBTAntennaFile.h>
#include <nrao/FITS/GBTBackendTable.h>
#include <nrao/FITS/GBTLO1File.h>
#include <nrao/FITS/GBTSampler.h>
#include <nrao/GBTFillers/GBTIFFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>

GBTFeedDDFiller::GBTFeedDDFiller()
    : itsAttached(False), itsNfeeds(0), itsNcorr(0), itsNstate(0), itsFeeds(2), itsFeedMap(-1), 
      itsCurrFeed(0), itsCurrIF(0), itsCurrCorr(0), itsCurrNif(0), 
      itsCurrNcorr(0), itsCurrCount(0),itsCurrFeedPtr(0), itsCurrIFPtr(0), itsCurrCorrPtr(0)
{;}

GBTFeedDDFiller::GBTFeedDDFiller(MeasurementSet &ms)
    : itsAttached(False), itsNfeeds(0), itsNcorr(0), itsNstate(0), itsFeeds(2), itsFeedMap(-1), 
      itsCurrFeed(0), itsCurrIF(0), itsCurrCorr(0), itsCurrNif(0), 
      itsCurrNcorr(0), itsCurrCount(0), itsCurrFeedPtr(0), itsCurrIFPtr(0), itsCurrCorrPtr(0)
{
    attach(ms);
}

GBTFeedDDFiller::~GBTFeedDDFiller()
{
    clear();
}

void GBTFeedDDFiller::attach(MeasurementSet &ms)
{
    clear();
    // attaching goes here
    itsDDFiller.attach(ms.dataDescription());
    itsFeedFiller.attach(ms.feed());
    itsPolFiller.attach(ms.polarization());
    itsSWFiller.attach(ms.spectralWindow());
    itsAttached = True;
}

Bool GBTFeedDDFiller::fill(const GBTBackendTable &backend,
			   const GBTIFFiller &iffiller,
			   const GBTAntennaFile &antFile,
			   const GBTLO1File &lo1aFile,
			   const GBTLO1File &lo1bFile,
			   const String &swtchsig,
			   Bool continuum, Bool bankCheck)
{
    Bool result = False;
    if (isAttached()) {
	result = True;
	clear();
	digestFeeds(antFile, iffiller, swtchsig);
	digestSampler(backend, iffiller, lo1aFile, lo1bFile, continuum, bankCheck);
	fillMSSW();
	origin();
	if (hasBeamSwitched() && iffiller.backend() == "Spectrometer") findOtherBanks(iffiller);
    }
    return result;
}

void GBTFeedDDFiller::digestFeeds(const GBTAntennaFile &antFile, 
				  const GBTIFFiller &iffiller,
				  const String &swtchsig)
{
    itsNfeeds = antFile.nfeeds();
    if (itsNfeeds >= itsFeeds.nelements()) {
	itsFeeds.resize(itsNfeeds);
	itsFeeds = 0;
    }
    String trckbeam = antFile.trckbeam();
    for (uInt i=0;i<itsNfeeds;i++) {
	Double xeloffset, eloffset;
	Int srfeed1, srfeed2;
	String feedName;
	Int whichFeed = antFile.getFeedValue(i);
	antFile.getFeedInfo(whichFeed, xeloffset, eloffset, 
			    srfeed1, srfeed2, feedName);
	itsFeeds[i] = new GBTFeed(xeloffset, eloffset, feedName,
				  srfeed1, srfeed2, trckbeam);
	AlwaysAssert(itsFeeds[i], AipsError);

	itsFeedMap(whichFeed) = i;
    }

    // go through IF table to set receptors for each feed
    if (iffiller.isValid()) {
	Bool nullFeed = iffiller.feed().isNull();
	for (uInt i=0;i<iffiller.polarize().nrow();i++) {
	    String receptorName;
	    if (!iffiller.receptor().isNull()) receptorName = iffiller.receptor()(i);
	    String receiverName;
	    if (!iffiller.receiver().isNull()) receiverName = iffiller.receiver()(i);
	    // even when there is no FEED column, there may be
	    // unique feeds here.  That is worked out below but
	    // we need a unique identifier in that case - use the
	    // row number. The itsFeedMap will be larger, but still
	    // functional.
	    Int ifFeed = nullFeed ? i : iffiller.feed()(i);
	    Int whichFeed = itsFeedMap(ifFeed);
	    Bool receptorAdded = False;
	    if (whichFeed < 0) {
		if (nullFeed) {
		    // try and addReceptor to each feed in turn until it returns OK
		    for (uInt testFeed=0;testFeed<itsNfeeds;testFeed++) {
			if (itsFeeds[testFeed]->receiver().length() == 0 ||
			    itsFeeds[testFeed]->receiver() == receiverName) {
			    if (itsFeeds[testFeed]->addReceptor(iffiller.polarize()(i),receptorName)) {
				// found it
				whichFeed = testFeed;
				receptorAdded = True;
				itsFeedMap(ifFeed) = whichFeed;
				itsFeeds[testFeed]->setReceiver(receiverName);
			    }
			    break;
			}
		    }
		    if (whichFeed < 0) {
			// need a new one
			itsNfeeds++;
			if (itsNfeeds > itsFeeds.nelements()) {
			    itsFeeds.resize(itsNfeeds);
			}
			whichFeed = itsNfeeds-1;
			itsFeeds[whichFeed] = new GBTFeed();
			AlwaysAssert(itsFeeds[whichFeed], AipsError);
			itsFeedMap(ifFeed) = whichFeed;
			itsFeeds[whichFeed]->setReceiver(receiverName);
		    }
		} else {
		    // need an undefined feed
		    itsNfeeds++;
		    if (itsNfeeds > itsFeeds.nelements()) {
			itsFeeds.resize(itsNfeeds);
		    }
		    whichFeed = itsNfeeds-1;
		    itsFeeds[whichFeed] = new GBTFeed();
		    AlwaysAssert(itsFeeds[whichFeed], AipsError);
		    itsFeedMap(ifFeed) = whichFeed;
		    itsFeeds[whichFeed]->setReceiver(receiverName);
		}
	    }
	    // whichFeed must be >= 0 by here
	    if (!receptorAdded) {
		if (itsFeeds[whichFeed]->receiver().length() == 0 ||
		    itsFeeds[whichFeed]->receiver() == receiverName) {
		    if (itsFeeds[whichFeed]->addReceptor(iffiller.polarize()(i), receptorName)) {
			// addition went just fine
			receptorAdded = True;
			itsFeeds[whichFeed]->setReceiver(receiverName);
		    }
		}
		if (!receptorAdded) {
		    // force a new feed
		    itsNfeeds++;
		    if (itsNfeeds > itsFeeds.nelements()) {
			itsFeeds.resize(itsNfeeds);
		    }
		    whichFeed = itsNfeeds-1;
		    itsFeeds[whichFeed] = new GBTFeed();
		    AlwaysAssert(itsFeeds[whichFeed], AipsError);
		    itsFeedMap(ifFeed) = whichFeed;
		    // no need to check on return value here
		    itsFeeds[whichFeed]->addReceptor(iffiller.polarize()(i),receptorName);
		    itsFeeds[whichFeed]->setReceiver(receiverName);
		} 
	    }
	}
    }

    // and freeze them - can be checked immediately as they are frozen
    Bool reuse = True;
    for (uInt i=0;i<itsNfeeds;i++) {
	itsFeeds[i]->freeze();
	if (itsFeeds[i]->numReceptors() > 0) {
	    reuse = reuse && itsFeedFiller.checkPrior(*itsFeeds[i], i);
	}
    }
    Bool hasBSSet = False;
    Bool bswSig = swtchsig == "BEAMSW";
    for (uInt i=0;i<itsNfeeds;i++) {
	if (itsFeeds[i]->numReceptors() > 0) {
	    if (reuse) {
		itsFeeds[i]->setFeedId(itsFeedFiller.fill(*itsFeeds[i],i));
	    } else {
		itsFeeds[i]->setFeedId(itsFeedFiller.fill(*itsFeeds[i],-1));
	    }
	    if (bswSig && !hasBSSet && itsFeeds[i]->srFitsFeedId() != itsFeeds[i]->fitsFeedId()) {
		itsFeedFiller.setHasBeamSwitched(True);
		hasBSSet = True;
	    }
	}
    }
    // and set the srfeed ID appropriately here and in the FEED table
    Bool bswErrorSeen = False;
    for (uInt i=0;i<itsNfeeds;i++) {
	if (itsFeeds[i]->numReceptors() > 0) {
	    Int feedid = itsFeeds[i]->feedId();
	    Int srfeedid = feedid;
	    const GBTFeed *srfeedPtr = itsFeeds[i];
	    if (itsFeeds[i]->srFitsFeedId() != itsFeeds[i]->fitsFeedId()) {
		// find srFitsFeed() == fitsFeedId()
		for (uInt j=0;j<itsNfeeds;j++) {
		    if (j!=i) {
			if (itsFeeds[i]->srFitsFeedId() == itsFeeds[j]->fitsFeedId()) {
			    // check that this really makes sense
			    if (itsFeeds[i]->numReceptors() == itsFeeds[j]->numReceptors()) {
				srfeedid = itsFeeds[j]->feedId();
				srfeedPtr = itsFeeds[j];
				break;
			    } else if (bswSig) {
				bswErrorSeen = True;
			    }
			}
		    }
		}
	    }
	    itsFeeds[i]->setSRFeedId(srfeedid);
	    itsFeeds[i]->setSRFeedPtr(srfeedPtr);
	    itsFeedFiller.setSRFeedId(feedid, srfeedid);
	}
    }
    if (bswErrorSeen) {
        LogIO os(LogOrigin("GBTFeedDDFiller",
           "GBTFeedDDFiller::digestFeeds(const GBTAntennaFile &antFile, const GBTIFFiller &iffiller)"));
        os << LogIO::SEVERE << WHERE
           << "The beam switching information is not consistent.\n" 
           << "This is probably due to missing IF information for at least one feed in the IF FITS file.\n"
           << "Data will be filled as if it all came from the same feed." << LogIO::POST;
    }
}


void GBTFeedDDFiller::digestSampler(const GBTBackendTable &backend, const GBTIFFiller &iffiller,
				    const GBTLO1File &lo1aFile, const GBTLO1File &lo1bFile,
				    Bool continuum, Bool bankCheck)
{
    GBTSampler sampler(backend,iffiller.currentTable().nrow());
    itsNstate = backend.nstate();
    Bool badIF = !iffiller.isValid();
    Bool nullFeed = badIF || iffiller.feed().isNull();
    for (uInt i=0;i<sampler.nrows();i++) {
	uInt nchan = sampler.nchan(i);
	String bank = sampler.bankA(i);
	Int port = sampler.portA(i);
	Double ifFreq = backend.centerIF(i);
	Double bw = backend.bw(i);
	Bool increases = backend.increases(i);

	// the IF might still be bad if there is no match for this bank and port
	// SO, try to use it first, if it seems okay now
	if (!badIF) {
	    GBTIF thisIF(nchan, itsNstate, bank, port,
			 ifFreq, bw, increases, iffiller,
			 lo1aFile, lo1bFile, continuum);
	    Int portArow = thisIF.ifrows()[0];
	    Int ifFeed = -1;
	    if (portArow >= 0) {
		ifFeed = nullFeed ? portArow : iffiller.feed()(portArow);
	    }
	    Int portBrow = iffiller.whichRow(sampler.bankB(i), sampler.portB(i));
	    Int whichFeed =  itsFeedMap(ifFeed);
	    if (whichFeed >= 0) {
		// ok
		GBTCorrelation corr(*itsFeeds[whichFeed], itsNstate, nchan, bank);
		String polA, polB, rcptA, rcptB;
		rcptA = rcptB = "";
		if (portArow >= 0) {
		    polA = iffiller.polarize()(portArow);
		    rcptA = iffiller.receptor()(portArow);
		} else {
		    polA = "X";
		}
		if (portBrow >= 0) {
		    polB = iffiller.polarize()(portBrow);
		    rcptB = iffiller.receptor()(portBrow);
		} else {
		    polB = "X";
		}
		corr.setSamplerRow(i, polA, polB, rcptA, rcptB);
		thisIF.addCorr(corr);
		// add it in to the appropriate feed
		itsFeeds[whichFeed]->addUniqueIF(thisIF, !continuum, bankCheck);
	    } else {
		// handle this shortly
		badIF = True;
	    }
	}
	// this handles truely bad or missing IF files as well as the case where
	// a specific port and bank combination is missing
	if (badIF) {
	    // just do channel numbers
	    Int whichFeed = itsFeedMap(0);
	    if (whichFeed < 0) {
		// need an undefined feed
		itsNfeeds++;
		if (itsNfeeds > itsFeeds.nelements()) {
		    itsFeeds.resize(itsNfeeds);
		}
		whichFeed = itsNfeeds-1;
		itsFeeds[whichFeed] = new GBTFeed();
		AlwaysAssert(itsFeeds[whichFeed], AipsError);
		itsFeedMap(0) = whichFeed;
		// this must have just one receptor "X"
		itsFeeds[whichFeed]->addReceptor("X","");
		itsFeeds[whichFeed]->freeze();
		// need to fill this
		if (itsFeedFiller.checkPrior(*itsFeeds[whichFeed], whichFeed)) {
		    itsFeeds[whichFeed]->setFeedId(itsFeedFiller.fill(*itsFeeds[whichFeed],whichFeed));
		} else {
		    itsFeeds[whichFeed]->setFeedId(itsFeedFiller.fill(*itsFeeds[whichFeed],-1));
		}
	    }
	    GBTIF thisIF(nchan, itsNstate);
	    GBTCorrelation corr(*itsFeeds[whichFeed], itsNstate, nchan, bank);
	    corr.setSamplerRow(i, "X", "X", "", "");
	    thisIF.addCorr(corr);
	    // add it in to the appropriate feed
	    itsFeeds[whichFeed]->addUniqueIF(thisIF, !continuum, bankCheck);
	}
    }

    // Need to ensure that each IF has only unique correlations (no duplicates)

    // need to freeze all correlations at this point, can count them here.
    itsNcorr = 0;
    for (uInt i=0;i<itsNfeeds;i++) {
	GBTFeed *thisFeed = itsFeeds[i];
	if (thisFeed->numReceptors() > 0) {
	    for (uInt j=0;j<thisFeed->nif();j++) {
		GBTIF *thisIF = &(thisFeed->getIF(j));
		itsNcorr += thisIF->ncorr();
		for (uInt k=0;k<thisIF->ncorr();k++) {
		    GBTCorrelation *thisCorr = &(thisIF->getCorr(k));
		    thisCorr->freeze();
		    // can immediately fill polarization sub-table
		    itsPolFiller.fill(thisCorr->numCorr(), 
				      thisCorr->corrType(), 
				      thisCorr->corrProduct());
		    
		    thisCorr->setPolId(itsPolFiller.polarizationId());
		}
	    }
	}
    }
}

void GBTFeedDDFiller::fillMSSW()
{
    // go through each feed
    for (uInt i=0;i<itsNfeeds;i++) {
	GBTFeed *thisFeed = itsFeeds[i];
	Vector<Int> usedSPW(thisFeed->nif(), -1);
	uInt spwCount = 0;
	for (uInt j=0; j<thisFeed->nif(); j++) {
	    GBTIF *thisIF = &(thisFeed->getIF(j));
	    Vector<Double> offsets = thisIF->offsets();
	    if (anyNE(offsets, 0.0)) {
		// get a separate spw for each unique state
		SimpleOrderedMap<Double, Int> offsetMap(-1,offsets.nelements());
		for (uInt k=0;k<offsets.nelements();k++) {
		    Int thisSpw = -1;
		    if (offsetMap(offsets[k]) >= 0) {
			thisSpw = offsetMap(offsets[k]);
		    } else {
			thisSpw = itsSWFiller.fill(thisIF->nchan(),
						   thisIF->refFreq()+offsets[k],
						   thisIF->refChan(),
						   thisIF->deltaFreq(),
						   thisIF->refFrame(),
						   thisIF->tolerance(),
						   usedSPW);
			if (spwCount >= usedSPW.nelements()) {
			    uInt oldSize = usedSPW.nelements();
			    usedSPW.resize(usedSPW.nelements()*2,True);
			    for (uInt newEl=oldSize;newEl<usedSPW.nelements();newEl++) {
				usedSPW[newEl] = -1;
			    }
			}
			offsetMap(offsets[k]) = thisSpw;
			usedSPW(spwCount++) = thisSpw;
		    }
		    thisIF->setSpwId(thisSpw,k);
		    // for each Corr, fill data desc
		    for (uInt c=0;c<thisIF->ncorr();c++) {
			GBTCorrelation *thisCorr = &(thisIF->getCorr(c));
			thisCorr->setDataDescId(itsDDFiller.fill(thisCorr->polId(),
								 thisSpw),
						k);
		    }
		}
	    } else {
		// one spw will suffice for all states
		Int thisSpw = itsSWFiller.fill(thisIF->nchan(),
					       thisIF->refFreq(),
					       thisIF->refChan(),
					       thisIF->deltaFreq(),
					       thisIF->refFrame(),
					       thisIF->tolerance(),
					       usedSPW);
		if (spwCount >= usedSPW.nelements()) {
		    uInt oldSize = usedSPW.nelements();
		    usedSPW.resize(usedSPW.nelements()*2,True);
		    for (uInt newEl=oldSize;newEl<usedSPW.nelements();newEl++) {
			usedSPW[newEl] = -1;
		    }
		}
		usedSPW(spwCount++) = thisSpw;
		for (uInt k=0;k<offsets.nelements();k++) {
		    thisIF->setSpwId(thisSpw,k);
		}
		// for each Corr, fill data desc
		for (uInt c=0;c<thisIF->ncorr();c++) {
		    GBTCorrelation *thisCorr = &(thisIF->getCorr(c));
		    Int thisDD = itsDDFiller.fill(thisCorr->polId(),
						  thisSpw);
		    for (uInt k=0;k<offsets.nelements();k++) {
			thisCorr->setDataDescId(thisDD, k);
		    }
		}
	    }   
	}
    }
}

void GBTFeedDDFiller::clear()
{
    for (uInt i=0;i<itsNfeeds;i++) {
	delete itsFeeds[i];
	itsFeeds[i] = 0;
    }
    
    itsNfeeds = 0;
    itsFeedMap.clear();
    
    itsCurrFeed = itsCurrIF = itsCurrCorr = itsCurrNif = itsCurrNcorr = 0;
    itsCurrFeedPtr = 0;
    itsCurrIFPtr = 0;
    itsCurrCorrPtr = 0;
}


void GBTFeedDDFiller::flush()
{
    itsFeedFiller.flush();
}

void GBTFeedDDFiller::origin() const
{
    itsCurrFeed = itsCurrIF = itsCurrCorr = itsCurrCount = 0;
    // Not all feeds have IFs - find first feed with non-zero IF rows
    while (itsFeeds[itsCurrFeed]->nif() == 0 && 
	   itsCurrFeed < itsNfeeds) {
	itsCurrFeed++;
    }
    if (itsCurrFeed >= itsNfeeds) {
	// nothing there - this probably should never happen
	itsCurrFeed = 0;
	itsCurrFeedPtr = itsFeeds[0];
	itsCurrNif = 0;
	itsCurrIFPtr = 0;
	itsCurrNcorr = 0;
	itsCurrCorrPtr = 0;
    } else {
	itsCurrFeedPtr = itsFeeds[itsCurrFeed];
	// every IF that is present will have at least one correlation
	itsCurrNif = itsCurrFeedPtr->nif();
	itsCurrIFPtr = &(itsCurrFeedPtr->getIF(0));
	itsCurrNcorr = itsCurrIFPtr->ncorr();
	itsCurrCorrPtr = &(itsCurrIFPtr->getCorr(0));
    }
}

void GBTFeedDDFiller::next() const
{
    itsCurrCount++;
    if (itsCurrCount < itsNcorr) {
	itsCurrCorr++;
	if (itsCurrCorr >= itsCurrNcorr) {
	    itsCurrCorr = 0;
	    itsCurrIF++;
	    if (itsCurrIF >= itsCurrNif) {
		itsCurrIF = 0;
		itsCurrFeed++;
		while (itsCurrFeed < itsNfeeds && 
		       itsFeeds[itsCurrFeed]->nif() == 0) {
		    itsCurrFeed++;
		}
		itsCurrFeedPtr = itsFeeds[itsCurrFeed];
		itsCurrNif = itsCurrFeedPtr->nif();
	    }
	    itsCurrIFPtr = &(itsCurrFeedPtr->getIF(itsCurrIF));
	    itsCurrNcorr = itsCurrIFPtr->ncorr();
	}
	itsCurrCorrPtr = &(itsCurrIFPtr->getCorr(itsCurrCorr)); 
    }
}

void GBTFeedDDFiller::findOtherBanks(const GBTIFFiller &iffiller)
{
    for (uInt whichFeed=0;whichFeed<nfeeds();whichFeed++) {
        for (uInt whichIF=0;whichIF<feed(whichFeed).nif();whichIF++) {
	    // find any row in iffiller matching this IF and at least one part of currCorr
	    // and not in the current bank.
	    if (feed(whichFeed).fitsFeedId() != feed(whichFeed).srFitsFeedId() &&
	        feed(whichFeed).getIF(whichIF).ifrows().nelements() > 0) {
                Vector<Int> ifrows = feed(whichFeed).getIF(whichIF).ifrows();
	        Int thisRow = ifrows[0];
	        Int thisSR1 = iffiller.srfeed1()(thisRow);
	        Int thisSR2 = iffiller.srfeed2()(thisRow);
	        String thisRec = iffiller.receiver()(thisRow);
	        String thisPol = iffiller.polarize()(thisRow);
	        Double thisMult = iffiller.sffMultiplier()(thisRow);
	        Double thisSB = iffiller.sffSideband()(thisRow);
	        Double thisOff = iffiller.sffOffset()(thisRow);
	        Float thisBW = iffiller.bandwidth()(thisRow);
	        for (uInt otherRow=0;otherRow<iffiller.srfeed1().nrow();otherRow++) {
		    // skip this row
		    if (!anyEQ(ifrows,Int(otherRow))) {
		        // match srfeed1, srfeed2, receptor, polarize,
		        // sffMultiplier, sffSideband, sffOffset, bandwidth
		        if (iffiller.srfeed1()(otherRow) == thisSR1 &&
		 	    iffiller.srfeed2()(otherRow) == thisSR2 &&
			    iffiller.receiver()(otherRow) == thisRec &&
			    iffiller.polarize()(otherRow) == thisPol &&
			    iffiller.sffMultiplier()(otherRow) == thisMult &&
			    iffiller.sffSideband()(otherRow) == thisSB &&
			    iffiller.sffOffset()(otherRow) == thisOff &&
			    iffiller.bandwidth()(otherRow) == thisBW) {
                            String srbank = iffiller.bank()(otherRow);
                            for (uInt whichCorr=0;
                                whichCorr<feed(whichFeed).getIF(whichIF).ncorr();whichCorr++) {
			        feed(whichFeed).getIF(whichIF).getCorr(whichCorr).setsrbank(srbank);
			        break;
		            }
                        }
                    }
		}
	    }
	}
    }
}
