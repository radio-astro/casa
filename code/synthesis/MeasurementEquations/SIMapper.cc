//# SIMapper.cc: Implementation of SIMapper.h
//# Copyright (C) 1997-2008
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

#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>

#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <casa/OS/DirectoryIterator.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>

#include <casa/OS/HostInfo.h>

#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <synthesis/MeasurementEquations/SIMapper.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SIMapper::SIMapper( CountedPtr<SIImageStore> imagestore, 
		      CountedPtr<FTMachine> ftmachine, 
		      Int mapperid) : SIMapperBase( imagestore, ftmachine, mapperid )
  {
    LogIO os( LogOrigin("SIMapper","Construct a mapper",WHERE) );
  }
  
  SIMapper::~SIMapper() 
  {
  }
  
  // #############################################
  // #############################################
  // #######  Gridding / De-gridding functions ###########
  // #############################################
  // #############################################


  /// All these take in vb's, and just pass them on.

  void SIMapper::initializeGrid(/* vb */)
  {
    LogIO os( LogOrigin("SIMapper","initializeGrid",WHERE) );
    // itsFTM->initializeToSky( itsResidual, vb )
  }

  void SIMapper::grid(/* vb */)
  {
    LogIO os( LogOrigin("SIMapper","grid",WHERE) );
  }

  //// The function that makes the PSF should check its validity, and fit the beam,
  void SIMapper::finalizeGrid()
  {
    LogIO os( LogOrigin("SIMapper","finalizeGrid",WHERE) );

    // TODO : Fill in itsResidual, itsPsf, itsWeight.
    // Do not normalize the residual by the weight. 
    //   -- Normalization happens later, via 'divideResidualImageByWeight' called from SI.divideImageByWeight()
    //   -- This will ensure that normalizations are identical for the single-node and parallel major cycles. 

  }

  void SIMapper::initializeDegrid()
  {
    LogIO os( LogOrigin("SIMapper", "initializeDegrid",WHERE) );

    ////  only if model exists...
    
  }

  void SIMapper::degrid()
  {
    LogIO os( LogOrigin("SIMapper","degrid",WHERE) );
  }

  void SIMapper::finalizeDegrid()
  {
    LogIO os( LogOrigin("SIMapper","finalizeDegrid",WHERE) );
  }

  Record SIMapper::getFTMRecord()
  {
    LogIO os( LogOrigin("SIMapper","getFTMRecord",WHERE) );
    Record rec;
    // rec = itsFTM->toRecord();
    return rec;
  }

   
} //# NAMESPACE CASA - END

