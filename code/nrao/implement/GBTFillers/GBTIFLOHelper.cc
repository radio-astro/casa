//# GBTIFLOHelper.cc: GBTIFLOHelper helps the GBT fillers use the IF and LO data
//# Copyright (C) 2001,2002,2003
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

#include <nrao/FITS/GBTLO1File.h>
#include <nrao/GBTFillers/GBTIFLOHelper.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/Exceptions/Error.h>
#include <nrao/GBTFillers/GBTIFFiller.h>
#include <nrao/GBTFillers/GBTLO1DAPFiller.h>
#include <measures/Measures/Stokes.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableColumn.h>
#include <casa/Quanta/MVEpoch.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Regex.h>

GBTIFLOHelper::GBTIFLOHelper(GBTIFFiller *ifFiller,
			     const GBTLO1File *lo1AFile,
			     const GBTLO1File *lo1BFile,
			     GBTLO1DAPFiller *lo1Filler,
			     const Table &samplerTable,
			     const Table &portTable,
			     uInt nphases, uInt nchannels,
			     const String &dcrBank)
    : itsNphases(nphases), itsNchannels(nchannels),
      itsLO1Filler(lo1Filler), itsRcvrRows(samplerTable.nrow()), itsLO1offset(nphases)
{
    itsRefChan = itsNchannels/2.0;
    if (itsNchannels == 1) itsRefChan = 0.0;

    // ensure that itsRcvrRows elements are all unset at first
    itsRcvrRows.set((Block<uInt> *)0);

    if (ifFiller && !ifFiller->currentTable().isNull() &&
	ifFiller->isValid() &&
	(ifFiller->backend() == "SpectralProcessor" ||
	 ifFiller->backend() == "Spectrometer" ||
	 ifFiller->backend() == "DCR"))
    {
	// determine how many true feeds there are
	// Map row of samplerTable to bank and channel of the IF table, from there
	// get the feed ID, receptor ID and total number of receptors for that feed.
	uInt nsamp = samplerTable.nrow();
	Vector<String> banks(nsamp);
	Vector<Int> ports(nsamp);
	Vector<Int> indx(nsamp);
	Vector<Double> bw(nsamp, 0.0);
	Vector<Double> r0(nsamp), sbMult(nsamp);
	// default - 0Hz
	r0 = 0.0;
	// default = 1.0 - same sign as sideband
	sbMult = 1.0;
	if (ifFiller->backend() == "SpectralProcessor") {
	    // This just works for SpectralProcessor data now.  Its also a hack because of
	    // deficiencies in the RECEIVER table there.
	    // This is the hack ...
	    if (nsamp == 2) {
		banks(0) = "A"; banks(1) = "B";
		ports = 1;
	    } else if (nsamp == 4) {
		banks(0) = banks(1) = "A";
		banks(2) = banks(3) = "B";
		ports(0) = ports(2) = 1;
		ports(1) = ports(3) = 2;
	    } else if (nsamp == 8) {
		banks(0) = banks(1) = banks(2) = banks(3) = "A";
		banks(4) = banks(5) = banks(6) = banks(7) = "B";
		ports(0) = ports(4) = 1;
		ports(1) = ports(5) = 2;
		ports(2) = ports(6) = 3;
		ports(3) = ports(7) = 4;
	    } else {
		// error if here, set to an impossible value so lookups fail
		// issue a warning?
		banks = "";
		ports = -1;
	    }
	    ROTableColumn rcvrId(samplerTable, "RCVRID");
	    ROTableColumn bandwd(samplerTable, "BANDWD");
	    for (uInt i=0;i<nsamp;i++) {
		indx(i) = rcvrId.asInt(i);
		bw(i) = bandwd.asdouble(i);
	    }
	} else if (ifFiller->backend() == "Spectrometer") {
	    // old style, empty portTable
	    if (portTable.nrow() == 0) {
		// the bank comes from BANK keyword in the receive table
		// the channel is the SAMPLERA or SAMPLERB column values
		// for now, assume SAMPLERA == SAMPLERB, will need to deal with
		// the cross-product case eventually
		banks = samplerTable.keywordSet().asString("BANK");
		ROScalarColumn<Short> samplera(samplerTable,"SAMPLERA");
		Vector<Short> sp(samplera.getColumn());
		ROScalarColumn<Double> bwcol(samplerTable,"BANDWIDT");
		bw = bwcol.getColumn();
		for (uInt i=0;i<sp.nelements();i++) ports[i] = sp[i];
	    } else {
		// the sampler index (0 through nsamp) gives a row in samplerTable
		// which gives a value for BANK_A (watch for Bank_A) and PORT_A
		// These in turn give a unique row in portTable which gives BANK
		// and PORT values for that sampler index.
		ROScalarColumn<String> bankACol;
		if (samplerTable.tableDesc().isColumn("BANK_A")) {
		    bankACol.attach(samplerTable, "BANK_A");
		} else {
		    bankACol.attach(samplerTable, "Bank_A");
		}
		ROScalarColumn<Short> portACol(samplerTable,"PORT_A");
		ROScalarColumn<Short> portCol(portTable,"PORT");
		ROScalarColumn<String> bankCol(portTable,"BANK");
		ROScalarColumn<Double> bwCol(portTable,"BANDWDTH");
		for (uInt i=0;i<nsamp;i++) {
		    String thisBankA = bankACol(i);
		    Short thisPortA = portACol(i);
		    
		    banks(i) = "";
		    ports(i) = -1;
		    for (uInt j=0;j<portTable.nrow();j++) {
			if (thisBankA == bankCol(j) && 
			    thisPortA == portCol(j)) {
			    banks(i) = thisBankA;
			    ports(i) = thisPortA;
			    bw(i) = bwCol(j);
			}
		    }
		}
	    }
	    for (uInt i=0;i<nsamp;i++) {
		if (bw(i) == 800e6) {
		    r0(i) = 1600e6;
		    sbMult(i) = -1.0;
		} else if (bw(i) == 200e6) {
		    r0(i) = 800e6;
		    sbMult(i) = 1.0;
		} else if (bw(i) == 50e6) {
		    r0(i) = 100e6;
		    sbMult(i) = -1.0;
		} else {
		    r0(i) = 25e6;
		    sbMult(i) = 1.0;
		}
	    }
	    indgen(indx);
	} else if (ifFiller->backend() == "DCR") {
	    // the bank comes dcrBank, which must be given because it comes 
	    // from the INPBNK keyword in the main table - not available here.
	    // the channel is the CHANNELID column value.
	    banks = dcrBank;
	    ROTableColumn channelid(samplerTable,"CHANNELID");
	    sbMult = 1.0;
	    for (uInt i=0;i<nsamp;i++) {
		// channel index in IF table is 1-relative, index in fits file is 0-relative
		ports[i] = channelid.asInt(i) + 1;
		// r0 is centerIF for the given port
		Int portRow = ifFiller->whichRow(dcrBank, ports[i]);
		if (portRow >= 0) {
		    r0[i] = ifFiller->centerIF()(portRow);
		} else {
		    r0[i] = 0.0;
		}
	    }
	    indgen(indx);	    
	}
	Vector<Int> feedIds(nsamp);
	Vector<Int> recptIds(nsamp);
	Vector<Int> nsamppts(nsamp);
	Vector<Int> whichRow(nsamp);
	Vector<String> recs(nsamp);
	whichRow = -1;
	recs = "";
	feedIds = 0;
	recptIds = 0;
	nsamppts = 1;
	for (uInt i=0;i<nsamp;i++) {
	    Int rid = indx(i);
	    if (rid >= 0 && uInt(rid) < nsamp) {
		whichRow(i) = ifFiller->whichRow(banks(rid), ports(rid));
		if (whichRow(i) >= 0) {
		    // do we need to watch for feedIds to fail?
		    ifFiller->feedIds(whichRow(i), feedIds(i), recptIds(i), nsamppts(i));
		    recs(i) = ifFiller->receiver()(whichRow(i));
		}
	    }
	}
	// make sense of these - count up unique feeds, and rows without feed info
	// how many unique receivers
	SimpleOrderedMap<String, Int> recMap(-1);
	for (uInt i=0;i<nsamp;i++) {
	    if (!recMap.isDefined(recs(i))) {
		// value is an index for that receiver
	      recMap.define(recs(i),recMap.ndefined());
	    }
	}
	// there are recMap.ndefined() receivers - for each receiver, 
	// find the maximum feed id == nfeeds
	Vector<uInt> nfeedsUsed(recMap.ndefined());
	Vector<uInt> sumFeeds(nfeedsUsed.nelements());
	nfeedsUsed = 0;
	sumFeeds = 0;
	for (uInt i=0;i<nsamp;i++) {
	    uInt whichRec = recMap(recs(i));
	    // feedIds are numbered from 0, so a feedId of 0 implies at least 1 feed
	    // and a feedId of 1 implies at least 2, etc.
	    nfeedsUsed(whichRec) = max(Int(nfeedsUsed(whichRec)), feedIds(i)+1);
	}
	// total number of unique feeds is the sum of nfeedsUsed
	itsNfeeds = sum(nfeedsUsed);
	for (uInt i=1;i<nfeedsUsed.nelements();i++) 
	    sumFeeds(i) = sumFeeds(i-1) + nfeedsUsed(i-1);

	// we can finally resize some things
	itsRcvrRows.resize(itsNfeeds);
	// ensure that itsRcvrRows elements are all unset at first
	itsRcvrRows.set((Block<uInt> *)0);

	itsLO1Files.resize(itsNfeeds);
	const GBTLO1File *nullFile = 0;
	itsLO1Files.set(nullFile);

	itsFeedIds.resize(itsNfeeds);
	itsT0CenterFreq.resize(itsNfeeds);
	itsSFFMultiplier.resize(itsNfeeds);
	itsCorrType.resize(itsNfeeds);
	itsPolType.resize(itsNfeeds);
	itsRxName.resize(itsNfeeds);
	itsBank.resize(itsNfeeds);
	itsSRBank.resize(itsNfeeds);
	itsFeedName.resize(itsNfeeds);
	itsSRFeedName.resize(itsNfeeds);
	itsCorrProduct.resize(itsNfeeds);
	itsCenterFreq.resize(itsNfeeds);
	itsDeltaFreq.resize(itsNfeeds);
	itsShapes.resize(itsNfeeds);
	itsFeed.resize(itsNfeeds);
	itsSrfeed1.resize(itsNfeeds);
	itsSrfeed2.resize(itsNfeeds);
	itsHighCal.resize(itsNfeeds);

	// this keeps track of elements in itsSRFeedName that have been set
	// when this counter is == itsSRFeedName[whichFeed].nelements() then
	// they have both been set.
	Vector<Int> srFeedNameCount(itsNfeeds,0);

	Bool hasFeedCol = !ifFiller->feed().isNull();
	itsFeed = itsSrfeed1 = itsSrfeed2 = itsHighCal = -1;

	itsFeedIds = -1;
	itsBank = "";
	itsSRBank = "";

	// one more time through to resize receptor-dependent parts
	Vector<Bool> ok(itsNfeeds);
	ok = False;

	

	for (uInt i=0;i<nsamp;i++) {
	    uInt whichRec = recMap(recs(i));
	    uInt whichFeed = sumFeeds(whichRec) + feedIds(i);
	    Int ifRow = whichRow(i);
	    Matrix<Int> corrProd;
	    if (!ok(whichFeed)) {
		itsFeedIds[whichFeed] = feedIds(i);
		uInt nrcpt = nsamppts(i);
		// unfortunately, nsamppts doesn't seem to always be reliable, so we
		uInt trueNrcpt = 0;
		for (uInt k=0;k<nsamp;k++) {
		    uInt thisRec = recMap(recs(k));
		    uInt thisFeed = sumFeeds(thisRec) + feedIds(k);
		    if (thisFeed == whichFeed) trueNrcpt++;
		}
		if (trueNrcpt != nrcpt) {
		    // need to renumber
		    nrcpt = trueNrcpt;
		    trueNrcpt = 0;
		    for (uInt k=0;k<nsamp;k++) {
			uInt thisRec = recMap(recs(k));
			uInt thisFeed = sumFeeds(thisRec) + feedIds(k);
			if (thisFeed == whichFeed) {
			    recptIds(k) = trueNrcpt++;
			}
		    }
		}
		(itsRcvrRows[whichFeed]) = new Block<uInt>(nrcpt);
		AlwaysAssert(itsRcvrRows[whichFeed], AipsError);
		(itsCorrType[whichFeed]).resize(nrcpt);
		(itsCenterFreq[whichFeed]).resize(itsNphases);
		(itsPolType[whichFeed]).resize(nrcpt);
		(itsFeedName[whichFeed]).resize(nrcpt);
		(itsSRFeedName[whichFeed]).resize(nrcpt);
		(itsCorrProduct[whichFeed]).resize(IPosition(2,2,nrcpt));
		// values which can be filled in at this point
		if (ifRow >= 0) {
		    // get the appropriate values
		    Double sffSideband, sffOffset;
		    ifParms(*ifFiller, ifRow, sffSideband, 
			    itsSFFMultiplier[whichFeed], sffOffset);
		    (itsT0CenterFreq[whichFeed]) = sffSideband*r0[i] + sffOffset;
		    // if bw is zero, fish it out from this table
		    if (bw[i] == 0.0) bw[i] = ifFiller->bandwidth()(ifRow);
		    itsDeltaFreq[whichFeed] = 
			sbMult[i] * sffSideband * bw(i) / itsNchannels;
		    // protect against bad values from the ifFiller
		    if (isNaN(itsT0CenterFreq[whichFeed]) ||
			isNaN(itsDeltaFreq[whichFeed])) {
			itsT0CenterFreq[whichFeed] = itsRefChan;
			itsDeltaFreq[whichFeed] = 1.0;
			itsSFFMultiplier[whichFeed] = 0.0;
		    } else {
			// shift itsT0CenterFreq to itsRefChan from channel 0 
			// using itsDeltaFreq
			itsT0CenterFreq[whichFeed] +=
			    itsDeltaFreq[whichFeed] * itsRefChan;
		    }
		    // which LO1 file?
		    if (!ifFiller->loCircuit().isNull() &&
			ifFiller->loCircuit()(ifRow) == "LO1B") {
			// LO1B
			itsLO1Files[whichFeed] = lo1BFile;
		    } else {
			// otherwise just default to LO1A
			// Should print out an error message if it isn't LO1A
			itsLO1Files[whichFeed] = lo1AFile;
		    }
		} else {
		    itsSFFMultiplier[whichFeed] = 0.0;
		    itsT0CenterFreq[whichFeed] = itsRefChan;
		    itsDeltaFreq[whichFeed] = 1.0;
		    itsSFFMultiplier[whichFeed] = 0.0;
		    // no LO1 file
		    itsLO1Files[whichFeed] = nullFile;
		}
   		corrProd.reference(itsCorrProduct[whichFeed]);
		for (uInt i=0;i<nrcpt;i++) {
		    corrProd.column(i) = i;
		}
		(itsCenterFreq[whichFeed]) = 0.0;
		itsShapes[whichFeed] = IPosition(2,nrcpt,itsNchannels);
		ok(whichFeed) = True;
	    }
	    // fill in appropriate values
	    (*(itsRcvrRows[whichFeed]))[recptIds(i)] = i;
	    if (ifRow >= 0) {
		String polString = ifFiller->polarize()(ifRow);
		if (polString.length() == 0) {
		    // attempt to use the feed name for that row
                    polString = ifFiller->receptor()(ifRow);
		    polString = polString[0];
		    if (polString.length() == 0) polString = "X";
		} 
		(itsPolType[whichFeed])(recptIds(i)) = polString;
		(itsCorrType[whichFeed])(recptIds(i)) = Int(Stokes::type(polString+polString));
		(itsFeedName[whichFeed])(recptIds(i)) = ifFiller->receptor()(ifRow);
		if (itsRxName[whichFeed]=="") {
		    itsRxName[whichFeed] = ifFiller->receiver()(ifRow);
		}
		if (itsBank[whichFeed]=="") {
		    itsBank[whichFeed] = ifFiller->bank()(ifRow);
		}
		if (hasFeedCol) {
		    if (itsFeed[whichFeed] == -1) {
			itsFeed[whichFeed] = ifFiller->feed()(ifRow);
			itsSrfeed1[whichFeed] = ifFiller->srfeed1()(ifRow);
			itsSrfeed2[whichFeed] = ifFiller->srfeed2()(ifRow);
			itsHighCal[whichFeed] = ifFiller->highCal()(ifRow);
		    }

		    // look for other feed in IF table if this is
		    // ACS data AND we haven't already seen
		    // all receptors for itsSRFeedName
		    if (srFeedNameCount[whichFeed] < Int(itsSRFeedName[whichFeed].nelements())) {
			// is this switched data
			Bool srfound = False;
			if (itsSrfeed1[whichFeed] > 0 &&
			    itsSrfeed2[whichFeed] > 0) {
			    // yes, look for a match in the other ifrows
			    Int otherFeed = itsSrfeed2[whichFeed];
			    if (itsFeed[whichFeed] == otherFeed) 
				otherFeed = itsSrfeed1[whichFeed];
			    Int thisSR1 = itsSrfeed1[whichFeed];
			    Int thisSR2 = itsSrfeed2[whichFeed];
			    String thisRec = ifFiller->receiver()(ifRow);
			    String thisPol = ifFiller->polarize()(ifRow);
			    Double thisMult = ifFiller->sffMultiplier()(ifRow);
			    Double thisSB = ifFiller->sffSideband()(ifRow);
			    Double thisOff = ifFiller->sffOffset()(ifRow);
			    Double thisBW = ifFiller->bandwidth()(ifRow);
			    for (uInt otherRow=0;otherRow<ifFiller->currentTable().nrow();
				 otherRow++) {
				// skip this row
				if (otherRow != uInt(ifRow)) {
				    // match srfeed1, srfeed2, receptor, polarize,
				    // sffMultiplier, sffSideband, sffOffset, bandwidth
				    if (ifFiller->srfeed1()(otherRow) == thisSR1 &&
					ifFiller->srfeed2()(otherRow) == thisSR2 &&
					ifFiller->receiver()(otherRow) == thisRec &&
					ifFiller->polarize()(otherRow) == thisPol &&
					ifFiller->sffMultiplier()(otherRow) == thisMult &&
					ifFiller->sffSideband()(otherRow) == thisSB &&
					ifFiller->sffOffset()(otherRow) == thisOff &&
					ifFiller->bandwidth()(otherRow) == thisBW) {
					srfound = True;
					if (ifFiller->backend() == "Spectrometer") {
					    itsSRBank[whichFeed] = ifFiller->bank()(otherRow);
					}
					for (uInt whichPol=0;whichPol<itsPolType[whichFeed].nelements();whichPol++) {
					    if ((itsPolType[whichFeed])[whichPol] == thisPol &&
						(itsSRFeedName[whichFeed])[whichPol] == "") {
						(itsSRFeedName[whichFeed])[whichPol] = ifFiller->receptor()(otherRow);
						srFeedNameCount[whichFeed]++;
					    }
					}
				    }
				}
			    }
			    if (!srfound) {
				if (ifFiller->backend() == "Spectrometer") itsSRBank[whichFeed] = itsBank[whichFeed];
				itsSRFeedName[whichFeed] = itsFeedName[whichFeed];
			    }
			} else {
			    // no, just use this bank
			    if (ifFiller->backend() == "Spectrometer") itsSRBank[whichFeed] = itsBank[whichFeed];
			    itsSRFeedName[whichFeed] = itsFeedName[whichFeed];
			}
		    }
		}
	    } else {
		itsPolType[whichFeed] = "X";
		itsCorrType[whichFeed] = Int(Stokes::type("XX"));
		itsFeedName[whichFeed] = "";
		itsSRFeedName[whichFeed] = "";
		itsRxName[whichFeed] = "";
		itsBank[whichFeed] = "";
		itsSRBank[whichFeed] = "";
	    }
	}
	// try and ensure "XX" comes before "YY" and "LL" before "RR"
	for (uInt i=0;i<nfeeds();i++) {
	    // 2 receptors?
	    if (polarizationType(i).nelements() == 2) {
		String pol0 = polarizationType(i)(0);
		if (pol0 == "Y" || pol0 == "R") {
		    // a switch is needed
		    uInt row0 = receiverRows(i)[0];
		    Int type0 = corrType(i)(0);
		    String feed0 = feedName(i)(0);
		    (*itsRcvrRows[i])[0] = receiverRows(i)[1];
		    (itsCorrType[i])(0) = corrType(i)(1);
		    (itsPolType[i])(0) = polarizationType(i)(1);
		    (itsFeedName[i])(0) = feedName(i)(1);
		    (itsSRFeedName[i])(0) = feedName(i)(1);
		    (*itsRcvrRows[i])[1] = row0;
		    (itsCorrType[i])(1) = type0;
		    (itsPolType[i])(1) = pol0;
		    (itsFeedName[i])(1) = feed0;
		    (itsSRFeedName[i])(1) = feed0;
		}
	    } // we can't have cross products yet, don't worry about it right now
	}
    } else {
	// nothing there to translate - every row in receiver table is its own feed
	// with a single receptor
	// resize
	itsNfeeds = samplerTable.nrow();
	itsRcvrRows.resize(itsNfeeds);
	// ensure that itsRcvrRows elements are all unset at first
	itsRcvrRows.set((Block<uInt> *)0);

	itsLO1Files.resize(itsNfeeds);
	// assume LO1A is appropriate - won't matter ultimately
	itsLO1Files.set(lo1AFile);

	itsFeedIds.resize(itsNfeeds);
	itsT0CenterFreq.resize(itsNfeeds);
	itsSFFMultiplier.resize(itsNfeeds);
	itsCorrType.resize(itsNfeeds);
	itsPolType.resize(itsNfeeds);
	itsRxName.resize(itsNfeeds);
	itsBank.resize(itsNfeeds);
	itsSRBank.resize(itsNfeeds);
	itsFeedName.resize(itsNfeeds);
	itsSRFeedName.resize(itsNfeeds);
	itsCorrProduct.resize(itsNfeeds);
	itsCenterFreq.resize(itsNfeeds);
	itsDeltaFreq.resize(itsNfeeds);
	itsShapes.resize(itsNfeeds);
	itsFeed.resize(itsNfeeds);
	itsSrfeed1.resize(itsNfeeds);
	itsSrfeed2.resize(itsNfeeds);
	itsHighCal.resize(itsNfeeds);

	// set values
	itsT0CenterFreq = itsRefChan;
	itsSFFMultiplier = 0.0;
	itsDeltaFreq = 1.0;
	itsFeed = itsSrfeed1 = itsSrfeed2 = itsHighCal = -1;
	itsRxName = "";
	itsBank = "";
	itsSRBank = "";
	itsShapes = IPosition(2,1,itsNchannels);
	for (uInt i=0;i<itsNfeeds;i++) {
	    (itsRcvrRows[i]) = new Block<uInt>(1);
	    AlwaysAssert(itsRcvrRows[i], AipsError);
	    *(itsRcvrRows[i]) = i;
	    itsFeedIds[i] = i;
	    (itsCorrType[i]).resize(1);
	    (itsCenterFreq[i]).resize(itsNphases);
	    (itsPolType[i]).resize(1);
	    (itsFeedName[i]).resize(1);
	    (itsSRFeedName[i]).resize(1);
	    (itsCorrProduct[i]).resize(IPosition(2,2,1));
	    itsCorrProduct[i]  = 0;
	    itsCenterFreq[i]  = 0.0;
	    itsPolType[i] = "X";
	    itsCorrType[i] = Int(Stokes::type("XX"));
	    itsFeedName[i] = "";
	    itsSRFeedName[i] = "";
	}
    }
    // set itsSamplerFeeds for all of the itsFeedIds >= 0
    // first pass, just count;
    uInt samplerFeedCount = 0;
    for (uInt i=0;i<itsNfeeds;i++) {
	if (itsFeedIds[i] >= 0) {
	    samplerFeedCount++;
	}
    }
    itsSamplerFeeds.resize(samplerFeedCount);
    samplerFeedCount = 0;
    for (uInt i=0;i<itsNfeeds;i++) {
	if (itsFeedIds[i] >= 0) {
	    itsSamplerFeeds[samplerFeedCount++] = i;
	}
    }
}

