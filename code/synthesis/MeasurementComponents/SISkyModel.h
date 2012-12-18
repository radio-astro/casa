//# SISkyModel.h: Definition for SISkyModel.cc
//# Copyright (C) 1996,1997,1998,1999,2000,2002
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_SISKYMODEL_H
#define SYNTHESIS_SISKYMODEL_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <synthesis/MeasurementComponents/SkyModel.h>
#include <casa/Arrays/Matrix.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/System/PGPlotter.h>

#include <synthesis/MeasurementEquations/SIMapperCollection.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// Forward Declarations
  class ViewerProxy;
  template<class T> class ImageInterface;
  class SISubIterBot;


class SISkyModel 
{
public:

  // Empty constructor
  SISkyModel();
  ~SISkyModel();


  // Copy constructor and assignment operator
  //Imager(const Imager&);
  //Imager& operator=(const Imager&);

  void init( ); // Send in iteration-control,etc params here...

  void runMinorCycle( SIMapperCollection &mappers, SISubIterBot &loopcontrols);

  void restore( SIMapperCollection &mappers );

protected:

  //void pauseForUserInteraction( SIMapperCollection &mappers, SIIterBot &loopcontrols );


};

} //# NAMESPACE CASA - END


#endif


