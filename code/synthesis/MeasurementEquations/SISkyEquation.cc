//# SISkyEquation.cc: Implementation of Imager.h
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

#include <synthesis/MeasurementEquations/SISkyEquation.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <casadbus/viewer/ViewerProxy.h>
#include <casadbus/plotserver/PlotServerProxy.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/session/DBusSession.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SISkyEquation::SISkyEquation()// : itsVisSet(VisSet())
  {
    
  }
  
  SISkyEquation::~SISkyEquation() 
  {
  }
  

  void SISkyEquation::init() // VisSet& visset )
  {
    LogIO os( LogOrigin("SISkyEquation","init",WHERE) );
    os << "Init SkyEquation" << LogIO::POST;

    //itsVisSet = visset;

  }

  void SISkyEquation::runMajorCycle( SIMapperCollection &mappers )
  {
    LogIO os( LogOrigin("SISkyEquation","runMajorCycle",WHERE) );

    Int nmappers = mappers.nMappers();
    
    //    os << "Run major cycle over all " << nmappers << " mappers" << LogIO::POST;

    for(Int mp=0;mp<nmappers;mp++)
      {
	// vb.selectChannel(....)
	mappers.getMapper(mp)->initializeDegrid();
	mappers.getMapper(mp)->initializeGrid();
      }

    for (Int vb=0;vb<1;vb++) 
      {
	 for(Int mp=0;mp<nmappers;mp++)
	   {
	     mappers.getMapper(mp)->degrid();
	     // resultvb.modelVisCube += vb.modelVisCube()
	   }

	 // resultvb.visCube -= resultvb.modelvisCube()

	 // save model either in the column, or in the record. 
	 // Access the FTM record as    rec=mappers[mp]->getFTMRecord();
	 
	 for(Int mp=0;mp<nmappers;mp++)
	   {
	     mappers.getMapper(mp)->grid();
	   }
      }// end of for vb.

    
    for(Int mp=0;mp<nmappers;mp++)
      {// Implement this loop inside SIMapperCollection if nothing else needs to be done here.
	mappers.getMapper(mp)->finalizeDegrid();
	mappers.getMapper(mp)->finalizeGrid();
      }
    
  }

   
} //# NAMESPACE CASA - END

