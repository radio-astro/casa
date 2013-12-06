//# FlagDataHandler.h: This file contains the interface definition of the FlagDataHandler class.
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

#ifndef FLAGDATAHANDLER_H_
#define FLAGDATAHANDLER_H_

// Measurement Set selection
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSProcessorColumns.h>

// VI/VB infrastructure
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisibilityIterator2.h>

// .casarc interface
#include <casa/System/AipsrcValue.h>

// Records interface
#include <casa/Containers/Record.h>

// System utilities (for profiling macros)
#include <casa/OS/HostInfo.h>
#include <sys/time.h>

// Data mapping
#include <algorithm>
#include <map>

#define STARTCLOCK timeval start,stop; double elapsedTime; if (profiling_p) gettimeofday(&start,0);
#define STOPCLOCK if (profiling_p) \
	{\
		gettimeofday(&stop,0);\
		elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;\
		*logger_p << LogIO::DEBUG2 << "FlagDataHandler::" << __FUNCTION__ << " Executed in: " << elapsedTime << " ms, Memory free: " << HostInfo::memoryFree( )/1024.0 << " MB" << LogIO::POST;\
	}

namespace casa { //# NAMESPACE CASA - BEGIN

// Type definitions
typedef std::map< std::pair<Int,Int>,std::vector<uInt> >::iterator antennaPairMapIterator;
typedef std::map< Double,std::vector<uInt> >::iterator subIntegrationMapIterator;
typedef std::map< uShort,uShort >::iterator polartizationMapIterator;
typedef std::map< std::pair<Int,Int>,std::vector<uInt> > antennaPairMap;
typedef std::map< Double,std::vector<uInt> > subIntegrationMap;
typedef std::map< uShort,uShort > polarizationMap;
typedef std::map< uInt,String > polarizationIndexMap;
typedef std::vector< vector<Double> > antennaPointingMap;
typedef std::map< Int,vector<Double> > scanStartStopMap;
typedef std::map< Int,Double > lambdaMap;

const Complex ImaginaryUnit = Complex(0,1);

// We need to have the CubeView definition here because its type is used by FlagDataHandler class
template<class T> class CubeView
{

public:

	CubeView(Cube<T> *parentCube, std::vector<uInt> *rows = NULL, std::vector<uInt> *channels = NULL, std::vector<uInt> *polarizations = NULL)
	{
		parentCube_p = parentCube;
		IPosition baseCubeShape = parentCube_p->shape();
		reducedLength_p = IPosition(3);

		if (((polarizations != NULL) and (polarizations->size() > 0)) and
			((channels != NULL) and (channels->size() > 0)) and
			((rows != NULL) and (rows->size() > 0)))
		{
			access_p = &CubeView::accessMapped;
		}
		else if (((polarizations != NULL) and (polarizations->size() > 0)) and
				((channels != NULL) and (channels->size() > 0)))
		{
			access_p = &CubeView::accessIndex12Mapped;
		}
		else if (((polarizations != NULL) and (polarizations->size() > 0)) and
				((rows != NULL) and (rows->size() > 0)))
		{
			access_p = &CubeView::accessIndex13Mapped;
		}
		else if (((channels != NULL) and (channels->size() > 0)) and
				((rows != NULL) and (rows->size() > 0)))
		{
			access_p = &CubeView::accessIndex23Mapped;
		}
		else if ((polarizations != NULL) and (polarizations->size() > 0))
		{
			access_p = &CubeView::accessIndex1Mapped;
		}
		else if ((channels != NULL) and (channels->size() > 0))
		{
			access_p = &CubeView::accessIndex2Mapped;
		}
		else if ((rows != NULL) and (rows->size() > 0))
		{
			access_p = &CubeView::accessIndex3Mapped;
		}
		else
		{
			access_p = &CubeView::accessUnmapped;
		}

		if ((polarizations != NULL) and (polarizations->size() > 0))
		{
			polarizations_p = polarizations;
			reducedLength_p(0) = polarizations_p->size();
		}
		else
		{
			polarizations_p = NULL;
			reducedLength_p(0) = baseCubeShape(0);
		}

		if ((channels != NULL) and (channels->size() > 0))
		{
			channels_p = channels;
			reducedLength_p(1) = channels_p->size();
		}
		else
		{
			channels_p = NULL;
			reducedLength_p(1) = baseCubeShape(1);
		}

		if ((rows != NULL) and (rows->size() > 0))
		{
			rows_p = rows;
			reducedLength_p(2) = rows_p->size();
		}
		else
		{
			rows_p = NULL;
			reducedLength_p(2) = baseCubeShape(2);
		}
	}

    T &operator()(uInt i1, uInt i2, uInt i3)
    {
    	return (*this.*access_p)(i1,i2,i3);
    }

    const IPosition &shape() const
    {
    	return reducedLength_p;
    }

    void shape(Int &s1, Int &s2, Int &s3) const
    {
    	s1 = reducedLength_p(0);
    	s2 = reducedLength_p(1);
    	s3 = reducedLength_p(2);
    	return;
    }

protected:

    vector<uInt> *createIndex(uInt size)
    {
    	vector<uInt> *index = new vector<uInt>(size);
    	index->clear();
    	for (uInt i=0; i<size; i++ )
    	{
    		index->push_back(i);
    	}
    	return index;
    }

    T &accessUnmapped(uInt i1, uInt i2, uInt i3)
    {
    	return parentCube_p->at(i1,i2,i3);
    }

    T &accessMapped(uInt i1, uInt i2, uInt i3)
    {
    	uInt i1_index = polarizations_p->at(i1);
    	uInt i2_index = channels_p->at(i2);
    	uInt i3_index = rows_p->at(i3);
    	return parentCube_p->at(i1_index,i2_index,i3_index);
    }

    T &accessIndex1Mapped(uInt i1, uInt i2, uInt i3)
    {
    	uInt i1_index = polarizations_p->at(i1);
    	return parentCube_p->at(i1_index,i2,i3);
    }

    T &accessIndex2Mapped(uInt i1, uInt i2, uInt i3)
    {
    	uInt i2_index = channels_p->at(i2);
    	return parentCube_p->at(i1,i2_index,i3);
    }

    T &accessIndex3Mapped(uInt i1, uInt i2, uInt i3)
    {
    	uInt i3_index = rows_p->at(i3);
    	return parentCube_p->at(i1,i2,i3_index);
    }

    T &accessIndex12Mapped(uInt i1, uInt i2, uInt i3)
    {
    	uInt i1_index = polarizations_p->at(i1);
    	uInt i2_index = channels_p->at(i2);
    	return parentCube_p->at(i1_index,i2_index,i3);
    }

    T &accessIndex13Mapped(uInt i1, uInt i2, uInt i3)
    {
    	uInt i1_index = polarizations_p->at(i1);
    	uInt i3_index = rows_p->at(i3);
    	return parentCube_p->at(i1_index,i2,i3_index);
    }

    T &accessIndex23Mapped(uInt i1, uInt i2, uInt i3)
    {
    	uInt i2_index = channels_p->at(i2);
    	uInt i3_index = rows_p->at(i3);
    	return parentCube_p->at(i1,i2_index,i3_index);
    }

private:
    Cube<T> *parentCube_p;
	std::vector<uInt> *rows_p;
	std::vector<uInt> *channels_p;
	std::vector<uInt> *polarizations_p;
	IPosition reducedLength_p;
	T &(casa::CubeView<T>::*access_p)(uInt,uInt,uInt);
};

template<class T> class VectorView
{

public:
	VectorView(Vector<T> *parentVector, std::vector<uInt> *rows = NULL)
	{
		IPosition parentVectorShape = parentVector->shape();
		parentVector_p = parentVector;
		reducedLength_p = IPosition(1);

		if ((rows != NULL) and (rows->size() > 0))
		{
			access_p = &VectorView::accessMapped;
		}
		else
		{
			access_p = &VectorView::accessUnmapped;
		}

		if ((rows != NULL) and (rows->size() > 0))
		{
			rows_p = rows;
			reducedLength_p(0) = rows_p->size();
		}
		else
		{
			rows_p = NULL;
			reducedLength_p(0) = parentVectorShape(0);
		}
	}

    T &operator()(uInt i1)
    {
    	return (*this.*access_p)(i1);
    }

    const IPosition &shape() const
    {
    	return reducedLength_p;
    }

    void shape(Int &s1) const
    {
    	s1 = reducedLength_p(0);
    	return;
    }

protected:

    vector<uInt> *createIndex(uInt size)
    {
    	vector<uInt> *index = new vector<uInt>(size);
    	index->clear();
    	for (uInt i=0; i<size; i++ )
    	{
    		index->push_back(i);
    	}
    	return index;
    }

    T &accessUnmapped(uInt i1)
    {
    	return parentVector_p->operator()(i1);
    }

    T &accessMapped(uInt i1)
    {
    	uInt i1_index = rows_p->at(i1);
    	return parentVector_p->operator()(i1_index);
    }

private:
    Vector<T> *parentVector_p;
	std::vector<uInt> *rows_p;
	IPosition reducedLength_p;
	T &(casa::VectorView<T>::*access_p)(uInt);
};

class VisMapper
{
	typedef Complex (casa::VisMapper::*corrProduct)(uInt,uInt);

public:

