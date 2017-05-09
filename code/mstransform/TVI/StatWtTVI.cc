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

#include <casacore/casa/Quanta/QuantumHolder.h>
#include <casacore/ms/MSOper/MSMetaData.h>
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

StatWtTVI::~StatWtTVI() {}

Bool StatWtTVI::_parseConfiguration(const Record& config) {
    if (config.isDefined("chanbin")) {
        // channel binning
        auto fieldNum = config.fieldNumber("chanbin");
        switch (config.type(fieldNum)) {
        case DataType::TpUInt:
            uInt binWidth;
            config.get("chanbin", binWidth);
            _setChanBinMap(binWidth);
            break;
        case DataType::TpRecord:
        {
            QuantumHolder qh;
            String err;
            ThrowIf(
                ! qh.fromRecord(err, config.asRecord("chanbin")),
                err
            );
            auto q = qh.asQuantity();
            _setChanBinMap(qh.asQuantity());
            break;
        }
        default:
            ThrowCc("Unsupported data type for chanbin");
        }
    }
    else {
        _setDefaultChanBinMap();
    }
	return True;
}

void StatWtTVI::_setChanBinMap(const Quantity& binWidth) {
    if (! binWidth.isConform(Unit("Hz"))) {
        ostringstream oss;
        oss << "If specified as a quantity, chanbin must have frequency units. "
            << binWidth << " does not.";
        ThrowCc(oss.str());
    }
    ThrowIf(
        binWidth.getValue() <= 0,
        "chanbin must be positive"
    );
    MSMetaData msmd(&ms(), 100.0);
    auto chanFreqs = msmd.getChanFreqs();
    auto nspw = chanFreqs.size();
    auto binWidthHz = binWidth.getValue("Hz");
    for (uInt i=0; i<nspw; ++i) {
        auto cfs = chanFreqs[i].getValue("Hz");
        auto citer = cfs.begin();
        auto cend = cfs.end();
        ChanBin bin;
        bin.start = 0;
        bin.end = 0;
        uInt chanNum = 0;
        auto startFreq = *citer;
        auto nchan = cfs.size();
        for (; citer!=cend; ++citer, ++chanNum) {
            if (abs(*citer - startFreq) > binWidthHz) {
                // start new bin
                _chanBins[i].push_back(bin);
                bin.start = chanNum;
                startFreq = *citer;
            }
            bin.end = chanNum;
            if (chanNum + 1 == nchan) {
                // need to add the last bin
                _chanBins[i].push_back(bin);
            }
        }
    }
}

void StatWtTVI::_setChanBinMap(uInt binWidth) {
    ThrowIf(binWidth < 2, "Channel bin width must >= 2");
    MSMetaData msmd(&ms(), 100.0);
    auto nchans = msmd.nChans();
    auto nspw = nchans.size();
    ChanBin bin;
    for (uInt i=0; i<nspw; ++i) {
        auto lastChan = nchans[i]-1;
        for (uInt j=0; j<nchans[i]; j += binWidth) {
            bin.start = j;
            bin.end = min(j+binWidth-1, lastChan);
            _chanBins[i].push_back(bin);
        }
    }
}

void StatWtTVI::_setDefaultChanBinMap() {
    MSMetaData msmd(&ms(), 100.0);
    auto nchans = msmd.nChans();
    auto niter = nchans.begin();
    auto nend = nchans.end();
    Int i = 0;
    ChanBin bin;
    bin.start = 0;
    for (; niter!=nend; ++niter, ++i) {
        bin.end = *niter - 1;
        _chanBins[i].push_back(bin);
    }
}

void StatWtTVI::_initialize() {}

void StatWtTVI::weightSpectrum(Cube<Float> & newWtsp) const {
    ThrowIf(! _weightsComputed, "Weights have not been computed yet");
    if (! _wtSpExists) {
        newWtsp.resize(IPosition(3, 0));
        return;
    }
    if (! _newWtSp.empty()) {
        // already calculated
        newWtsp = _newWtSp.copy();
        return;
    }
    getVii()->weightSpectrum(newWtsp);
    Vector<Int> ant1, ant2, spws;
    antenna1(ant1);
    antenna2(ant2);
    spectralWindows(spws);
    IPosition blc(3, 0);
    auto trc = newWtsp.shape() - 1;
    auto nrows = nRows();
    Vector<uInt> rowIDs;
    getRowIds(rowIDs);
    for (Int i=0; i<nrows; ++i) {
        blc[2] = i;
        trc[2] = i;
        BaselineChanBin blcb;
        blcb.baseline = _baseline(ant1[i], ant2[i]);
        auto spw = spws[i];
        blcb.spw = spw;
        auto bins = _chanBins.find(spw)->second;
        auto biter = bins.begin();
        auto bend = bins.end();
        for (; biter!=bend; ++biter) {
            blc[1] = biter->start;
            trc[1] = biter->end;
            blcb.chanBin = *biter;
            newWtsp(blc, trc) = _weights.find(blcb)->second;
        }
    }
    // cache it
    _newWtSp = newWtsp.copy();
}

