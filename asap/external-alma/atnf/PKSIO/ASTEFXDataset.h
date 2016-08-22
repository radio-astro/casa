//#---------------------------------------------------------------------------
//# ASTEFXDataset.h: Class for ASTE data header.
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

#ifndef ASTE_FX_DATASET_H
#define ASTE_FX_DATASET_H

#define ASTE_ARYMAX_FX 12

#include <atnf/PKSIO/ASTEDataset.h>

#include <string>

using namespace std ;

// <summary>
// Accessor class for ASTE FX data.
// </summary>
//
// <prerequisite>
//   <li> <linkto class=ASTEFXReader>ASTEFXReader</linkto>
//   <li> <linkto class=ASTEDataset>ASTEDataset</linkto>
//   <li> <linkto class=NRODataset>NRODataset</linkto>
// </prerequisite>
//
// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>
//
// <etymology>
// This class actually accesses data from NRO telescopes. This is specialized class 
// for ASTE telescope with FX spectrometer. Only fillHeader method is implemented here. 
// The fillRecord method is implemented in <linkto class=NRODataset>NRODataset</linkto>.
// </etymology>
//
// <synopsis>
// Accessor class for ASTE FX data.
// </synopsis>
//

class ASTEFXDataset : public ASTEDataset 
{
 public:
  // constructor
  ASTEFXDataset( string name ) ;

  // destructor
  virtual ~ASTEFXDataset() ;

 protected:
  // return ARRAYMAX
  virtual int arrayMax() { return ASTE_ARYMAX_FX; } ;
} ;


#endif /* ASTE_FX_DATASET_H */