	enum calsolutions {

		CALSOL1=Stokes::NumberOfTypes,
		CALSOL2,
		CALSOL3,
		CALSOL4
	};

	VisMapper(String expression,polarizationMap *polMap,CubeView<Complex> *leftVis,CubeView<Complex> *rightVis=NULL);
	VisMapper(String expression,polarizationMap *polMap);
	~VisMapper();

    void setParentCubes(CubeView<Complex> *leftVis,CubeView<Complex> *rightVis=NULL);

    vector< vector<uInt> > getSelectedCorrelations() { return selectedCorrelations_p;}
    vector< string > getSelectedCorrelationStrings() { return selectedCorrelationStrings_p;}

	Float operator()(uInt chan, uInt row);
	Float operator()(uInt pol, uInt chan, uInt row);

	// Direct access to the complex correlation product
	Complex correlationProduct(uInt pol, uInt chan, uInt row);

    // NOTE: reducedLength_p is defined as [chan,row,pol]
    const IPosition &shape() const
    {
    	return reducedLength_p;
    }

    void shape(Int &chan, Int &row) const
    {
    	chan = reducedLength_p(0);
    	row = reducedLength_p(1);
    	return;
    }

    void shape(Int &pol, Int &chan, Int &row) const
    {
    	chan = reducedLength_p(0);
    	row = reducedLength_p(1);
    	pol = reducedLength_p(2);
    	return;
    }


protected:
    void setExpressionMapping(String expression,polarizationMap *polMap);
	Float real(Complex val) {return val.real();}
	Float imag(Complex val) {return val.imag();}
	Float abs(Complex val) {return std::abs(val);}
	Float arg(Complex val) {return std::arg(val);}
	Float norm(Complex val) {return std::norm(val);}
	Complex leftVis(uInt pol, uInt chan, uInt row);
	Complex diffVis(uInt pol, uInt chan, uInt row);
	Complex stokes_i(uInt pol, uInt chan);
	Complex stokes_q(uInt pol, uInt chan);
	Complex stokes_u(uInt pol, uInt chan);
	Complex stokes_v(uInt pol, uInt chan);
	Complex linear_xx(uInt pol, uInt chan);
	Complex linear_yy(uInt pol, uInt chan);
	Complex linear_xy(uInt pol, uInt chan);
	Complex linear_yx(uInt pol, uInt chan);
	Complex circular_rr(uInt pol, uInt chan);
	Complex circular_ll(uInt pol, uInt chan);
	Complex circular_rl(uInt pol, uInt chan);
	Complex circular_lr(uInt pol, uInt chan);
	Complex stokes_i_from_linear(uInt chan, uInt row);
	Complex stokes_q_from_linear(uInt chan, uInt row);
	Complex stokes_u_from_linear(uInt chan, uInt row);
	Complex stokes_v_from_linear(uInt chan, uInt row);
	Complex stokes_i_from_circular(uInt chan, uInt row);
	Complex stokes_q_from_circular(uInt chan, uInt row);
	Complex stokes_u_from_circular(uInt chan, uInt row);
	Complex stokes_v_from_circular(uInt chan, uInt row);
	Complex calsol1(uInt chan, uInt row);
	Complex calsol2(uInt chan, uInt row);
	Complex calsol3(uInt chan, uInt row);
	Complex calsol4(uInt chan, uInt row);


private:
	Float (casa::VisMapper::*applyVisExpr_p)(Complex);
	Complex (casa::VisMapper::*getVis_p)(uInt,uInt,uInt);
	Complex (casa::VisMapper::*getCorr_p)(uInt,uInt);
	vector<corrProduct> selectedCorrelationProducts_p;
	vector< vector<uInt> > selectedCorrelations_p;
	vector<string> selectedCorrelationStrings_p;
	CubeView<Complex> *leftVis_p;
	CubeView<Complex> *rightVis_p;
	IPosition reducedLength_p;
	polarizationMap *polMap_p;
	String expression_p;
};

class FlagMapper
{

public:

