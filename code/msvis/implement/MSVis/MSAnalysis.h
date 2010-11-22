//# MSAnalysis.h: MS analysis and handling tool
//# Copyright (C) 2007
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

#ifndef _MS_MSALYSIS_H_
#define _MS_MSANALYSIS_H_

// includes
#include <msvis/MSVis/MSMoments.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Logging/LogIO.h>

namespace casa {

class MeasurementSet ;
template<class T> class MSMoments ;

// <summary>
// This class is a single dish spectral data analysis tool
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// This class is a single dish spectral data analysis tool.
// </etymology>
//
// <synopsis>
// MSAnalysis is a single dish spectral data analysis tool.
// Input data should be in the form of MeasurementSet with 
// FLOAT_DATA column (autocorrelation data).
//
// </synopsis>
//
class MSAnalysis
{
public:

  // Constructor
  MSAnalysis( const casa::MeasurementSet *inMS ) ;

  // Destructor
  virtual ~MSAnalysis() ;

  // moment calculation
  MeasurementSet *moments( const Vector<Int> &whichmoments,
                           //const String &mask,
                           const String &antenna,
                           const String &field,
                           const String &spw,
                           const Vector<String> &method,
                           const Vector<Int> &smoothAxes,
                           const Vector<String> &kernels,
                           const Vector<Quantity> &kernelWidths,
                           const Vector<Float> &includemask,
                           const Vector<Float> &excludemask,
                           const Double peaksnr,
                           const Double stddev,
                           const String &velocityType,
                           const String &out,
                           const String &smoothOut,
                           //const String &pgdevice,
                           //const Int nx,
                           //const Int ny,
                           //const Bool yind,
                           const Bool overwrite
                           ) ;
private:
  // Set other data
  // <group>
  void setMS( casa::MeasurementSet *inMS ) ;
  void setMS( const casa::String name ) ;
  // </group>

  // Select data from input MS
  void selectMS( const String antenna, const String field, const String spw ) ;

  // Clean up temporary files
  void cleanup() ;

  // Initialization
  void init( const casa::MeasurementSet *inMS ) ;

  // Add comments on HISTORY table
  void addHistory( String tableName, String suffix ) ;

  // MeasurementSet data (should have FLOAT_DATA column)
  MeasurementSet *ms_p ;

  // Selected Table name
  String tableName_ ;

  // Original Table name
  String tableIn_ ;

  // Logger
  LogIO *itsLog ;

};

} // casac namespace
#endif
