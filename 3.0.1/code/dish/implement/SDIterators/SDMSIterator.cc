//# SDMSIterator.cc:  this defines SDMSIterator
//# Copyright (C) 2000,2001,2002,2003
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
//# $Id$

//# Includes

#include <dish/SDIterators/SDMSIterator.h>

#include <casa/sstream.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slice.h>
#include <casa/Containers/Block.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MeasureHolder.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/Stokes.h>
#include <casa/OS/File.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <tables/Tables/TableAttr.h>
#include <tables/Tables/ColumnsIndex.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableVector.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Regex.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SDMSIterator::SDMSIterator(const String &msName, const Record &selection,
			   Table::TableOption opt, TableLock::LockOption lockoption,
			   Bool useCorrectedData)
    : ok_p(False), rec_p(0), ms_p(0), thisRow_p(0), correctedData_p(useCorrectedData),
      romsCols_p(0), stdTabRows_p(0), nsTabRows_p(0), nsIndexes_p(0), nsTables_p(0), 
      sec_p("s"), toAzEl_p(0)
{
    if (opt == Table::New || 
	opt == Table::NewNoReplace || 
	opt == Table::Scratch) {
	createMS(msName, lockoption, opt);
    } else {
	ms_p = new MeasurementSet(msName, TableLock(lockoption), opt);
    }
    AlwaysAssert(ms_p, AipsError);

    applySelection(selection);
    init();
}

SDMSIterator::SDMSIterator(const SDMSIterator& other)
    : ok_p(False), rec_p(0), ms_p(0), thisRow_p(0), correctedData_p(other.correctedData_p),
      romsCols_p(0), stdTabRows_p(0), nsTabRows_p(0), nsIndexes_p(0), nsTables_p(0), 
      sec_p("s"), toAzEl_p(0)
{
    ms_p = new MeasurementSet(*other.ms_p);
    AlwaysAssert(ms_p, AipsError);

    init();
}

SDMSIterator::SDMSIterator(const SDMSIterator& other, const Record& selection)
    : ok_p(False), rec_p(0), ms_p(0), thisRow_p(0), correctedData_p(other.correctedData_p),
      romsCols_p(0), stdTabRows_p(0), nsTabRows_p(0), nsIndexes_p(0), nsTables_p(0), 
      sec_p("s"), toAzEl_p(0)
{
    ms_p = new MeasurementSet(*(other.ms_p));
    AlwaysAssert(ms_p, AipsError);

    applySelection(selection);
    init();
}

Bool SDMSIterator::unlock()
{
    if (ms_p) {
	ms_p->unlock();
	// and the sub-tables - starting with the standard ones
	if (!ms_p->antenna().isNull()) ms_p->antenna().unlock();
	if (!ms_p->dataDescription().isNull()) ms_p->dataDescription().unlock();
	if (!ms_p->doppler().isNull()) ms_p->doppler().unlock();
	if (!ms_p->feed().isNull()) ms_p->feed().unlock();
	if (!ms_p->field().isNull()) ms_p->field().unlock();
	if (!ms_p->flagCmd().isNull()) ms_p->flagCmd().unlock();
	if (!ms_p->freqOffset().isNull()) ms_p->freqOffset().unlock();
	if (!ms_p->history().isNull()) ms_p->history().unlock();
	if (!ms_p->observation().isNull()) ms_p->observation().unlock();
	if (!ms_p->pointing().isNull()) ms_p->pointing().unlock();
	if (!ms_p->polarization().isNull()) ms_p->polarization().unlock();
	if (!ms_p->processor().isNull()) ms_p->processor().unlock();
	if (!ms_p->source().isNull()) ms_p->source().unlock();
	if (!ms_p->spectralWindow().isNull()) ms_p->spectralWindow().unlock();
	if (!ms_p->state().isNull()) ms_p->state().unlock();
	if (!ms_p->sysCal().isNull()) ms_p->sysCal().unlock();
	if (!ms_p->weather().isNull()) ms_p->weather().unlock();
	// and the nsTables_p
	for (uInt i=0;i<nsTables_p.nelements();i++) {
	    if (nsTables_p[i]) nsTables_p[i]->unlock();
	}
    }
    return True;
}

Bool SDMSIterator::lock(uInt nattempts)
{
    Bool result = True;
    // currently this is always read-only
    Bool write = False;
    if (ms_p) {
	result = ms_p->lock(write, nattempts);
	// and the sub-tables - starting with the standard ones
	if (result && !ms_p->antenna().isNull()) {
	    result = ms_p->antenna().lock(write, nattempts);
	}
	if (result && !ms_p->dataDescription().isNull()) {
	    result = ms_p->dataDescription().lock(write, nattempts);
	}
	if (result && !ms_p->doppler().isNull()) {
	    result = ms_p->doppler().lock(write, nattempts);
	}
	if (result && !ms_p->feed().isNull()) {
	    result = ms_p->feed().lock(write, nattempts);
	}
	if (result && !ms_p->field().isNull()) {
	    result = ms_p->field().lock(write, nattempts);
	}
	if (result && !ms_p->flagCmd().isNull()) {
	    result = ms_p->flagCmd().lock(write, nattempts);
	}
	if (result && !ms_p->freqOffset().isNull()) {
	    result = ms_p->freqOffset().lock(write, nattempts);
	}
	if (result && !ms_p->history().isNull()) {
	    result = ms_p->history().lock(write, nattempts);
	}
	if (result && !ms_p->observation().isNull()) {
	    result = ms_p->observation().lock(write, nattempts);
	}
	if (result && !ms_p->pointing().isNull()) {
	    result = ms_p->pointing().lock(write, nattempts);
	}
	if (result && !ms_p->polarization().isNull()) {
	    result = ms_p->polarization().lock(write, nattempts);
	}
	if (result && !ms_p->processor().isNull()) {
	    result = ms_p->processor().lock(write, nattempts);
	}
	if (result && !ms_p->source().isNull()) {
	    result = ms_p->source().lock(write, nattempts);
	}
	if (result && !ms_p->spectralWindow().isNull()) {
	    result = ms_p->spectralWindow().lock(write, nattempts);
	}
	if (result && !ms_p->state().isNull()) {
	    result = ms_p->state().lock(write, nattempts);
	}
	if (result && !ms_p->sysCal().isNull()) {
	    result = ms_p->sysCal().lock(write, nattempts);
	}
	if (result && !ms_p->weather().isNull()) {
	    result = ms_p->weather().lock(write, nattempts);
	}
	// and the nsTables_p
	for (uInt i=0;i<nsTables_p.nelements();i++) {
	    if (result && nsTables_p[i]) {
		result = nsTables_p[i]->lock(write, nattempts);
	    }
	}
    }
    return result;
}

SDMSIterator &SDMSIterator::operator=(const SDMSIterator& other)
{
    if (this == &other) return *this;

    cleanup();

    ms_p = new MeasurementSet(*(other.ms_p));
    AlwaysAssert(ms_p, AipsError);

    correctedData_p = other.correctedData_p;

    init();

    return *this;
}

void SDMSIterator::copyAll()
{
    copyData();
    copyHeader();
    copyHist();
    copyOther();
}

