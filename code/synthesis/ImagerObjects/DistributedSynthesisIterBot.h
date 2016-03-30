/* -*- mode: c++ -*- */
//# DistributedSynthesisIterBot.h: Parallel imaging iteration control
//# Copyright (C) 2016
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
#ifndef DISTRIBUTED_SYNTHESIS_SYNTHESISITERBOT_H_
#define DISTRIBUTED_SYNTHESIS_SYNTHESISITERBOT_H_

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <synthesis/ImagerObjects/DistSIIterBot.h>

//#include<casa/random.h>
#include <synthesis/ImagerObjects/MPIGlue.h>
#include<synthesis/ImagerObjects/InteractiveMasking.h>

#include <thread>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class MeasurementSet;
class ViewerProxy;
template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>

class DistributedSynthesisIterBot
{
public:
	// Default constructor

	DistributedSynthesisIterBot(MPI_Comm comm);
	virtual  ~DistributedSynthesisIterBot();

	// Copy constructor and assignment operator

	// launch thread which opens DBus connection...
	void openDBus();

	// make all pure-inputs const
	void setupIteration(Record iterpars);

	void setInteractiveMode(Bool interactiveMode);
	virtual void   setIterationDetails(Record iterpars);
	Record getIterationDetails();
	Record getIterationSummary();

	int cleanComplete();

	Record getSubIterBot();

	void startMinorCycle(const Vector<Record> &initializationRecords);

	void startMinorCycle(const Record &initializationRecord) {
		Vector<Record> v(1);
		v[0] = initializationRecord;
		startMinorCycle(v);
	};

	void endMinorCycle(const Vector<Record> &executionRecords);

	void endMinorCycle(const Record &executionRecord) {
		Vector<Record> v(1);
		v[0] = executionRecord;
		endMinorCycle(v);
	};

	void endMajorCycle();

	void changeStopFlag( Bool stopflag );

	virtual void pauseForUserInteraction();

protected:
	/////////////// Member Objects

	SHARED_PTR<SIIterBot_callback> actionRequestSync;
	SHARED_PTR<DistSIIterBot_state> itsLoopController;

	/////////////// All input parameters

private:
	std::thread  *dbus_thread;
	void dbus_thread_launch_pad( );

	/// Parameters to control the old interactive GUI. Can be moved somewhere more appropriate...
	/*  Vector<String> itsImageList;
	    Vector<Int> itsNTermList;
	    Vector<Int> itsActionCodes;
	    CountedPtr<InteractiveMasking> itsInteractiveMasker;
	*/
};


class DistributedSynthesisIterBotWithOldGUI
	: public DistributedSynthesisIterBot
{
public:
	// Default constructor

	DistributedSynthesisIterBotWithOldGUI(MPI_Comm comm);

	void setIterationDetails(Record iterpars);
	void pauseForUserInteraction();

private:
	Vector<String> itsImageList;
	Vector<Bool> itsMultiTermList;
	Vector<Int> itsActionCodes;
	CountedPtr<InteractiveMasking> itsInteractiveMasker;

	bool is_root;
};


} //# NAMESPACE CASA - END

#endif // DISTRIBUTED_SYNTHESIS_SYNTHESISITERBOT_H_