	FlagMapper(Bool flag,	vector < vector<uInt> > selectedCorrelations,
							CubeView<Bool> *commonFlagsView,
							CubeView<Bool> *originalFlagsView,
							CubeView<Bool> *privateFlagsView=NULL,
							VectorView<Bool> *commonFlagRowView=NULL,
							VectorView<Bool> *originalFlagRowView=NULL,
							VectorView<Bool> *privateFlagRowView=NULL);
	FlagMapper(Bool flag,vector< vector<uInt> > selectedCorrelations);
	~FlagMapper();

	void setParentCubes(CubeView<Bool> *commonFlagsView,CubeView<Bool> *originalFlagsView,CubeView<Bool> *privateFlagsView=NULL);
	void setParentFlagRow(VectorView<Bool> *commonFlagRowView,VectorView<Bool> *originalFlagRowView,VectorView<Bool> *privateFlagRowView=NULL);

	void applyFlag(uInt chan, uInt row);
	void applyFlag(uInt pol, uInt channel, uInt row);
	void applyFlagRow(uInt row);
	void applyFlagInRow(uInt row);

	Bool getOriginalFlags(uInt chan, uInt row);
	Bool getModifiedFlags(uInt chan, uInt row);
	Bool getPrivateFlags(uInt chan, uInt row);

