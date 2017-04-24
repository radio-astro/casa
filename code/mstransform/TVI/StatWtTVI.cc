//# StatWtTVI.cc: This file contains the implementation of the StatWtTVI class.
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

#include <mstransform/TVI/StatWtTVI.h>

#include <casacore/scimath/Mathematics/ClassicalStatistics.h>

using namespace casacore;
namespace casa { 
namespace vi { 

StatWtTVI::StatWtTVI(ViImplementation2 * inputVii, const Record &configuration)
    : TransformingVi2 (inputVii) {
	// Parse and check configuration parameters
	// Note: if a constructor finishes by throwing an exception, the memory
	// associated with the object itself is cleaned up there is no memory leak.
    ThrowIf(
        ! _parseConfiguration(configuration),
	    "Error parsing StatWtTVI configuration"
    );
	_initialize();
	// Initialize attached VisBuffer
	setVisBuffer(createAttachedVisBuffer(VbPlain, VbRekeyable));
}

StatWtTVI::~StatWtTVI() {
	// The parent class destructor (~TransformingVi2) deletes the inner
	// ViImplementation2 object. However if it might have been already
	// deleted at the top level context
	// 2/8/2016 (jagonzal): As per request from George M. (via CAS-8220)
	// I allow TransformingVi2 destructor to delete its inner input VI;
	// This relies on the application layer that produces the inner VI not
	// deleting it which can be guaranteed when using the Factory pattern.
	// inputVii_p = NULL;

	// return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
Bool StatWtTVI::_parseConfiguration(const Record &configuration)
{
    /*
    int exists = -1;
	Bool ret = true;

	// Parse spw selection (optional)
	exists = -1;
	exists = configuration.fieldNumber ("spw");
	if (exists >= 0)
	{
		configuration.get (exists, spwSelection_p);
		logger_p << LogIO::DEBUG1 << LogOrigin("FreqAxisTVI", __FUNCTION__)
				<< "spw selection is " << spwSelection_p
				<< LogIO::POST;
	}
	else
	{
		spwSelection_p = "*";
	}
    */
	return true;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void StatWtTVI::_initialize()
{
/*
  if (inputVii_p->msName()=="<noms>")
    // Handle "no-MS" case  (SimpleSimVi2 as base layer)
    formSelectedChanMap();
  else {

	// Get list of selected SPWs and channels
	MSSelection mssel;
	mssel.setSpwExpr(spwSelection_p);
	Matrix<Int> spwchan = mssel.getChanList(&(inputVii_p->ms()));
	logger_p << LogIO::DEBUG1 << LogOrigin("FreqAxisTVI", __FUNCTION__)
			<< "Selected SPW:Channels are " << spwchan << LogIO::POST;

	// Convert list of selected SPWs/Channels into a map
	spwInpChanIdxMap_p.clear();
    uInt nSelections = spwchan.shape()[0];
	Int spw,channelStart,channelStop,channelStep;
	for(uInt selection_i=0;selection_i<nSelections;selection_i++)
	{
		spw = spwchan(selection_i,0);
		channelStart = spwchan(selection_i,1);
		channelStop = spwchan(selection_i,2);
		channelStep = spwchan(selection_i,3);

		if (spwInpChanIdxMap_p.find(spw) == spwInpChanIdxMap_p.end())
		{
			spwInpChanIdxMap_p[spw].clear(); // Accessing the vector creates it
		}

		for (Int inpChan=channelStart;inpChan<=channelStop;inpChan += channelStep)
		{
			spwInpChanIdxMap_p[spw].push_back(inpChan);
		}
	}
  }

	return;
    */
}
/*
// Method implementing main loop  (with auxiliary data)
template <class T> void StatWtTVI::_transformStatWt(
    casacore::Cube<T> const &inputDataCube, casacore::Cube<T> &outputDataCube,
    StatWtTransformEngine<T> &transformer
) const {
		// Re-shape output data cube
		outputDataCube.resize(getVisBufferConst()->getShape(),false);

		// Get data shape for iteration
		const casacore::IPosition &inputShape = inputDataCube.shape();
		casacore::uInt nRows = inputShape(2);
		casacore::uInt nCorrs = inputShape(0);

		// Initialize input-output planes
		casacore::Matrix<T> inputDataPlane;
		casacore::Matrix<T> outputDataPlane;

		// Initialize input-output vectors
		casacore::Vector<T> inputDataVector;
		casacore::Vector<T> outputDataVector;

		for (casacore::uInt row=0; row < nRows; row++)
		{
			// Assign input-output planes by reference
			transformer.setRowIndex(row);
			inputDataPlane.reference(inputDataCube.xyPlane(row));
			outputDataPlane.reference(outputDataCube.xyPlane(row));

			for (casacore::uInt corr=0; corr < nCorrs; corr++)
			{
				// Assign input-output vectors by reference
				transformer.setCorrIndex(corr);
				inputDataVector.reference(inputDataPlane.row(corr));
				outputDataVector.reference(outputDataPlane.row(corr));

				// Transform data
				transformer.transform(inputDataVector,outputDataVector);
			}
		}

		return;
	}
*/
// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void StatWtTVI::_formSelectedChanMap()
{
	// This triggers realization of the channel selection
	inputVii_p->originChunks();

    // Refresh map
	spwInpChanIdxMap_p.clear();

	for (Int ispw = 0; ispw < inputVii_p->nSpectralWindows(); ++ispw)
	{

		// TBD trap unselected spws with a continue

		Vector<Int> chansV;
		chansV.reference(inputVii_p->getChannels(0.0, -1, ispw, 0));

		Int nChan = chansV.nelements();
		if (nChan > 0)
		{
			spwInpChanIdxMap_p[ispw].clear(); // creates ispw's map
			for (Int ich = 0; ich < nChan; ++ich)
			{
				spwInpChanIdxMap_p[ispw].push_back(chansV[ich]); // accum into map
			}
		}
	} // ispw

	return;
}

void StatWtTVI::weightSpectrum(Cube<Float> & newWtsp) const {
    _wtSpExists = weightSpectrumExists();
    if (_wtSpExists) {
        _computeNewWeights();
        newWtsp = _newWtSp.copy();
    }
    else {
        newWtsp.resize(IPosition(3, 0));
    }
}

void StatWtTVI::weight(Matrix<Float> & wtmat) const {
    _wtSpExists = weightSpectrumExists();
    _computeNewWeights();
    wtmat = _newWt.copy();
}

void StatWtTVI::flag(Cube<Bool>& flagCube) const {
    _computeNewWeights();
    flagCube = _newFlag.copy();
}

void StatWtTVI::flagRow (casacore::Vector<casacore::Bool> & flagRow) const {
    _computeNewWeights();
    flagRow = _newFlagRow.copy();
}

void StatWtTVI::_computeNewWeights() const {
    if (_weightsComputed) {
        // weights have already been computed for this subchunk
        return;
    }
    Cube<Complex> vis;
    getVii()->visibilityCorrected(vis);
    if (_wtSpExists) {
        getVii()->weightSpectrum(_newWtSp);
    }
    getVii()->weight(_newWt);
    getVii()->flag(_newFlag);
    getVii()->flagRow(_newFlagRow);
    auto shape = vis.shape();
    auto nchan = shape[1];
    if (nchan == 1) {
        // only one spectral channel, so variance is by definition 0
        if (_wtSpExists) {
            _newWtSp = 0;
        }
        _newWt = 0;
        if (! allTrue(_newFlag)) {
            _newFlag = True;
            _newFlagRow = True;
        }
    }
    else {
        auto ncorr = shape[0];
        auto nrow = shape[2];
        ClassicalStatistics<Double, Array<Float>::const_iterator, Array<Bool>::const_iterator> csReal, csImag;
        std::set<StatisticsData::STATS> stats;
        stats.insert(StatisticsData::VARIANCE);
        csReal.setStatsToCalculate(stats);
        csImag.setStatsToCalculate(stats);
        // find variance across spectral channels for each correlation and row
        IPosition blc(3, 0, 0, 0);
        auto trc = blc;
        Cube<Float> wtSpec;
        trc[1] = nchan - 1;
        for (uInt i=0; i<nrow; ++i, ++blc[2], ++trc[2]) {
            blc[0] = 0;
            trc[0] = 0;
            Bool flagRow = True;
            for (uInt j=0; j<ncorr; ++j, ++blc[0], ++trc[0]) {
                auto dataSpec = vis(blc, trc);
                auto realPart = real(dataSpec);
                auto imagPart = imag(dataSpec);
                auto flagSpec = _newFlag(blc, trc);
                Double modVar = 0;
                if (! allTrue(flagSpec)) {
                    // some data not flagged
                    auto mask = ! flagSpec;
                    const auto riter = realPart.begin();
                    const auto iiter = imagPart.begin();
                    const auto miter = mask.begin();
                    const auto npts = realPart.size();
                    csReal.setData(riter, miter, npts);
                    csImag.setData(iiter, miter, npts);
                    auto varReal = csReal.getStatistic(StatisticsData::VARIANCE);
                    auto varImag = csImag.getStatistic(StatisticsData::VARIANCE);
                    modVar = (varReal + varImag)/2;
                    if (modVar == 0) {
                        _newFlag(blc, trc) = True;
                    }
                    else {
                        flagRow = False;
                    }
                }
                auto wt = modVar == 0 ? 0 : 1/modVar;
                if (_wtSpExists) {
                    _newWtSp(blc, trc) = wt;
                }
                // needs to be redone when wtspec varies by channel
                _newWt(j, i) = wt;
            }
            _newFlagRow[i] = flagRow;
        }
    }
    _weightsComputed = True;
}

void StatWtTVI::origin() {
    //cout << "StatWtTVI::" << __func__ << endl;
    // Drive underlying ViImplementation2
    getVii()->origin();
    // Synchronize own VisBuffer
    configureNewSubchunk();
    _weightsComputed = False;
}

void StatWtTVI::next() {
    // Drive underlying ViImplementation2
    getVii()->next();
    // Synchronize own VisBuffer
    configureNewSubchunk();
    _weightsComputed = False;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
/*
Bool StatWtTVI::existsColumn (VisBufferComponent2 id) const
{

	Bool ret;
	switch (id)
	{
		case VisBufferComponent2::WeightSpectrum:
		{
			ret = true;
			break;
		}
		case VisBufferComponent2::SigmaSpectrum:
		{
			ret = true;
			break;
		}
		default:
		{
			ret = getVii()->existsColumn(id);
			break;
		}
	}

	return ret;
}
*/
// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
/*
Vector<Int> StatWtTVI::getChannels (Double,Int,Int spectralWindowId,Int) const
{
	Vector<Int> ret(spwOutChanNumMap_p[spectralWindowId]);

	for (uInt chanIdx = 0; chanIdx<spwOutChanNumMap_p[spectralWindowId];chanIdx++)
	{
		ret(chanIdx) = chanIdx;
	}

	return ret;
}
*/
// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
/*
void StatWtTVI::writeFlagRow (const Vector<Bool> & flag)
{
	getVii()->writeFlagRow(flag);
}
*/
// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
/*
void StatWtTVI::flagRow (Vector<Bool> & flagRow) const
{
	// Get flagCube from own VisBuffer
	const Cube<Bool> &flagCube = getVisBufferConst()->flagCube();

	// Calculate output flagRow
	accumulateFlagCube(flagCube,flagRow);
}
*/
// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
/*
void StatWtTVI::weight (Matrix<Float> & weight) const
{
	if (weightSpectrumExists()) // Defined by each derived class or inner TVI
	{
		// Get flags and weightSpectrum from own VisBuffer
		const Cube<Bool> &flags = getVisBufferConst()->flagCube();
		const Cube<Float> &weightSpectrum = getVisBufferConst()->weightSpectrum();

		// Calculate output weight
		accumulateWeightCube(weightSpectrum,flags,weight);
	}
	else
	{
		getVii()->weight (weight);
	}

	return;
}
*/
// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
/*
void StatWtTVI::sigma (Matrix<Float> & sigma) const
{
	if (sigmaSpectrumExists())
	{
		// Get flags and sigmaSpectrum from own VisBuffer
		const Cube<Bool> &flags = getVisBufferConst()->flagCube();
		const Cube<Float> &sigmaSpectrum = getVisBufferConst()->sigmaSpectrum();

		// Calculate output sigma
		accumulateWeightCube(sigmaSpectrum,flags,sigma);
	}
	else
	{
		getVii()->sigma (sigma);
	}

	return;
}
*/
} //# NAMESPACE VI - END

} //# NAMESPACE CASA - END


