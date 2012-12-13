//# GBTIFFiller.cc: A filler for the GBT IF Manager FITS files.
//# Copyright (C) 2000,2001,2002,2003
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

#include <nrao/GBTFillers/GBTIFFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <fits/FITS/FITSTable.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/ColumnsIndex.h>
#include <tables/Tables/TableDesc.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Regex.h>

GBTIFFiller::GBTIFFiller(Table &parent, const String &backend)
    : itsMSTable(0), itsIndex(0), itsLastRow(-1), itsBackend(backend),
      itsSingleReceptor(False),
      itsMaxFeedIdMap(-1), itsFeedIdMap(-1), itsValid(False)
{
    // new or already existing?
    if (parent.keywordSet().fieldNumber("NRAO_GBT_IF") < 0) {
	// it does not yet exist yet
	itsMSTable = new GBTSimpleTable(parent, "NRAO_GBT_IF", "NRAO_GBT_IF", "NRAO_GBT_IF_ID");
    } else {
	// it does exist, reopen it
	itsMSTable = new GBTSimpleTable(parent.tableName() + "/NRAO_GBT_IF", "NRAO_GBT_IF_ID");
    }
    AlwaysAssert(itsMSTable, AipsError);

    if (itsBackend == "DCR") itsSingleReceptor = True;
}

GBTIFFiller::~GBTIFFiller() 
{
    delete itsMSTable;
    itsMSTable = 0;

    delete itsIndex;
    itsIndex = 0;
}

