//#---------------------------------------------------------------------------
//# NRO45FITSReader.h: Class to read NRO 45m FITS data.
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
//# Original: 2009/02/26, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#ifndef NRO45_FITS_READER_H
#define NRO45_FITS_READER_H

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
#include <atnf/PKSIO/NRO45Reader.h>

#include <string>

using namespace std ;

// <summary>
// Class to read NRO 45m FITS data.
// </summary>
//
// <prerequisite>
//   <li> <linkto class=NROReader>NROReader</linkto>
//   <li> <linkto class=NRODataRecord>NRODataset</linkto>
//   <li> <linkto class=NROFITSDataset>NROFITSDataset</linkto>
// </prerequisite>
//
// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>
//
// <etymology>
// This class is a reader class for NRO 45m telescope. This is specialized class 
// for data obtained by pointing observation (non-OTF observations) that outputs 
// data in NRO FITS format. The class uses 
// <linkto class=NROFITSDataset>NROFITSDataset</linkto> object to access data. 
// </etymology>
//
// <synopsis>
// Reader class for NRO 45m FITS data.
// </synopsis>
//

class NRO45FITSReader : public NRO45Reader
{
public:
  // Constructor 
  NRO45FITSReader( string name ) ;

  // Destructor.
  ~NRO45FITSReader() ;

//   // Read data header
//   virtual int read() ;
protected:
  // initialize Dataset
  void initDataset();  
};

#endif /* NRO45_FITS_READER_H */