	Bool getOriginalFlags(uInt pol, uInt channel, uInt row);
	Bool getModifiedFlags(uInt pol, uInt channel, uInt row);
	Bool getPrivateFlags(uInt pol, uInt channel, uInt row);

	// These methods are needed for flag extension
	void setModifiedFlags(uInt pol, uInt channel, uInt row);
	void setPrivateFlags(uInt pol, uInt channel, uInt row);

	Bool getOriginalFlagRow(uInt row);
	Bool getModifiedFlagRow(uInt row);
	Bool getPrivateFlagRow(uInt row);

    const IPosition &shape() const
    {
    	return reducedLength_p;
    }

    void shape(Int &chan, Int &row) const
    {
    	chan = reducedLength_p(0);
    	row = reducedLength_p(1);
    	return;
    }

    void shape(Int &pol, Int &chan, Int &row) const
    {
    	chan = reducedLength_p(0);
    	row = reducedLength_p(1);
    	pol = reducedLength_p(2);
    	return;
    }

	vector< vector<uInt> > getSelectedCorrelations() {return selectedCorrelations_p;}

    void activateCheckMode() {applyFlag_p = &FlagMapper::checkCommonFlags;}

    uInt nSelectedCorrelations() {return nSelectedCorrelations_p;}
    uInt flagsPerRow() {return flagsPerRow_p;}

protected:

	void setExpressionMapping(vector< vector<uInt> > selectedCorrelations);

	// Apply flags to common flag cube
	void applyCommonFlags(uInt pol, uInt channel, uInt row);
	// Apply flags to common and private flag cubes
	void applyPrivateFlags(uInt pol, uInt channel, uInt row);
	// Apply flags to common and private flag cubes
	void checkCommonFlags(uInt pol, uInt channel, uInt row);

	// Apply flags to common flag rows
	void applyCommonFlagRow(uInt row);
	// Apply flags to common and private flag rows
	void applyPrivateFlagRow(uInt row);

private:

