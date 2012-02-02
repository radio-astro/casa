//#---------------------------------------------------------------------------
//# ASTEFXReader.h: Class to read ASTE-FX data.
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

#ifndef ASTE_FX_READER_H
#define ASTE_FX_READER_H

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>

#include <string>

using namespace std ;

// <summary>
// Class to read ASTE-FX data.
// </summary>
//
// <prerequisite>
//   <li> <linkto class=NROReader>NROReader</linkto>
//   <li> <linkto class=NRODataRecord>NRODataset</linkto>
//   <li> <linkto class=ASTEFXDataset>ASTEFXDataset</linkto>
// </prerequisite>
//
// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>
//
// <etymology>
// This class is a reader class for ASTE OTF data. This is specialized for FX type 
// spectrometer data. The class uses 
// <linkto class=ASTEFXDataset>ASTEFXDataset</linkto> object to access data. 
// </etymology>
//
// <synopsis>
// Reader class for ASTE OTF (FX) data.
// </synopsis>
//

#include <casa/namespace.h>
#include <atnf/PKSIO/ASTEReader.h>

class ASTEFXReader : public ASTEReader
{
  public:
  // Constructor 
  ASTEFXReader( string name ) ;

  // Destructor.
  ~ASTEFXReader() ;

  // Read data header
  virtual int read() ;
};

#endif /* ASTE_FX_READER_H */
