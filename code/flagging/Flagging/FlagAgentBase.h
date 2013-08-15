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

#include <flagging/Flagging/FlagMSHandler.h>
#include <flagging/Flagging/FlagCalTableHandler.h>
#include <flagging/Flagging/FlagReport.h>
#include <casa/Containers/OrdMapIO.h>
#include <measures/Measures/Stokes.h>
#include <synthesis/MSVis/AsynchronousTools.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// A top level class defining the interface for flagging agents
// </summary>
//
// <use visibility=export>
//
// <prerequisite>
//   <li> <linkto class="VisBuffer2:description">FlagDataHandler</linkto>
//   <li> <linkto class="VisBuffer2:description">FlagReport</linkto>
// </prerequisite>
//
// <etymology>
// FlagAgentBase stands for a generic class, specific to the flagging operations
// </etymology>
//
// <synopsis>
//
// This is a top-level class defining the interface for flagging agents.
// There are various methods (virtual) that must be re-implemented by the specific derived
// classes, depending on the implemented algorithm. Here we find three categories:
//
// - Iteration approach methods:
//
//   - computeRowFlags(const VisBuffer &visBuffer, FlagMapper &flags, uInt row)
//     - For agents that only depend on meta-data for their flagging operations (for FlagAgentManual,FlagAgentElevation,FlagAgentShadow,FlagAgentQuack)
//     - This iteration method can also be used by agents that have to inspect the already existing flags (for FlagAgentSummary, FlagAgentExtension)
//
//   - computeInRowFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flags, uInt row);
//     - For agents that have to look into the visibility points, but regardless of their source baseline, like FlagAgentDisplay
//
//   - computeAntennaPairFlags(const VisBuffer &visBuffer,FlagMapper &flags,Int antenna1,Int antenna2,vector<uInt> &rows);
//     - For agents that have to look into the visibility points grouped by baseline (FlagAgentTimeFreqCrop,FlagAgentRFlag)
//
//   - computeAntennaPairFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flags,Int antenna1,Int antenna2,vector<uInt> &rows)
//     - For agents that have to look into the visibility points grouped by baseline, allowing user-driven navigation (FlagAgentDisplay)
//     - NOTE: This method has to be used in combination with iterateAntennaPairsInteractive(antennaPairMap *antennaPairMap_ptr)
//
// - Configuration methods:
//
//   - FlagAgentBase::FlagAgentBase
//     - Even though each derived agent has its specific constructor, it is necessary to call the base class constructor to set:
//       - The FlagDataHandler implementation pointer
//       - The iteration approach: For this the FlagAgentBase class contains an enumeration FlagAgentBase::iteration with the following modes:
//         - FlagAgentBase::ROWS: Iterate row by row and flag depending on the corresponding meta-data (for FlagAgentManual,FlagAgentQuack)
//         - FlagAgentBase::ROWS_PREPROCESS_BUFFER: Iterate row by row doing a pre-processing common to all the rows of each chunk (for FlagAgentElevation, FlagAgentShadow, FlagAgentSummary)
//         - FlagAgentBase::IN_ROWS: Iterate row by row and flag depending on the data column (for FlagAgentClipping)
//         - FlagAgentBase::ANTENNA_PAIRS: Iterate per baselines and flag depending on the data column (for FlagAgentTimeFreqCrop, FlagAgentRFlag)
//         - FlagAgentBase::ANTENNA_PAIRS_FLAGS: Iterate per baselines accessing the individual flag points (for  FlagAgentExtension)
//         - FlagAgentBase::ANTENNA_PAIRS_INTERACTIVE: Iterate per baselines interactively accessing the data column (for FlagAgentDisplay)
//
//   - setAgentParameters(Record config)
//     - To parse the agent-specific parameters, although there is also an implementation of
//       this method in the base class which has to be called to handle the following parameters:
//        - datacolumn: To specify the column in which the agent has to operate (see FlagAgentBase::datacolumn enumeration)
//        - correlation: To specify the correlation to be inspected for flagging (this also includes visibility expressions)
//        - meta-data selection parameters (field, spw, scan, baseline, etc): To feed the agent-level data selection engine (row filtering)
//
// - Information methods
//
//   - FlagReport getReport()
//     - To return a specific report to be shown by the display agent, or containing accumulated information (e.g.: summaries, rflag tresholds)
//
// Additionally there are public non-virtual methods to:
//
// - Handle processing in 'background' mode (for parallel flagging)
//   - start(): Start service (start thread run method)
//   - terminate(): Terminate service (forcing run method to finish)
//   - queueProcess(): To signal flagging of the current VisBuffer
//   - completeProcess(): Wait until completion of flagging process for the current VisBuffer
//
// - Print out percentage of flags produced by chunk or in total
//   - chunkSummary(): Accumulates statistics for each chunk
//   - tableSummary(): Accumulates statistics across the entire table selection
//
// </synopsis>
//
// <motivation>
// The motivation for the FlagAgentBase class is having all the iteration and filtering capabilities
// grouped in one single class, with a common interface for all the agents w/o introducing anything
// specific to the implementation of each algorithm, thus improving modularization and maintainability.
// </motivation>
//
//
// <example>
// The top level interface of a flagging agent is quite simple once it is configured, this is due to
// the fact that most of the complexity lies in the FlagDataHandler-FlagAgentBase interaction,
// which is hidden from the application layer (already explained in the FlagDataHandler documentation).
//
// <srcblock>
//
// // Create FlagDataHandler
// FlagDataHandler *dh = new FlagMSHandler(inputFile,iterationMode);
//
// // First of all define a configuration record (e.g.: quack)
// Record agentConfig;
// agentConfig.define("mode","quack");
// agentConfig.define("quackinterval",(Double)20);
//
// // Use the factory method to create the agent, and put it into a FlagAgentList
// FlagAgentList agentList;
// FlagAgentBase *agent = FlagAgentBase::create(dh,agentConfig);
// agentList.push_back(agent);
//
// // Iterate over chunks
// while (dh->nextChunk())
// {
//    // Iterates over buffers
//	  while (dh->nextBuffer())
//    {
//       // Apply agents on current VisBuffer
//       agentList.apply();
//
//       // Flush flags (only effective if there is a write access to the flag cube)
//       dh->flushFlags();
//    }
//
//	// Print chunk stats from each agent
//	agentList.chunkSummary();
// }
//
// // Print total stats from each agent
// agentList.tableSummary();
//
// // Stop flag agent
// agentList.terminate();
//
// </srcblock>
// </example>