	Bool flag_p;
    IPosition reducedLength_p;
	CubeView<Bool> *commonFlagsView_p;
	CubeView<Bool> *originalFlagsView_p;
	CubeView<Bool> *privateFlagsView_p;
	VectorView<Bool> *commonFlagRowView_p;
	VectorView<Bool> *originalFlagRowView_p;
	VectorView<Bool> *privateFlagRowView_p;
	vector< vector<uInt> > selectedCorrelations_p;
	uInt nSelectedCorrelations_p;
	uInt flagsPerRow_p;
	void (casa::FlagMapper::*applyFlag_p)(uInt,uInt,uInt);
	void (casa::FlagMapper::*applyFlagRow_p)(uInt);
};

// <summary>
// A top level class defining the data handling interface for the flagging module
// </summary>
//
// <use visibility=export>
//
// <prerequisite>
//   <li> <linkto class="VisBuffer:description">VisBuffer</linkto>
//   <li> <linkto class="FlagMapper:description">FlagMapper</linkto>
//   <li> <linkto class="VisMapper:description">VisMapper</linkto>
// </prerequisite>
//
// <etymology>
// FlagDataHandler stands for generic data handling (i.e. MSs, CalTables, ...) specific to the flagging module
// </etymology>
//
// <synopsis>
//
// This is a top-level class defining the data handling interface for the flagging module.
// There are various methods (virtual) that must be re-implemented by the specific derived
// classes (e.g. FlagMSHandler, FlagCalTableHandler). These methods essentially cover:
//
// - Table access (i.e. open/close/iteration/rw)
//
// - Table selection (i.e. expression parsing, sub-table selection)
//
// Additionally there are public non-virtual methods to:
//
// - Set configuration parameters (pre-sort columns, data selection, time interval, async I/O)
//
// - Enable and access different kinds of mapping (baselines, correlation products, antenna pointing)
//
// Also at this top level there are public members which are used by the FlagAgent classes,
// so that there is no dependency with the specific implementation classes (e.g. FlagMsHandler,
// FlagCalTableHandler), and thus no re-implementation is required at the FlagAgent level.
//
// - VisBuffer (for accessing the data and meta data columns)
//
// - Chunk and Table flag counters (for statistics)
//
// </synopsis>
//
// <motivation>
// The motivation for the FlagDataHandler class is having all the data operations encapsulated
// in one single class, with a common interface for all types of tables (MSs, CalTables, SingleDish),
// so that no specific specific table type implementation has to be done at the FlagAgent level.
// </motivation>
//
// <example>
// <srcblock>
//
// // The following code sets up a FlagDataHandler with either CalTable or MS implementation and
// // iterates through the table applying a clip agent, flushing the flags, and extracting summaries.
//
// // IMPORTANT NOTE:
// // The order of FlagDataHandler and FlagAgent initialization is critical to have everything right,
// // in particular data selection must happen before initializing FlagAgents, and iterator generation
// // must be done after initializing FlagAgents, so that each agent can communicate to the FlagDataHandler
// // which columns have to be pre-fetched (async i/o or parallel mode), and what mapping options are necessary.
//
// // NOTE ON ASYNC I/O:
// // Asyncnronous I/O is only enabled for MS-type tables, but not for CalTables, and it is necessary to switch
// // it on before generating the iterators. Something else to take into account, is that there are 2 global
// // switches at .casarc level which invalidate the application code selection:
// //
// // VisibilityIterator.async.enabled rules over
// // |-> FlagDataHandler.asyncio, and in turns rules over
// //     |-> FlagDataHandler.enableAsyncIO(True)
//
// // Identify table type
// Table table(msname_p,TableLock(TableLock::AutoNoReadLocking));
// TableInfo& info = table.tableInfo();
// String type=info.type();
//
// // Create derived FlagDataHandler object with corresponding implementation
// FlagDataHandler *fdh_p = NULL;
// if (type == "Measurement Set")
// {
//    fdh_p = new FlagMSHandler(msname_p, FlagDataHandler::COMPLETE_SCAN_UNMAPPED, timeInterval_p);
// }
// else
// {
//    fdh_p = new FlagCalTableHandler(msname_p, FlagDataHandler::COMPLETE_SCAN_UNMAPPED, timeInterval_p);
// }
//
// // NOTE: It is also possible to independently set the iteration approach via the setIterationApproach
// //       method which accepts the following modes, defined in the FlagDataHandler iteration enumeration
// //
// //       COMPLETE_SCAN_MAPPED:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, SCAN_NUMBER, FIELD_ID, DATA_DESC_ID and TIME
// //       - Group all time steps together, so that there is no sub-chunk iteration
// //       - Generate baseline maps (to iterate trough rows with the same antenna1, antenna2)
// //       - Generate sub-integration maps (to iterate trough rows with the same timestamp)
// //       COMPLETE_SCAN_MAP_SUB_INTEGRATIONS_ONLY:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, SCAN_NUMBER, FIELD_ID, DATA_DESC_ID and TIME
// //       - Group all time steps together, so that there is no sub-chunk iteration
// //       * Don't generate baseline maps
// //       - Generate sub-integration maps (to iterate trough rows with the same timestamp)
// //       COMPLETE_SCAN_MAP_ANTENNA_PAIRS_ONLY:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, SCAN_NUMBER, FIELD_ID, DATA_DESC_ID and TIME
// //       - Group all time steps together, so that there is no sub-chunk iteration
// //       - Generate baseline maps (to iterate trough rows with the same antenna1, antenna2)
// //       * Don't generate sub-integration maps
// //       COMPLETE_SCAN_UNMAPPED:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, SCAN_NUMBER, FIELD_ID, DATA_DESC_ID and TIME
// //       - Group all time steps together, so that there is no sub-chunk iteration
// //       * Don't generate baseline maps
// //       * Don't generate sub-integration maps
// //       COMBINE_SCANS_MAPPED:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, FIELD_ID, DATA_DESC_ID and TIME
// //       - Group all time steps together, so that there is no sub-chunk iteration
// //       - Generate baseline maps (to iterate trough rows with the same antenna1, antenna2)
// //       - Generate sub-integration maps (to iterate trough rows with the same timestamp)
// //       COMBINE_SCANS_MAP_SUB_INTEGRATIONS_ONLY:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, FIELD_ID, DATA_DESC_ID and TIME
// //       - Group all time steps together, so that there is no sub-chunk iteration
// //       * Don't generate baseline maps
// //       - Generate sub-integration maps (to iterate trough rows with the same timestamp)
// //       COMBINE_SCANS_MAP_ANTENNA_PAIRS_ONLY:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, FIELD_ID, DATA_DESC_ID and TIME
// //       - Group all time steps together, so that there is no sub-chunk iteration
// //       - Generate baseline maps (to iterate trough rows with the same antenna1, antenna2)
// //       * Don't generate sub-integration maps
// //       COMBINE_SCANS_UNMAPPED:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, FIELD_ID, DATA_DESC_ID and TIME
// //       - Group all time steps together, so that there is no sub-chunk iteration
// //       * Don't generate baseline maps
// //       * Don't generate sub-integration maps
// //       ANTENNA_PAIR:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, FIELD_ID, ANTENNA1, ANTENNA2, DATA_DESC_ID and TIME
// //       - Group all time steps together, so that there is no sub-chunk iteration
// //       * Don't generate baseline maps (they are not necessary because the chunks have constant ANTENNA1,ANTENNA2)
// //       * Don't generate sub-integration maps
// //       SUB_INTEGRATION:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, SCAN_NUMBER, FIELD_ID, DATA_DESC_ID and TIME
// //       - Don't group all time steps together, so it is necessary to add an inner sub-chunk iteration loop
// //       * Don't generate baseline maps (it is not possible because not all the rows corresponding to a given baseline are available)
// //       * Don't generate sub-integration maps (they are not necessary because the sub-chunks have constant TIME)
// //       ARRAY_FIELD:
// //       - Sort by OBSERVATION_ID, ARRAY_ID, FIELD_ID, DATA_DESC_ID and TIME
// //       - Don't group all time steps together, so it is necessary to add an inner sub-chunk iteration loop
// //       * Don't generate baseline maps (it is not possible because not all the rows corresponding to a given baseline are available)
// //       * Don't generate sub-integration maps (they are not necessary because the sub-chunks have constant TIME)
// //       * NOTE: This is the iteration approach used by the old flagger framework
//
// // Open table
// fdh_p->open();
//
// // Parse data selection to Flag Data Handler
// fdh_p->setDataSelection(dataSelection);
//
// // Select data (thus creating selected table)
// fdh_p->selectData();
//
// // Create flagging agent and list
// Record agentConfig;
// agentConfig.define("mode","clip")
// FlagAgentBase *agent = FlagAgentBase::create(fdh_p,agentConfig);
// FlagAgentList agentList;
// agentList.push_back(agent);
//
// // Switch on/off async i/p
// fdh_p->enableAsyncIO(true);
//
// // Generate table iterator
// fdh_p->generateIterator();
//
// // Start Flag Agent
// agentList.start();
//
// // Iterates over chunks (constant column values)
// while (fdh_p->nextChunk())
// {
//    // Iterates over buffers (time steps)
//    while (fdh_p->nextBuffer())
//    {
//       // Apply flags
//       agentList.apply();
//       // Flush flag cube
//       fdh_p->flushFlags();
//    }
//
//    // Print end-of-chunk statistics
//    agentList.chunkSummary();
// }
//
// // Print total stats from each agent
// agentList.msSummary();
//
// // Stop Flag Agent
// agentList.terminate();
// agentList.join();
//
// // Close MS
// fdh_p->close();
//
// // Clear Flag Agent List
// agentList.clear();
//
// </srcblock>
// </example>
//
// <example>
// <srcblock>
//
// // The following code shows the FlagAgent-FlagDataHandler interaction works internally:
//
// // First of all, at construction time, each agent has to communicate to the FlagDataHandler
// // which columns have to be pre-fetched (for async i/o or parallel mode) and what mapping
// // options are necessary

// // ...for instance in the case of FlagAgentShadow we need Ant1,Ant2,UVW,TimeCentroid and PhaseCenter:
// flagDataHandler_p->preLoadColumn(vi::Antenna1);
// flagDataHandler_p->preLoadColumn(vi::Antenna2);
// flagDataHandler_p->preLoadColumn(vi::uvw);
// flagDataHandler_p->preLoadColumn(vi::TimeCentroid);
// flagDataHandler_p->preLoadColumn(vi::PhaseCenter);
//
// // ...and FlagAgentElevation needs to have the antenna pointing information globally available for each chunk:
// flagDataHandler_p->setMapAntennaPointing(true);
//
// // Then, at iteration time, the FlagAgentBase class has access to the VisBuffer held
// // in the FlagDataHandler, in order to retrieve the meta-data columns needed for the
// // data selection engine (agent-level row filtering).
// // NOTE: The VisBuffer is actually held within an auto-pointer wrapper,
// //       thus there is an additional get() involved when accessing it.
//
// if (spwList_p.size())
// {
//    if (!find(spwList_p,visibilityBuffer_p->get()->spectralWindow())) return false;
// }
//
// // The sorting columns used for the iteration are also accessible to optimize the selection engine:
// // (e.g.: If scan is constant check only 1st row)
// if ( (scanList_p.size()>0) and (find(flagDataHandler_p->sortOrder_p,MS::SCAN_NUMBER)==true) )
// {
//    if (!find(scanList_p,visibilityBuffer_p->get()->scan()[0])) return false;
// }
//
// // Once that chunk/rows are evaluated as eligible for the flagging process
// // by the data selection engine, the previously booked maps at construction
// // time can be access in order to iterate trough the data as desired:
// // e.g.: Baseline (antenna pairs) iteration
// for (myAntennaPairMapIterator=flagDataHandler_p->getAntennaPairMap()->begin();
//      myAntennaPairMapIterator != flagDataHandler_p->getAntennaPairMap()->end();
//      ++myAntennaPairMapIterator)
// {
//    // NOTE: The following code is also encapsulated in the FlagAgentBase::processAntennaPair(Int antenna1,Int antenna2) code
//
//    // From the antenna map we can retrieve the rows corresponding to the baseline defined by the antenna pair
//    vector<uInt> baselineRows = (*flagDataHandler_p->getAntennaPairMap())[std::make_pair(antennaPair.first,antennaPair.second)];
//
//    // This rows can be now inserted in the mapper classes (VisMapper and FlagMapper using the CubeView<T> template class)
//    VisMapper visibilitiesMap = VisMapper(expression_p,flagDataHandler_p->getPolarizationMap());
//    FlagMapper flagsMap = FlagMapper(flag_p,visibilitiesMap.getSelectedCorrelations());
//    setVisibilitiesMap(antennaRows,&visibilitiesMap);
//    setFlagsMap(antennaRows,&flagsMap);
// }
//
// // Finally, after flagging time, the FlagAgent can communicate to the FlagDataHandler
// // that the modified FlagCube has to be flushed to disk, this is a small but very important
// // step in order to avoid unnecessary I/O activity when a chunk is not eligible for flagging
// // or the auto-flagging algorithms don't produce any flags.
//
// // If any row was flag, then we have to flush the flagRow
// if (flagRow_p) flagDataHandler_p->flushFlagRow_p = true;
// // If any flag was raised, then we have to flush the flagCube
// if (visBufferFlags_p>0) flagDataHandler_p->flushFlags_p = true;
//
// </srcblock>
// </example>

class FlagDataHandler
{

public:

