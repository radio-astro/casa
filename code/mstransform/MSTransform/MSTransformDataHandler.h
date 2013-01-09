//# MSTransformDataHandler.h: This file contains the interface definition of the MSTransformDataHandler class.
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

#ifndef MSTransformDataHandler_H_
#define MSTransformDataHandler_H_

#include <casacore/casa/aipstype.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Containers/Record.h>
#include <casacore/casa/Logging/LogIO.h>

#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <synthesis/MSVis/SubMS.h>

#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/VisBuffer2.h>

#include <tables/Tables/TableRow.h>


namespace casa { //# NAMESPACE CASA - BEGIN


typedef map<MS::PredefinedColumns,MS::PredefinedColumns> dataColMap;
typedef map<Int,map<uInt, pair<uInt,Double> > > inputOutputWeightChannelMap;
typedef map< pair<Int,Int>,vector<uInt> > baselineMap;

struct channelInfo {

	Int SPW_id;
	uInt inpChannel;
	uInt outChannel;
	Double CHAN_FREQ;
	Double CHAN_WIDTH;
	Double EFFECTIVE_BW;
	Double RESOLUTION;
	Double outChannelFraction;

	channelInfo()
	{
		SPW_id = -1;
		inpChannel = 0;
		outChannel = 0;
		outChannelFraction = 0;

		CHAN_FREQ = -1;
		CHAN_WIDTH = -1;
		EFFECTIVE_BW = -1;
		RESOLUTION = -1;
	}

