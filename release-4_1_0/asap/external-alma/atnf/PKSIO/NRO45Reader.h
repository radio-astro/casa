//#---------------------------------------------------------------------------
//# NRO45Reader.h: Class to read NRO 45m OTF data.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2006
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
//#---------------------------------------------------------------------------
//# Original: 2008/10/30, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#ifndef NRO45_READER_H
#define NRO45_READER_H

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
#include <atnf/PKSIO/NROReader.h>

#include <string>

using namespace std ;

// <summary>
// Class to read NRO 45m OTF data.
// </summary>
//
// <prerequisite>
//   <li> <linkto class=NROReader>NROReader</linkto>
//   <li> <linkto class=NRODataRecord>NRODataset</linkto>
//   <li> <linkto class=NROOTFDataset>NROOTFDataset</linkto>
// </prerequisite>
//
// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>
//
// <etymology>
// This class is a reader class for NRO 45m telescope. This is specialized class 
// for data obtained by OTF observing mode. The class uses 
// <linkto class=NROOTFDataset>NROOTFDataset</linkto> object to access data. 
// </etymology>
//
// <synopsis>
// Reader class for NRO 45m OTF data.
// </synopsis>
//

class NRO45Reader : public NROReader
{
 public:
  // Constructor 
  NRO45Reader( string name ) ;

  // Destructor.
  ~NRO45Reader() ;

  // get number of IF
  virtual Int getNumIF() ;

  // get number of Beams
  virtual Int getNumBeam() ;

 protected:
  // initialize Dataset
  virtual void initDataset();
  
  // Get Antenna Position in ITRF coordinate
  virtual vector<double> getAntennaPosition() ;

};

#endif /* NRO45_READER_H */