class FlagAgentBase : public casa::async::Thread {

public:

	enum datacolumn {

		DATA=0,
		CORRECTED,
		MODEL,
		RESIDUAL,
		RESIDUAL_DATA,
		FPARAM,
		CPARAM,
		PARAMERR,
		SNR,
		WEIGHT_SPECTRUM,
		FLOAT_DATA
	};

	enum iteration {

		ROWS=0,
		ROWS_PREPROCESS_BUFFER,
		IN_ROWS,
		IN_ROWS_PREPROCESS_BUFFER,
		ANTENNA_PAIRS,
		ANTENNA_PAIRS_FLAGS,
		ANTENNA_PAIRS_INTERACTIVE,
		ANTENNA_PAIRS_PREPROCESS_BUFFER
	};

	FlagAgentBase(FlagDataHandler *dh, Record config, uShort iterationApproach, Bool writePrivateFlagCube = false, Bool flag = true);
	virtual ~FlagAgentBase ();
	static FlagAgentBase *create (FlagDataHandler *dh,Record config);

	void start();
	void terminate ();
	void queueProcess();
	void chunkSummary();
	void tableSummary();
	void completeProcess();
	void * run ();

	// Set function to activate profiling
	void setProfiling(bool enable) {profiling_p = enable;}

	// Set function to activate check mode
	void setCheckMode(bool enable) {checkFlags_p = enable;}

	// Externally visible configuration
	Bool backgroundMode_p;
	LogIO::Command logLevel_p;
	Bool apply_p;
	Bool flag_p;

        // Get a report Record from the agent, at the end of the run
        // The report returned by getReport() can be of multiple types
        //   -- a single report of type "none"  : FlagReport("none",agentName_p)
        //   -- a single report of type "plot" : FlagReport("plot",agentName_p)
        //   -- a list of reports  : 
        //          FlagReport repList("list");
        //          repList.addReport( FlagReport("plot",agentName_p) );
        //          repList.addReport( FlagReport("plot",agentName_p) );
        virtual FlagReport getReport();

protected:

	void initialize();

	// Convenience function to be shared by parallel/non-parallel mode
	void runCore();

	void setDataSelection(Record config);
	// TODO: This class must be re-implemented in the derived classes
	virtual void setAgentParameters(Record config);
	// Method to sanitize correlation expression and keep going
	String sanitizeCorrExpression(String corrExpression, std::vector<String> *corrProducts);

	void generateAllIndex();
	void generateRowsIndex(uInt nRows);
	void generateChannelIndex(uInt nChannels);
	void generatePolarizationIndex(uInt nPolarizations);
	std::vector<uInt> * generateAntennaPairRowsIndex(Int antenna1, Int antenna2);

	// Generate index for all rows
	void indigen(vector<uInt> &index, uInt size);

	// For checking ids
	bool find(Vector<Int> &validRange, Int element);

	// For checking ranges
	bool find(Matrix<Double> &validRange, Double element);

	// For checking pairs
	bool find(Matrix<Int> &validPairs, Int element1, Int element2);

	// For checking columns
	bool find(Block<int> &columns, int col);

