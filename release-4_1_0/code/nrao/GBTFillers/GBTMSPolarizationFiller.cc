//# GBTMSPolarizationFiller.cc: GBTMSPolarizationFiller fills the MSPolarization table for GBT fillers
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

#include <nrao/GBTFillers/GBTMSPolarizationFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <tables/Tables/ColumnsIndex.h>
#include <casa/Utilities/Assert.h>

GBTMSPolarizationFiller::GBTMSPolarizationFiller()
    : index_p(0), msPolarization_p(0), msPolCols_p(0), polarizationId_p(-1)
{;}


GBTMSPolarizationFiller::GBTMSPolarizationFiller(MSPolarization &msPolarization)
    : index_p(0), msPolarization_p(0), msPolCols_p(0), polarizationId_p(-1)
{init(msPolarization);}

GBTMSPolarizationFiller::~GBTMSPolarizationFiller()
{
    delete index_p;
    index_p = 0;

    delete msPolarization_p;
    msPolarization_p = 0;

    delete msPolCols_p;
    msPolCols_p = 0;
}

void GBTMSPolarizationFiller::attach(MSPolarization &msPolarization)
{init(msPolarization);}

void GBTMSPolarizationFiller::fill(Int ncorr, const Vector<Int> &corrType, 
				   const Matrix<Int> &corrProduct)
{
    *ncorrKey_p = ncorr;
    // there might be more than one NCORR match (not very likely, but still...)
    Vector<uInt> rows = index_p->getRowNumbers();
    polarizationId_p = -1;
    // look for any true matches
    for (uInt i=0;i<rows.nelements();i++) {
        // static_casts are here because of an SGI compiler bug. wky 01/12/2000.
	if (allEQ(msPolCols_p->corrType()(rows(i)),static_cast< const Vector<Int> >(corrType)) &&
	    allEQ(msPolCols_p->corrProduct()(rows(i)),static_cast< const Matrix< Int> >(corrProduct))) {
	    polarizationId_p = Int(rows(i));
	    break;
	}
    }
    if (polarizationId_p < 0) {
	// need a new row
	polarizationId_p = msPolarization_p->nrow();
	msPolarization_p->addRow();
	msPolCols_p->numCorr().put(polarizationId_p, ncorr);
	msPolCols_p->corrType().put(polarizationId_p, corrType);
	msPolCols_p->corrProduct().put(polarizationId_p, corrProduct);
	msPolCols_p->flagRow().put(polarizationId_p, False);
    }
}

void GBTMSPolarizationFiller::init(MSPolarization &msPolarization)
{
    msPolarization_p = new MSPolarization(msPolarization);
    AlwaysAssert(msPolarization_p, AipsError);

    msPolCols_p = new MSPolarizationColumns(msPolarization);
    AlwaysAssert(msPolCols_p, AipsError);

    String indexString = MSPolarization::columnName(MSPolarization::NUM_CORR);
    index_p = new ColumnsIndex(*msPolarization_p, stringToVector(indexString));
    AlwaysAssert(index_p, AipsError);

    ncorrKey_p.attachToRecord(index_p->accessKey(), 
			      MSPolarization::columnName(MSPolarization::NUM_CORR));
    polarizationId_p = -1;
}
