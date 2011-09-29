//# FlagAgentBase.h: This file contains the interface definition of the FlagAgentBase class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef FlagAgentBase_H_
#define FlagAgentBase_H_

#include <flagging/Flagging/FlagDataHandler.h>
#include <casa/Containers/OrdMapIO.h>

using namespace casa::async;

namespace casa { //# NAMESPACE CASA - BEGIN


class FlagAgentBase : public casa::async::Thread {

	enum datacolumn {

		DATA=0,
		CORRECTED,
		MODEL,
		RESIDUAL,
		RESIDUAL_DATA
	};


public:

	FlagAgentBase(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube = false, Bool antennaMap = false, Bool flag = true);
	~FlagAgentBase ();
	static FlagAgentBase *create (FlagDataHandler *dh,Record config);

	void start();
	void terminate ();
	void queueProcess();
	void completeProcess();
	void * run ();

	// Set function to activate profiling
	void setProfiling(bool enable) {profiling_p = enable;}

	// Activate check mode to test that the flags were written to the MS
	void setCheckMode() {applyFlag_p = &FlagAgentBase::checkFlags;}


protected:

	void initialize();

	// Convenience function to be shared by parallel/non-parallel mode
	void runCore();

	void setDataSelection(Record config);
	// TODO: This class must be re-implemented in the derived classes
	virtual void setAgentParameters(Record config);

	void generateAllIndex();
	void generateRowsIndex(uInt nRows);
	void generateChannelIndex(uInt nChannels);
	void generatePolarizationIndex(uInt nPolarizations);

	// Generate index for all rows
	void indigen(vector<uInt> &index, uInt size);

	// For checking ids
	bool find(Vector<Int> validRange, Int element);

	// For checking ranges
	bool find(Matrix<Double> validRange, Double element);

	// For checking pairs
	bool find(Matrix<Int> validPairs, Int element1, Int element2);

	// Check if buffer has to be processed
	bool checkIfProcessBuffer();

	// Main iteration procedure to be called per buffer
	void iterateRows();

	// Compute flags for a given (row,channel,polarization)
	virtual Bool computeFlag(uInt row, uInt channel, uInt pol);

	// Main iteration procedure to be called per buffer
	void iterateMaps();

	// Compute flags for a given (time,freq) map
	virtual void flagMap(Int antenna1,Int antenna2,CubeView<Complex> &visibilities);

	// Mapping functions as requested by Urvashi
	CubeView<Bool> * getCommonFlagsView(Int antenna1, Int antenna2);
	CubeView<Bool> * getPrivateFlagsView(Int antenna1, Int antenna2);
	CubeView<Complex> * getVisibilitiesView(Int antenna1, Int antenna2);

	// Apply flags to common flag cube
	void applyCommonFlags(uInt row, uInt channel, uInt pol);

	// Apply flags to common and private flag cubes
	void applyPrivateFlags(uInt row, uInt channel, uInt pol);

	// Apply flags to common flag cube view
	void applyCommonFlagsView(uInt row, uInt channel, uInt pol);

	// Apply flags to common and private flag cubes
	void applyPrivateFlagsView(uInt row, uInt channel, uInt pol);

	// Wrapper to avoid complexity of calling a function pointer
	void applyFlag(uInt row, uInt channel, uInt pol);

	// Check flags is a test function to check that the flags are set where they should
	void checkFlags(uInt row, uInt channel, uInt pol);

	// Wrappers for the complex unitary operators
	Float real(const Complex &val) {return val.real();}
	Float imag(const Complex &val) {return val.imag();}
	Float abs(const Complex &val) {return std::abs(val);}
	Float arg(const Complex &val) {return std::arg(val);}
	Float norm(const Complex &val) {return std::norm(val);}

	// Wrapper to avoid complexity of calling a function pointer
	Float applyVisExpr(const Complex &val);

	// Utility function to make the difference between two columns
	void visCubeDiff(Cube<Complex> *leftOperand, Cube<Complex> *rightOperand);

	// Logger
	casa::LogIO *logger_p;

private:
	
	// MS-related objects
	FlagDataHandler *flagDataHandler_p;
	MeasurementSet *selectedMeasurementSet_p;
	VisBufferAutoPtr *visibilityBuffer_p;
	Cube<Bool> *commonFlagCube_p;
	Cube<Bool> *privateFlagCube_p;

	// Own data selection ranges
	casa::String arraySelection_p;
	casa::String fieldSelection_p;
	casa::String scanSelection_p;
	casa::String timeSelection_p;
	casa::String spwSelection_p;
	casa::String channelSelection_p;
	casa::String baselineSelection_p;
	casa::String uvwSelection_p;
	casa::String polarizationSelection_p;
	casa::String observationSelection_p;
	casa::String scanIntentSelection_p;
	bool filterRows_p;
	bool filterPols_p;
	bool filterChannels_p;

	// Own data selection indexes
	Vector<Int> arrayList_p;
	Vector<Int> fieldList_p;
	Vector<Int> scanList_p;
	Matrix<Double> timeList_p;
	Vector<Int> spwList_p;
	Matrix<Int> channelList_p;
	Vector<Int> antenna1List_p;
	Vector<Int> antenna2List_p;
	Matrix<Int> baselineList_p;
	Matrix<Double> uvwList_p;
	OrderedMap<Int, Vector<Int> > polarizationList_p;
	Vector<Int> observationList_p;
	Vector<Int> scanIntentList_p;

	// Lists of elements to be process
	vector<uInt> rowsIndex_p;
	vector<uInt> channelIndex_p;
	vector<uInt> polarizationIndex_p;

	// Flag CubeViews as requested by Urvashi
	CubeView<Bool> *privateFlagsView_p;
	CubeView<Bool> *commonFlagsView_p;

	// Thread state parameters
	volatile Bool terminationRequested_p;
	volatile Bool threadTerminated_p;
	volatile Bool processing_p;

	// Configuration
	void (casa::FlagAgentBase::*applyFlag_p)(uInt,uInt,uInt);
	Float (casa::FlagAgentBase::*applyVisExpr_p)(const Complex &);
	uShort dataReference_p;
	string expression_p;
	string dataColumn_p;
	Bool writePrivateFlagCube_p;
	Bool parallel_processing_p;
	Bool antennaNegation_p;
	Bool antennaMap_p;
	Bool profiling_p;
	Bool flag_p;
};

class FlagAgentList
{
	public:
		FlagAgentList();
		~FlagAgentList();

		// Methods to mimic vector
		void push_back(FlagAgentBase *agent_i);
		void pop_back();
		void clear();
		bool empty();
		size_t size();

		// Methods to mimic FlagAgentBase
		void start();
		void terminate ();
		void join ();
		void queueProcess();
		void completeProcess();
		void setProfiling(bool enable);
		void setCheckMode();

	protected:

	private:
		vector<FlagAgentBase *> container_p;
		vector<FlagAgentBase *>::iterator iterator_p;
};

} //# NAMESPACE CASA - END

#endif /* FlagAgentBase_H_ */

