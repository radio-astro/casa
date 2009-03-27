//# NFRA_Misc.cc: This program demonstrates conversion of UVW for WSRT
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

//====================================================================
// Convert string to time
// Input format is expected to be:
// <year>/<month>/<day> <hour>:<minute>:<seconds>
// output is in seconds
//
Double Str2Time(String s)
{
  Int i = s.find('/');
  uInt y = atoi(s.substr(0, i).c_str());
  s = s.substr(i+1);
  i = s.find('/');
  uInt m = atoi(s.substr(0, i).c_str());
  s = s.substr(i+1);
  i = s.find(' ');
  uInt d = atoi(s.substr(0, i).c_str());
  s = s.substr(i+1);
  i = s.find(':');
  uInt hr = atoi(s.substr(0, i).c_str());
  s = s.substr(i+1);
  i = s.find(':');
  uInt mn = atoi(s.substr(0, i).c_str());
  uInt se = atoi(s.substr(i+1).c_str());
  Double tm = (hr + mn/60.0 + se/3600.0) / 24.0;

  MVTime T(y, m, d, tm);
  return T.second();

}

//====================================================================
// ...
Double deg2rad(Double x)
{
  Double y = x * C::pi / 180.0;
  return y;
}

//====================================================================
// Convert a comma-separated string of integers to an integer vector
//
vector<uInt> CommaSepStr2vuInt(String str)
{
  vector<uInt> rtn;

  //
  // Find the comma's,
  // Add the substring as integer
  //
  // The first comma is located before the loop
  // The last int is added after the loop
  //
  Int i = str.find(',');
  while (i > 0){
    rtn.push_back(atoi(str.substr(0, i).c_str()));
    str = str.substr(i+1);
    i = str.find(',');
  }
  rtn.push_back(atoi(str.substr(0, i).c_str()));

  return rtn;
}

//====================================================================
// Convert a comma-separated string to a String vector
//
vector<String> CommaSepStr2vStr(String str)
{
  uInt cnt = 0;
  vector<String> rtn;

  //
  // Find the comma's,
  // Add substring
  //
  // The first comma is located before the loop
  // The last int is added after the loop
  //
  Int i = str.find(',');
  while (i > 0){
    rtn.push_back(str.substr(0, i));
    cnt++;
    str = str.substr(i+1);
    i = str.find(',');
  }
  rtn.push_back(str.substr(0, i));

  return rtn;
}

//====================================================================
// Convert a comma-separated string to a String vector
//
vector<Double> CommaSepStr2vDbl(String str)
{
  vector<Double> rtn;

  //
  // Find the comma's,
  // Add the substring as Double
  //
  // The first comma is located before the loop
  // The last int is added after the loop
  //
  Int i = str.find(',');
  while (i > 0){
    Double d = atof(str.substr(0, i).c_str());
    rtn.push_back(d);
    str = str.substr(i+1);
    i = str.find(',');
  }
  Double d = atof(str.substr(0, i).c_str());
  rtn.push_back(d);

  return rtn;
}

//====================================================================
// Convert time, ra, dec to hour angle
// It seems to work ...
///
Double calcHA(Double t, Double ra, Double dec)
{
  MPosition wsrt;
  MeasTable::Observatory(wsrt, "WSRT");

  MVTime t2(Quantity(t, "s"));
  MVEpoch t3(t2);
  MEpoch obstime(t3, MEpoch::UTC);

  MeasFrame frame;
  frame.set(wsrt);
  frame.set(obstime);

  Quantity ra1(ra, "rad");
  Quantity dec1(dec, "rad");
  MDirection obj(ra1, dec1, MDirection::Ref(MDirection::J2000, frame));

  MDirection hadec =  MDirection::Convert(obj, 
                      MDirection::Ref(MDirection::HADEC, frame))();
  Vector<Double> a = hadec.getAngle().getBaseValue();

  return a[0];
}

//======================================================================
// class NFRA_BaseLine
//
//----------------------------------------------------------------------
// constructor
//
NFRA_BaseLine::NFRA_BaseLine(char c)
{
  Telescope = c;
  Reference = '?';
  X = Y = Z = Distance = -1.0;
}

//--------------------------------------------------------------------
// Position difference wrt other telescope
//
void NFRA_BaseLine::operator-=(NFRA_BaseLine b)
{
  X -= b.X;
  Y -= b.Y;
  Z -= b.Z;
  Distance = sqrt(X*X + Y*Y + Z*Z);
  Reference = b.Telescope;
}

//--------------------------------------------------------------------
// Dump to screen
//
void NFRA_BaseLine::dump(uInt i)
{
  cout << "BL" << i << ".Telescope=" << Telescope << endl;
  cout << "BL" << i << ".Reference=" << Reference << endl;
  cout << "BL" << i << ".X=" << X << endl;
  cout << "BL" << i << ".Y=" << Y << endl;
  cout << "BL" << i << ".Z=" << Z << endl;
  cout << "BL" << i << ".Distance=" << Distance << endl;
}

//======================================================================
// class NFRA_SubArray
//
//----------------------------------------------------------------------
// Dump to screen
//
void NFRA_SubArray::dump(uInt i)
{
  cout << "SubArr" << i << ".SANr=" << SANr << endl;
  cout << "SubArr" << i << ".NTimeSlices=" << NTimeSlices << endl;
}

