//# SynthesisDeconvolver.h: Imager functionality sits here; 
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

#ifndef SYNTHESIS_SYNTHESISITERBOT_H
#define SYNTHESIS_SYNTHESISITERBOT_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <synthesis/ImagerObjects/SIIterBot.h>
#include <boost/thread/thread.hpp>

//#include<casa/random.h>

#include <boost/scoped_ptr.hpp>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class MeasurementSet;
class ViewerProxy;
template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>

class SynthesisIterBot
{
 public:
  // Default constructor

  SynthesisIterBot();
  ~SynthesisIterBot();

  // Copy constructor and assignment operator

  // launch thread which opens DBus connection...
  void openDBus( );

  // make all pure-inputs const
  void setupIteration(Record iterpars);

  void setInteractiveMode(Bool interactiveMode);
  void   setIterationDetails(Record iterpars);
  Record getIterationDetails();
  Record getIterationSummary();

  bool cleanComplete();

  Record getSubIterBot();

  void startMinorCycle(Record& initializationRecord);
  void endMinorCycle(Record& executionRecord);

  void endMajorCycle();

protected:

  void pauseForUserInteraction();

  /////////////// Member Objects

	shared_ptr<SIIterBot_callback> actionRequestSync;
	shared_ptr<SIIterBot_state> itsLoopController;

    /////////////// All input parameters

 private:
  boost::thread  *dbus_thread;
  void dbus_thread_launch_pad( );
};


} //# NAMESPACE CASA - END

#endif
