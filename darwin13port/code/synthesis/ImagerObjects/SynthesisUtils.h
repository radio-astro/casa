//# SynthesisUtils.h: Imager functionality sits here; 
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//#
//# $Id$

#ifndef SYNTHESIS_SYNTHESISUTILS_H
#define SYNTHESIS_SYNTHESISUTILS_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SynthesisUtils 
{
 public:
  // Default constructor

  SynthesisUtils();
  ~SynthesisUtils();

  // All functions here are stand-alone, self-contained methods.


  // Partitioning syntax for Selection parameters
  //
  // Input Record (example) :
  //  { 'ms0' : { 'msname':xxx1, 'spw':yyy } ,
  //     'ms1' : { 'msname':xxx2, 'spw':yyy } }
  //
  //  Output Record (example for partitioning on spw) : 
  //  { '0' : { 'ms0' : { 'msname':xxx1, 'spw':yyy1 } ,
  //              'ms1' : { 'msname':xxx2, 'spw':yyy1 }    }
  //    '1' : { 'ms0' : { 'msname':xxx1, 'spw':yyy2 } ,
  //               'ms1' : { 'msname':xxx2, 'spw':yyy2 }   }   }

  // Data partitioning rules for CONTINUUM imaging
  Record continuumDataPartition(Record &selpars, const Int npart);

  // Data partitioning rules for CUBE imaging
  Record cubeDataPartition(Record &selpars, Int npart);

  // Image cube partitioning rules for CUBE imaging
  Record cubeImagePartition(Record &impars, Int npart);

  // Parse outlier-field specification file

  // Generate Coordinate System ( spectral definition, especially )
  //  Input : all specs.  Output : LSRK freqs.



protected:

};


} //# NAMESPACE CASA - END

#endif
