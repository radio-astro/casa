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

#include <synthesis/TransformMachines/BeamSkyJones.h>
#include <synthesis/TransformMachines/SkyJones.h>
#include <synthesis/TransformMachines/SimpleComponentFTMachine.h>
#include <synthesis/TransformMachines/SimpCompGridMachine.h>
//#include <synthesis/ImagerObjects/SIMapperBase.h>
#include <synthesis/ImagerObjects/SIMapper.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SIMapper::SIMapper( CountedPtr<SIImageStore>& imagestore, 
		      CountedPtr<FTMachine>& ftm, 
		      CountedPtr<FTMachine>& iftm) 
  {
    LogIO os( LogOrigin("SIMapper","Construct a mapper",WHERE) );
    ft_p=ftm;
    ift_p=iftm;
    cft_p=NULL;
    itsImages=imagestore;
  }
  
  SIMapper::SIMapper(const ComponentList& cl, 
		     String& whichMachine)
  {
    ft_p=NULL;
    ift_p=NULL;
    itsImages=NULL;

    if(whichMachine=="SimpleComponentFTMachine")
      cft_p=new SimpleComponentFTMachine();
    else
      //SD style component gridding
      cft_p=new SimpleComponentGridMachine();
    cl_p=cl;
      
  }
  
  SIMapper::~SIMapper() 
  {
  }
  
  // #############################################
  // #############################################
  // #######  Gridding / De-gridding functions ###########
  // #############################################
  // #############################################


  void SIMapper::initializeGrid(VisBuffer& vb, Bool dopsf, Bool /*firstaccess*/)
     {

       LogIO os( LogOrigin("SIMapper","initializeGrid",WHERE) );
       //Componentlist FTM has nothing to do
       if(ift_p.null())
       	return;

       ift_p->initializeToSkyNew( dopsf, vb, itsImages);

     }

  /////////////////OLD vi/vb version
  void SIMapper::grid(VisBuffer& vb, Bool dopsf, FTMachine::Type col,
		      const Int whichFTM)
   {
     LogIO os( LogOrigin("SIMapper","grid",WHERE) );
     //Componentlist FTM has no gridding to do
     (void)whichFTM;

     if(ift_p.null())
       return;
     
     ift_p->put(vb, -1, dopsf, col);
     
   }

  //////////////OLD VI/VB version
  void SIMapper::finalizeGrid(VisBuffer& vb, Bool dopsf)
    {
      LogIO os( LogOrigin("SIMapper","finalizeGrid",WHERE) );

      if(ift_p.null())
      	return;

      ift_p->finalizeToSkyNew( dopsf, vb, itsImages );

    }

  
  //////////////////OLD vi/vb version
  void SIMapper::initializeDegrid(VisBuffer& vb, const Int /*row*/)
  {
    LogIO os( LogOrigin("SIMapper", "initializeDegrid",WHERE) );
    if(ft_p.null() && cft_p.null())
      return;

    ft_p->initializeToVisNew(vb, itsImages);

  }


  ////////////////////Old vi/Vb version

  void SIMapper::degrid(VisBuffer& vb)
    {
      LogIO os( LogOrigin("SIMapper","degrid",WHERE) );
      ///This should not be called even but heck let's ignore
      if(ft_p.null() and cft_p.null())
      	return;

      Cube<Complex> origCube;
      origCube.assign(vb.modelVisCube()); 
      
      if( ! ft_p.null() ) { ft_p->get(vb); }
      if( ! cft_p.null() ) { cft_p->get(vb, cl_p); }
      
      vb.modelVisCube()+=origCube;

    }


  void SIMapper::finalizeDegrid()
  {
    LogIO os( LogOrigin("SIMapper","finalizeDegrid",WHERE) );
    ft_p->finalizeToVis();
  }


  Bool SIMapper::getFTMRecord(Record& rec, const String diskimage)
  {
    LogIO os( LogOrigin("SIMapper","getFTMRecord",WHERE) );
    if(ft_p.null())
    	return False;
    String err;
    return ft_p->toRecord(err, rec, True, diskimage);
    // rec = itsFTM->toRecord();

  }
  Bool SIMapper::getCLRecord(Record& rec)
  {
	  if(cft_p.null())
	      	return False;
	  String err;
	  return cl_p.toRecord(err, rec);

  }

} //# NAMESPACE CASA - END


