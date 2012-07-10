//# GBTFieldFiller.cc: fills the FIELD table for GBT fillers
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
//# along with this library; if not, write to th7e Free Software Foundation,
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

#include <nrao/GBTFillers/GBTMSFieldFiller.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasFrame.h>
#include <tables/Tables/ColumnsIndex.h>
#include <casa/Utilities/Assert.h>

GBTMSFieldFiller::GBTMSFieldFiller()
    : index_p(0), msField_p(0), msFieldCols_p(0), fieldId_p(-1)
{;}

GBTMSFieldFiller::GBTMSFieldFiller(MSField &msField)
    : index_p(0), msField_p(0), msFieldCols_p(0), fieldId_p(-1)
{init(msField);}

GBTMSFieldFiller::~GBTMSFieldFiller()
{
    delete index_p;
    index_p = 0;
    delete msField_p;
    msField_p = 0;
    delete msFieldCols_p;
    msFieldCols_p = 0;
}

void GBTMSFieldFiller::attach(MSField &msField)
{init(msField);}

void GBTMSFieldFiller::fill(Int sourceId, const String &name,
			    const MDirection &fieldDir)
{
    *nameKey_p = name;
    *sourceIdKey_p = sourceId;

    // get fieldDir in J2000 coordinates
    // are they there yet
    Matrix<Double> dir(2,1);
    if (fieldDir.getRef().getType() != MDirection::J2000) {
	MDirection mdir = 
	    MDirection::Convert(fieldDir,
				MDirection::Ref(MDirection::J2000))();
	dir.column(0) = mdir.getAngle().getValue();
    } else {
	dir.column(0) = fieldDir.getAngle().getValue();
    }

    // there can be more than one match
    Vector<uInt> rows = index_p->getRowNumbers();
    fieldId_p = -1;
    // look for any true matches
    for (uInt i=0;i<rows.nelements();i++) {
	if (allEQ(msFieldCols_p->delayDir()(rows(i)),dir)) {
	    fieldId_p = Int(rows(i));
	    break;
	}
    }
    if (fieldId_p < 0) {
	// need a new row
	fieldId_p = msField_p->nrow();
	msField_p->addRow();
	msFieldCols_p->name().put(fieldId_p, *nameKey_p);
	msFieldCols_p->time().put(fieldId_p, 0.0);
	msFieldCols_p->delayDir().put(fieldId_p, dir);
	msFieldCols_p->phaseDir().put(fieldId_p, dir);
	msFieldCols_p->referenceDir().put(fieldId_p, dir);
	msFieldCols_p->sourceId().put(fieldId_p, *sourceIdKey_p);
	msFieldCols_p->flagRow().put(fieldId_p, False);
    }
}


void GBTMSFieldFiller::init(MSField &msField) 
{
    msField_p = new MSField(msField);
    AlwaysAssert(msField_p, AipsError);
    msFieldCols_p = new MSFieldColumns(*msField_p);
    AlwaysAssert(msFieldCols_p, AipsError);

    String indexString = MSField::columnName(MSField::NAME);
    indexString += ",";
    indexString += MSField::columnName(MSField::SOURCE_ID);
    index_p = new ColumnsIndex(*msField_p, stringToVector(indexString));
    AlwaysAssert(index_p, AipsError);

    nameKey_p.attachToRecord(index_p->accessKey(),
			     MSField::columnName(MSField::NAME));
    sourceIdKey_p.attachToRecord(index_p->accessKey(),
				 MSField::columnName(MSField::SOURCE_ID));

    fieldId_p = -1;
}
