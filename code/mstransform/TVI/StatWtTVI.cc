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
Bool StatWtTVI::_parseConfiguration(const Record& /*configuration*/)
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

void StatWtTVI::weightSpectrum(Cube<Float> & newWtsp) const {
    _wtSpExists = weightSpectrumExists();
    if (_wtSpExists) {
        // _computeNewWeights();
        getVii()->weightSpectrum(newWtsp);
        Vector<Int> ant1, ant2;
        antenna1(ant1);
        antenna2(ant2);
        IPosition blc(3, 0);
        auto trc = newWtsp.shape() - 1;
        auto nrows = nRows();
        Vector<uInt> rowIDs;
        getRowIds(rowIDs);
        for (Int i=0; i<nrows; ++i) {
            blc[2] = i;
            trc[2] = i;
            auto baseline = _baseline(ant1[i], ant2[i]);
            newWtsp(blc, trc) = _weights.find(baseline)->second;
        }
    }
    else {
        newWtsp.resize(IPosition(3, 0));
    }
}

void StatWtTVI::weight(Matrix<Float> & wtmat) const {
    // _wtSpExists = weightSpectrumExists();
    // _computeNewWeights();
    getVii()->weight(wtmat);
    Vector<Int> ant1, ant2;
    antenna1(ant1);
    antenna2(ant2);
    auto nrows = nRows();
    for (Int i=0; i<nrows; ++i) {
        auto baseline = _baseline(ant1[i], ant2[i]);
        wtmat.column(i) = _weights.find(baseline)->second;
    }
}

void StatWtTVI::flag(Cube<Bool>& flagCube) const {
    // _computeNewWeights();
    // flagCube = _newFlag.copy();
    getVii()->flag(flagCube);
    Vector<Int> ant1, ant2;
    antenna1(ant1);
    antenna2(ant2);
    auto nrows = nRows();
    IPosition blc(3, 0);
    auto trc = flagCube.shape() - 1;
    for (Int i=0; i<nrows; ++i) {
        auto baseline = _baseline(ant1[i], ant2[i]);
        if (_weights.find(baseline)->second == 0) {
            blc[2] = i;
            trc[2] = i;
            flagCube(blc, trc) = True;
        }
    }
}

void StatWtTVI::flagRow (Vector<Bool>& flagRow) const {
    getVii()->flagRow(flagRow);
    Vector<Int> ant1, ant2;
    antenna1(ant1);
    antenna2(ant2);
    auto nrows = nRows();
    for (Int i=0; i<nrows; ++i) {
        auto baseline = _baseline(ant1[i], ant2[i]);
        flagRow[i] = _weights.find(baseline)->second == 0;
    }
}

void StatWtTVI::originChunks(Bool forceRewind) {
    // Drive next lower layer
    getVii()->originChunks(forceRewind);
    _weightsComputed = False;
    _gatherAndComputeWeights();
    _weightsComputed = True;

    // Weights not yet ready in this chunk
    // weightsReady_p=false;

    // Do calculation for current chunk
    //  NB: this drives lower-layer sub-chunks, in general
    // this->gatherAndCalculateWts();

    // Weights now ready in this chunk
    // weightsReady_p=true;

    // re-origin this chunk in next layer
    //  (ensures wider scopes see start of the this chunk)
    getVii()->origin();
}

void StatWtTVI::nextChunk() {
    // Drive next lower layer
    getVii()->nextChunk();
    _weightsComputed = False;
    _gatherAndComputeWeights();
    _weightsComputed = True;
    // Weights not yet ready in this chunk
    //weightsReady_p=false;

    // Do calculation for current chunk
    //  NB: this drives lower-layer sub-chunks, in general
    //this->gatherAndCalculateWts();

    // Weights now ready in this chunk
    //weightsReady_p=true;

    // re-origin this chunk next layer
    //  (ensures wider scopes see start of the this chunk)
    getVii()->origin();
}