Int GBTIFFiller::fill(const String &fileName)
{
    LogIO os;
    os << LogOrigin("GBTIFFiller","fill(const String &fileName)");

    delete itsIndex;
    itsIndex = 0;
    itsLastRow = -1;
    itsValid = True;

    // open this as a FITSTable
    FITSTable fitsTab(fileName);
    if (fitsTab.isValid()) {
	// and fill it 
	RORecordFieldPtr<String> backendCol;
	// old version uses all lowercase letters
	if (fitsTab.currentRow().fieldNumber("BACKEND") >= 0) {
	    backendCol.attachToRecord(fitsTab.currentRow(),"BACKEND");
	} else if (fitsTab.currentRow().fieldNumber("backend") >= 0) {
	    backendCol.attachToRecord(fitsTab.currentRow(),"backend");
	} else {
	    itsValid = False;
	    os << LogIO::SEVERE
	       << "IF FITS BACKEND column does not exist in "
	       << fileName
	       << LogIO::POST;
	}
	uInt rownr = 0;
	// remember where this started from
	uInt startRow = itsMSTable->nrow();

	while (rownr < fitsTab.nrow()) {
	    if (!backendCol.isAttached() || 
		*backendCol == itsBackend) {
		itsMSTable->add(fitsTab.currentRow(),(rownr==0));
	    }
	    fitsTab.next();
	    rownr++;
	}

	// get a reference to the subtable we just filled
	uInt nrow = itsMSTable->nrow() - startRow;
	Vector<uInt> theseRows(nrow);
	indgen(theseRows, startRow);
	itsCurrTable = itsMSTable->table()(theseRows);

	if (nrow > 0) {
	    // attach the rows, emit a SEVERE warning whenever one doesn't exist
	    if (itsCurrTable.tableDesc().isColumn("RECEIVER") &&
		itsCurrTable.tableDesc()["RECEIVER"].dataType() == TpString) {
		itsReceiverCol.attach(itsCurrTable,"RECEIVER");
	    } else if (itsCurrTable.tableDesc().isColumn("receiver") &&
		itsCurrTable.tableDesc()["receiver"].dataType() == TpString) {
		itsReceiverCol.attach(itsCurrTable,"receiver");
	    } else {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS RECEIVER column does not exist or is of wrong type in "
		   << fileName
		   << LogIO::POST;
	    }
	    if (itsCurrTable.tableDesc().isColumn("RECEPTOR") &&
		itsCurrTable.tableDesc()["RECEPTOR"].dataType() == TpString) {
		itsReceptorCol.attach(itsCurrTable,"RECEPTOR");
	    } else if (itsCurrTable.tableDesc().isColumn("feed") &&
		itsCurrTable.tableDesc()["feed"].dataType() == TpString) {
		// old FITSVER values had a mostly equivalent FEED column
		itsReceptorCol.attach(itsCurrTable,"feed");
	    } else {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS RECEPTOR column does not exist or is of wrong type in "
		   << fileName
		   << LogIO::POST;
	    }
	    // LO_CIRCUIT is only in the new version, don't emit a warning for now
	    if (itsCurrTable.tableDesc().isColumn("LO_CIRCUIT") &&
		itsCurrTable.tableDesc()["LO_CIRCUIT"].dataType() == TpString) {
		itsLOCircuit.attach(itsCurrTable,"LO_CIRCUIT");
	    }
	    // LO_COMPONENT is only in the new version, don't emit a warning for now
	    if (itsCurrTable.tableDesc().isColumn("LO_COMPONENT") &&
		itsCurrTable.tableDesc()["LO_COMPONENT"].dataType() == TpString) {
		itsLOComponent.attach(itsCurrTable,"LO_COMPONENT");
	    }
	    if (itsCurrTable.tableDesc().isColumn("SIDEBAND") &&
		itsCurrTable.tableDesc()["SIDEBAND"].dataType() == TpString) {
		itsSidebandCol.attach(itsCurrTable,"SIDEBAND");
	    } else if (itsCurrTable.tableDesc().isColumn("sideband") &&
		itsCurrTable.tableDesc()["sideband"].dataType() == TpString) {
		itsSidebandCol.attach(itsCurrTable,"sideband");
	    } else {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS SIDEBAND column does not exist or is of wrong type in "
		   << fileName
		   << LogIO::POST;
	    }
	    if (itsCurrTable.tableDesc().isColumn("POLARIZE") &&
		itsCurrTable.tableDesc()["POLARIZE"].dataType() == TpString) {
		itsPolarizeCol.attach(itsCurrTable,"POLARIZE");
	    } else if (itsCurrTable.tableDesc().isColumn("polarize") &&
		itsCurrTable.tableDesc()["polarize"].dataType() == TpString) {
		itsPolarizeCol.attach(itsCurrTable,"polarize");
	    } else {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS POLARIZE column does not exist or is of wrong type in "
		   << fileName
		   << LogIO::POST;
	    }
	    if (itsCurrTable.tableDesc().isColumn("CENTER_IF") &&
		itsCurrTable.tableDesc()["CENTER_IF"].dataType() == TpFloat) {
		itsCenterIfCol.attach(itsCurrTable,"CENTER_IF");
	    } else if (itsCurrTable.tableDesc().isColumn("center_IF") &&
		itsCurrTable.tableDesc()["center_IF"].dataType() == TpFloat) {
		itsCenterIfCol.attach(itsCurrTable,"center_IF");
	    } else {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS CENTER_IF column does not exist or is of wrong type in "
		   << fileName
		   << LogIO::POST;
	    }
	    if (itsCurrTable.tableDesc().isColumn("CENTER_SKY") &&
		itsCurrTable.tableDesc()["CENTER_SKY"].dataType() == TpFloat) {
		itsCenterSkyCol.attach(itsCurrTable,"CENTER_SKY");
	    } else if (itsCurrTable.tableDesc().isColumn("center_sky") &&
		itsCurrTable.tableDesc()["center_sky"].dataType() == TpFloat) {
		itsCenterSkyCol.attach(itsCurrTable,"center_sky");
	    } else {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS CENTER_SKY column does not exist or is of wrong type in "
		   << fileName
		   << LogIO::POST;
	    }
	    if (itsCurrTable.tableDesc().isColumn("BANDWDTH") &&
		itsCurrTable.tableDesc()["BANDWDTH"].dataType() == TpFloat) {
		itsBandwidthCol.attach(itsCurrTable,"BANDWDTH");
	    } else if (itsCurrTable.tableDesc().isColumn("bandwidth") &&
		itsCurrTable.tableDesc()["bandwidth"].dataType() == TpFloat) {
		itsBandwidthCol.attach(itsCurrTable,"bandwidth");
	    } else {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS BANDWDTH column does not exist or is of wrong type in "
		   << fileName
		   << LogIO::POST;
	    }
	    if (itsCurrTable.tableDesc().isColumn("SFF_MULTIPLIER") &&
		itsCurrTable.tableDesc()["SFF_MULTIPLIER"].dataType() == TpDouble) {
		itsSffMultiplierCol.attach(itsCurrTable,"SFF_MULTIPLIER");
	    } else if (itsCurrTable.tableDesc().isColumn("SFF_multiplier") &&
		itsCurrTable.tableDesc()["SFF_multiplier"].dataType() == TpDouble) {
		itsSffMultiplierCol.attach(itsCurrTable,"SFF_multiplier");
	    } else {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS SFF_MULTIPLIER column does not exist or is of wrong type in "
		   << fileName
		   << LogIO::POST;
	    }
	    if (itsCurrTable.tableDesc().isColumn("SFF_SIDEBAND") &&
		itsCurrTable.tableDesc()["SFF_SIDEBAND"].dataType() == TpDouble) {
		itsSffSidebandCol.attach(itsCurrTable,"SFF_SIDEBAND");
	    } else if (itsCurrTable.tableDesc().isColumn("SFF_sideband") &&
		itsCurrTable.tableDesc()["SFF_sideband"].dataType() == TpDouble) {
		itsSffSidebandCol.attach(itsCurrTable,"SFF_sideband");
	    } else {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS SFF_SIDEBAND column does not exist or is of wrong type in "
		   << fileName
		   << LogIO::POST;
	    }
	    if (itsCurrTable.tableDesc().isColumn("SFF_OFFSET") &&
		itsCurrTable.tableDesc()["SFF_OFFSET"].dataType() == TpDouble) {
		itsSffOffsetCol.attach(itsCurrTable,"SFF_OFFSET");
	    } else if (itsCurrTable.tableDesc().isColumn("SFF_offset") &&
		itsCurrTable.tableDesc()["SFF_offset"].dataType() == TpDouble) {
		itsSffOffsetCol.attach(itsCurrTable,"SFF_offset");
	    } else {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS SFF_OFFSET column does not exist or is of wrong type in "
		   << fileName
		   << LogIO::POST;
	    }

	    if (itsCurrTable.tableDesc().isColumn("FEED")) {
		if (itsCurrTable.tableDesc()["FEED"].dataType() == TpInt) {
		    itsFeedCol.attach(itsCurrTable,"FEED");
		} else {
		    itsValid = False;
		    os << LogIO::SEVERE
		       << "IF FITS FEED column is of wrong type in "
		       << fileName
		       << LogIO::POST;
		}
	    } // else this is an older FITS file and it isn't present, leave unattached

	    if (itsCurrTable.tableDesc().isColumn("SRFEED1")) {
		if (itsCurrTable.tableDesc()["SRFEED1"].dataType() == TpInt) {
		    itsSRFeed1Col.attach(itsCurrTable,"SRFEED1");
		} else {
		    itsValid = False;
		    os << LogIO::SEVERE
		       << "IF FITS SRFEED1 column is of wrong type in "
		       << fileName
		       << LogIO::POST;
		}
	    } // else this is an older FITS file and it isn't present, leave unattached

	    if (itsCurrTable.tableDesc().isColumn("SRFEED2")) {
		if (itsCurrTable.tableDesc()["SRFEED2"].dataType() == TpInt) {
		    itsSRFeed2Col.attach(itsCurrTable,"SRFEED2");
		} else {
		    itsValid = False;
		    os << LogIO::SEVERE
		       << "IF FITS SRFEED2 column is of wrong type in "
		       << fileName
		       << LogIO::POST;
		}
	    } // else this is an older FITS file and it isn't present, leave unattached

	    if (itsCurrTable.tableDesc().isColumn("HIGH_CAL")) {
		if (itsCurrTable.tableDesc()["HIGH_CAL"].dataType() == TpInt) {
		    itsHighCalCol.attach(itsCurrTable,"HIGH_CAL");
		} else {
		    itsValid = False;
		    os << LogIO::SEVERE
		       << "IF FITS HIGH_CAL column is of wrong type in "
		       << fileName
		       << LogIO::POST;
		}
	    } // else this is an older FITS file and it isn't present, leave unattached

	    // bank column - leave unattached if not present - it should not be used
	    // in that case
	    if (itsCurrTable.tableDesc().isColumn("BANK") &&
		itsCurrTable.tableDesc()["BANK"].dataType() == TpString) {
		itsBankCol.attach(itsCurrTable,"BANK");
	    } else if (itsCurrTable.tableDesc().isColumn("bank") &&
		itsCurrTable.tableDesc()["bank"].dataType() == TpString) {
		itsBankCol.attach(itsCurrTable,"bank");
	    }

	    // and set up the index
	    // emit a SEVERE warning if the two expected columns don't exist
	    Bool okToIndex = True;
	    if (fitsTab.currentRow().fieldNumber("BANK") < 0 &&
		fitsTab.currentRow().fieldNumber("bank") < 0) {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS BANK column does not exist in "
		   << fileName
		   << LogIO::POST;
		okToIndex = False;
	    }
	    if (fitsTab.currentRow().fieldNumber("PORT") < 0 &&
		fitsTab.currentRow().fieldNumber("channel") < 0) {
		itsValid = False;
		os << LogIO::SEVERE
		   << "IF FITS PORT column does not exist in "
		   << fileName
		   << LogIO::POST;
		okToIndex = False;
	    }
	    if (okToIndex) {
		// look for the PORT column
		Bool oldStyle = False;
		if (fitsTab.currentRow().fieldNumber("PORT") >= 0) {
		    itsIndex = new ColumnsIndex(itsCurrTable, stringToVector("BANK,PORT"));
		} else {
		    itsIndex = new ColumnsIndex(itsCurrTable, stringToVector("bank,channel"));
		    oldStyle = True;
		}
		AlwaysAssert(itsIndex, AipsError);

		// and attach the keys
		if (!oldStyle) {
		    itsBankKey.attachToRecord(itsIndex->accessKey(),"BANK");
		    itsPortKey.attachToRecord(itsIndex->accessKey(),"PORT");
		} else {
		    itsBankKey.attachToRecord(itsIndex->accessKey(),"bank");
		    itsPortKey.attachToRecord(itsIndex->accessKey(),"channel");
		}
		
		*itsBankKey = "";
		*itsPortKey = -1;
	    }
	} else {
	    itsCurrTable = Table();
	    itsValid = False;
	}
    } else {
	itsCurrTable = Table();
	itsValid = False;
    }
    digestTable();

    return (itsCurrTable.isNull() ? -1 : itsMSTable->index());
}