	bool operator<(const channelInfo& right_operand) const
	{
		if (CHAN_FREQ<right_operand.CHAN_FREQ)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	Double upperBound() const
	{
		return CHAN_FREQ+0.5*CHAN_WIDTH;
	}

	Double lowerBound() const
	{
		return CHAN_FREQ+0.5*CHAN_WIDTH;
	}

	bool overlap(const channelInfo& other) const
	{
		if (CHAN_FREQ<other.CHAN_FREQ)
		{
			if (upperBound() > other.lowerBound())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (other.upperBound() > lowerBound())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
};

struct spwInfo {

	spwInfo()
	{
		NUM_CHAN = 0;
		CHAN_FREQ.resize(0);
		CHAN_WIDTH.resize(0);
		EFFECTIVE_BW.resize(0);
		RESOLUTION.resize(0);
		TOTAL_BANDWIDTH = 0;
		REF_FREQUENCY = 0;
		upperBound = 0;
		lowerBound = 0;
	}


	spwInfo(uInt nChannels)
	{
		NUM_CHAN = nChannels;
		CHAN_FREQ.resize(nChannels);
		CHAN_WIDTH.resize(nChannels);
		EFFECTIVE_BW.resize(nChannels);
		RESOLUTION.resize(nChannels);
		TOTAL_BANDWIDTH = 0;
		REF_FREQUENCY = 0;
		upperBound = 0;
		lowerBound = 0;
	}

	void update()
	{
		upperBound = CHAN_FREQ(NUM_CHAN-1)+0.5*CHAN_FREQ(NUM_CHAN-1);
		lowerBound = CHAN_FREQ(0)-0.5*CHAN_FREQ(0);
		TOTAL_BANDWIDTH =upperBound - lowerBound;
	}

	uInt NUM_CHAN;
	Vector<Double> CHAN_FREQ;
	Vector<Double> CHAN_WIDTH;
	Vector<Double> EFFECTIVE_BW;
	Vector<Double> RESOLUTION;
	Double TOTAL_BANDWIDTH;
	Double REF_FREQUENCY;
	Double upperBound;
	Double lowerBound;
};


namespace tvf
{
	// Returns 1/sqrt(wt) or -1, depending on whether wt is positive..
	Double wtToSigma(Double wt);
}

class MSTransformDataHandler
{

public:

	MSTransformDataHandler();
	MSTransformDataHandler(Record configuration);

	~MSTransformDataHandler();

	void initialize();
	void configure(Record &configuration);

	void open();
	void setup();
	void close();

	vi::VisibilityIterator2 * getVisIter() {return visibilityIterator_p;}

	void fillOutputMs(vi::VisBuffer2 *vb);


protected:

	void checkFillFlagCategory();
	void checkFillWeightSpectrum();
	void checkDataColumnsToFill();
	void setIterationApproach();
	void generateIterator();

	void combineSpwSubtable();
	void reindexSourceSubTable();
	void reindexDDISubTable();
	void reindexFeedSubTable();
	void reindexSysCalSubTable();
	void reindexFreqOffsetSubTable();
	void reindexColumn(ScalarColumn<Int> &inputCol, Int value);

	void fillIdCols(vi::VisBuffer2 *vb,RefRows &rowRef);
	void fillDataCols(vi::VisBuffer2 *vb,RefRows &rowRef);
	void fillAuxCols(vi::VisBuffer2 *vb,RefRows &rowRef);

	template <class T> void writeVector(const Vector<T> &inputVector,ScalarColumn<T> &outputCol, RefRows &rowRef);
	template <class T> void writeMatrix(const Matrix<T> &inputMatrix,ArrayColumn<T> &outputCol, RefRows &rowRef);
	template <class T> void writeCube(const Cube<T> &inputCube,ArrayColumn<T> &outputCol, RefRows &rowRef);

	template <class T> void transformVector(const Vector<T> &inputVector, Vector<T> &outputVector, vi::VisBuffer2 *vb, Bool convolveFlags=False);
	template <class T> void transformMatrix(const Matrix<T> &inputMatrix, Matrix<T> &outputMatrix, vi::VisBuffer2 *vb, Bool convolveFlags=False);

	template <class T> void writeTransformedVector(const Vector<T> &inputVector,ScalarColumn<T> &outputCol, RefRows &rowRef, vi::VisBuffer2 *vb, Bool convolveFlags=False);
	template <class T> void writeTransformedMatrix(const Matrix<T> &inputMatrix,ArrayColumn<T> &outputCol, RefRows &rowRef, vi::VisBuffer2 *vb, Bool convolveFlags=False);
	template <class T> void writeTransformedCube(const Cube<T> &inputCube,ArrayColumn<T> &outputCol, RefRows &rowRef, vi::VisBuffer2 *vb, Bool convolveFlags=False);

	// Input-Output MS parameters
	String inpMsName_p;
	String outMsName_p;
	String datacolumn_p;
	Vector<Int> tileShape_p;

	// Data selection parameters
	String arraySelection_p;
	String fieldSelection_p;
	String scanSelection_p;
	String timeSelection_p;
	String spwSelection_p;
	String baselineSelection_p;
	String uvwSelection_p;
	String polarizationSelection_p;
	String scanIntentSelection_p;
	String observationSelection_p;

	// Spectral transformation parameters
	Bool combinespws_p;
	Vector<Int> chanSpec_p;

	// Time transformation parameters
	Double timeBin_p;
	String timespan_p;

	// MS-related members
	SubMS *splitter_p;
	MeasurementSet *inputMs_p;
	MeasurementSet *selectedInputMs_p;
	MeasurementSet *outputMs_p;
	ROMSColumns *inputMsCols_p;
	MSColumns *outputMsCols_p;

	// VI/VB related members
	Block<Int> sortColumns_p;
	vi::VisibilityIterator2 *visibilityIterator_p;

	// Output MS structure related members
	Bool fillFlagCategory_p;
	Bool fillWeightSpectrum_p;
	Bool correctedToData_p;
	dataColMap dataColMap_p;

	// Spectral transformation members
	inputOutputWeightChannelMap inputOutputWeightChannelMap_p;
	baselineMap baselineMap_p;
	spwInfo combinedSpw_p;

	// Looging
	LogIO logger_p;
};

} //# NAMESPACE CASA - END

#endif /* MSTransformDataHandler_H_ */
