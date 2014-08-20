//# SDMaskHandler.cc: Implementation of SDMaskHandler classes
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
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/HostInfo.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ComponentList.h>
#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <casa/OS/File.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/Directory.h>
#include <tables/Tables/TableLock.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>

#include <synthesis/ImagerObjects/SDMaskHandler.h>


namespace casa { //# NAMESPACE CASA - BEGIN


  SDMaskHandler::SDMaskHandler()
  {
    interactiveMasker_p = new InteractiveMasking();
  }
  
  SDMaskHandler::~SDMaskHandler()
  {
    if (interactiveMasker_p != 0)
      delete interactiveMasker_p;
  }
  
  void SDMaskHandler::resetMask(CountedPtr<SIImageStore> imstore)
  {
    imstore->mask()->set(1.0);
  }

  void SDMaskHandler::makeMask()
  {
    LogIO os( LogOrigin("SDMaskHandler","makeMask",WHERE) );
    //os << "Make mask" << LogIO::POST;
  }

  Int SDMaskHandler::makeInteractiveMask(CountedPtr<SIImageStore>& imstore,
					  Int& niter, Int& ncycles, String& threshold)
  {
    Int ret;
    // Int niter=1000, ncycles=100;
    // String thresh="0.001mJy";
    String imageName = imstore->getName()+".residual";
    String maskName = imageName + ".mask";
    ret = interactiveMasker_p->interactivemask(imageName, maskName, 
					       niter, ncycles, threshold);
    return ret;
  }

  void SDMaskHandler::makeAutoMask(CountedPtr<SIImageStore> imstore)
  {
    LogIO os( LogOrigin("SDMaskHandler","makeAutoMask",WHERE) );

    Array<Float> localres;
    imstore->residual()->get( localres , True );

    Array<Float> localmask;
    imstore->mask()->get( localmask , True );
    
    IPosition posMaxAbs( localmask.shape().nelements(), 0);
    Float maxAbs=0.0;
    Float minVal;
    IPosition posmin(localmask.shape().nelements(), 0);
    minMax(minVal, maxAbs, posmin, posMaxAbs, localres);

    //    cout << "Max position : " << posMaxAbs << endl;

    Int dist=5;
    IPosition pos(2,0,0); // Deal with the input shapes properly......
    for (pos[0]=posMaxAbs[0]-dist; pos[0]<posMaxAbs[0]+dist; pos[0]++)
      {
	for (pos[1]=posMaxAbs[1]-dist; pos[1]<posMaxAbs[1]+dist; pos[1]++)
	  {
	    if( pos[0]>0 && pos[0]<localmask.shape()[0] && pos[1]>0 && pos[1]<localmask.shape()[1] )
	      {
		localmask( pos ) = 1.0;
	      }
	  }
      }

    //cout << "Sum of mask : " << sum(localmask) << endl;
    Float summask = sum(localmask);
    if( summask==0.0 ) { localmask=1.0; summask = sum(localmask); }
    os << LogIO::NORMAL1 << "Make Autobox mask with " << summask << " available pixels " << LogIO::POST;

    imstore->mask()->put( localmask );

    //    imstore->mask()->get( localmask , True );
    //    cout << "Sum of imstore mask : " << sum( localmask ) << endl;

  }

  void SDMaskHandler::makePBMask(CountedPtr<SIImageStore> imstore, Float weightlimit)
  {
    LogIO os( LogOrigin("SDMaskHandler","makeAutoMask",WHERE) );
    os << "Make autobox mask" << LogIO::POST;

    if( ! imstore->hasSensitivity() )
      { throw(AipsError("Need PB/Sensitivity/Weight image before a PB-based mask can be made for "+imstore->getName())); }

    LatticeExpr<Float> themask( iif( (*(imstore->weight())) > weightlimit , 1.0, 0.0 ) );
    imstore->mask()->copyData( themask );
  }


} //# NAMESPACE CASA - END