Int GBTIFFiller::whichRow(const String &bank, Int port) const
{
    if (itsIndex) {
	if (itsLastRow < 0 || bank != *itsBankKey || port != *itsPortKey) {
	    *itsBankKey = bank;
	    *itsPortKey = port;
	    // find a new row
	    if (itsIndex->isUnique()) {
		Bool found;
		uInt matchedRow = itsIndex->getRowNumber(found);
		itsLastRow = found ? Int(matchedRow) : -1;
	    } else {
		Vector<uInt> rows(itsIndex->getRowNumbers());
		itsLastRow = (rows.nelements()>0) ? Int(rows(0)) : -1;
		LogIO os;
		os << LogIO::SEVERE
		   << LogOrigin("GBTIFFiller","whichRow(const String &bank, Int port)")
		   << "BANK = " << bank
		   << " and PORT = " << port
		   << " are not unique keys as they should be.  Using first row found"
		   << LogIO::POST;
	    }
	}
    }
    return itsLastRow;
}

Bool GBTIFFiller::feedIds(Int whichRow, Int &feedId, Int &receptorId, 
			  Int &nReceptors) const {
    Bool result = False;
    if (!receiver().isNull() && Int(receiver().nrow())>whichRow) {
	feedId = itsFeedIDs[whichRow];
	receptorId = itsReceptorIDs[whichRow];
	nReceptors = itsNReceptors[whichRow];
	result = True;
    }
    return result;
}

