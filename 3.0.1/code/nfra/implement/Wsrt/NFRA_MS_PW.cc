//# NFRA_PS_PW.cc: This program demonstrates conversion of UVW for WSRT
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
//# $Id$
//
// Implements the Classes:
// - NFRA_Position
// - NFRA_PosMos
// - NFRA_PW
//

//# Includes
#include <strstream>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasData.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasRef.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/Muvw.h>
#include <measures/Measures/Stokes.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVBaseline.h>
#include <casa/Quanta/MVuvw.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSSummary.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
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

#include <nfra/Wsrt/NFRA_MS.h>

#include <casa/namespace.h>

//======================================================================
// Position information methods
//
//====================================================================
// NFRA_Position
//
//--------------------------------------------------------------------
// constructor - init all properties
//
NFRA_Position::NFRA_Position()
{
  RA = DEC = DTStart = DTEnd = HaMin = HaMax = 0;
  Name = "";
}

//--------------------------------------------------------------------
// Set a new DTStart -> calculate HaMin
// 
void NFRA_Position::setDTStart(Double T)
{
  DTStart = T;
  HaMin = calcHA(T, RA, DEC);  // located in NFRA_Misc.cc
}

//--------------------------------------------------------------------
// Set a new DTEnd -> calculate HaMax
//
void NFRA_Position::setDTEnd(Double T)
{
  DTEnd = T;
  HaMax = calcHA(T, RA, DEC);  // located in NFRA_Misc.cc
}

//--------------------------------------------------------------------
// dump to screen
//
void NFRA_Position::dump(String pre, uInt i)
{
  cout << pre << "PS" << i << ".RA=" << RA << endl;
  cout << pre << "PS" << i << ".DEC=" << DEC << endl;
  cout << pre << "PS" << i << ".Name=" << Name << endl;
  cout << pre << "PS" << i << ".DTStart=" << DTStart << endl;
  cout << pre << "PS" << i << ".DTStart=" << DTStart << endl;
  cout << pre << "PS" << i << ".HaMin=" << HaMin << endl;
  cout << pre << "PS" << i << ".HaMax=" << HaMax << endl;
}

//====================================================================
// NFRA_PosMos
//
//--------------------------------------------------------------------
// set a new DTStart -> distribute to all positions
//
void NFRA_PosMos::setDTStart(Double T)
{
  for (uInt i = 0; i < Positions.size(); i++){
    Positions[i].setDTStart(T);
  }
}

//--------------------------------------------------------------------
// set a new DTEnd -> distribute to all positions
//
void NFRA_PosMos::setDTEnd(Double T)
{
  for (uInt i = 0; i < Positions.size(); i++){
    Positions[i].setDTEnd(T);
  }
}

//----------------------------------------------------------------------
// return the min/max HA
//
Double NFRA_PosMos::getHaMin()
{
  Double rtn = 100.0;
  for (uInt i = 0; i < Positions.size(); i++){
    rtn = min(rtn, Positions[i].getHaMin());
  }
  return rtn;
}
Double NFRA_PosMos::getHaMax()
{
  Double rtn = 0;
  for (uInt i = 0; i < Positions.size(); i++){
    rtn = max(rtn, Positions[i].getHaMax());
  }
  return rtn;
}

//----------------------------------------------------------------------
// RA, DEC centroids
//
Double NFRA_PosMos::getRACentroid()
{
  Double sum = 0;
  for (uInt i = 0; i < Positions.size(); i++){
    sum += Positions[i].getRA();
  }
  return sum / Positions.size();
}
Double NFRA_PosMos::getDECCentroid()
{
  Double sum = 0;
  for (uInt i = 0; i < Positions.size(); i++){
    sum += Positions[i].getDEC();
  }
  return sum / Positions.size();
}

//----------------------------------------------------------------------
// Dump to screen
//
void NFRA_PosMos::dump(String pre, uInt i)
{
  cout << pre << "PM" << i << ".DwellTime=" << DwellTime << endl;
  uInt N = Positions.size();
  cout << pre << "PM" << i << ".NPositions=" << N << endl;
  std::ostringstream ppre;
  ppre << pre << "PM" << i << '.';
  for (uInt n = 0; n < N; n++) Positions[n].dump(ppre.str(), n);
}

//====================================================================
// NFRA_PW
//
//--------------------------------------------------------------------
// set a new DTStart -> distribute to all pos-moss's
//
void NFRA_PW::setDTStart(Double T)
{
  for (uInt i = 0; i < PosMoss.size(); i++){
    PosMoss[i].setDTStart(T);
  }
}

//--------------------------------------------------------------------
// set a new DTEnd -> distribute to all pos-moss's
//
void NFRA_PW::setDTEnd(Double T)
{
  for (uInt i = 0; i < PosMoss.size(); i++){
    PosMoss[i].setDTEnd(T);
  }
}

//----------------------------------------------------------------------
// get the total of positions
//
uInt NFRA_PW::getNPos()
{
  uInt rtn = 0;
  for (uInt i = 0; i < PosMoss.size(); i++){
    rtn += PosMoss[i].getNPos();
  }
  return rtn;
}


//----------------------------------------------------------------------
// return the min/max HA
//
Double NFRA_PW::getHaMin()
{
  Double rtn = 100.0;
  for (uInt i = 0; i < PosMoss.size(); i++){
    rtn = min(rtn, PosMoss[i].getHaMin());
  }
  return rtn;
}
Double NFRA_PW::getHaMax()
{
  Double rtn = 0;
  for (uInt i = 0; i < PosMoss.size(); i++){
    rtn = max(rtn, PosMoss[i].getHaMax());
  }
  return rtn;
}
//----------------------------------------------------------------------
// RA, DEC centroids
//
Double NFRA_PW::getRACentroid()
{
  Double sum = 0;
  for (uInt i = 0; i < PosMoss.size(); i++){
    sum += PosMoss[i].getRACentroid();
  }
  return sum / PosMoss.size();
}
Double NFRA_PW::getDECCentroid()
{
  Double sum = 0;
  for (uInt i = 0; i < PosMoss.size(); i++){
    sum += PosMoss[i].getDECCentroid();
  }
  return sum / PosMoss.size();
}

//----------------------------------------------------------------------
// Dump to screen
//
void NFRA_PW::dump(uInt i)
{
  cout << "PW" << i << ".Telescopes=" << Telescopes << endl;
  uInt N = PosMoss.size();
  cout << "PW" << i << ".NPosMospoints=" << N << endl;
  std::ostringstream pre;
  pre << "PW" << i << '.';
  for (uInt n = 0; n < N; n++) PosMoss[n].dump(pre.str(), n);
}
