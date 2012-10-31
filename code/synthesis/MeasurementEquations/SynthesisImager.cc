//# SynthesisImager.cc: Implementation of Imager.h
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

#include <synthesis/MeasurementEquations/SynthesisImager.h>
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
  
  SynthesisImager::SynthesisImager() : startmodel_p(String("")), 
				       niter_p(0),
				       usescratch_p(True)
  {
    
  }
  
  SynthesisImager::~SynthesisImager() 
  {
  }
  
  // Make this read in a list of MS's and selection pars....
  void  SynthesisImager::selectData(Record selpars)
  {
    LogIO os( LogOrigin("SynthesisImager","selectData",WHERE) );

    Vector<String> mslist,fieldlist,spwlist;

    try
      {

	if( selpars.isDefined("vis") ) { selpars.get( RecordFieldId("vis") , mslist ); }
	if( selpars.isDefined("field") ) { selpars.get( RecordFieldId("field") , fieldlist ); }
	if( selpars.isDefined("spw") ) { selpars.get( RecordFieldId("spw") , spwlist ); }

      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in reading input parameter record : "+x.getMesg()) );
      }

    Int nms = mslist.nelements();
    if(fieldlist.nelements() != nms){os << LogIO::EXCEPTION << "Need " << nms << " field selection strings, one for each specified MS" << LogIO::POST; }
    if(spwlist.nelements() != nms){os << LogIO::EXCEPTION << "Need " << nms << " spw selection strings, one for each specified MS" << LogIO::POST; }

    for(Int sel=0; sel<nms; sel++)
      {
	os << "MS : " << mslist[sel];
	os << "   Selection : spw='" << spwlist[sel] << "'";
	os << " field='" << fieldlist[sel] << "'" << LogIO::POST;
      }

    if( selpars.isDefined("usescratch") ) { selpars.get( RecordFieldId("usescratch"), usescratch_p ); }
    
  }
  
  void  SynthesisImager::defineImage(Record impars)
  {
    LogIO os( LogOrigin("SynthesisImager","defineImage",WHERE) );
    
    os << "Define Image Coordinates" << LogIO::POST;
    
  }
  
  void  SynthesisImager::setupImaging(Record gridpars)
  {
    LogIO os( LogOrigin("SynthesisImager","setupImaging",WHERE) );
    os << "Set Imaging Options" << LogIO::POST;
    if( gridpars.isDefined("startmodel") ) { gridpars.get( RecordFieldId("startmodel"), startmodel_p ); }

  }
  
  void SynthesisImager::setupDeconvolution(Record decpars)
  {
    LogIO os( LogOrigin("SynthesisImager","setupDeconvolution",WHERE) );
    os << "Set Deconvolution Options" << LogIO::POST;
  }
  
  void  SynthesisImager::setupIteration(Record iterpars)
  {
    LogIO os( LogOrigin("SynthesisImager","setupIteration",WHERE) );
    os << "Set Iteration Control Options" << LogIO::POST;
   
    if( iterpars.isDefined("niter") ) { iterpars.get( RecordFieldId("niter"), niter_p ); }
  }

  /*
  void SynthesisImager::setOtherOptions(Bool usescratch)
  {
    LogIO os( LogOrigin("SynthesisImager","setOtherOptions", WHERE) );
    os << "Set all other options" << LogIO::POST;
    usescratch_p = usescratch;
  }
  */

  /*
  Record  SynthesisImager::initLoops()
  {
    LogIO os( LogOrigin("SynthesisImager","initLoops",WHERE) );
    os << "Initialize Cleaning" << LogIO::POST;
    
    Record loopcontrols;
    checkLoopControls(loopcontrols);
    return loopcontrols;
    
  }
  */

  void  SynthesisImager::endLoops(Record& loopcontrols)
  {
    LogIO os( LogOrigin("SynthesisImager","endLoops",WHERE) );

    checkLoopControls(loopcontrols);
    
    Bool updatedmodel;
    loopcontrols.get( RecordFieldId("updatedmodel") , updatedmodel );

    if(updatedmodel==True)
      {
	os << "Restore Image (and normalize to Flat Sky) " << LogIO::POST;
      }
    
  }
  
  void  SynthesisImager::runMajorCycle(Record& loopcontrols)   // Or nothing...
  {
    LogIO os( LogOrigin("SynthesisImager","runMajorCycle",WHERE) );

    checkLoopControls(loopcontrols);

    Bool updatedmodel;
    loopcontrols.get( RecordFieldId("updatedmodel") , updatedmodel );
    Int nmajordone;
    loopcontrols.get( RecordFieldId("nmajordone") , nmajordone );
    Int niterdone;
    loopcontrols.get( RecordFieldId("niterdone") , niterdone );
    Bool stop;
    loopcontrols.get( RecordFieldId("stop") , stop );

    if(niterdone >= niter_p)
      {
	stop = True;
	loopcontrols.define( RecordFieldId("stop") , stop );
      }

    if( niterdone==0 && startmodel_p.length()>1 )
      {
	updatedmodel=True;
      }

    if(nmajordone==0)
      {
	os << "Make PSFs, weights and initial dirty/residual images. " ;
      }
    else
      {
	if(updatedmodel==False)
	  {
	    os << "No new model. No need to update residuals in a major cycle." << LogIO::POST;
	    return; // loopcontrols;
	  }
	os << "Update residual image in major cycle "<< String::toString(nmajordone) << ". ";
      }

    if(stop==True && updatedmodel==True)
      {
	if(usescratch_p==True)
	  {
	    os << "Save image model to MS in MODEL_DATA column on disk" << LogIO::POST;
	  }
	else
	  {
	    os << "Save image model to MS as a Record for on-the-fly prediction" << LogIO::POST;
	  }
      }
    else
      {
	os << LogIO::POST;
      }

    // se.runMajorCycle(xxxxx . modeltoms = usescratch)
    loopcontrols.define( RecordFieldId("nmajordone") , nmajordone+1 );

    //return loopcontrols;
  }
  
  void  SynthesisImager::runMinorCycle(Record& loopcontrols)
  {
    LogIO os( LogOrigin("SynthesisImager","runMinorCycle",WHERE) );
    checkLoopControls(loopcontrols);

    Int niterdone;
    loopcontrols.get( RecordFieldId("niterdone") , niterdone );

    os << "Start Minor Cycle at iteration " << niterdone+1 << LogIO::POST;
   
    // iters,newmodel = sm.runMinorCycle()
    Int iters = 5;
    Bool updatedmodel=True;

    os << "Stop Minor Cycle at iteration " << niterdone + iters << LogIO::POST; // Goes into sm, for chan/stokes/field loops...
    loopcontrols.define( RecordFieldId("niterdone") , niterdone + iters );

    // Check convergence ( make an 'isConverged()' call or have this returned from minor cycle )
    Bool stop = niterdone + iters >= niter_p;
    loopcontrols.define( RecordFieldId("stop") , stop );
    loopcontrols.define( RecordFieldId("updatedmodel"), updatedmodel );

    //return loopcontrols;
  }
  
  
  Bool SynthesisImager::checkLoopControls(Record &loopcontrols)
  {

    if( ! loopcontrols.isDefined("stop") )
      {
	loopcontrols.define( RecordFieldId("stop"), (Bool) False );
      }
    
    if( ! loopcontrols.isDefined("updatedmodel") )
      {
	loopcontrols.define( RecordFieldId("updatedmodel"), (Bool) False );
      }
    
    if( ! loopcontrols.isDefined("niterdone") )
      {
	loopcontrols.define( RecordFieldId("niterdone"), (Int) 0 );
      }
    
    if( ! loopcontrols.isDefined("nmajordone") )
      {
	loopcontrols.define( RecordFieldId("nmajordone"), (Int) 0 );
      }
    
    return True;
  }
  
} //# NAMESPACE CASA - END