	enum iteration {

		COMPLETE_SCAN_MAPPED=0,
		COMPLETE_SCAN_MAP_SUB_INTEGRATIONS_ONLY,
		COMPLETE_SCAN_MAP_ANTENNA_PAIRS_ONLY,
		COMPLETE_SCAN_UNMAPPED,
		COMBINE_SCANS_MAPPED,
		COMBINE_SCANS_MAP_SUB_INTEGRATIONS_ONLY,
		COMBINE_SCANS_MAP_ANTENNA_PAIRS_ONLY,
		COMBINE_SCANS_UNMAPPED,
		ANTENNA_PAIR,
		SUB_INTEGRATION,
		ARRAY_FIELD
	};

	enum tableType {

		MEASUREMENT_SET=0,
		CALIBRATION_TABLE
	};

	// Default constructor
	// NOTE: Time interval 0 groups all time steps together in one chunk.
	FlagDataHandler(string msname, uShort iterationApproach = SUB_INTEGRATION, Double timeInterval = 0);

	// Default destructor
	virtual ~FlagDataHandler();

	// Common MS/CalTables public interface
	virtual bool open() {return false;}
	virtual bool close() {return false;}
	virtual bool selectData() {return false;}
	virtual bool generateIterator() {return false;}
	virtual bool nextChunk() {return false;}
	virtual bool nextBuffer() {return false;}
	virtual bool flushFlags() {return false;}
	virtual String getTableName() {return String("none");}
	virtual bool parseExpression(MSSelection &/*parser*/) {return true;}
	virtual bool checkIfColumnExists(String /*column*/) {return true;}
	virtual bool summarySignal() {return true;}