GBTIFLOHelper::~GBTIFLOHelper()
{
    for (uInt i=0; i<itsNfeeds; i++) {
	delete itsRcvrRows[i];
	itsRcvrRows[i] = 0;
    }
}

const Vector<Double> &GBTIFLOHelper::centerFreqs(uInt whichFeed, 
						 MFrequency::Types &refframe,
						 MDoppler::Types &doptype,
						 Double &souvel)
{
    refframe = MFrequency::TOPO;
    doptype =  MDoppler::RADIO;
    souvel = 0.0;
    Double offset = 0.0;
    if (itsLO1Files[whichFeed] || itsLO1Filler) {
	if (itsLO1Files[whichFeed]) {
	    Vector<Double> firstLO1(itsLO1Files[whichFeed]->firstLO1(offset,
								     refframe,
								     doptype,
								     souvel));
	    if (firstLO1.nelements() == itsNphases) {
		itsLO1offset = firstLO1;
		// this is multiplied by sffMultiplier for that feed and added to the rest of
		// the frequency calculation for that feed
		itsCenterFreq[whichFeed] = itsT0CenterFreq[whichFeed] + itsSFFMultiplier[whichFeed] * itsLO1offset;
		// add the offset which converts this from TOPO to refframe
		itsCenterFreq[whichFeed] += offset;
	    } else {
		// all bets are off.
		// should emit an error here
		itsCenterFreq[whichFeed] = 1.0;
		refframe = MFrequency::TOPO;
		doptype = MDoppler::RADIO;
		souvel = 0.0;
	    }
	}
    } else {
      // without the LO filler, this is rather pointless.  Just set it to 1;
	itsCenterFreq[whichFeed] = 1;
    }
    return itsCenterFreq[whichFeed];
}

