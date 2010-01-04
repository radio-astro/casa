//# NFRA_MF.h: header file for NFRA_MF class
//# Copyright (C) 1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: 
//
// Defines:
// - class NFRA_MF
//
//

#ifndef NFRA_MF_H
#define NFRA_MF_H

#include <strstream>
//#include <measures/Measures/MBaseline.h>
//#include <measures/Measures/MDirection.h>
//#include <measures/Measures/MEpoch.h>
//#include <measures/Measures/MPosition.h>
//#include <measures/Measures/MeasConvert.h>
//#include <measures/Measures/MeasData.h>
//#include <measures/Measures/MeasFrame.h>
//#include <measures/Measures/MeasRef.h>
//#include <measures/Measures/MeasTable.h>
//#include <measures/Measures/Muvw.h>
//#include <measures/Measures/Stokes.h>
//#include <casa/Quanta/Quantum.h>
//#include <casa/Quanta/MVAngle.h>
//#include <casa/Quanta/MVTime.h>
//#include <casa/Quanta/MVBaseline.h>
//#include <casa/Quanta/MVuvw.h>
//#include <casa/Containers/Record.h>
//#include <ms/MeasurementSets/MeasurementSet.h>
//#include <ms/MeasurementSets/MSColumns.h>
//#include <ms/MeasurementSets/MSField.h>
//#include <ms/MeasurementSets/MSFieldColumns.h>
//#include <ms/MeasurementSets/MSAntenna.h>
//#include <ms/MeasurementSets/MSAntennaColumns.h>
//#include <ms/MeasurementSets/MSSummary.h>
//#include <tables/Tables/Table.h>
//#include <tables/Tables/TableRow.h>
//#include <tables/Tables/TableDesc.h>
//#include <tables/Tables/ArrayColumn.h>
//#include <tables/Tables/TableRecord.h>
//#include <tables/Tables/ExprNode.h>
//#include <tables/Tables/ColumnDesc.h>
//#include <tables/Tables/ArrColDesc.h>
//#include <measures/TableMeasures/ArrayMeasColumn.h>
//#include <measures/TableMeasures/TableMeasDesc.h>
//#include <measures/TableMeasures/TableMeasValueDesc.h>
//#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/System/ProgressMeter.h>
#include <casa/Inputs.h>
#include <casa/OS/Path.h>
#include <casa/OS/File.h>
#include <casa/BasicMath/Math.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Sort.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/namespace.h>

class NFRA_MF {
 public:

  NFRA_MF(String);

  String getSemester(){return "Semester";}
  String getPrjNr(){return "PrjNr";}
  String getFieldName(){return "FieldName";}
  String getBackEnd(){return "BackEnd";}


 private:
  Vector<String> lines;
};

#endif
