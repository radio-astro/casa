//# GBTMSSysCalFiller.cc: GBTMSSysCalFiller fills the MSSysCal table for GBT fillers
//# Copyright (C) 2000,2001,2002
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

#include <nrao/GBTFillers/GBTMSSysCalFiller.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSSysCalColumns.h>
#include <tables/Tables/ColumnsIndex.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <casa/Utilities/Assert.h>

GBTMSSysCalFiller::GBTMSSysCalFiller()
    : msSysCal_p(0), msSysCalCols_p(0), dummySysCal_p(0), dummyCols_p(0),
      dummyIndex_p(0)
{;}

GBTMSSysCalFiller::GBTMSSysCalFiller(MSSysCal &msSysCal)
    : msSysCal_p(0), msSysCalCols_p(0), dummySysCal_p(0), dummyCols_p(0),
      dummyIndex_p(0)
{init(msSysCal);}

GBTMSSysCalFiller::~GBTMSSysCalFiller()
{
    delete msSysCal_p;
    msSysCal_p = 0;

    delete msSysCalCols_p;
    msSysCalCols_p = 0;

    delete dummySysCal_p;
    dummySysCal_p = 0;

    delete dummyCols_p;
    dummyCols_p = 0;

    delete dummyIndex_p;
    dummyIndex_p = 0;
}

void GBTMSSysCalFiller::attach(MSSysCal &msSysCal)
{init(msSysCal);}

void GBTMSSysCalFiller::prefill(const Matrix<Float> &tcal, 
				const Matrix<Float> &trx,
				Int antennaId, Int feedId,
				Int spectralWindowId)
{
    *antIdKey_p = antennaId;
    *feedIdKey_p = feedId;
    *spwIdKey_p = spectralWindowId;
    Bool found;
    uInt rownr = dummyIndex_p->getRowNumber(found);
    if (!found) {
	rownr = dummySysCal_p->nrow();
	dummySysCal_p->addRow(1);
    } // otherwise this replaces any previously set values
    dummyCols_p->antennaId().put(rownr, antennaId);
    dummyCols_p->feedId().put(rownr, feedId);
    dummyCols_p->spectralWindowId().put(rownr, spectralWindowId);
    dummyCols_p->tcalSpectrum().put(rownr, tcal);
    Vector<Float> meanCal(tcal.nrow()), meanRx(trx.nrow());
    for (uInt i=0;i<tcal.nrow();i++) meanCal[i] = mean(tcal.row(i));
    for (uInt i=0;i<trx.nrow();i++) meanRx[i] = mean(trx.row(i));
    dummyCols_p->tcal().put(rownr, meanCal);
    dummyCols_p->trx().put(rownr, meanRx);
    // TSYS has default value of 1, actual value filled in by calibration code
    // store it here so we don't have to construct this each time we need it
    dummyCols_p->tsys().put(rownr, Vector<Float>(tcal.nrow(),1.0));

    lastIndex_p = rownr;
}

void GBTMSSysCalFiller::fill(Double time, Double interval,
			     Int antennaId, Int feedId, Int spectralWindowId)
{
    if (lastIndex_p < 0 || *antIdKey_p != antennaId || *feedIdKey_p != feedId ||
	*spwIdKey_p != spectralWindowId) {
	*antIdKey_p = antennaId;
	*feedIdKey_p = feedId;
	*spwIdKey_p = spectralWindowId;
	Bool found;
	lastIndex_p = dummyIndex_p->getRowNumber(found);
	if (!found) lastIndex_p = -1;
    }
    // get the values as appropriate
    Matrix<Float> tcalSpec;
    Vector<Float> tcal, trx, tsys;
    Bool ok = False;
    if (lastIndex_p >= 0) {
	tcalSpec = dummyCols_p->tcalSpectrum()(lastIndex_p);
	tcal = dummyCols_p->tcal()(lastIndex_p);
	trx = dummyCols_p->trx()(lastIndex_p);
	tsys = dummyCols_p->tsys()(lastIndex_p);
	ok = True;
    }
	
    // add a row 
    Int rownr = msSysCal_p->nrow();
    msSysCal_p->addRow(1);
    msSysCalCols_p->antennaId().put(rownr, antennaId);
    msSysCalCols_p->feedId().put(rownr, feedId);
    msSysCalCols_p->interval().put(rownr, interval);
    msSysCalCols_p->spectralWindowId().put(rownr, spectralWindowId);
    msSysCalCols_p->time().put(rownr, time);
    msSysCalCols_p->tcal().put(rownr, tcal);
    msSysCalCols_p->tcalFlag().put(rownr, !ok);
    msSysCalCols_p->trx().put(rownr, trx);
    msSysCalCols_p->trxFlag().put(rownr, !ok);
    msSysCalCols_p->tsys().put(rownr, tsys);
    msSysCalCols_p->tsysFlag().put(rownr, !ok);
    // these columns may or may not be present
    if (!msSysCalCols_p->tcalSpectrum().isNull()) {
	msSysCalCols_p->tcalSpectrum().put(rownr, tcalSpec);
    }
}

void GBTMSSysCalFiller::init(MSSysCal &msSysCal) 
{
    msSysCal_p = new MSSysCal(msSysCal);
    AlwaysAssert(msSysCal_p, AipsError);

    msSysCalCols_p = new MSSysCalColumns(*msSysCal_p);
    AlwaysAssert(msSysCalCols_p, AipsError);

    // and the dummy table
    TableDesc sysCalTd = MSSysCal::requiredTableDesc();
    // add in the optional columns used here
    MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TCAL);
    MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TRX);
    MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TSYS);
    MSSysCal::addColumnToDesc(sysCalTd, MSSysCal::TCAL_SPECTRUM);

    SetupNewTable newSysCal("", sysCalTd, Table::Scratch);
    dummySysCal_p = new MSSysCal(newSysCal);
    AlwaysAssert(dummySysCal_p, AipsError);

    dummyCols_p = new MSSysCalColumns(*dummySysCal_p);
    AlwaysAssert(dummyCols_p, AipsError);

    dummyIndex_p = new ColumnsIndex(*dummySysCal_p,
				    stringToVector("ANTENNA_ID,FEED_ID,SPECTRAL_WINDOW_ID"));
    AlwaysAssert(dummyIndex_p, AipsError);
    antIdKey_p.attachToRecord(dummyIndex_p->accessKey(),"ANTENNA_ID");
    feedIdKey_p.attachToRecord(dummyIndex_p->accessKey(),"FEED_ID");
    spwIdKey_p.attachToRecord(dummyIndex_p->accessKey(),"SPECTRAL_WINDOW_ID");

    lastIndex_p = -1;
}