void SDMSIterator::copyData()
{
    // do nothing if there is nothing in ms_p
    if (!ms_p || ms_p->nrow() == 0) return;

    // resize - no penalty if called more than once on this row
    if (arr_p.isAttached()) {
	rec_p->resize(romsCols_p->floatData().shape(thisRow_p));
    } else {
	if (correctedData_p) {
	    rec_p->resize(romsCols_p->correctedData().shape(thisRow_p));
	} else {
	    rec_p->resize(romsCols_p->data().shape(thisRow_p));
	}
    }

    // necessary IDs for lookups done here
    // ANTENNA1 and ANTENNA2
    Int ant1Id = ids_p.antenna1(thisRow_p);
    Int ant2Id = ids_p.antenna2(thisRow_p);

    // FEED1 (assumed to be same as FEED2
    // what should happen if ant1!=ant2?
    // and what about feed2 != feed1?
    Int feed1Id = romsCols_p->feed1()(thisRow_p);

    // DATA_DESC_ID
    Int dataDescId = ids_p.dataDescId(thisRow_p);
    Bool ddOK = dataDescId >= 0 && !(romsCols_p->dataDescription().flagRow()(dataDescId));

    // SPECTRAL_WINDOW_ID
    Int spwId = ids_p.spectralWindowId(thisRow_p);
    Bool spwOK = ddOK && spwId >= 0 && !(romsCols_p->spectralWindow().flagRow()(spwId));

    // POLARIZATION_ID
    Int polId = ids_p.polarizationId(thisRow_p);
    Bool polOK = ddOK && polId >= 0 && !(romsCols_p->polarization().flagRow()(polId));

    // TIME and INTERVAL for lookups
    const Quantity interval = romsCols_p->intervalQuant()(thisRow_p);
    const MEpoch time = romsCols_p->timeMeas()(thisRow_p);
    Double intSecs = interval.getValue(sec_p);
    Double stime = time.getValue().getTime().getValue(sec_p);

    // chan_freqs and related
    // get any frequency offset first
    Quantity offset = 0.0;
    if (ant1Id >= 0 && ant2Id >= 0 && feed1Id >= 0 && spwId >= 0 && !freqOffsetIndex_p.isNull()) {
	freqOffsetIndex_p.antenna1Id() = ant1Id;
	freqOffsetIndex_p.antenna2Id() = ant2Id;
	freqOffsetIndex_p.feedId() = feed1Id;
	freqOffsetIndex_p.spectralWindowId() = spwId;
	freqOffsetIndex_p.time() = stime;
	freqOffsetIndex_p.interval() = intSecs;
	Bool freqOffsetRowFound;
	uInt freqOffsetRow = freqOffsetIndex_p.getNearestRow(freqOffsetRowFound);
	if (freqOffsetRowFound) {
	    offset = (romsCols_p->freqOffset().offsetQuant()(freqOffsetRow));
	} 
    }

    Vector<Double> freqs((rec_p->shape())(1));
    Quantity refFreq;
    // default values
    String units="pixel";
    *refframe_p = MFrequency::showType(MFrequency::LSRK);

    if (spwOK) {
	if (freqs.nelements() > 0) {
	    // this would be much simpler if I could get Quantum<Array<Double> > from the CHAN_FREQ column
	    freqs = (romsCols_p->spectralWindow().chanFreq())(spwId);
	    units = ms_p->spectralWindow().unit(MSSpectralWindow::CHAN_FREQ);
	} else {
	    units = "Hz";
	}
	refFreq = (romsCols_p->spectralWindow().refFrequencyQuant())(spwId);
	*refframe_p = MFrequency::showType((romsCols_p->spectralWindow().measFreqRef())(spwId));
    } else {
	// just use pixel values, from 1
	indgen(freqs, 1.0);
	refFreq = Quantity(1.0,units);
    }
    // add in offset as necessary
    if (offset.getValue() != 0.0) {
	Double doffset = offset.get(units).getValue();
	freqs += doffset;
    }
    // okay, we can now make chan_freq
    Quantum<Vector<Double> > chanFreqQuant(freqs, units);
    String holderErr;
    if (!QuantumHolder(chanFreqQuant).toRecord(holderErr, *chanFreq_p)) {
	String errMsg = 
	    "SDMSIterator::copyAll() unexpected problem saving chan_freq Quanta to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    }

    // and store the reference frequency value
    *refFreq_p = refFreq.get(units).getValue();

    // channel width
    Quantity chanWidth;
    if (spwOK && freqs.nelements() > 0) {
	// find a reference pixel and channel width appropriate for REF_FREQUENCY
	Double refwidth, refpix;
	IPosition usePix(1,0);
	if (freqs.nelements() > 1) {
	    refwidth = freqs(1) - freqs(0);
	    refpix = getRefPix(freqs, refFreq.getValue(units), refwidth);
	    if (refpix < 0.0) usePix = 0;
	    else {
		uInt uPix;
		uPix = uInt(refpix + 0.5);
		if (uPix >= freqs.nelements()) uPix = freqs.nelements() - 1;
		usePix = uPix;
	    }
	} 
	chanWidth = ((romsCols_p->spectralWindow().chanWidthQuant())(spwId))(usePix);
    } else {
	chanWidth = Quantity(1.0, units);
    }

    *chanWidth_p = chanWidth.get(units).getValue();
  
    // corr_types
    Vector<String> stokes(rec_p->shape()(0));
    (*corrType_p).resize(IPosition(1,stokes.nelements()));
    if (polOK && Int(stokes.nelements()) == (romsCols_p->polarization().numCorr())(polId)) {
	Vector<Int> istokes = (romsCols_p->polarization().corrType())(polId);
	for (uInt i=0;i<istokes.nelements();i++) {
	    stokes(i) = Stokes::name(Stokes::type(istokes(i)));
	}
    } else {
	// everything is undefined
	stokes = Stokes::name(Stokes::Undefined);
    }
    *corrType_p = stokes;

    // restfrequency
    // rest frequency comes from the SOURCE table
    // SOURCE_ID requires the FIELD table
    Int fieldId = ids_p.fieldId(thisRow_p);
    Int sourceId = -1;
    if (fieldId >= 0) sourceId = romsCols_p->field().sourceId()(fieldId);
    Quantity restFreq;
    Bool sourceRowFound = False;
    uInt sourceRow = 0;
    // does the SOURCE table exist
    if (sourceId >= 0 && spwId >= 0 && !sourceIndex_p.isNull()) {
	sourceIndex_p.sourceId() = sourceId;
	sourceIndex_p.spectralWindowId() = spwId;
	sourceIndex_p.time() = stime;
	sourceIndex_p.interval() = intSecs;
	sourceRow = sourceIndex_p.getNearestRow(sourceRowFound);
	if (sourceRowFound) {
	    if (romsCols_p->source().numLines()(sourceRow)>0) {
	      // use the first line in the group if there's a choice
	      IPosition firstLine(1,0);
	      if (!romsCols_p->source().restFrequency().isNull()) {
		restFreq = (romsCols_p->source().restFrequencyQuant()(sourceRow))(firstLine);
	      }
	    }
	}
    }
    *restFreq_p = restFreq.get("Hz").getValue();

    // data array
    if (floatData_p) {
	*arr_p = (romsCols_p->floatData())(thisRow_p);
	rec_p->desc().define("units",dataUnits(MS::FLOAT_DATA));
    } else {
	if (correctedData_p) {
	    //	    *carr_p = (romsCols_p->correctedData())(thisRow_p);
	    // everything is always float now - get the real part here
	    *arr_p = real((romsCols_p->correctedData())(thisRow_p));
	    rec_p->desc().define("units",dataUnits(MS::CORRECTED_DATA));
	} else {
	    //	    *carr_p = (romsCols_p->data())(thisRow_p);
	    // everything is always float now - get the real part here
	    *arr_p = real((romsCols_p->data())(thisRow_p));
	    rec_p->desc().define("units",dataUnits(MS::DATA));
	}
    }
    if (weightSpectrum_p) {
	*weight_p = (romsCols_p->weightSpectrum())(thisRow_p);
    } else {
	Vector<Float> weight((romsCols_p->weight())(thisRow_p));
	Matrix<Float> mweight(*weight_p);
	for (uInt i=0;i<weight.nelements();i++) {
	    mweight.row(i) = weight(i);
	}
    }
    if (sigmaSpectrum_p) {
	*sigma_p = (romsCols_p->sigmaSpectrum())(thisRow_p);
    } else {
	Vector<Float> sigma((romsCols_p->sigma())(thisRow_p));
	Matrix<Float> msigma(*sigma_p);
	for (uInt i=0;i<sigma.nelements();i++) {
	    msigma.row(i) = sigma(i);
	}
    }
    // flag
    *flag_p = (romsCols_p->flag())(thisRow_p);
    // need to deal with flag_category and flag_row somewhere, but that is complicated
    // by the fact that intervals may overlap.
}

void SDMSIterator::copyHeader()
{
    // do nothing if there is nothing in ms_p
    if (!ms_p || ms_p->nrow() == 0) return;
    
    // resize - no penalty if called more than once on this row
    if (arr_p.isAttached()) {
	rec_p->resize(romsCols_p->floatData().shape(thisRow_p));
    } else {
	if (correctedData_p) {
	    rec_p->resize(romsCols_p->correctedData().shape(thisRow_p));
	} else {
	    rec_p->resize(romsCols_p->data().shape(thisRow_p));
	}
    }

    // things which come just from the MAIN table

    // TIME
    const MEpoch time = romsCols_p->timeMeas()(thisRow_p);
    String holderErr;
    if (!MeasureHolder(time).toRecord(holderErr, *time_p)) {
	String errMsg = 
	    "SDMSIterator::copyAll() unexpected problem saving time measure to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    }

    // scan_number
    *scan_number_p = (romsCols_p->scanNumber())(thisRow_p);

    // duration is INTERVAL in the MAIN table
    *duration_p = (romsCols_p->intervalQuant()(thisRow_p)).get("s").getValue();

    // exposure
    *exposure_p = (romsCols_p->exposureQuant()(thisRow_p)).get("s").getValue();

    // TIME and INTERVAL for lookups
    const Quantity interval = romsCols_p->intervalQuant()(thisRow_p);
    Double intSecs = interval.getValue(sec_p);
    Double stime = time.getValue().getTime().getValue(sec_p);

     // this will eventually come from the SOURCE table using the SYSVEL column when present, I think
    *veldef_p = MDoppler::showType(MDoppler::RADIO);

    // DATA_DESC_ID
    Int dataDescId = ids_p.dataDescId(thisRow_p);
    Bool ddOK = dataDescId >= 0 && !(romsCols_p->dataDescription().flagRow()(dataDescId));

    // SPECTRAL_WINDOW_ID
    Int spwId = ids_p.spectralWindowId(thisRow_p);
    Bool spwOK = ddOK && spwId >= 0 && !(romsCols_p->spectralWindow().flagRow()(spwId));

    // POLARIZATION_ID
    Int polId = ids_p.polarizationId(thisRow_p);
    Bool polOK = ddOK && polId >= 0 && !(romsCols_p->polarization().flagRow()(polId));

    Quantity resolution, bandwidth;
    if (spwOK) {
	Vector<Double> freqs((rec_p->shape()(1)));
	Quantity refFreq;
	String units="pixel";
	if (freqs.nelements() > 0) {
	    freqs = (romsCols_p->spectralWindow().chanFreq())(spwId);
	    units = ms_p->spectralWindow().unit(MSSpectralWindow::CHAN_FREQ);
	    refFreq = (romsCols_p->spectralWindow().refFrequencyQuant())(spwId);
	    IPosition usePix(1,0);
	    Double refwidth, refpix;
	    if (freqs.nelements() > 1) {
		refwidth = freqs(1) - freqs(0);
		refpix = getRefPix(freqs, refFreq.getValue(units), refwidth);
		if (refpix < 0.0) usePix = 0;
		else {
		    uInt uPix;
		    uPix = uInt(refpix + 0.5);
		    if (uPix >= freqs.nelements()) uPix = freqs.nelements() - 1;
		    usePix = uPix;
		}
	    }
	    // resolution
	    resolution = ((romsCols_p->spectralWindow().resolutionQuant())(spwId))(usePix);
	    *resolution_p = resolution.get(units).getValue();
	
	    // bandwidth
	    bandwidth = (romsCols_p->spectralWindow().totalBandwidthQuant())(spwId);
	    *bandwidth_p = bandwidth.get(units).getValue();
	} else {
	    resolution = bandwidth = Quantity(0.0,"Hz");
	}
    } else {
	resolution = bandwidth = Quantity(0.0,"pixels");
    }

    // direction comes from the POINTING table, assumes ANTENNA1==ANTENNA2
    // POINTING set index keys
    Int ant1Id = ids_p.antenna1(thisRow_p);
    Bool pointingRowFound = False;
    uInt pointingRow = 0;
    if (!pointingIndex_p.isNull()) {
	pointingIndex_p.antennaId() = ant1Id;
	pointingIndex_p.time() = stime;
	pointingIndex_p.interval() = intSecs;
	pointingRow = pointingIndex_p.getNearestRow(pointingRowFound);
    }

    MDirection dir;
    if (pointingRowFound) {
	// get the first element of the polynomial, don't worry about interpolation
	dir = romsCols_p->pointing().directionMeas(pointingRow);
	// store row for pointing
	rec_p->other().defineRecord("pointing", stdTabRows_p[pointingRow_p]->get(pointingRow));
    } else {	
	rec_p->other().defineRecord("pointing", Record());
    }
    if (!MeasureHolder(dir).toRecord(holderErr, *direction_p)) {
	String errMsg = 
	    "SDMSIterator::copyAll() unexpected problem saving direction Measure to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    }

    // things from the SOURCE table
    // SOURCE_ID requires the FIELD table
    Int fieldId = ids_p.fieldId(thisRow_p);
    Int sourceId = -1;
    if (fieldId >= 0) sourceId = romsCols_p->field().sourceId()(fieldId);
    Bool sourceRowFound = False;
    uInt sourceRow = 0;
    String trans, sourceName;
    // does the SOURCE table exist
    if (sourceId >= 0 && spwId >= 0 && !sourceIndex_p.isNull()) {
	sourceIndex_p.sourceId() = sourceId;
	sourceIndex_p.spectralWindowId() = spwId;
	sourceIndex_p.time() = stime;
	sourceIndex_p.interval() = intSecs;
	sourceRow = sourceIndex_p.getNearestRow(sourceRowFound);
	if (sourceRowFound) {
	    sourceName = romsCols_p->source().name()(sourceRow);
	    if (romsCols_p->source().numLines()(sourceRow)>0) {
	      // use the first line in the group if there's a choice
	      IPosition firstLine(1,0);
	      if ( !romsCols_p->source().transition().isNull()) {
		trans = romsCols_p->source().transition()(sourceRow)(firstLine);
	      }
	    }
	}
    }
    *transition_p = trans;
    *sourceName_p = sourceName;

    // weather, set index keys
    Bool weatherRowFound = False;
    uInt weatherRow = 0;
    if (!weatherIndex_p.isNull()) {
	weatherIndex_p.antennaId() = ant1Id;
	weatherIndex_p.time() = stime;
	weatherIndex_p.interval() = intSecs;
	weatherRow = weatherIndex_p.getNearestRow(weatherRowFound);
    }

    Quantum<Float> pres, dewp, temp, windd, winds;
    if (weatherRowFound) {
	// pressure
	if (!romsCols_p->weather().pressure().isNull()) {
	    if (romsCols_p->weather().pressureFlag().isNull() ||
		!romsCols_p->weather().pressureFlag()(weatherRow))
		pres = romsCols_p->weather().pressureQuant()(weatherRow);
	}
	
	// dewpoint
	if (!romsCols_p->weather().dewPoint().isNull()) {
	    if (romsCols_p->weather().dewPointFlag().isNull() ||
		!romsCols_p->weather().dewPointFlag()(weatherRow))
		dewp = romsCols_p->weather().dewPointQuant()(weatherRow);
	}

	// tambient
	if (!romsCols_p->weather().temperature().isNull()) {
	    if (romsCols_p->weather().temperatureFlag().isNull() ||
		!romsCols_p->weather().temperatureFlag()(weatherRow))
		temp = romsCols_p->weather().temperatureQuant()(weatherRow);
	}

	// winddir
	if (!romsCols_p->weather().windDirection().isNull()) {
	    if (romsCols_p->weather().windDirectionFlag().isNull() ||
		!romsCols_p->weather().windDirectionFlag()(weatherRow))
		windd = romsCols_p->weather().windDirectionQuant()(weatherRow);
	}

	// windspeed
	if (!romsCols_p->weather().windSpeed().isNull()) {
	    if (romsCols_p->weather().windSpeedFlag().isNull() ||
		!romsCols_p->weather().windSpeedFlag()(weatherRow))
		winds = romsCols_p->weather().windSpeedQuant()(weatherRow);
	}
    }
    *pressure_p = pres.get("hPa").getValue();
    *dewpoint_p = dewp.get("K").getValue();
    *tambient_p = temp.get("K").getValue();
    *winddir_p = windd.get("rad").getValue();
    *windspeed_p = winds.get("m/s").getValue();

    // telescope information from ANTENNA table
    // need to do something clever for ANTENNA1 != ANTENNA2, its not SD data.
    MPosition obsPos;
    Bool validObsPos = False;
    if (ant1Id>= 0 && !(romsCols_p->antenna().flagRow())(ant1Id)) {
	*telescope_p = (romsCols_p->antenna().name())(ant1Id);
	obsPos = (romsCols_p->antenna().positionMeas())(ant1Id);
	validObsPos = True;
    } else {
	// default values
	*telescope_p = "";
    }
    if (!MeasureHolder(obsPos).toRecord(holderErr, *telescopePosition_p)) {
	String errMsg = 
	    "SDMSIterator::copyAll() unexpected problem saving telescope position measure to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    }

    // azel
    MDirection azel;
    if (validObsPos) {
	// azel from direction and observatory position
	if (obsPos_p.getValue() != obsPos.getValue()) {
	    obsPos_p = obsPos;
	    frame_p.set(obsPos);
	}
	frame_p.set(time);
	azel = (*toAzEl_p)(dir.getValue());
    }
    if (!MeasureHolder(azel).toRecord(holderErr, *azel_p)) {
    	String errMsg = 
    	    "SDMSIterator::copyAll() unexpected problem saving azel direction  measure to record : " +
    	    holderErr;
    	throw(AipsError(errMsg));
    }

    // refdirection from FIELD table
    MDirection refDir;
    if (fieldId >= 0 && !romsCols_p->field().flagRow()(fieldId)) {
	// get the first element of the polynomial, don't worry about interpolation
	dir = romsCols_p->field().referenceDirMeas(fieldId);
    }
    if (!MeasureHolder(refDir).toRecord(holderErr, *refDirection_p)) {
	String errMsg = 
	    "SDMSIterator::copyAll() unexpected problem saving refdirection Measure to record : " +
	    holderErr;
	throw(AipsError(errMsg));
    } 
   
    // SYSCAL table
    Bool syscalRowFound = False;
    uInt syscalRow = 0;
    // FEED1 (assumed to be same as FEED2
    // what should happen if ant1!=ant2?
    // and what about feed2 != feed1?
    Int feed1Id = romsCols_p->feed1()(thisRow_p);
    if (feed1Id >= 0 && ant1Id >= 0 && spwId >= 0 && !syscalIndex_p.isNull()) {
	syscalIndex_p.antennaId() = ant1Id;
	syscalIndex_p.feedId() = feed1Id;
	syscalIndex_p.spectralWindowId() = spwId;
	syscalIndex_p.time() = stime;
	syscalIndex_p.interval() = intSecs;
	syscalRow = syscalIndex_p.getNearestRow(syscalRowFound);
    }
    if (syscalRowFound) {
	// what is there? Ignore the SPECTRUM ones for now - 
	//  come back to this at some time
	// mapping between receptors and correlations
	Int ncorr = rec_p->shape()(0);
	Vector<Int> rcpt1(ncorr), rcpt2(ncorr);
	if (polOK) {
	    Matrix<Int> corrProd = 
		romsCols_p->polarization().corrProduct()(polId);
	    // necessary to watch for bad pol tables
	    if (corrProd.shape()[0] == 2 &&
		corrProd.shape()[1] == ncorr) {
		for (Int i=0;i<corrProd.shape()[1];i++) {
		    rcpt1[i] = corrProd(0,i);
		    rcpt2[i] = corrProd(1,i);
		}
	    }
	} else {
	    indgen(rcpt1);
	    rcpt2 = rcpt2;
	}
	Bool cross = anyNE(rcpt1,rcpt2);
	Bool tcalOK = !romsCols_p->sysCal().tcal().isNull() &&
	    (romsCols_p->sysCal().tcalFlag().isNull() ||
	     !romsCols_p->sysCal().tcalFlag()(syscalRow));
	Bool trxOK = !romsCols_p->sysCal().trx().isNull() &&
	    (romsCols_p->sysCal().trxFlag().isNull() ||
	     !romsCols_p->sysCal().trxFlag()(syscalRow));
	Bool tsysOK = !romsCols_p->sysCal().tsys().isNull() &&
	    (romsCols_p->sysCal().tsysFlag().isNull() ||
	     !romsCols_p->sysCal().tsysFlag()(syscalRow));
	Vector<Float> tcalRcpt, trxRcpt, tsysRcpt,
	    thisTcal, thisTrx, thisTsys;
	thisTcal.resize(ncorr);
	thisTrx.resize(ncorr);
	thisTsys.resize(ncorr);
	thisTcal = thisTrx = thisTsys = 0.0;
	if (tcalOK) {
	    tcalRcpt = romsCols_p->sysCal().tcal()(syscalRow);
	}
	if (trxOK) {
	    trxRcpt = romsCols_p->sysCal().trx()(syscalRow);
	}
	if (tsysOK) {
	    tsysRcpt = romsCols_p->sysCal().tsys()(syscalRow);
	}
	for (uInt i=0;i<rcpt1.nelements();i++) {
	    if (!cross) {
		if (tcalOK) {
		    thisTcal[i] = tcalRcpt[rcpt1[i]];
		}
		if (trxOK) {
		    thisTrx[i] = trxRcpt[rcpt1[i]];
		} 
		if (tsysOK) {
		    thisTsys[i] = tsysRcpt[rcpt1[i]];
		}
	    } else {
		if (tcalOK) {
		    thisTcal[i] = 
			sqrt(tcalRcpt[rcpt1[i]]*tcalRcpt[rcpt2[i]]);
		}
		if (trxOK) {
		    thisTrx[i] = sqrt(trxRcpt[rcpt1[i]]*trxRcpt[rcpt2[i]]);
		}
		if (tsysOK) {
		    thisTsys[i] = 
			sqrt(tsysRcpt[rcpt1[i]]*tsysRcpt[rcpt2[i]]);
		}
	    }
	}
	*tcal_p = thisTcal;
	*trx_p = thisTrx;
	*tsys_p = thisTsys;
    } else {
	*tcal_p = 0.0;
	*trx_p = 0.0;
	*tsys_p = 1.0;
    } 

    // OBSERVATION table
    Int obsId = ids_p.observationId(thisRow_p);
    if (obsId >= 0) {
	*observer_p = (romsCols_p->observation().observer())(obsId);
	*project_p = (romsCols_p->observation().project())(obsId);
    } else {
	*observer_p = "";
	*project_p = "";
    }
}

void SDMSIterator::copyHist()
{
    // do nothing if there is nothing in ms_p
    if (!ms_p || ms_p->nrow() == 0) return;
    
}

void SDMSIterator::copyOther()
{
    // do nothing if there is nothing in ms_p
    if (!ms_p || ms_p->nrow() == 0) return;
    
    // TIME and INTERVAL for lookups
    const Quantity interval = romsCols_p->intervalQuant()(thisRow_p);
    const MEpoch time = romsCols_p->timeMeas()(thisRow_p);
    Double intSecs = interval.getValue(sec_p);
    Double stime = time.getValue().getTime().getValue(sec_p);

    // store main row in other - this doesn't re-read the current row, which will
    // be a problem when this becomes more than read-only
    rec_p->other().defineRecord("main", stdTabRows_p[mainRow_p]->get(thisRow_p));

    // IDs into standard subtables
    Int dataDescId = ids_p.dataDescId(thisRow_p);
    Int spwId = ids_p.spectralWindowId(thisRow_p);
    Int polId = ids_p.polarizationId(thisRow_p);
    Bool ddOK = dataDescId >= 0 && !(romsCols_p->dataDescription().flagRow()(dataDescId));
    Bool spwOK = ddOK && spwId >= 0 && !(romsCols_p->spectralWindow().flagRow()(spwId));
    Bool polOK = ddOK && polId >= 0 && !(romsCols_p->polarization().flagRow()(polId));

    // store row for data description
    if (ddOK) {
	rec_p->other().defineRecord("data_description", stdTabRows_p[dataDescRow_p]->get(dataDescId));
    } else {
	rec_p->other().defineRecord("data_description", Record());
    }

    // store row for spectral window
    if (spwOK) {
	rec_p->other().defineRecord("spectral_window", stdTabRows_p[spwRow_p]->get(spwId));
    } else {
	rec_p->other().defineRecord("spectral_window", Record());
    }

    // store row for polarization
    if (polOK) {
	rec_p->other().defineRecord("polarization", stdTabRows_p[polRow_p]->get(polId));
    } else {
	rec_p->other().defineRecord("polarization", Record());
    }

    Int ant1Id = ids_p.antenna1(thisRow_p);
    // store row for antenna1
    if (ant1Id >= 0) {
	rec_p->other().defineRecord("antenna", stdTabRows_p[antRow_p]->get(ant1Id));
    } else {
	rec_p->other().defineRecord("antenna", Record());
    }

    
    // store row for field
    Int fieldId = ids_p.fieldId(thisRow_p);
    if (fieldId >= 0) {
	rec_p->other().defineRecord("field", stdTabRows_p[fieldRow_p]->get(fieldId));
    } else {
	rec_p->other().defineRecord("field", Record());
    }

    Int sourceId = -1;
    if (fieldId >= 0) sourceId = romsCols_p->field().sourceId()(fieldId);
    Bool sourceRowFound = False;
    uInt sourceRow = 0;
    // does the SOURCE table exist
    if (sourceId >= 0 && spwId >= 0 && !sourceIndex_p.isNull()) {
	sourceIndex_p.sourceId() = sourceId;
	sourceIndex_p.spectralWindowId() = spwId;
	sourceIndex_p.time() = stime;
	sourceIndex_p.interval() = intSecs;
	sourceRow = sourceIndex_p.getNearestRow(sourceRowFound);
	if (sourceRowFound) {
	    // store row for source
	    rec_p->other().defineRecord("source", stdTabRows_p[sourceRow_p]->get(sourceRow));
	} else {
	    rec_p->other().defineRecord("source", Record());
	}
    } else if (sourceRow_p>0) {
	rec_p->other().defineRecord("source", Record());
    }
    // weather, set index keys
    Bool weatherRowFound = False;
    uInt weatherRow = 0;
    if (!weatherIndex_p.isNull()) {
	weatherIndex_p.antennaId() = ant1Id;
	weatherIndex_p.time() = stime;
	weatherIndex_p.interval() = intSecs;
	weatherRow = weatherIndex_p.getNearestRow(weatherRowFound);
    }

    if (weatherRowFound) {
	// store row for weather
	rec_p->other().defineRecord("weather", stdTabRows_p[weatherRow_p]->get(weatherRow));
    } else if (weatherRow_p >= 0) {
	rec_p->other().defineRecord("weather", Record());
    }

    // SYSCAL table
    Bool syscalRowFound = False;
    uInt syscalRow = 0;
    // FEED1 (assumed to be same as FEED2
    // what should happen if ant1!=ant2?
    // and what about feed2 != feed1?
    Int feed1Id = romsCols_p->feed1()(thisRow_p);

    if (feed1Id >= 0 && ant1Id >= 0 && spwId >= 0 && !syscalIndex_p.isNull()) {
	syscalIndex_p.antennaId() = ant1Id;
	syscalIndex_p.feedId() = feed1Id;
	syscalIndex_p.spectralWindowId() = spwId;
	syscalIndex_p.time() = stime;
	syscalIndex_p.interval() = intSecs;
	syscalRow = syscalIndex_p.getNearestRow(syscalRowFound);
    }
    if (syscalRowFound) {
	// store row for syscal
	rec_p->other().defineRecord("syscal", stdTabRows_p[syscalRow_p]->get(syscalRow));
    } else {
	if (syscalRow_p >= 0) {
	    rec_p->other().defineRecord("syscal", Record());
	}
    }

    // store observation row
    if (observationRow_p >= 0) {
	Int obsId = ids_p.observationId(thisRow_p);
	if (obsId >= 0) {
	    rec_p->other().defineRecord("observation", stdTabRows_p[observationRow_p]->get(obsId));
	} else {
	    rec_p->other().defineRecord("observation", Record());
	}
    }

    // store processor row
    if (procRow_p >= 0) {
	Int procId = ids_p.processorId(thisRow_p);
	if (procId >= 0) {
	    rec_p->other().defineRecord("processor", stdTabRows_p[procRow_p]->get(procId));
	} else {
	    rec_p->other().defineRecord("processor", Record());
	}
    }

    // store state row
    if (stateRow_p >= 0) {
	Int stateId = ids_p.stateId(thisRow_p);
	if (stateId >= 0) {
	    rec_p->other().defineRecord("state", stdTabRows_p[stateRow_p]->get(stateId));
	} else {
	    rec_p->other().defineRecord("state", Record());
	}
    }

    // store doppler row
    if (dopplerRow_p >= 0 && spwOK && sourceId >= 0 && !romsCols_p->spectralWindow().dopplerId().isNull()) {
	dopplerIndex_p.dopplerId() = romsCols_p->spectralWindow().dopplerId()(spwId);
	dopplerIndex_p.sourceId() = sourceId;
	Bool dopplerRowFound = False;
	uInt dopplerRow = dopplerIndex_p.getNearestRow(dopplerRowFound);
	if (dopplerRowFound) {
	    rec_p->other().defineRecord("doppler", stdTabRows_p[dopplerRow_p]->get(dopplerRow));
	} else {
	    rec_p->other().defineRecord("doppler", Record());
	}
    }
    // store feed row
    if (ant1Id >= 0 && feed1Id >= 0 && spwOK && !feedIndex_p.isNull()) {
	feedIndex_p.antennaId() = ant1Id;
	feedIndex_p.feedId() = feed1Id;
	feedIndex_p.spectralWindowId() = spwId;
	feedIndex_p.time() = stime;
	feedIndex_p.interval() = intSecs;
	Bool feedRowFound;
	uInt feedRow = feedIndex_p.getNearestRow(feedRowFound);
	if (feedRowFound) {
	    rec_p->other().defineRecord("feed", stdTabRows_p[feedRow_p]->get(feedRow));
	} else {
	    rec_p->other().defineRecord("feed", Record());
	}
    }

    // store any non-standard subtables
    for (uInt i=0;i<nsTabRows_p.nelements();i++) {
	if (!nsIndexes_p[i]->isNull()) {
	    nsIndexes_p[i]->time() = stime;
	    nsIndexes_p[i]->interval() = intSecs;
	    Bool nsRowFound;
	    uInt nsRow = nsIndexes_p[i]->getNearestRow(nsRowFound);
	    if (nsRowFound) {
		rec_p->other().defineRecord(nsOtherNames_p[i], nsTabRows_p[i]->get(nsRow));
	    } else {
		rec_p->other().defineRecord(nsOtherNames_p[i], Record());
	    }
	}
    }
}

Bool SDMSIterator::replaceAll(const SDRecord& rec, Bool rowIsNew)
{
    // this doesn't work yet
    return False;
}

void SDMSIterator::cleanRec()
{
    delete rec_p;
    rec_p = 0;
}

void SDMSIterator::cleanup()
{
    cleanRec();

    delete toAzEl_p;
    toAzEl_p = 0;

    // do ms_p last
    // if ms_p has no rows, mark it for deletion
    if (ms_p && ms_p->nrow() == 0) ms_p->markForDelete();
    delete ms_p;
    ms_p = 0;

    delete romsCols_p;
    romsCols_p = 0;

    for (uInt i=0;i<stdTabRows_p.nelements();i++) {
	delete stdTabRows_p[i];
	stdTabRows_p[i] = 0;
    }
    for (uInt i=0;i<nsTabRows_p.nelements();i++) {
	delete nsTabRows_p[i];
	nsTabRows_p[i] = 0;
    }
    for (uInt i=0;i<nsIndexes_p.nelements();i++) {
	delete nsIndexes_p[i];
	nsIndexes_p[i] = 0;
    }

    for (uInt i=0;i<nsTables_p.nelements();i++) {
	delete nsTables_p[i];
	nsTables_p[i] = 0;
    }

    mainRow_p = antRow_p = dopplerRow_p = feedRow_p = fieldRow_p = 
	observationRow_p = pointingRow_p = polRow_p = procRow_p = sourceRow_p =
	spwRow_p = stateRow_p = syscalRow_p = weatherRow_p = -1;
}

void SDMSIterator::initRec()
{
    //    if (floatData_p || noData_p) {
    //	rec_p = new SDRecord();
    //    } else {
    //	rec_p = new SDRecord(TpArrayComplex);
    //    }
    // we always return a Float type here
    rec_p = new SDRecord();
    AlwaysAssert(rec_p, AipsError);

    // attach field pointers to fields in rec_p
    chanFreq_p.attachToRecord(rec_p->desc(),"chan_freq");
    refframe_p.attachToRecord(rec_p->desc(),"refframe");

//     if (floatData_p) {
// 	arr_p.attachToRecord(rec_p->data(),"arr");
//     } else {
// 	carr_p.attachToRecord(rec_p->data(),"arr");
//     }
    // things are currently always Float
    arr_p.attachToRecord(rec_p->data(),"arr");

    flag_p.attachToRecord(rec_p->data(),"flag");
    weight_p.attachToRecord(rec_p->data(),"weight");
    sigma_p.attachToRecord(rec_p->data(),"sigma");

    veldef_p.attachToRecord(rec_p->header(),"veldef");
    transition_p.attachToRecord(rec_p->header(),"transition");
    scan_number_p.attachToRecord(rec_p->header(),"scan_number");

    azel_p.attachToRecord(rec_p->header(),"azel");
    direction_p.attachToRecord(rec_p->header(),"direction");
    refDirection_p.attachToRecord(rec_p->header(),"refdirection");
    time_p.attachToRecord(rec_p->header(),"time");
    sourceName_p.attachToRecord(rec_p->header(),"source_name");
    telescopePosition_p.attachToRecord(rec_p->header(),"telescope_position");
    duration_p.attachToRecord(rec_p->header(),"duration");
    exposure_p.attachToRecord(rec_p->header(),"exposure");
    observer_p.attachToRecord(rec_p->header(),"observer");
    project_p.attachToRecord(rec_p->header(),"project");
    refFreq_p.attachToRecord(rec_p->desc(),"reffrequency");
    restFreq_p.attachToRecord(rec_p->desc(),"restfrequency");
    chanWidth_p.attachToRecord(rec_p->desc(),"chan_width");
    resolution_p.attachToRecord(rec_p->header(),"resolution");
    bandwidth_p.attachToRecord(rec_p->header(),"bandwidth");
    telescope_p.attachToRecord(rec_p->header(),"telescope");
    corrType_p.attachToRecord(rec_p->desc(),"corr_type");
    pressure_p.attachToRecord(rec_p->header(),"pressure");
    dewpoint_p.attachToRecord(rec_p->header(),"dewpoint");
    tambient_p.attachToRecord(rec_p->header(),"tambient");
    winddir_p.attachToRecord(rec_p->header(),"wind_dir");
    windspeed_p.attachToRecord(rec_p->header(),"wind_speed");

    tcal_p.attachToRecord(rec_p->header(),"tcal");
    trx_p.attachToRecord(rec_p->header(),"trx");
    tsys_p.attachToRecord(rec_p->header(),"tsys");
    // now do the same things for the case where the table is writable
    if (isWritable()) {
	// nothing to do here yet
    }
    // initialize the stringFields_ record
    initStringFields(stringFields_p, *rec_p);
    // fill the data.desc.units with whatever is in QuantumUnits of the
    // DATA, FLOAT_DATA, or CORRECTED_DATA columns
    String dunits = "CNTS";
    if (correctedData_p) {
	dunits = dataUnits(MS::CORRECTED_DATA);	
    } else {
	if (floatData_p) {
	    dunits = dataUnits(MS::FLOAT_DATA);
	} else if (!noData_p) {
	    dunits = dataUnits(MS::DATA);
	}
    }
    rec_p->desc().define("units",dunits);
}

void SDMSIterator::resetDataType()
{
    // FLOAT or COMPLEX ??
    correctedData_p = correctedData_p && !romsCols_p->correctedData().isNull();
    if (!correctedData_p) {
	floatData_p = !romsCols_p->floatData().isNull();
	noData_p = (!floatData_p && romsCols_p->data().isNull());
    } else {
	// if CORRECTED_DATA has been requested, its always complex
	// if we get here, correctedData has been required AND it exists.
	floatData_p = False;
	noData_p = False;
    }
}

void SDMSIterator::init()
{
    // only ms_p is set by the time it gets here
    // ids
    ids_p.attach(*ms_p);

    // columns
    romsCols_p = new ROMSColumns(*ms_p);
    AlwaysAssert(romsCols_p, AipsError);

    // using current value of correctedData_p, reset the data type
    resetDataType();

    weightSpectrum_p = !romsCols_p->weightSpectrum().isNull();
    sigmaSpectrum_p = !romsCols_p->sigmaSpectrum().isNull();

    // create tableRows as appropriate
    uInt nStdTabs, nNSTabs;
    nStdTabs = nNSTabs = 0;
    TableRecord kwSet(ms_p->keywordSet());
    uInt nFields = kwSet.nfields();
    stdTabRows_p.resize(nFields+1);

    // there's always the MAIN table - exclude the columns handled here
    stdTabRows_p[nStdTabs] = 
	new ROTableRow(*ms_p, 
		       stringToVector("TIME,INTERVAL,EXPOSURE,SCAN_NUMBER,DATA,FLOAT_DATA,CORRECTED_DATA,CORRECTED_FLOAT_DATA,LAG_DATA,SIGMA,SIGMA_SPECTRUM,WEIGHT,WEIGHT_SPECTRUM,FLAG,FLAG_CATEGORY,FLAG_ROW,MODEL_DATA,IMAGING_WEIGHT,MODEL_DATA_SCALE,MODEL_DATA_OFFSET,MODEL_DATA_COMPRESSED,IMAGING_WEIGHT_SCALE,IMAGING_WEIGHT_OFFSET,IMAGING_WEIGHT_COMPRESSED,CORRECTED_DATA_SCALE,CORRECTED_DATA_OFFSET,CORRECTED_DATA_COMPRESSED"),True);
    AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
    mainRow_p = nStdTabs;
    nStdTabs++;

    Vector<Bool> handledKwds(nFields, False);
    for (uInt i=0;i<nFields;i++) {
	if (kwSet.dataType(i) == TpTable) {
	    String name = kwSet.name(i);
	    if (name == "ANTENNA") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->antenna(), 
				   stringToVector("STATION,TYPE,MOUNT,OFFSET,FLAG_ROW"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		antRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "DATA_DESCRIPTION") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->dataDescription(), stringToVector("FLAG_ROW"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		dataDescRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "DOPPLER") {
		stdTabRows_p[nStdTabs] = new ROTableRow(ms_p->doppler());
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		dopplerRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "FEED") {
		stdTabRows_p[nStdTabs] = new ROTableRow(ms_p->feed());
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		feedRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "FIELD") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->field(), stringToVector("REFERENCE_DIR,FLAG_ROW"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		fieldRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "FLAG_CMD") {
		// I don't know what FLAG_CMD should do here, but nearest row must be wrong, so don't yet.
		handledKwds(i) = True;
	    } else if (name == "FREQ_OFFSET") {
		// if present OFFSET will have been used so don't include it here as there's nothing left
		handledKwds(i) = True;
	    } else if (name == "HISTORY") {
		// Not sure what to do about HISTORY either
		handledKwds(i) = True;
	    } else if (name == "OBSERVATION") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->observation(), stringToVector("FLAG_ROW"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		observationRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "POINTING") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->pointing(), stringToVector("DIRECTION,FLAG_ROW"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		pointingRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "POLARIZATION") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->polarization(), stringToVector("CORR_TYPE,FLAG_ROW"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		polRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "PROCESSOR") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->processor(), stringToVector("FLAG_ROW"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		procRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "SOURCE") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->source(), stringToVector("NAME,TRANSITION,REST_FREQUENCY"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		sourceRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "SPECTRAL_WINDOW") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->spectralWindow(), 
				   stringToVector("REF_FREQUENCY,CHAN_FREQ,CHAN_WIDTH,MEAS_FREQ_REF,EFFECTIVE_BW,RESOLUTION_TOTAL_BANDWIDTH,FLAG_ROW"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		spwRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "STATE") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->state(), stringToVector("FLAG_ROW"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		stateRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "SYSCAL") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->sysCal(), stringToVector("TCAL,TRX,TSYS,FLAG_ROW"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		syscalRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name == "WEATHER") {
		stdTabRows_p[nStdTabs] = 
		    new ROTableRow(ms_p->weather(), 
				   stringToVector("PRESSURE,TEMPERATURE,DEW_POINT,WIND_DIRECTION,WIND_SPEED"),True);
		AlwaysAssert(stdTabRows_p[nStdTabs], AipsError);
		weatherRow_p = nStdTabs;
		handledKwds(i) = True;
		nStdTabs++;
	    } else if (name != "SORTED_TABLE") {
		// count up the non-standard sub-tables, ignore SORTED_TABLE - its a
		// reference back to this table, so its redundent here
                nNSTabs++;
	    } else {
	        // whatever this is (probably SORTED_TABLE) ignore it
		handledKwds(i) = True;
	    }	
	} else {
	    handledKwds(i) = True;
	}
    }
    // resize things
    stdTabRows_p.resize(nStdTabs,True);
    nsTabRows_p.resize(nNSTabs,True);
    nsIndexes_p.resize(nNSTabs,True);
    nsTables_p.resize(nNSTabs,True);
    nsOtherNames_p.resize(nNSTabs,True);
    nNSTabs = 0;

    // and once more, for the non-standard sub-tables
    for (uInt i=0;i<nFields;i++) {
	if (!handledKwds(i)) {
	    String kwname = kwSet.name(i);
            String name = kwSet.tableAttributes(i).name();
	    nsTables_p[nNSTabs] = new Table(name, ms_p->lockOptions());
	    AlwaysAssert(nsTables_p[nNSTabs], AipsError);
	    nsTabRows_p[nNSTabs] = new ROTableRow(*nsTables_p[nNSTabs]);
	    AlwaysAssert(nsTabRows_p[nNSTabs], AipsError);
	    nsIndexes_p[nNSTabs] = new MSTableIndex(*nsTables_p[nNSTabs],
						    stringToVector(""));
	    AlwaysAssert(nsIndexes_p[nNSTabs], AipsError);
	    String otherName = kwname;
	    otherName.downcase();
	    if (otherName == "nrao_gbt_glish"){
		// make use with old table transparent
		otherName = "gbt_go";
	    } else if (otherName.contains("spectrometer")) {
		// make use with old tables transparent
		otherName.gsub("spectrometer","acs");
	    }
	    nsOtherNames_p[nNSTabs] = otherName;
	    nNSTabs++;
	}
    }

    // initialize rec_p and related
    initRec();

    // initialize the indexes
    if (!ms_p->weather().isNull()) {
	weatherIndex_p.attach(ms_p->weather());
    }
    if (!ms_p->pointing().isNull()) {
	pointingIndex_p.attach(ms_p->pointing());
    }
    if (!ms_p->sysCal().isNull()) {
	syscalIndex_p.attach(ms_p->sysCal());
    }
    if (!ms_p->feed().isNull()) {
	feedIndex_p.attach(ms_p->feed());
    }
    if (!ms_p->doppler().isNull()) {
	dopplerIndex_p.attach(ms_p->doppler());
    }
    if (!ms_p->source().isNull()) {
	sourceIndex_p.attach(ms_p->source());
    }
    if (!ms_p->freqOffset().isNull()) {
	freqOffsetIndex_p.attach(ms_p->freqOffset());
    }

    // finally, the azel conversion engine stuff
    // make sure frame corresponds to current obsPos_p value
    frame_p.set(obsPos_p);
    // set up a conversion engine : from the fixed reference 
    // type of the POINTING::DIRECTION column to AZEL
    MDirection::Ref pointRef = romsCols_p->pointing().directionMeasCol().getMeasRef();
    delete toAzEl_p;
    toAzEl_p = 0;
    toAzEl_p = new MDirection::Convert(pointRef, MDirection::Ref(MDirection::AZEL, frame_p));
    AlwaysAssert(toAzEl_p, AipsError);

    ok_p = True;
}


void SDMSIterator::createMS(const String &msName, TableLock::LockOption lockoption,
			    Table::TableOption opt)
{
}

Bool SDMSIterator::put(const SDRecord& rec)
{
    return False;
}

Bool SDMSIterator::appendRec(const SDRecord& rec)
{
    return False;
}

Bool SDMSIterator::deleteRec()
{
    return False;
}

Bool SDMSIterator::useCorrectedData(Bool correctedData) {
    if (correctedData != correctedData_p) {
	cleanRec();
	correctedData_p = correctedData;
	resetDataType();
	initRec();
    }
    // this check will return F only if there is no CORRECTED_DATA column
    // to be used - in which case this will have fallen back on FLOAT_DATA
    return (correctedData == correctedData_p);
}

void SDMSIterator::initStringFields(Record &fieldRec, const Record &modelRec)
{
    RecordDesc modelDesc = modelRec.description();
    for (uInt i=0;i<modelDesc.nfields();i++) {
	if (modelDesc.isSubRecord(i)) {
	    fieldRec.defineRecord(modelDesc.name(i), Record());
	    initStringFields(fieldRec.rwSubRecord(modelDesc.name(i)),
			    modelRec.subRecord(modelDesc.name(i)));
	} else if (modelDesc.type(i) == TpString &&
		   modelDesc.isScalar(i)) {
	    fieldRec.define(modelDesc.name(i), True);
	}
    }
    // and add in the date field IF there is a source_name at this level
    if (fieldRec.fieldNumber("source_name") >= 0) {
	fieldRec.define("date", True);
    }
}

// this returns it zero-based
Double SDMSIterator::getRefPix(const Vector<Double> &chanVals, Double refVal, Double &delta,
			       Int count)
{
    Double newDelta, result;

    if (delta == 0.0) {
	delta = (chanVals(1) - chanVals(0));
    }

    // if it is still zero, give up
    if (delta == 0.0) return 0.0;

    result = (refVal - chanVals(0)) / delta;

    // find delta at nearest pixel to result
    if (result < 0) {
	newDelta = (chanVals(1) - chanVals(0));
    } else {
	// truncate
	uInt ilower = Int(result);
	if (ilower >= (chanVals.nelements()-1)) {
	    ilower = chanVals.nelements()-2;
	}
	newDelta = (chanVals(ilower+1) - chanVals(ilower));
    }

    // if newDelta is zero, give up
    if (newDelta == 0.0) {
	delta = newDelta;
	return result;
    }

    // stop after 10 iterations no matter what
    if (!near(newDelta, delta) && count < 10) {
	// iterate
	result = getRefPix(chanVals, refVal, newDelta, count+1);
    }
    delta = newDelta;
    return result;
}

void SDMSIterator::applySelection(const Record &selection)
{
    LogIO os(LogOrigin("SDMSIterator","applySelection"));
    // first, if there is a row selection indicated, do that first
    if (selection.isDefined("row")) {
	*ms_p = (*ms_p)(parseRow(selection,ms_p->nrow()));
    }
    // the rest of this is going to be tricky
    String query;

    // for now, this just does some fields

    // Things which depend on the SOURCE table
    Vector<Int> fieldIds, dataDescIds;
    String sourceQuery;

    // source name
    if (selection.isDefined("header") &&
	selection.subRecord("header").isDefined("source_name")) {
	Vector<String> list;
	Int objectField = selection.subRecord("header").fieldNumber("source_name");
	if (selection.subRecord("header").type(objectField) == TpString) {
	    // single value
	    list.resize(1);
	    selection.subRecord("header").get(objectField, list(0));
	} else if (selection.subRecord("header").type(objectField) == TpArrayString) {
	    // vector of values
	    selection.subRecord("header").get(objectField, list);
	} 
	if (list.nelements() == 0) {
	    os << LogIO::WARN
	       << "Selections must be string vectors only for "
	       << "fields which contain strings."
	       << "\nSelection field=header.source_name"	       
	       << " violates this rule."
	       << "\nThis field will be ignored."
	       << LogIO::POST;
	} else {
	    sourceQuery = sourceQuery + parseList(list, "NAME");
	}
    }
    // restfrequency
    if (selection.isDefined("data") &&
	selection.subRecord("data").isDefined("desc") &&
	selection.subRecord("data").subRecord("desc").isDefined("restfrequency")) {
	Matrix<Double> ranges;
	Int rfField = selection.subRecord("data").subRecord("desc").
	  fieldNumber("restfrequency");
	IPosition rfFieldShape = selection.subRecord("data").subRecord("desc").
	  shape(rfField);
	if (rfFieldShape.nelements() == 2) {
	    selection.subRecord("data").subRecord("desc").get(rfField, ranges);
	} else if (rfFieldShape.nelements() == 1 &&
		   rfFieldShape(0) % 2 == 0) {
	    // assume this is a 2 x nranges Matrix
	    Vector<Double> vranges;
	    selection.subRecord("data").subRecord("desc").get(rfField, vranges);
	    IPosition mshape(2);
	    mshape(0) = 2;
	    mshape(1) = vranges.nelements()/2;
	    ranges = vranges.reform(mshape);
	}
	if (ranges.nrow() != 2) {
	    os << LogIO::WARN
	       << "The shape of the selection Matrix must be (2,nranges)\n"
		   << "Selection field=data.desc.restfrequency"
		   << " violates this rule."
		   << "\nThis field will be ignored."
		   << LogIO::POST;
	} else {
	    if (sourceQuery.length() > 0) sourceQuery = sourceQuery + " AND ";
	    sourceQuery = sourceQuery + parseRanges(ranges, "REST_FREQUENCY", True);
	}
    }
    if (sourceQuery.length() != 0) {
	String tmpSource = File::newUniqueName("/tmp", "SDMS_TMP_SOURCE").absoluteName();
	sourceQuery = "SELECT FROM $1 WHERE " + sourceQuery + " GIVING " + tmpSource;
	Table sourceTab(tableCommand(sourceQuery, ms_p->source()));
	sourceTab.markForDelete();
	// if any spectral window IDs in the resulting SOURCE selection are -1, they match all
	// spectral window IDs and hence there is no selection on data description
	ROTableVector<Int> spWinIdCol(sourceTab, "SPECTRAL_WINDOW_ID");
	ROTableVector<Int> sourceIdCol(sourceTab, "SOURCE_ID");
	String spIds = asStringVec(spWinIdCol.makeVector());
	String sourceIds = asStringVec(sourceIdCol.makeVector());
	// asStringVec returns an empty string if there are any sp win IDs are -1
	if (spIds.length() > 0) {
	    if (query.length() > 0) query = query + " AND ";
	    query = query + 
		" DATA_DESC_ID IN [SELECT FROM ::DATA_DESCRIPTION WHERE SPECTRAL_WINDOW_ID IN " +
		spIds + " giving [ROWID()]] ";
	}
	if (sourceIds.length() > 0) {
	    if (query.length() > 0) query = query + " AND ";
	    query = query + 
		" FIELD_ID IN [SELECT FROM ::FIELD WHERE SOURCE_ID IN " + sourceIds + " giving [ROWID()]] ";
	}
    }

    // main table selection
    // scan number
    if (selection.isDefined("header") && 
	selection.subRecord("header").isDefined("scan_number")) {
	Matrix<Double> ranges;
	Int scanField = selection.subRecord("header").fieldNumber("scan_number");
	IPosition scanFieldShape = selection.subRecord("header").shape(scanField);
	if (scanFieldShape.nelements() == 2) {
	    selection.subRecord("header").get(scanField, ranges);
	} else if (scanFieldShape.nelements() == 1 &&
		   scanFieldShape(0) % 2 == 0) {
	    // assume this is a 2 x nranges Matrix
	    Vector<Double> vranges;
	    selection.subRecord("header").get(scanField, vranges);
	    IPosition mshape(2);
	    mshape(0) = 2;
	    mshape(1) = vranges.nelements()/2;
	    ranges = vranges.reform(mshape);
	}
	if (ranges.nrow() != 2) {
	    os << LogIO::WARN
	       << "The shape of the selection Matrix must be (2,nranges)\n"
		   << "Selection field=header.scan_number"
		   << " violates this rule."
		   << "\nThis field will be ignored."
		   << LogIO::POST;
	} else {
	    if (query.length() > 0) query = query + " AND ";
	    query = query + parseRanges(ranges, "SCAN_NUMBER");
	}
    }
    // date
    if (selection.isDefined("header") &&
	selection.subRecord("header").isDefined("date")) {
	// DATE is a string
	// eventually need some way to do a range of dates here
	Vector<String> list;
	Int dateField = selection.subRecord("header").fieldNumber("date");
	if (selection.subRecord("header").type(dateField) == TpString) {
	    // single value
	    list.resize(1);
	    selection.subRecord("header").get(dateField, list(0));
	} else if (selection.subRecord("header").type(dateField) == TpArrayString) {
	    // vector of values
	    selection.subRecord("header").get(dateField, list);
	} 
	if (list.nelements() == 0) {
	    os << LogIO::WARN
	       << "Selections must be string vectors only for "
	       << "fields which contain strings."
	       << "\nSelection field=header.date"	       << " violates this rule."
	       << "\nThis field will be ignored."
	       << LogIO::POST;
	} else {
	    String thisQuery;
	    // allowing wildcards here seems to be not something simple
	    for (uInt i=0;i<list.nelements();i++) {
		if (list(i).length() != 0) {
		    String item(list(i));
		    // translate FITS strings to MVTime standards
		    if (item.matches(Regex("[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]"))) item.gsub("-","/");
		    if (thisQuery.length() > 0) thisQuery = thisQuery + " || ";
		    thisQuery = thisQuery + "DATE(MJDTODATE(TIME/(60*60*24))) == DATE(DATETIME(" + '"' + item + '"' + "))";
		}
	    }
	    if (query.length() > 0) query = query + " AND ";
	    query = query + " ( " + thisQuery + " ) ";
	}
    }
    // ut
    if (selection.isDefined("header") && 
	selection.subRecord("header").isDefined("ut")) {
	Matrix<Double> ranges;
	Int utField = selection.subRecord("header").fieldNumber("ut");
	IPosition utFieldShape = selection.subRecord("header").shape(utField);
	if (utFieldShape.nelements() == 2) {
	    selection.subRecord("header").get(utField, ranges);
	} else if (utFieldShape.nelements() == 1 &&
		   utFieldShape(0) % 2 == 0) {
	    // assume this is a 2 x nranges Matrix
	    Vector<Double> vranges;
	    selection.subRecord("header").get(utField, vranges);
	    IPosition mshape(2);
	    mshape(0) = 2;
	    mshape(1) = vranges.nelements()/2;
	    ranges = vranges.reform(mshape);
	}
	if (ranges.nrow() != 2) {
	    os << LogIO::WARN
	       << "The shape of the selection Matrix must be (2,nranges)\n"
		   << "Selection field=header.ut"
		   << " violates this rule."
		   << "\nThis field will be ignored."
		   << LogIO::POST;
	} else {
	    // convert the ranges to radians - values are in seconds
	    ranges *= (C::pi*2.0/(60.0*60.0*24.0));
	    if (query.length() > 0) query = query + " AND ";
	    query = query + parseRanges(ranges, "TIME(MJDTODATE(TIME/(60*60*24)))");
	}
    }

    if (query.length() != 0) {
	query = String("SELECT FROM $1 WHERE ") + query;
	*ms_p = tableCommand(query, *ms_p).table();
    }
    selection_p = selection;
}

Vector<uInt> SDMSIterator::parseRow(const Record& selection, Int maxRow)
{
    // row is already known to exist as a field in rowValue
    // any error in parsing simply cause the returned vector having 0 elements
    Vector<uInt> result;
    IPosition shape(selection.shape("row"));
    // the value must be a matrix, otherwise it is ignored
    if (shape.nelements() == 2 || 
	(shape.nelements() == 1 && shape(0) % 2 == 0)) {
	// make sure its a double, else ignore it
	// treat even element vectors as doubles of shape [2,x]
	Matrix<Double> ranges;
	Matrix<Int> iranges;
	Int rmin, rmax;
	if (selection.dataType("row") == TpArrayDouble) {
	    if (shape.nelements() == 2) {
		selection.get("row",ranges);
	    } else {
		Vector<Double> vranges;
		selection.get("row", vranges);
		IPosition mshape(2);
		mshape(0) = 2;
		mshape(1) = vranges.nelements()/2;
		ranges = vranges.reform(mshape);
	    }
	    iranges.resize(3,ranges.ncolumn());
	    if (ranges.nrow() == 2) {
		// count the number of elements in result we need
		Int nrows = 0;
		uInt nvalidRanges = 0;
		Int len;
		for (uInt i=0;i<ranges.ncolumn();i++) {
		    if (ranges(0,i) <= 0) ranges(0,i) = 1.0;
		    if (ranges(1,i) > double(maxRow)) ranges(1,i) = double(maxRow);
		    rmin = Int(ranges(0,i));
		    rmax = Int(ranges(1,i));
		    if (rmax <= 0 || rmax < rmin) continue;
		    if (rmin > maxRow) continue;
		    len = rmax - rmin + 1;
		    iranges(0,nvalidRanges) = nrows;
		    iranges(1,nvalidRanges) = rmin;
		    iranges(2,nvalidRanges) = len;
 		    nrows += len;
		    nvalidRanges++;
		}
		// now fill it in
		result.resize(nrows);
		for (uInt i=0;i<nvalidRanges;i++) {
		    // copy constuctor makes a reference
		    Vector<uInt> sliceOfResult(result(Slice(iranges(0,i), 
							    iranges(2,i))));
		    indgen(sliceOfResult, uInt(iranges(1,i)));
		}
	    }
	}
    }
    // make result 0-relative
    result = result - uInt(1);
    return result;
}

Record SDMSIterator::getVectorShortCuts(const Record &recTemplate)
{
    Bool hasTime, hasScanNumber, hasSourceName, hasTelescopePosition,
	hasDate, hasUT, hasRestFreq;
    hasTime = hasScanNumber = hasSourceName = hasTelescopePosition = 
	hasDate = hasUT = hasRestFreq = False;
    Record result;
    if (recTemplate.fieldNumber("header")>=0) {
	hasTime = recTemplate.subRecord("header").fieldNumber("time") >= 0;
	hasScanNumber = recTemplate.subRecord("header").fieldNumber("scan_number") >= 0;
	hasSourceName = recTemplate.subRecord("header").fieldNumber("source_name") >= 0;
	hasTelescopePosition = recTemplate.subRecord("header").
	    fieldNumber("telescope_position") >= 0;
	hasDate = recTemplate.subRecord("header").fieldNumber("date") >= 0;
	hasUT = recTemplate.subRecord("header").fieldNumber("ut") >= 0;
    }
    if (hasTime || hasScanNumber || hasSourceName || hasTelescopePosition 
	|| hasDate || hasUT) {
	result.defineRecord("header", Record());
    }
    hasRestFreq = recTemplate.fieldNumber("data") >= 0 &&
	recTemplate.subRecord("data").fieldNumber("desc") >= 0 &&
	recTemplate.subRecord("data").subRecord("desc").fieldNumber("restfrequency") >= 0;
    if (hasRestFreq) {
	result.defineRecord("data", Record());
	result.rwSubRecord("data").defineRecord("desc", Record());
    }

    // and add in the short-circuits
    if (hasTime) {
	result.rwSubRecord("header").define("time", romsCols_p->time().getColumn());
    }
    if (hasDate || hasUT) {
	Vector<Double> times(romsCols_p->time().getColumn());
	Vector<String> date;
	Vector<Double> ut;
	const Double *timePtr;
	Double *utPtr;
	String *datePtr;
	Bool deleteItTimes, deleteItDates, deleteItUts;
	timePtr = times.getStorage(deleteItTimes);
	if (hasDate) {
	    date.resize(times.nelements());
	    datePtr = date.getStorage(deleteItDates);
	}
	if (hasUT) {
	    ut.resize(times.nelements());
	    utPtr = ut.getStorage(deleteItUts);
	}
	String fitsT("T");
	Unit timeUnit("s");
	Double secondsPerDay = 24.0*60.0*60.0;
	for (uInt i=0;i<times.nelements();i++) {
	    Quantity tq(timePtr[i], timeUnit);
	    MVTime thisTime(tq);
	    if (hasDate) {
		datePtr[i] = thisTime.string(MVTime::FITS).before(10);
	    }
	    if (hasUT) {
		Double day = thisTime.day();
		utPtr[i] = (day-Int(day))*secondsPerDay;
	    }
	}
	times.freeStorage(timePtr, deleteItTimes);
	if (hasDate) {
	    date.putStorage(datePtr, deleteItDates);
	    result.rwSubRecord("header").define("date",date);
	}
	if (hasUT) {
	    ut.putStorage(utPtr, deleteItUts);
	    result.rwSubRecord("header").define("ut",ut);
	}
    }
    if (hasScanNumber) {
	result.rwSubRecord("header").define("scan_number", romsCols_p->scanNumber().getColumn());
    }
    if (hasTelescopePosition) {
	Matrix<Double> telPos(3,nrecords());
	Vector<Double> emptyPos(3,0.0);
	for (uInt i=0;i<nrecords();i++) {
	    Int ant1Id = ids_p.antenna1(i);
	    if (ant1Id >= 0 && !(romsCols_p->antenna().flagRow())(ant1Id)) {
		telPos.column(i) = (romsCols_p->antenna().position())(ant1Id);
	    } else {
		telPos.column(i) = emptyPos;
	    }
	}
	result.rwSubRecord("header").define("telescope_position", telPos);
    }
    if (hasSourceName || hasRestFreq) {
	Vector<String> sourceNames(nrecords());
	Vector<Double> restFreqs;

	Vector<Double> times(romsCols_p->time().getColumn());
	Vector<Double> intervals(romsCols_p->interval().getColumn());

	String *sourcePtr;
	Double *restFreqPtr;
	const Double *timePtr;
	const Double *intervalPtr;
	Bool sourceDeleteIt, timeDeleteIt, intervalDeleteIt, restFreqDeleteIt;

	if (hasSourceName) {
	    sourceNames.resize(times.nelements());
	    sourcePtr = sourceNames.getStorage(sourceDeleteIt);
	}
	if (hasRestFreq) {
	    restFreqs.resize(times.nelements());
	    restFreqPtr = restFreqs.getStorage(restFreqDeleteIt);
	}

	timePtr = times.getStorage(timeDeleteIt);
	intervalPtr = intervals.getStorage(intervalDeleteIt);
	for (uInt i=0; i<times.nelements(); i++) {
	    Int fieldId = ids_p.fieldId(i);
	    Int sourceId = -1;
	    if (fieldId >= 0) sourceId = romsCols_p->field().sourceId()(fieldId);
	    Bool foundIt;
	    uInt sourceRow = 0;
	    Int spwId = ids_p.spectralWindowId(i);
	    if (sourceId >= 0 && spwId >= 0 && !sourceIndex_p.isNull()) {
		sourceIndex_p.sourceId() = sourceId;
		sourceIndex_p.spectralWindowId() = spwId;
		sourceIndex_p.time() = timePtr[i];
		sourceIndex_p.interval() = intervalPtr[i];
		sourceRow = sourceIndex_p.getNearestRow(foundIt);
		if (foundIt) {
		    if (hasSourceName)
			sourcePtr[i] = romsCols_p->source().name()(sourceRow);
		    if (hasRestFreq && romsCols_p->source().numLines()(sourceRow)>0 &&
			!romsCols_p->source().restFrequency().isNull()) {
			IPosition firstLine(1,0);
			restFreqPtr[i] = (romsCols_p->source().restFrequency()(sourceRow))(firstLine);
		    } else if (hasRestFreq) {
			restFreqPtr[i] = 0.0;
		    }
		} else {
		    if (hasSourceName) sourcePtr[i] = "";
		    if (hasRestFreq) restFreqPtr[i] = 0.0;
		}
	    }
	}
	if (hasSourceName) {
	    sourceNames.putStorage(sourcePtr, sourceDeleteIt);
	    result.rwSubRecord("header").define("source_name", sourceNames);
	}
	if (hasRestFreq) {
	    restFreqs.putStorage(restFreqPtr, restFreqDeleteIt);
	    result.rwSubRecord("data").rwSubRecord("desc").define("restfrequency", restFreqs);
	}
	times.freeStorage(timePtr, timeDeleteIt);
	intervals.freeStorage(intervalPtr, intervalDeleteIt);
    }
    return result;
}

void SDMSIterator::resync() {
    // the main table
    ms_p->resync();
    // of the standard sub-tables, watch out for null tables, which
    // should only happen for the optional tables
    ms_p->antenna().resync();
    ms_p->dataDescription().resync();
    if (!ms_p->doppler().isNull()) ms_p->doppler().resync();
    ms_p->feed().resync();
    ms_p->field().resync();
    ms_p->flagCmd().resync();
    if (!ms_p->freqOffset().isNull()) ms_p->freqOffset().resync();
    ms_p->history().resync();
    ms_p->observation().resync();
    ms_p->pointing().resync();
    ms_p->polarization().resync();
    ms_p->processor().resync();
    if (!ms_p->source().isNull()) ms_p->source().resync();
    ms_p->spectralWindow().resync();
    ms_p->state().resync();
    if (!ms_p->sysCal().isNull()) ms_p->sysCal().resync();
    if (!ms_p->weather().isNull()) ms_p->weather().resync();

    // and all of the ns_indexed tables
    for (uInt i=0; i<nsTables_p.nelements(); i++) {
	nsTables_p[i]->resync();
    }
}

void SDMSIterator::reselect()
{
    applySelection(selection_p);
    init();
}

String SDMSIterator::asStringVec(const Vector<Int> &vec)
{
    String result;
    if (vec.nelements() == 0) {
	// this is an impossible value for any integer index key, I think
	// use this to ensure no matches (an empty [] doesn't work)
	result = "[-9999]";
    } else if (!anyEQ(vec,-1)) {
	ostringstream os;
	os << "[";
	for (uInt i=0; i<vec.nelements();i++) {
	    if (i != 0) {
		os << ",";
	    }
	    os << vec(i);
	}
	os << "]";
	result = String(os);
    }
    return result;
}

const Array<String> SDMSIterator::getHist()
{
    Array<String> hist;
    rec_p->get("hist", hist);
    return hist;
}
String SDMSIterator::dataUnits(const MSMainEnums::PredefinedColumns whichCol)
{
    String result;
    result = "CNTS";
    String colName = MS::columnName(whichCol);
    ROTableColumn dataCol(*ms_p,colName);
    Int kw = dataCol.keywordSet().fieldNumber("QuantumUnits");
    if (kw >= 0) {
	result = dataCol.keywordSet().asString(kw);
    }
    return result;
}

} //# NAMESPACE CASA - END

