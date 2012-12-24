//# SIMapperCollection.cc: Implementation of Imager.h
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

#include <synthesis/MeasurementEquations/SIMapperCollection.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SIMapperCollection::SIMapperCollection() 
  {
    LogIO os( LogOrigin("SIMapperCollection","Construct a mapperCollection",WHERE) );
    
    itsMappers.resize(0);

  }
  
  SIMapperCollection::~SIMapperCollection() 
  {
  }
  
  // Allocate Memory and open images.
  void SIMapperCollection::addMapper( CountedPtr<FTMachine> ftmachine, CountedPtr<SIDeconvolver> deconvolver, CountedPtr<CoordinateSystem> imcoordsys, CountedPtr<SIMaskHandler> maskhandler )
  {

    LogIO os( LogOrigin("SIMapperCollection","addMapper",WHERE) );

    Int nMappers = itsMappers.nelements();
    itsMappers.resize(nMappers+1, True);
    itsMappers[nMappers] = new SIMapper( ftmachine, deconvolver, imcoordsys , maskhandler, nMappers );

  }

  Int SIMapperCollection::nMappers()
  {
    return itsMappers.nelements();
  }

  CountedPtr<SIMapper> & SIMapperCollection::getMapper( Int mapindex )
  {
    if(static_cast<uInt>(mapindex)>=itsMappers.nelements()) throw(AipsError("Internal Error : Map Index larger than nmappers"));

    return itsMappers[mapindex];
  }

  /**************************************************************************/
  /* Below here are all the functions that loop over Mappers to gather info. */
  /**************************************************************************/

  Float SIMapperCollection::findPeakResidual()
  {
    Float peakresidual=0.0, maxpeakresidual=0.0;
    for(uInt mp=0;mp<itsMappers.nelements();mp++)
      {
	peakresidual = itsMappers[mp]->getPeakResidual();
	if( peakresidual > maxpeakresidual )
	  {
	    maxpeakresidual = peakresidual;
	  }
      }

    return maxpeakresidual;
  }

  Float SIMapperCollection::addIntegratedFlux()
  {

    Float modelflux=0.0;
    for(uInt mp=0;mp<itsMappers.nelements();mp++)
      {
	modelflux += itsMappers[mp]->getModelFlux();
      }

    return modelflux;
  }


  Float SIMapperCollection::findMaxPsfSidelobe()
  {
    // Calculate a PSF fraction from the PSFs across all mappers.
    Float psfsidelobe=0.0, maxpsfsidelobe=0.0;
    for(uInt mp=0;mp<itsMappers.nelements();mp++)
      {
	psfsidelobe = itsMappers[mp]->getPSFSidelobeLevel();
	if( psfsidelobe > maxpsfsidelobe )
	  {
	    maxpsfsidelobe = psfsidelobe;
	  }
      }

    return maxpsfsidelobe;
  }


  Bool SIMapperCollection::anyUpdatedModel()
  {
    Bool updatedmodel = False;
    for(uInt mp=0;mp<itsMappers.nelements();mp++)
      {
	updatedmodel |= itsMappers[mp]->isModelUpdated();
      }

    return updatedmodel;
  }

  /**************************************************************************/

   
} //# NAMESPACE CASA - END