void GBTIFFiller::unknownReceiver(const String &rec, Int whichRow, Int &feedId)
{
    // has this row been seen before
    if (itsFeedIdMap.isDefined(whichRow)) {
	// just use that
	feedId = itsFeedIdMap(whichRow);
    } else {
	// has this receiver been seen before
	if (itsMaxFeedIdMap.isDefined(rec)) {
	    // this is the next one
	    feedId = itsMaxFeedIdMap(rec) + 1;
	} else {
	    // completely new
	    feedId = 0;
	}
	itsMaxFeedIdMap.define(rec, feedId);
	itsFeedIdMap.define(whichRow, feedId);
    }
}

void GBTIFFiller::digestTable()
{
    // resize things
    uInt nrow = itsCurrTable.nrow();

    itsFeedIDs.resize(nrow);
    itsReceptorIDs.resize(nrow);
    itsNReceptors.resize(nrow);
    itsIFs.resize(nrow);
    itsRefChans.resize(nrow);
    itsDeltaFreqs.resize(nrow);
    itsNchans.resize(nrow);
    itsIFs = 0.0;
    itsRefChans = 0.0;
    itsDeltaFreqs = 0.0;
    itsNchans = 0;
    // first pass, count how many unique receivers there are
    SimpleOrderedMap<String, Int> receivers(-1);
    for (uInt i=0;i<nrow;i++) {
	String rec = receiver()(i);
	if (!receivers.isDefined(rec)) {
	    // not seen yet
	    receivers.define(rec, receivers.ndefined());
	}
    }
    // second pass, ensure that receptor IDs are unique for each feedID for each receiver
    // currently, I think should be adequately large enough - for any receiver
    // there shouldn't be more than 16 feeds and 4 receptors.  This is all ad-hoc at any
    // rate, the final version won't use any of this code.  The value at any cell is
    // the row number containing that feed,receptor,receiver.
    Int maxFeedId = 16;
    Int maxReceptorId = 4;
    Cube<Int> inUse(maxFeedId, maxReceptorId, receivers.ndefined(), -1);
    for (uInt i=0;i<nrow;i++) {
	String rec = receiver()(i);
	String feedName = receptor()(i);
	Int receiverId = receivers(rec);
	Int feedId, receptorId, nReceptors;
	feedId = receptorId = nReceptors = 0;
	if (rec == "Rcvr12_18" || rec == "Rcvr18_26" || rec == "Rcvr40_52") {
	    // hopefully this is the way they will all eventually be
	    // "R" and "L" + 1,2,3, or 4
	    nReceptors = 2;
	    if (feedName.matches(Regex("^[RL][1-4]"))) {
		char rcpt = feedName[0];
		char beam = feedName[1];
		if (rcpt == 'L') receptorId = 0;
		else receptorId = 1;
		if (beam == '1') feedId = 0;
		else if (beam == '2') feedId = 1;
		else if (beam == '3') feedId = 2;
		else feedId = 3;
	    } else {
		unknownReceiver(rec, i, feedId);
		receptorId = 0;
		nReceptors = 1;
	    }
	} else if (rec == "Rcvr1_2" || rec == "Rcvr2_3" || rec == "Rcvr4_6") {
	    nReceptors = 2;
	    feedId = 0;
	    if (feedName.matches(Regex("^[XY][RL]"))) {
		// single feed
		if (feedName[0] == 'X' || feedName[0] == 'R') receptorId = 0;
		else receptorId = 1;
	    } else {
		unknownReceiver(rec, i, feedId);
		receptorId = 0;
		nReceptors = 1;
	    }
	} else if (rec == "Rcvr8_10") {
	    nReceptors = 2;
	    feedId = 0;
	    if (feedName.matches(Regex("^[RL]"))) {
		if (feedName[0] == 'L') receptorId = 0;
		else receptorId = 1;
	    } else {
		unknownReceiver(rec, i, feedId);
		receptorId = 0;
		nReceptors = 1;
	    }
	} else if (rec == "RcvrPF_1") {
	    if (feedName.matches(Regex(".*342.*")) ||
		feedName.matches(Regex(".*600.*")) ||
		feedName.matches(Regex(".*800.*"))) {
		nReceptors = 2;
		if (feedName[0] == 'X') receptorId = 0;
		else receptorId = 1;
	    } else {
		nReceptors = 1;
		unknownReceiver(rec, i, feedId);
	receptorId = 0;
		nReceptors = 1;
	    }
	} else {
	    unknownReceiver(rec, i, feedId);
	    receptorId = 0;
	    nReceptors = 1;
	}

	// the DCR is special - it only fills one recepter per feed.
	// Take care of that labelling here.  itsSingleReceptor is only
	// true if this is from the DCR
	if (itsSingleReceptor) {
	    feedId = feedId*2 + receptorId;
	    receptorId = 0;
	    nReceptors = 1;
	}

	// now ensure unique ones for indexing purposes
	if (inUse(feedId, receptorId, receiverId) >= 0) {
	    // this one has already been seen, find the next highest feedId that is unused
	    // for this receiverId and receptorId
	    for (Int k=(feedId+1);k<maxFeedId;k++) {
		if (inUse(k, receptorId, receiverId) < 0) {
		    // found one
		    feedId = k;
		    inUse(feedId, receptorId, receiverId) = i;
		    break;
		}
	    }
	} else {
	    inUse(feedId, receptorId, receiverId) = i;
	}
    }
    // summarize inUse ...
    for (uInt i=0;i<receivers.ndefined();i++) {
	Matrix<Int> ids(inUse.xyPlane(i));
	Int feedId, receptorId;
	feedId = -1;
	for (Int j=0;j<maxFeedId;j++) {
	    Vector<Int> rcpts(ids.row(j));
	    if (anyGT(rcpts,-1)) {
		feedId++;
		// two passes, first make sure no gaps
		receptorId = -1;
		for (Int k=0;k<maxReceptorId;k++) {
		    if (rcpts(k) >= 0) {
			receptorId++;
			if (receptorId < k) {
			    rcpts(receptorId) = rcpts(k);
			}
		    }
		}
		// then assign for posterity
		Int nrcpt = receptorId+1;
		for (Int k=0;k<nrcpt;k++) {
		    Int row = rcpts(k);
		    itsFeedIDs[row] = feedId;
		    itsReceptorIDs[row] = k;
		    itsNReceptors[row] = nrcpt;
		}
	    }
	}
    }
}

Bool GBTIFFiller::associateIF(const String &bank, Int port,
			      Double ifFreq, Double refChan, Double deltaFreq, 
			      Int nchan)
{
    Int thisRow = whichRow(bank, port);
    Bool result = thisRow >= 0;
    if (result) { 
	itsIFs[thisRow] = ifFreq;
	itsRefChans[thisRow] = refChan;
	itsDeltaFreqs[thisRow] = deltaFreq;
	itsNchans[thisRow] = nchan;
    }
    return result;
}

Bool GBTIFFiller::getIF(const String &bank, Int port,
			Double &ifFreq, Double &refChan, Double &deltaFreq, 
			Int &nchan)
{
    Int thisRow = whichRow(bank, port);
    Bool result = thisRow >= 0;
    if (result) { 
	ifFreq = itsIFs[thisRow];
	refChan = itsRefChans[thisRow];
	deltaFreq = itsDeltaFreqs[thisRow];
	nchan = itsNchans[thisRow];
    }
    return result;
}