	// Check if a given number is nan (for visibilities,gains and Tsys primarily)
	bool isNaN(Double number);
	bool isNaN(Float number);
	bool isZero(Double number);
	bool isZero(Float number);
	bool isNaNOrZero(Float number);
	bool isNaNOrZero(Double number);

	// Check if buffer has to be processed
	bool checkIfProcessBuffer();

	// Common functionality for each visBuffer (don't repeat at the row level)
	virtual void preProcessBuffer(const vi::VisBuffer2 &visBuffer);

	// Iterate trough list of rows
	void iterateRows();

	// Iterate trough visibilities mapper
	void iterateInRows();

	// Iterate trough list of antenna pairs
	void iterateAntennaPairs();

	// Iterate trough list of antenna pairs w/o loading visibilities
	void iterateAntennaPairsFlags();

	// Methods to interactively iterate trough list of antenna pairs
	void processAntennaPair(Int antenna1,Int antenna2);
	virtual void iterateAntennaPairsInteractive(antennaPairMap *antennaPairMap_ptr);

	// Iter-passes method
	virtual void passIntermediate(const vi::VisBuffer2 &visBuffer);
	virtual void passFinal(const vi::VisBuffer2 &visBuffer);

	// Mapping functions as requested by Urvashi
	void setVisibilitiesMap(std::vector<uInt> *rows,VisMapper *visMap);
	void setFlagsMap(std::vector<uInt> *rows, FlagMapper *flagMap);
	Bool checkVisExpression(polarizationMap *polMap);

	// Compute flags for a given visibilities point
	virtual bool computeRowFlags(const vi::VisBuffer2 &visBuffer, FlagMapper &flags, uInt row);

	// Compute flags for a given visibilities point
	virtual bool computeInRowFlags(const vi::VisBuffer2 &visBuffer, VisMapper &visibilities,FlagMapper &flags, uInt row);

	// Compute flags for a given (time,freq) antenna pair map
	virtual bool computeAntennaPairFlags(const vi::VisBuffer2 &visBuffer, VisMapper &visibilities,FlagMapper &flags,Int antenna1,Int antenna2,vector<uInt> &rows);

	// Compute flags for a given (time,freq) antenna pair map w/o using visibilities
	virtual bool computeAntennaPairFlags(const vi::VisBuffer2 &visBuffer,FlagMapper &flags,Int antenna1,Int antenna2,vector<uInt> &rows);

	// Common used members that must be accessible to derived classes
	FlagDataHandler *flagDataHandler_p;
	casa::LogIO *logger_p;
	String agentName_p;
	String mode_p;

	// Flag counters
	uInt64 chunkFlags_p;
	uInt64 chunkNaNs_p;
	uInt64 tableFlags_p;
	uInt64 tableNaNs_p;
	uInt64 visBufferFlags_p;
	bool flagRow_p;

	// Multithreading configuration and agent id
	Bool multiThreading_p;
	Int nThreads_p;
	Int threadId_p;

	// Running configuration
	Bool prepass_p;

private:
	
	vi::VisBuffer2 *visibilityBuffer_p;

	// MS-related objects
	Cube<Bool> *commonFlagCube_p;
	Cube<Bool> *originalFlagCube_p;
	Cube<Bool> *privateFlagCube_p;

	Vector<Bool> *commonFlagRow_p;
	Vector<Bool> *originalFlagRow_p;
	Vector<Bool> *privateFlagRow_p;

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
	bool flagAutoCorrelations_p;
	Bool antennaNegation_p;

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
	Bool uvwUnits_p;
	OrderedMap<Int, Vector<Int> > polarizationList_p;
	Vector<Int> observationList_p;
	Vector<Int> scanIntentList_p;

	// Thread state parameters
	volatile Bool terminationRequested_p;
	volatile Bool threadTerminated_p;
	volatile Bool processing_p;

	// Data source configuration
	String expression_p;
	String dataColumn_p;
	uShort dataReference_p;

	// Debugging configuration
	Bool profiling_p;
	Bool checkFlags_p;

	// Running mode configuration
	uShort iterationApproach_p;

	// Flagging mode configuration
	Bool writePrivateFlagCube_p;

protected:
	// Lists of elements to process
	// jagonzal (CAS-4312): We need channelIndex_p available for the Rflag agent,
	// in order to take into account channel selection for the frequency mapping
	vector<uInt> rowsIndex_p;
	vector<uInt> channelIndex_p;
	vector<uInt> polarizationIndex_p;

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
		void apply(bool sequential = false);
		void chunkSummary();
		void tableSummary();
		void setProfiling(bool enable);
		void setCheckMode(bool enable);

                // Method to accumulate reports from all agents
                FlagReport gatherReports();

	protected:

	private:
		vector<FlagAgentBase *> container_p;
		vector<FlagAgentBase *>::iterator iterator_p;
};

} //# NAMESPACE CASA - END

#endif /* FlagAgentBase_H_ */

