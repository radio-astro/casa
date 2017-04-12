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
#include <thread>

//#include<casa/random.h>
#include<synthesis/ImagerObjects/InteractiveMasking.h>

namespace casacore{

class MeasurementSet;
template<class T> class ImageInterface;
}

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class ViewerProxy;

// <summary> Class that contains functions needed for imager </summary>

class SynthesisIterBot
{
 public:
  // Default constructor

  SynthesisIterBot();
 virtual  ~SynthesisIterBot();

  // Copy constructor and assignment operator

  // launch thread which opens DBus connection...
  void openDBus( );

  // make all pure-inputs const
  void setupIteration(casacore::Record iterpars);

  void setInteractiveMode(casacore::Bool interactiveMode);
  virtual void   setIterationDetails(casacore::Record iterpars);
  casacore::Record getIterationDetails();
  casacore::Record getIterationSummary();

  int cleanComplete(casacore::Bool lastcyclecheck=casacore::False);

  casacore::Record getSubIterBot();

  void startMinorCycle(casacore::Record& initializationRecord);
  void endMinorCycle(casacore::Record& executionRecord);

  void endMajorCycle();
  void resetMinorCycleInfo();

  void changeStopFlag( casacore::Bool stopflag );

  virtual casacore::Record pauseForUserInteractionOld(){return casacore::Record();};

protected:

  virtual void pauseForUserInteraction();
  /////////////// Member Objects

  SHARED_PTR<SIIterBot_callback> actionRequestSync;
  SHARED_PTR<SIIterBot_state> itsLoopController;

    /////////////// All input parameters

 private:
  std::thread  *dbus_thread;
  void dbus_thread_launch_pad( );

  /// Parameters to control the old interactive GUI. Can be moved somewhere more appropriate...
  /*  casacore::Vector<casacore::String> itsImageList;
  casacore::Vector<casacore::Int> itsNTermList;
  casacore::Vector<casacore::Int> itsActionCodes;
  casacore::CountedPtr<InteractiveMasking> itsInteractiveMasker;
  */
};


  class SynthesisIterBotWithOldGUI : public SynthesisIterBot
{
 public:
  // Default constructor

  SynthesisIterBotWithOldGUI();
  ~SynthesisIterBotWithOldGUI(){};

  void   setIterationDetails(casacore::Record iterpars);
  casacore::Record pauseForUserInteractionOld();

protected:

  void pauseForUserInteraction(){};

 private:
  casacore::Vector<casacore::String> itsImageList;
  casacore::Vector<casacore::Bool> itsMultiTermList;
  casacore::Vector<casacore::Int> itsActionCodes;
  casacore::CountedPtr<InteractiveMasking> itsInteractiveMasker;

  };


} //# NAMESPACE CASA - END

#endif
