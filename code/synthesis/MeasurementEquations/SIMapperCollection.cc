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

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  
  SIMapperCollection::SIMapperCollection() 
  {
    LogIO os( LogOrigin("SIMapperCollection","Construct a mapperCollection",WHERE) );
    
    itsMappers.resize(0);

  }
  
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  SIMapperCollection::~SIMapperCollection() 
  {
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  
  // Allocate Memory and open images.
  void SIMapperCollection::addMapper( String mappertype,  
				      String imagename, 
				      CountedPtr<FTMachine> ftmachine, 
				      CountedPtr<CoordinateSystem> imcoordsys, 
				      IPosition imshape)
  {

    LogIO os( LogOrigin("SIMapperCollection","addMapper",WHERE) );

    CountedPtr<SIMapperBase> localMapper=NULL;
    Int nMappers = itsMappers.nelements();

    // Check 'mappertype' for valid types....
    if( mappertype == "basetype" )
      {
	localMapper = new SIMapperBase( imagename, ftmachine, imcoordsys , imshape, nMappers );
      }
    else if( mappertype == "default" )
      {
	localMapper = new SIMapper( imagename, ftmachine, imcoordsys , imshape, nMappers );
      }
    /*
    else if( mappertype == "multiterm" )
      {
	localMapper = new SIMapperMultiTerm( imagename, ftmachine, imcoordsys , imshape, nMappers );
      }
    */
    else
      {
	throw ( AipsError("Internal Error : Unrecognized Mapper Type in MapperCollection.addMapper") );
      }

    // If all is well, add to the list.
    itsMappers.resize(nMappers+1, True);
    itsMappers[nMappers] = localMapper;

  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  Int SIMapperCollection::nMappers()
  {
    return itsMappers.nelements();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  void SIMapperCollection::initializeGrid(Int mapperid)
  {
    AlwaysAssert( mapperid >=0 && mapperid < nMappers() , AipsError );
    itsMappers[mapperid]->initializeGrid();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  void SIMapperCollection::grid(Int mapperid)
  {
    AlwaysAssert( mapperid >=0 && mapperid < nMappers() , AipsError );
    itsMappers[mapperid]->grid();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  void SIMapperCollection::finalizeGrid(Int mapperid)
  {
    AlwaysAssert( mapperid >=0 && mapperid < nMappers() , AipsError );
    itsMappers[mapperid]->finalizeGrid();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////


  void SIMapperCollection::initializeDegrid(Int mapperid)
  {
    AlwaysAssert( mapperid >=0 && mapperid < nMappers() , AipsError );
    itsMappers[mapperid]->initializeDegrid();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  void SIMapperCollection::degrid(Int mapperid)
  {
    AlwaysAssert( mapperid >=0 && mapperid < nMappers() , AipsError );
    itsMappers[mapperid]->degrid();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

  void SIMapperCollection::finalizeDegrid(Int mapperid)
  {
    AlwaysAssert( mapperid >=0 && mapperid < nMappers() , AipsError );
    itsMappers[mapperid]->finalizeDegrid();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////


  Record SIMapperCollection::getFTMRecord(Int mapperid)
  {
    AlwaysAssert( mapperid >=0 && mapperid < nMappers() , AipsError );
    return itsMappers[mapperid]->getFTMRecord();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////

} //# NAMESPACE CASA - END