void StatWtTVI::weight(Matrix<Float> & wtmat) const {
    ThrowIf(! _weightsComputed, "Weights have not been computed yet");
    if (! _newWt.empty()) {
        wtmat = _newWt.copy();
        return;
    }
    auto nrows = nRows();
    getVii()->weight(wtmat);
    if (_wtSpExists) {
        ClassicalStatistics<Double, Array<Float>::const_iterator, Array<Bool>::const_iterator> cs;
        Cube<Float> newWtsp;
        Cube<Bool> flagCube;
        weightSpectrum(newWtsp);
        flag(flagCube);
        for (Int i=0; i<nrows; ++i) {
            auto weights = newWtsp.xyPlane(i);
            auto flags = flagCube.xyPlane(i);
            if (allTrue(flags)) {
                wtmat.column(i) = 0;
            }
            else {
                auto mask = ! flags;
                cs.setData(weights.begin(), mask.begin(), weights.size());
                wtmat.column(i) = cs.getMedian();
            }
        }
    }
    else {
        // the only way this can happen is if there is a single channel bin
        // for each baseline/spw pair
        Vector<Int> ant1, ant2, spws;
        antenna1(ant1);
        antenna2(ant2);
        spectralWindows(spws);
        BaselineChanBin blcb;
        for (Int i=0; i<nrows; ++i) {
            auto bins = _chanBins.find(spws[i])->second;
            blcb.baseline = _baseline(ant1[i], ant2[i]);
            blcb.spw = spws[1];
            blcb.chanBin = bins[0];
            wtmat.column(i) = _weights.find(blcb)->second;
        }
    }
    _newWt = wtmat.copy();
}

void StatWtTVI::flag(Cube<Bool>& flagCube) const {
    ThrowIf(! _weightsComputed, "Weights have not been computed yet");
    if (! _newFlag.empty()) {
        flagCube = _newFlag.copy();
        return;
    }
    getVii()->flag(flagCube);
    Vector<Int> ant1, ant2, spws;
    antenna1(ant1);
    antenna2(ant2);
    spectralWindows(spws);
    auto nrows = nRows();
    IPosition blc(3, 0);
    auto trc = flagCube.shape() - 1;
    BaselineChanBin blcb;
    for (Int i=0; i<nrows; ++i) {
        blcb.baseline = _baseline(ant1[i], ant2[i]);
        auto spw = spws[i];
        blcb.spw = spw;
        auto bins = _chanBins.find(spw)->second;
        auto biter = bins.begin();
        auto bend = bins.end();
        blc[2] = i;
        trc[2] = i;
        for (; biter!=bend; ++biter) {
            blc[1] = biter->start;
            trc[1] = biter->end;
            if (_weights.find(blcb)->second == 0) {
                flagCube(blc, trc) = True;
            }
        }
    }
    _newFlag = flagCube.copy();
}

void StatWtTVI::flagRow (Vector<Bool>& flagRow) const {
    ThrowIf(! _weightsComputed, "Weights have not been computed yet");
    if (! _newFlagRow.empty()) {
        flagRow = _newFlagRow.copy();
        return;
    }
    Cube<Bool> flags;
    flag(flags);
    getVii()->flagRow(flagRow);
    auto nrows = nRows();
    for (Int i=0; i<nrows; ++i) {
        flagRow[i] = anyTrue(flags.xyPlane(i));
    }
    _newFlagRow = flagRow.copy();
}

void StatWtTVI::originChunks(Bool forceRewind) {
    // Drive next lower layer
    getVii()->originChunks(forceRewind);
    _weightsComputed = False;
    _gatherAndComputeWeights();
    _weightsComputed = True;
    _clearCache();
    // re-origin this chunk in next layer
    //  (ensures wider scopes see start of the this chunk)
    getVii()->origin();
    _wtSpExists = weightSpectrumExists();
}

