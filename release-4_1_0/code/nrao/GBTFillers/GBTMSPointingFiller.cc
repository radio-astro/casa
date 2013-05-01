//# GBTPointingDAPFiller.cc: fill the POINTING table for GBT fillers
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

#include <nrao/GBTFillers/GBTMSPointingFiller.h>
#include <nrao/GBTFillers/GBTMeanFocusFiller.h>
#include <nrao/GBTFillers/GBTPointModelFiller.h>
#include <nrao/GBTFillers/GBTAntennaDAPFiller.h>
#include <nrao/FITS/GBTAntennaFile.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/Constants.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSPointing.h>
#include <ms/MeasurementSets/MSPointingColumns.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableVector.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Fallible.h>
#include <casa/BasicSL/String.h>

GBTMSPointingFiller::GBTMSPointingFiller()
    : msPointing_p(0), msPointingCols_p(0), antDAPFiller_p(0),
      antennaFile_p(0), modelFiller_p(0), focusFiller_p(0), 
      j2000_p(2000, 1, 0.0)
{;}

GBTMSPointingFiller::GBTMSPointingFiller(MSPointing &msPointing)
    : msPointing_p(0), msPointingCols_p(0), antDAPFiller_p(0),
      antennaFile_p(0), modelFiller_p(0), focusFiller_p(0), 
      j2000_p(2000, 1, 0.0)
{init(msPointing);}

GBTMSPointingFiller::~GBTMSPointingFiller()
{
    delete msPointing_p;
    msPointing_p = 0;
    delete msPointingCols_p;
    msPointingCols_p = 0;
    delete modelFiller_p;
    modelFiller_p = 0;
    delete focusFiller_p;
    focusFiller_p = 0;

    // don't delete antDAPFiller_p 
    antDAPFiller_p = 0;
    // don't delete antennaFile_p
    antennaFile_p = 0;
}

void GBTMSPointingFiller::attach(MSPointing &msPointing)
{init(msPointing);}

void GBTMSPointingFiller::setAntennaDAPFiller(GBTAntennaDAPFiller *antennaDAPFiller)
{antDAPFiller_p = antennaDAPFiller;}

void GBTMSPointingFiller::setAntennaFile(GBTAntennaFile *antennaFile)
{antennaFile_p = antennaFile;}

