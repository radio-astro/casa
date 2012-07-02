//# NFRA_MS_FW.cc: WSRT Frequency objects
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

//======================================================================
//======================================================================
// Frequency information methods

//======================================================================
// IVC object
//
NFRA_IVC::NFRA_IVC()
{
  IVCName = "";
  CenterFreq = BandWidth = ChanWidth = RestFreq = VelValue = 0;
  NChan = NPol = SubArray = 0;
  VelDef = ConvType = ObsMode = "";
  EF = "..";
}
//
String NFRA_IVC::getPolsString()
{
  std::ostringstream rtn;
  rtn << PolsUsed[0];
  for (uInt n = 1; n < PolsUsed.size(); n++)
    rtn << "," << PolsUsed[n];
  return rtn.str();
}
//----------------------------------------------------------------------
void NFRA_IVC::dump(String pre, uInt i)
{
  cout << pre << "IVC" << i << ".IVCName=" << IVCName << endl;
  //  cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
  //  cout.setf(std::ios_base::right, std::ios_base::adjustfield);
  Int p = cout.precision(10);
  cout << pre << "IVC" << i << ".CenterFreq=" << CenterFreq << endl;
  cout << pre << "IVC" << i << ".ObsMode=" << ObsMode << endl;
  cout.precision(p);
  cout << pre << "IVC" << i << ".RestFreq=" << RestFreq << endl;
  cout << pre << "IVC" << i << ".VelValue=" << VelValue << endl;
  cout << pre << "IVC" << i << ".VelDef=" << VelDef << endl;
  cout << pre << "IVC" << i << ".ConvType=" << ConvType << endl;
  cout << pre << "IVC" << i << ".BandWidth=" << BandWidth << endl;
  cout << pre << "IVC" << i << ".NChan=" << NChan << endl;
  cout << pre << "IVC" << i << ".ChanWidth=" << ChanWidth << endl;
  cout << pre << "IVC" << i << ".NPol=" << NPol << endl;
  cout << pre << "IVC" << i << ".PolsUsed=";
  for (uInt n = 0; n < PolsUsed.size(); n++) cout << " " << PolsUsed[n];
  cout << endl;
  cout << pre << "IVC" << i << ".EF=" << EF << endl;
  cout << pre << "IVC" << i << ".SubArray=" << SubArray << endl;
}


//======================================================================
// MFFE object
// All other methods are defined in the .h file
//
void NFRA_MFFE::dump(String pre, uInt i)
{
  cout << pre << "MFFEBand=" << MFFEBand << endl;
  cout << pre << "MFFESkyFreq=" << MFFESkyFreq << endl;
  cout << pre << "MFFEBandWidth=" << MFFEBandWidth << endl;
}

//======================================================================
// Frequency mosaic object
//
//----------------------------------------------------------------------
// Copy IVCs from another FreqMos object to this one
//
void NFRA_FreqMos::addIVCs(NFRA_FreqMos FM)
{
  for (uInt i = 0; i < FM.IVCs.size(); i++){
    IVCs.push_back(FM.IVCs[i]);
  }
}
//
vector<Double> NFRA_FreqMos::getCenterFreqs()
{
  vector<Double> rtn;
  for (uInt i = 0; i < IVCs.size(); i++){
    rtn.push_back(IVCs[i].getCenterFreq());
  }
  return rtn;
}
//----------------------------------------------------------------------
void NFRA_FreqMos::dump(String pre, uInt i)
{
  cout << pre <<".FM" << i << ".DwellTime=" << DwellTime << endl;
  uInt N = IVCs.size();
  std::ostringstream ppre;
  ppre << pre << ".FM" << i << '.';
  MFFE.dump(ppre.str(), i);
  for (uInt n = 0; n < N; n++) IVCs[n].dump(ppre.str(), n);
}

//======================================================================
// FW object
//
//----------------------------------------------------------------------
// Copy IVCs from the FreqMos objects of another Frequency Window to
// the FreqMos objects of this one.
//
void NFRA_FW::addIVCs(NFRA_FW FW)
{
  for (uInt i = 0; i < FreqMoss.size(); i++){
    FreqMoss[i].addIVCs(FW.FreqMoss[i]);
  }
}

//----------------------------------------------------------------------
void NFRA_FW::dump(uInt i)
{
  cout << "FW" << i << ".Telescopes=" << Telescopes << endl;
  uInt N = FreqMoss.size();
  std::ostringstream pre;
  pre << "FW" << i;
  for (uInt n = 0; n < N; n++) FreqMoss[n].dump(pre.str(), n);
}