void StatWtTVI::nextChunk() {
    // Drive next lower layer
    getVii()->nextChunk();
    _weightsComputed = False;
    _gatherAndComputeWeights();
    _weightsComputed = True;
    _clearCache();
    // re-origin this chunk next layer
    //  (ensures wider scopes see start of the this chunk)
    getVii()->origin();
}

void StatWtTVI::_clearCache() {
    _newWtSp.resize(0, 0, 0);
    _newWt.resize(0, 0);
    _newFlag.resize(0, 0, 0);
    _newFlagRow.resize(0);
}

void StatWtTVI::_gatherAndComputeWeights() const {
    // Drive NEXT LOWER layer's ViImpl to gather data into allvis:
    //  Assumes all sub-chunks in the current chunk are to be used
    //   for the variance calculation
    //  Essentially, we are sorting the incoming data into
    //   allvis, to enable a convenient variance calculation
    ViImplementation2* vii = getVii();
    VisBuffer2* vb = vii->getVisBuffer();
    _newRowIDs.resize(vii->nRowsInChunk());
    // baseline to visibility, flag maps
    std::map<BaselineChanBin, Cube<Complex>> data;
    std::map<BaselineChanBin, Cube<Bool>> flags;
    IPosition blc, trc;
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
        //const auto nchan = dataCube.ncolumn();
        const auto spws = vb->spectralWindows();
        blc = dataCubeBLC;
        trc = dataCubeTRC;
        for (Int i=0; i<nrows; ++i) {
            dataCubeBLC[2] = i;
            dataCubeTRC[2] = i;
            BaselineChanBin blcb;
            blcb.baseline = _baseline(ant1[i], ant2[i]);
            auto spw = spws[i];
            auto bins = _chanBins.find(spw)->second;
            blcb.spw = spw;
            auto citer = bins.begin();
            auto cend = bins.end();
            for (; citer!=cend; ++citer) {
                dataCubeBLC[1] = citer->start;
                dataCubeTRC[1] = citer->end;
                blcb.chanBin.start = citer->start;
                blcb.chanBin.end = citer->end;
                if (data.find(blcb) == data.end()) {
                    data[blcb] = dataCube(dataCubeBLC, dataCubeTRC);
                    flags[blcb] = flagCube(dataCubeBLC, dataCubeTRC);
                }
                else {
                    const auto nplane = data[blcb].nplane();
                    blc[2] = nplane;
                    trc[2] = nplane;
                    auto nchan = citer->end - citer->start + 1;
                    data[blcb].resize(npol, nchan, nplane+1, True);
                    flags[blcb].resize(npol, nchan, nplane+1, True);
                    data[blcb](blc, trc) = dataCube(dataCubeBLC, dataCubeTRC);
                    flags[blcb](blc, trc) = flagCube(dataCubeBLC, dataCubeTRC);
                }
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
    const map<BaselineChanBin, Cube<Complex>>& data,
    const map<BaselineChanBin, Cube<Bool>>& flags
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
        auto blcb = diter->first;
        auto dataForBLCB = diter->second;
        const auto npts = dataForBLCB.size();
        if (npts == 1) {
            // one data point, trivial
            _weights[blcb] = 0;
        }
        else {
            auto flagsForBLCB = fiter->second;
            if (allTrue(flagsForBLCB)) {
                // all data flagged, trivial
                _weights[blcb] = 0;
            }
            else {
                // some data not flagged
                const auto realPart = real(dataForBLCB);
                const auto imagPart = imag(dataForBLCB);
                const auto mask = ! flagsForBLCB;
                const auto riter = realPart.begin();
                const auto iiter = imagPart.begin();
                const auto miter = mask.begin();
                csReal.setData(riter, miter, npts);
                csImag.setData(iiter, miter, npts);
                auto varSum = csReal.getStatistic(StatisticsData::VARIANCE)
                    + csImag.getStatistic(StatisticsData::VARIANCE);
                _weights[blcb] = varSum == 0 ? 0 : 2/varSum;
            }
        }
    }
}

void StatWtTVI::origin() {
    // Drive underlying ViImplementation2
    getVii()->origin();
    // Synchronize own VisBuffer
    configureNewSubchunk();
    _clearCache();
}

void StatWtTVI::next() {
    // Drive underlying ViImplementation2
    getVii()->next();
    // Synchronize own VisBuffer
    configureNewSubchunk();
    _clearCache();
}

}

}