	// Set the iteration approach
	void setIterationApproach(uShort iterationApproach);

	// Set Data Selection parameters
	bool setDataSelection(Record record);

	// Set time interval (also known as ntime)
	void setTimeInterval(Double timeInterval);

	// Methods to switch on/off async i/o
	void enableAsyncIO(Bool enable);

	// Pre-Load columns (in order to avoid parallelism problems when not using
	// async i/o, and also to know what columns to pre-fetch in async i/o mode)
	void preLoadColumn(uInt column);
	void preFetchColumns();

	// Stop iterating
	void stopIteration() {stopIteration_p = true;};

	// As requested by Urvashi R.V. provide access to the original and modified flag cubes
	Cube<Bool> * getModifiedFlagCube() {return &modifiedFlagCube_p;}
	Cube<Bool> * getOriginalFlagCube() {return &originalFlagCube_p;}
	Vector<Bool> * getModifiedFlagRow() {return &modifiedFlagRow_p;}
	Vector<Bool> * getOriginalFlagRow() {return &originalFlagRow_p;}

	// Functions to switch on/off mapping functions
	void setMapAntennaPairs(bool activated);
	void setMapSubIntegrations(bool activated);
	void setMapPolarizations(bool activated);
	void setMapAntennaPointing(bool activated);
	void setScanStartStopMap(bool activated);
	void setScanStartStopFlaggedMap(bool activated);