void GBTMSPointingFiller::fill(MeasurementSet &ms, Int antennaId, const Vector<Double> &times, Double interval) 
{
    pointingId_p.resize(times.nelements());
    if (times.nelements() == 0) return;

    // remember where we started from
    uInt lastSize = msPointing_p->nrow();
    
    if (antennaFile_p && antennaFile_p->isAttached()) {
	// use the antenna file for antenna positions
	MDirection j2000, user;
	Vector<MDirection> pos(1);
	// fill the pointing model 
	if (!modelFiller_p) {
	    modelFiller_p = new GBTPointModelFiller(ms);
	    AlwaysAssert(modelFiller_p, AipsError);
	    // add the user direction columns if necessary
	    if (!msPointing_p->tableDesc().isColumn("NRAO_GBT_USER_DIR")) {
		TableDesc td;
		// first, the column which will hold the directions
		td.addColumn(ArrayColumnDesc<Double>("NRAO_GBT_USER_DIR",
						     "Mean MAJOR and MINOR Positions from ANTENNA file",
						     2));
		// Second, the column to hold the reference frame
		td.addColumn(ScalarColumnDesc<Int>("NRAO_GBT_USER_DIR_REF",
						   "Reference column for USER_DIR"));
		// and create the MDirection column
		TableMeasRefDesc userRef(td, "NRAO_GBT_USER_DIR_REF");
		TableMeasValueDesc  userValue(td, "NRAO_GBT_USER_DIR");
		TableMeasDesc<MDirection> userMeas(userValue, userRef);
		userMeas.write(td);
		// these are hacks until we figure out what to really do with the User defined coordinates
		// the equinox of the system
		td.addColumn(ScalarColumnDesc<Double>("NRAO_GBT_USER_EQUINOX",
						      "Equinox of NRAO_GBT_USER_DIR"));
		// boolean flag indicating whether the NRAO_GBT_USER_DIR_REF is accurate or not
		// when this is True, then NRAO_GBT_USER_DIR is the USER-defined coordinate system
		// and no the coordinate system indicated by NRAO_GBT_USER_DIR_REF
		td.addColumn(ScalarColumnDesc<Bool>("NRAO_GBT_USER_DEFINED",
						    "Is NRAO_GBT_USER_DIR in a user-defined coordinate system"));
		// okay, finally, add everything in td to the existing table
		for (uInt i=0;i<td.ncolumn();i++) {
		    msPointing_p->addColumn(td[i]);
		}
	    }
	    //attach userDirection_p
	    userDirection_p.attach(*msPointing_p, "NRAO_GBT_USER_DIR");
	    userEquinox_p.attach(*msPointing_p, "NRAO_GBT_USER_EQUINOX");
	    userDefined_p.attach(*msPointing_p, "NRAO_GBT_USER_DEFINED");
	}
	if (!focusFiller_p) {
	    focusFiller_p = new GBTMeanFocusFiller(ms);
	    AlwaysAssert(focusFiller_p, AipsError);
	    // add the ID column if necessary
	    if (!msPointing_p->tableDesc().isColumn("NRAO_GBT_MEAN_FOCUS_ID")) {
		msPointing_p->addColumn(ScalarColumnDesc<Int>("NRAO_GBT_MEAN_FOCUS_ID",
							      "Index into NRAO_GBT_MEAN_FOCUS table"));
	    }
	    // and attach the ID column
	    meanFocusId_p.attach(*msPointing_p, "NRAO_GBT_MEAN_FOCUS_ID");
	}
	modelFiller_p->fill(*antennaFile_p);
	for (uInt i=0;i<times.nelements();i++) {
	    if (antennaFile_p->getPointing(times(i), interval, j2000, user)) {
		// valid position found
		Int thisId = msPointing_p->nrow();
		pointingId_p(i) = thisId;
		msPointing_p->addRow(1);
		pos = j2000;
		msPointingCols_p->directionMeasCol().put(thisId, pos);
		msPointingCols_p->targetMeasCol().put(thisId, pos);
		msPointingCols_p->time().put(thisId, times(i));
		pos = user;
		userDirection_p.put(thisId, pos);
		userEquinox_p.put(thisId, antennaFile_p->equinox());
		userDefined_p.put(thisId, antennaFile_p->isUserCoordSys());
		// and the pointing model id
		if (modelFiller_p) {
		    msPointingCols_p->pointingModelId().put(thisId, modelFiller_p->pointingModelId());
		} else {
		    msPointingCols_p->pointingModelId().put(thisId, -1);
		}
		// fill the focus information
		focusFiller_p->fill(times(i), interval, *antennaFile_p);
		meanFocusId_p.put(thisId, focusFiller_p->meanFocusId());
	    } // ignore times outside of the available range
	}
    } else if (antDAPFiller_p && !antDAPFiller_p->j2000RA().isNull() &&
	       antDAPFiller_p->j2000RA().nrow() > 0) {
	// use the antenna DAP for antenna positions
  
	ROTableVector<Double> timeVec(antDAPFiller_p->time());
	// we need to assemble antenna positions and statistics
	uInt start;
	Int iend;
	start = 0;
	iend = 0;
	uInt npos = antDAPFiller_p->j2000RA().nrow();
	// this is the shape for all of the position fields
	IPosition posFieldShape(2,2,1);
	for (uInt i=0;i<times.nelements();i++) {
	    Double startTime = times(i) - interval/2.0;
	    Double stopTime = startTime + interval;
	    for (;start<npos;start++) {
		// is this after the startTime
		Double thisTime=antDAPFiller_p->time().asdouble(start);
		if (thisTime >= startTime) break;
	    }
	    for (iend=start;iend<Int(npos);iend++) {
		Double thisTime=antDAPFiller_p->time().asdouble(iend);
		if (thisTime > stopTime) break;
	    }
	    // we end with the previous point
	    iend--;
	    if (iend >= 0 && uInt(iend) >= start) {
		uInt end = uInt(iend);
		// everything went as expected, add a row and calculate the statistics
		Int thisId = msPointing_p->nrow();
		pointingId_p(i) = thisId;
		msPointing_p->addRow(1);
		IPosition ipstart(1,start);
		IPosition ipend(1,end);
		raStatAcc_p.reset();
		decStatAcc_p.reset();
		ROTableVector<Double> raTabVec(antDAPFiller_p->j2000RA());
		ROTableVector<Double> decTabVec(antDAPFiller_p->j2000DEC());
		Vector<Double> raVec(raTabVec.makeVector());
		// need to watch for mixtures of near 0 and near 360 in raVec
		if (anyLT(raVec,10.0) && anyGT(raVec, 350.0)) {
		    // subtract 360.0 from all values > 180.0
		    MaskedArray<Double> maskedRAVec(raVec, raVec > 350.0);
		    Vector<Double> subRAVec(maskedRAVec.getCompressedArray());
		    subRAVec -= 360.0;
		    maskedRAVec.setCompressedArray(subRAVec);
		}
		raStatAcc_p.put(raVec(ipstart,ipend));
		Vector<Double> decVec(decTabVec.makeVector());
		decStatAcc_p.put(decVec(ipstart,ipend));
		Vector<Double> pos(2, ValType::undefDouble());
		Fallible<Double> result;
		// mean -> position 
		result = raStatAcc_p.getMean();
		if (result.isValid()) pos(0) = result;
		result = decStatAcc_p.getMean();
		if (result.isValid()) pos(1) = result;
		msPointingCols_p->direction().put(thisId, pos.reform(posFieldShape));
		msPointingCols_p->target().put(thisId, pos.reform(posFieldShape));
		msPointingCols_p->time().put(thisId, times(i));
	    } else {
		// don't interpolate when the interval is off the end, just don't fill anything in that case
		if (start != 0 && start < npos) {
		    // this means that the entire interval is between adjacent
		    // values - do a simple linear interpolation
		    // when this happens, start is past the interval and end is
		    // before the interval - i.e. their meaning is reversed
		    uInt end = iend;
		    // add a row
		    Int thisId = msPointing_p->nrow();
		    pointingId_p(i) = thisId;
		    msPointing_p->addRow(1);
		    Double rarange = antDAPFiller_p->j2000RA().asdouble(start) -
			antDAPFiller_p->j2000RA().asdouble(end);
		    Double decrange = antDAPFiller_p->j2000DEC().asdouble(start) -
			antDAPFiller_p->j2000DEC().asdouble(end);
		    Double timerange = antDAPFiller_p->time().asdouble(start) -
			antDAPFiller_p->time().asdouble(end);
		    Double timeFrac = (times(i) - antDAPFiller_p->time().asdouble(end))/timerange;
		    Vector<Double> pos(2);
		    pos(0) = antDAPFiller_p->j2000RA().asdouble(end) + rarange*timeFrac;
		    pos(1) = antDAPFiller_p->j2000DEC().asdouble(end) + decrange*timeFrac;
		    msPointingCols_p->direction().put(thisId, pos.reform(posFieldShape));
		    msPointingCols_p->target().put(thisId, pos.reform(posFieldShape));
		    msPointingCols_p->time().put(thisId, times(i));
		}
	    }
	    // by starting from the previous end, this assumes that intervals never overlap
	    if (iend >= 0) start = iend;
	    else start = 0;
	}
    } 
    // don't add any rows if there are no antenna positions
    // were any rows added, set things that don't vary from row to row
    if (msPointing_p->nrow() > lastSize) {
	Int newRows = msPointing_p->nrow() - lastSize;
	Slicer rowRange(IPosition(1,lastSize), IPosition(1,newRows));
	// the TIME is time origin for directions and rates, it is an EPOCH and for right
	// now everything is just j2000.   At least, I think that is true.  I need
	// to double check this.
	msPointingCols_p->antennaId().putColumnRange(rowRange,Vector<Int>(newRows, antennaId));
	msPointingCols_p->interval().putColumnRange(rowRange,Vector<Double>(newRows, interval));
	msPointingCols_p->name().putColumnRange(rowRange, Vector<String>(newRows,""));
	// just simple pointings, no drift
	msPointingCols_p->numPoly().putColumnRange(rowRange, Vector<Int>(newRows, 0));
	msPointingCols_p->timeOrigin().putColumnRange(rowRange, Vector<Double>(newRows, 0.0));
	msPointingCols_p->tracking().putColumnRange(rowRange, Vector<Bool>(newRows, True));
    }
}


void GBTMSPointingFiller::init(MSPointing &msPointing) 
{
    msPointing_p = new MSPointing(msPointing);
    AlwaysAssert(msPointing_p, AipsError);
    msPointingCols_p = new MSPointingColumns(*msPointing_p);
    AlwaysAssert(msPointingCols_p, AipsError);

    pointingId_p.resize(0);
}

void GBTMSPointingFiller::flush()
{
    msPointing_p->flush();
    if (modelFiller_p) modelFiller_p->flush();
    if (focusFiller_p) focusFiller_p->flush();
}