void StatWtTVI::_gatherAndComputeWeights() const {
    // Shape this to enable gather of
    //   all sub-chunks on all baselines,corrs,channels
    //   probably has a time axis, too
    //Array<Complex> allvis;

    // Drive NEXT LOWER layer's ViImpl to gather data into allvis:
    //  Assumes all sub-chunks in the current chunk are to be used
    //   for the variance calculation
    //  Essentially, we are sorting the incoming data into
    //   allvis, to enable a convenient variance calculation
    ViImplementation2* vii = getVii();
    VisBuffer2* vb = vii->getVisBuffer();
    _newRowIDs.resize(vii->nRowsInChunk());
    // baseline to visibility, flag maps
    std::map<Baseline, Cube<Complex>> data;
    std::map<Baseline, Cube<Bool>> flags;
    IPosition blc, trc;
   // vector<Baseline> baselines;
    //cout << "BEGIN" << endl;
    for (vii->origin();vii->more();vii->next()) {
        const auto rowIDs = vb->rowIds();
        const auto ant1 = vb->antenna1();
        const auto ant2 = vb->antenna2();
        // [nC,nF,nR)
        const auto dataCube = vb->visCubeCorrected();
        IPosition dataCubeBLC(3, 0);
        auto dataCubeTRC = dataCube.shape() - 1;
        dataCubeTRC[2] = 0;
        const auto flagCube = vb->flagCube();
        const auto nrows = vb->nRows();
        const auto npol = dataCube.nrow();
        const auto nchan = dataCube.ncolumn();
        // debug
        // const auto times = vb->time();
        blc = dataCubeBLC;
        trc = dataCubeTRC;
        for (Int i=0; i<nrows; ++i) {
            dataCubeBLC[2] = i;
            dataCubeTRC[2] = i;
            auto baseline = _baseline(ant1[i], ant2[i]);
            if (data.find(baseline) == data.end()) {
                data[baseline] = dataCube(dataCubeBLC, dataCubeTRC);
                flags[baseline] = flagCube(dataCubeBLC, dataCubeTRC);
            }
            else {
                const auto nplane = data[baseline].nplane();
                blc[2] = nplane;
                trc[2] = nplane;
                data[baseline].resize(npol, nchan, nplane+1, True);
                flags[baseline].resize(npol, nchan, nplane+1, True);
                data[baseline](blc, trc) = dataCube(dataCubeBLC, dataCubeTRC);
                flags[baseline](blc, trc) = flagCube(dataCubeBLC, dataCubeTRC);
            }
        }
    }
    // data has been gathered, now compute weights
    _computeWeights(data, flags);
}

void StatWtTVI::writeBackChanges(VisBuffer2 * vb) {
    // Pass to next layer down
    getVii()->writeBackChanges(vb);
}

StatWtTVI::Baseline StatWtTVI::_baseline(uInt ant1, uInt ant2) {
    Baseline baseline;
    if (ant1 < ant2) {
        // this may always be the case, but I'm not certain,
        baseline.first = ant1;
        baseline.second = ant2;
    }
    else {
        baseline.first = ant2;
        baseline.second = ant1;
    }
    return baseline;
}

void StatWtTVI::_computeWeights(
    const map<Baseline, Cube<Complex>>& data,
    const map<Baseline, Cube<Bool>>& flags
) const {
    ClassicalStatistics<Double, Array<Float>::const_iterator, Array<Bool>::const_iterator> csReal, csImag;
    std::set<StatisticsData::STATS> stats;
    stats.insert(StatisticsData::VARIANCE);
    csReal.setStatsToCalculate(stats);
    csImag.setStatsToCalculate(stats);
    auto diter = data.begin();
    auto dend = data.end();
    auto fiter = flags.begin();
    for (; diter!=dend; ++diter, ++fiter) {
        auto baseline = diter->first;
        auto dataForBaseline = diter->second;
        const auto npts = dataForBaseline.size();
        if (npts == 1) {
            // one data point, trivial
            _weights[baseline] = 0;
        }
        else {
            auto flagsForBaseline = fiter->second;
            if (allTrue(flagsForBaseline)) {
                // all data flagged, trivial
                _weights[baseline] = 0;
            }
            else {
                // some data not flagged
                const auto realPart = real(dataForBaseline);
                const auto imagPart = imag(dataForBaseline);
                const auto mask = ! flagsForBaseline;
                const auto riter = realPart.begin();
                const auto iiter = imagPart.begin();
                const auto miter = mask.begin();
                csReal.setData(riter, miter, npts);
                csImag.setData(iiter, miter, npts);
                auto varSum = csReal.getStatistic(StatisticsData::VARIANCE)
                    + csImag.getStatistic(StatisticsData::VARIANCE);
                _weights[baseline] = varSum == 0 ? 0 : 2/varSum;
            }
        }
    }
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


