//# SIMapperSingle.cc: Implementation of SIMapperSingle.h
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

#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisBuffer2Adapter.h>
#include <synthesis/TransformMachines/BeamSkyJones.h>
#include <synthesis/TransformMachines/SkyJones.h>
#include <synthesis/TransformMachines/SimpleComponentFTMachine.h>
#include <synthesis/TransformMachines/SimpCompGridMachine.h>
#include <synthesis/ImagerObjects/SIMapperSingle.h>


#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SIMapperSingle::SIMapperSingle( CountedPtr<SIImageStore>& imagestore, 
				  CountedPtr<FTMachine>& ftm, CountedPtr<FTMachine>& iftm,
				  CountedPtr<VPSkyJones>& vp)
  : SIMapperBase(vp),
    vb_p (vi::VisBuffer2::factory (vi::VbPlain, vi::VbRekeyable))
  {
    LogIO os( LogOrigin("SIMapperSingle","Construct a mapper",WHERE) );
    ft_p=ftm;
    ift_p=iftm;
    cft_p=NULL;

    itsImages=imagestore;
  }
  
  SIMapperSingle::SIMapperSingle(const ComponentList& cl, String& whichMachine,
				 CountedPtr<VPSkyJones>& vp)
    : SIMapperBase(vp),
    vb_p (vi::VisBuffer2::factory (vi::VbPlain, vi::VbRekeyable))
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
  
  SIMapperSingle::~SIMapperSingle() 
  {
      delete vb_p;
  }
  
  // #############################################
  // #############################################
  // #######  Gridding / De-gridding functions ###########
  // #############################################
  // #############################################


  /// NEW
  void SIMapperSingle::initializeGrid(const vi::VisBuffer2& vb, const Bool /*dopsf*/)
  {
    LogIO os( LogOrigin("SIMapperSingle","initializeGrid",WHERE) );
    Matrix<Float> wgt;
    initializeGridCore2( vb, ift_p, *(itsImages->backwardGrid()) , wgt );
  }
  /// OLD
  void SIMapperSingle::initializeGrid(VisBuffer&  vb, Bool /*dopsf*/)
  {
    LogIO os( LogOrigin("SIMapperSingle","initializeGrid",WHERE) );
    Matrix<Float> wgt;
    
    if( !itsDoImageMosaic ) {
      initializeGridCore( vb, ift_p, *(itsImages->backwardGrid()) , wgt );
    }
    else{
      initializeGridCoreMos( vb, ift_p, *(itsImages->backwardGrid()) , wgt );
    }

  }


  /// NEW
  void SIMapperSingle::grid(const vi::VisBuffer2& vb, Bool dopsf, FTMachine::Type col)
  {
    LogIO os( LogOrigin("SIMapperSingle","grid",WHERE) );
    //Componentlist FTM has no gridding to do
    if(ift_p.null())
      return;
    gridCore2( vb, dopsf, col, ift_p, -1 );
  }
  /// OLD vi/vb version
  void SIMapperSingle::grid(VisBuffer& vb, Bool dopsf, FTMachine::Type col)
  {
    LogIO os( LogOrigin("SIMapperSingle","grid",WHERE) );
    //Componentlist FTM has no gridding to do
    if(ift_p.null())
      return;

    if( !itsDoImageMosaic ) {
      gridCore( vb, dopsf, col, ift_p, -1 );
    }
    else
      {
	Matrix<Float> wgt;
	gridCoreMos( vb, dopsf, col, ift_p, -1,  
		     *(itsImages->residual()), 
		     *(itsImages->weight()), 
		     wgt, 
		     *(itsImages->backwardGrid())  );
      }
  }
  

  /// NEW
  void SIMapperSingle::finalizeGrid(const vi::VisBuffer2& /* vb */, const Bool dopsf)
  {
      LogIO os( LogOrigin("SIMapperSingle","finalizeGrid",WHERE) );
      Matrix<Float> wgt;
      finalizeGridCore(dopsf,  ift_p, (dopsf ? *(itsImages->psf()) : *(itsImages->residual()) ) ,
		       *(itsImages->weight()) ,   wgt);
  }
  //// OLD VI/VB version
  void SIMapperSingle::finalizeGrid(VisBuffer& vb , Bool dopsf)
    {
      LogIO os( LogOrigin("SIMapperSingle","finalizeGrid",WHERE) );
      Matrix<Float> wgt;

      if( !itsDoImageMosaic or dopsf ) {
	finalizeGridCore(dopsf,  ift_p, (dopsf ? *(itsImages->psf()) : *(itsImages->residual()) ) ,
			 *(itsImages->weight()) ,   wgt); 
      }
      else{
	finalizeGridCoreMos(dopsf,  ift_p,  *(itsImages->residual())  ,
			    *(itsImages->weight()) ,   wgt, *(itsImages->backwardGrid()), vb);
      }

    }
  


  void SIMapperSingle::initializeDegrid(const vi::VisBuffer2& vb, const Int /*row*/)
  {
    LogIO os( LogOrigin("SIMapperSingle", "initializeDegrid",WHERE) );
    initializeDegridCore2( vb, ft_p, *(itsImages->model()) , *(itsImages->forwardGrid()) );
  }

  //////////////////OLD vi/vb version
  void SIMapperSingle::initializeDegrid(VisBuffer& vb, Int /*row*/)
  {
    LogIO os( LogOrigin("SIMapperSingle", "initializeDegrid",WHERE) );

    if( !itsDoImageMosaic ) {
      initializeDegridCore( vb, ft_p, *(itsImages->model()) , *(itsImages->forwardGrid()) );
    }
    else {
      initializeDegridCoreMos( vb, ft_p, *(itsImages->model()) , *(itsImages->forwardGrid()), cft_p, cl_p );
    }
  }
  

  /// NEW 
  void SIMapperSingle::degrid(vi::VisBuffer2& vb)
  {
      LogIO os( LogOrigin("SIMapperSingle","degrid",WHERE) );
      degridCore2( vb, ft_p, cft_p, cl_p );
  }
  //// Old vi/Vb version
  void SIMapperSingle::degrid(VisBuffer& vb)
    {
      LogIO os( LogOrigin("SIMapperSingle","degrid",WHERE) );
      if( !itsDoImageMosaic ) {
	degridCore( vb, ft_p, cft_p, cl_p );
      }
      else {
	degridCoreMos( vb, ft_p, cft_p,  *(itsImages->model()) , *(itsImages->forwardGrid()) );
	}
    }
  

  void SIMapperSingle::finalizeDegrid()
  {
    LogIO os( LogOrigin("SIMapperSingle","finalizeDegrid",WHERE) );
  }

  Bool SIMapperSingle::getFTMRecord(Record& rec)
  {
    LogIO os( LogOrigin("SIMapperSingle","getFTMRecord",WHERE) );
    if(ft_p.null())
    	return False;
    String err;
    return ft_p->toRecord(err, rec, True);
    // rec = itsFTM->toRecord();

  }
  Bool SIMapperSingle::getCLRecord(Record& rec)
  {
	  if(cft_p.null())
	      	return False;
	  String err;
	  return cl_p.toRecord(err, rec);

  }

} //# NAMESPACE CASA - END