	// Accessors for the mapping functions
	antennaPairMap * getAntennaPairMap() {return antennaPairMap_p;}
	subIntegrationMap * getSubIntegrationMap() {return subIntegrationMap_p;}
	polarizationMap * getPolarizationMap() {return polarizationMap_p;}
	polarizationIndexMap * getPolarizationIndexMap() {return polarizationIndexMap_p;}
	antennaPointingMap * getMapAntennaPointing() {return antennaPointingMap_p;}
	scanStartStopMap * getMapScanStartStop() {return scanStartStopMap_p;}
	lambdaMap * getLambdaMap() {return lambdaMap_p;}

	void setProfiling(Bool value) {profiling_p=value;}

	// Get a Float visCube and return a Complex one
	Cube<Complex>& weightVisCube();
	Cube<Complex> weight_spectrum_p;

	// Make the logger public to that we can use it from FlagAgentBase::create
	casa::LogIO *logger_p;

	// Measurement set section
	String tablename_p;
	MSSelection *measurementSetSelection_p;
	Vector<String> *antennaNames_p;
	std::map< string, std::pair<Int,Int> > baselineToAnt1Ant2_p;
	std::map< std::pair<Int,Int>, string > Ant1Ant2ToBaseline_p;
	ROScalarMeasColumn<MPosition> *antennaPositions_p;
	Vector<Double> *antennaDiameters_p;
	Vector<String> *fieldNames_p;
	std::vector<String> *corrProducts_p;

	// RO Visibility Iterator
	VisBufferComponents2 *prefetchColumns_p;
	// Iteration counters
	uLong processedRows;
	uShort chunkNo;
	uShort bufferNo;

	// FlagDataHanler-FlagAgents interaction
	bool flushFlags_p;
	bool flushFlagRow_p;
	uInt64 chunkCounts_p;
	uInt64 progressCounts_p;
	uInt64 msCounts_p;
	uShort summaryThreshold_p;
	bool printChunkSummary_p;
	uShort tableTye_p;
	Bool loadProcessorTable_p;

	// PROCESSOR sub-table section
	ScalarColumn<Bool> isCorrelatorType_p;
	bool processorTableExist_p;


	// Visibility Buffer
	// WARNING: The attach mechanism only works with pointers or
	// referenced variables. Otherwise the VisBuffer is created
	// and attached, but when it is assigned to the member it is
	// detached because of the dynamically called destructor
	vi::VisBuffer2 *visibilityBuffer_p;

	// Vis buffer characteristics (constant values)
	bool groupTimeSteps_p;
	Block<int> sortOrder_p;


protected:

	// Common MS/CalTables private interface
	virtual void generateAntennaPairMap();
	virtual void generateSubIntegrationMap();
	virtual void generatePolarizationsMap();
	virtual void generateAntennaPointingMap();
	virtual void generateScanStartStopMap();

	// Data Selection ranges
	bool anySelection_p;
	bool inrowSelection_p;
	casa::String arraySelection_p;
	casa::String fieldSelection_p;
	casa::String scanSelection_p;
	casa::String timeSelection_p;
	casa::String spwSelection_p;
	casa::String baselineSelection_p;
	casa::String uvwSelection_p;
	casa::String polarizationSelection_p;
	casa::String scanIntentSelection_p;
	casa::String observationSelection_p;

	// Async I/O stuff
	bool asyncio_enabled_p;
	// Pre-Load columns (in order to avoid parallelism problems when not using
	// async i/o, and also to know what columns to pre-fetch in async i/o mode)
	vector<uInt> preLoadColumns_p;

	// Iteration parameters
	uShort iterationApproach_p;
	Double timeInterval_p;
	// Slurp flag
	bool slurp_p;
	// Iteration initialization parameters
	bool chunksInitialized_p;
	bool buffersInitialized_p;
	bool iteratorGenerated_p;
	bool stopIteration_p;

	// Flag Cubes
	Cube<Bool> originalFlagCube_p;
	Cube<Bool> modifiedFlagCube_p;

	// FlagRows
	Vector<Bool> originalFlagRow_p;
	Vector<Bool> modifiedFlagRow_p;

	// Mapping members
	antennaPairMap *antennaPairMap_p;
	subIntegrationMap *subIntegrationMap_p;
	polarizationMap *polarizationMap_p;
	polarizationIndexMap *polarizationIndexMap_p;
	antennaPointingMap *antennaPointingMap_p;
	scanStartStopMap *scanStartStopMap_p;
	lambdaMap *lambdaMap_p;
	bool mapAntennaPairs_p;
	bool mapSubIntegrations_p;
	bool mapPolarizations_p;
	bool mapAntennaPointing_p;
	bool mapScanStartStop_p;
	bool mapScanStartStopFlagged_p;

	// Stats members
	bool stats_p;
	uLong cubeAccessCounter_p;
	double cubeAccessTime_p;
	uLong cubeAccessCounterTotal_p;
	double cubeAccessTimeTotal_p;

	// Profiling
	bool profiling_p;

};

} //# NAMESPACE CASA - END

#endif /* FLAGDATAHANDLER_H_ */