void GBTIFLOHelper::ifParms(GBTIFFiller&ifFiller,
			    Int whichRow, Double &sffSideband, Double &sffMultiplier,
			    Double &sffOffset) 
{
    // start from the defaults
    sffSideband = ifFiller.sffSideband()(whichRow);
    sffMultiplier = ifFiller.sffMultiplier()(whichRow);
    sffOffset = ifFiller.sffOffset()(whichRow);
    if (itsLO1Filler) {
	// can only double check if there is a frequency
	Double lo1 = itsLO1Filler->initialFreq();
	Double centerSky = ifFiller.centerSky()(whichRow);
	Double centerIF = ifFiller.centerIF()(whichRow);
	Double diff = abs(sffSideband*centerIF + sffMultiplier*lo1 + sffOffset - centerSky);
	if (diff > centerIF) {
	    // there's a problem, two possible solutions
	    // first, flip sign of sffOffset
	    sffOffset = -sffOffset;
	    diff = abs(sffSideband*centerIF + sffMultiplier*lo1 + sffOffset - centerSky);
	    if (diff > centerIF) {
		// flip sffSideband
		sffSideband = -sffSideband;
		diff = abs(sffSideband*centerIF + sffMultiplier*lo1 + sffOffset - centerSky);
		if (diff > centerIF) {
		    // one last chance, fflip sffOffset back
		    sffOffset = -sffOffset;
		    diff = abs(sffSideband*centerIF + sffMultiplier*lo1 +sffOffset - centerSky);
		    if (diff > centerIF) {
			// just return the originals
			sffSideband = -sffSideband;
		    }
		}
	    }
	}
    }
}
