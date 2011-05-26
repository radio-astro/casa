//# GBTACSTable.cc: GBT backend table for the ACS.
//# Copyright (C) 2001,2002,2003
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

#include <nrao/FITS/GBTACSTable.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/LogiVector.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Slice.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/Constants.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableColumn.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Regex.h>

#include <iostream>
#include <fstream>


GBTACSTable::GBTACSTable(const String &fileName,
			 VanVleckCorr vanVleckCorr,
			 Smoothing smoothing,
			 Int vvsize,
			 Bool useDCBias,
			 Double dcbias,
			 Int minbiasfactor,
			 Bool fixlags,
			 String fixlagslog,
			 Double sigmaFactor,
			 Int spikeStart)
    : GBTBackendTable(fileName),
      itsVanVleckCorr(vanVleckCorr), itsSmoothing(smoothing),
      itsVVsize(vvsize), itsMinbiasfactor(minbiasfactor), 
      itsSpikeStart(spikeStart), itsUseDCBias(useDCBias),
      itsNormalTintAxes(True), itsNew(True), itsFixlags(fixlags),
      itsDCBias(dcbias), itsTruncError(0.5/65536.), 
      itsSigmaFactor(sigmaFactor), itsTimestamp(""), itsScan(-1),
      itsCachedDataRow(-1), itsCachedLagRow(-1), itsSpacing(0.0),
      itsCachedDataMatrix(0), itsCachedZeroVector(0),
      itsCachedBadDataVector(0),
      itsCachedLevel(-1), itsNlagWeight(0.0),
      itsHasBadData(False), itsErrorEmitted(False), 
      itsHasDiscontinuities(False), itsDiscErrorEmitted(False),
      isValid_p(False), itsBank("A"),
      itsFixLagsLog(fixlagslog)
{
    isValid_p = GBTBackendTable::isValid() && init();
}

GBTACSTable::~GBTACSTable()
{
    delete itsCachedDataMatrix;
    delete itsCachedZeroVector;
    delete itsCachedBadDataVector;
}

Bool GBTACSTable::reopen(const String &fileName, Bool resync)
{
    isValid_p = GBTBackendTable::reopen(fileName, resync);
    itsHasBadData = itsErrorEmitted = False;
    itsHasDiscontinuities = itsDiscErrorEmitted = False;
    if (isValid_p) isValid_p = init();
    return isValid_p;
}

Array<Float> GBTACSTable::rawdata(uInt whichSampler)
{
    if (itsCachedLagRow != Int(rownr())) {
	itsCachedLagRow = rownr();
	itsCachedLag = *itsDataField;
    }
    itsStart[itsSampAxis] = itsEnd[itsSampAxis] = whichSampler;
    return itsCachedLag(itsStart,itsEnd).nonDegenerate();
}

Array<Float> GBTACSTable::data(uInt whichSampler)
{
    // this all relies on the LAGS axis being the first one
    if (itsCachedDataRow != Int(rownr())) {
	itsCachedDataRow = Int(rownr());

	// read the data
	itsCachedData = *itsDataField;
	// set itsCachedDataMatrix reference to this
	uInt nlags = itsCachedData.shape()(0);
	uInt nspectra = itsCachedData.nelements()/nlags;
	delete itsCachedDataMatrix;
	itsCachedDataMatrix = 
	    new Matrix<Float>(itsCachedData.reform(IPosition(2,nlags,nspectra)));
	AlwaysAssert(itsCachedDataMatrix, AipsError);
	delete itsCachedZeroVector;
	itsCachedZeroVector =
	    new Vector<Float>(itsCachedZeroChannel.reform(IPosition(1,nspectra)));
	AlwaysAssert(itsCachedZeroVector, AipsError);

	// this may not be the complete picture, this may be
	// reset in vanVleck.  Delay printing any warning until later.
	itsHasBadData = checkForBadData(nspectra);
	itsHasDiscontinuities = anyEQ(itsCachedDiscontinuities, True);
	vanVleck();
	smooth();
	applyFFT();
	if (!itsErrorEmitted && itsHasBadData) {
	    LogIO os(LogOrigin("GBTACSTable","data"));
	    os << LogIO::SEVERE << WHERE
	       << "Some bad data (zero-lag <= 0 or > maximum expected value "
	       << "or sharp discontinuity) is present in this scan."
	       << " - that data will be flagged as bad as it is filled."
	       << LogIO::POST;
	    itsErrorEmitted = True;
	}
    }
    itsStart[itsSampAxis] = itsEnd[itsSampAxis] = whichSampler;
    return itsCachedData(itsStart, itsEnd).nonDegenerate();
}

Array<Float> GBTACSTable::zeroChannel(uInt whichSampler)
{
    // this ensures that the conversion has been done, it should
    // be painless if its already happened 
    // no need to keep the return from the data call
    data(whichSampler);
    IPosition start, end;
    start = itsStart;
    end = itsEnd;
    start[0] = end[0] = 0;
    start[itsSampAxis] = end[itsSampAxis] = whichSampler;
    return itsCachedZeroChannel(start, end).nonDegenerate();
}

Array<Bool> GBTACSTable::badData(uInt whichSampler)
{
    // this ensures that the conversion has been done, it should
    // be painless if its already happened 
    // no need to keep the return from the data call
    data(whichSampler);
    IPosition start, end;
    start = itsStart;
    end = itsEnd;
    start[0] = end[0] = 0;
    start[itsSampAxis] = end[itsSampAxis] = whichSampler;
    return itsCachedBadData(start, end).nonDegenerate();
}

Bool GBTACSTable::init()
{

    // should verify that the LAGS axis is the first one

    isValid_p = True;

    itsDataField.attachToRecord(currentRow(), "DATA");
    IPosition dataShape = (*itsDataField).shape();
    uInt nlags = dataShape(0);
    uInt nspectra = dataShape(1)*dataShape(2);
    IPosition dataShape2d(2,nlags,nspectra);
    itsCachedData.resize(dataShape);
    itsCachedLag.resize(dataShape);
    IPosition zeroChanShape = dataShape;
    zeroChanShape(0) = 1;
    itsCachedZeroChannel.resize(zeroChanShape);
    itsCachedBadData.resize(zeroChanShape);
    itsVVTemp.resize(dataShape2d);
    itsPowerLevels.resize(nspectra);
    itsFTTemp.resize(nlags*2);
    itsFFTResult.resize(0);
    // the size of zero here signifies that it has to be set before
    // it can be used.
    itsSmoother.resize(0);
    itsNlagWeight = 0.0;

    itsCachedDataRow = -1;
    itsCachedLagRow = -1;


    // assumes that the level is constant with sampler/port - need to be fixed
    // old version doesn't have a port table
    if (port().nrow()>0) {
	// early versions had Bank_A - perhaps this can be eliminated 
	// eventually
	ROTableColumn bankACol;
	if (sampler().tableDesc().isColumn("BANK_A")) {
	    bankACol.attach(sampler(),"BANK_A");
	} else {
	    bankACol.attach(sampler(),"Bank_A");
	}
	ROTableColumn bankBCol;
	if (sampler().tableDesc().isColumn("BANK_B")) {
	    bankBCol.attach(sampler(),"BANK_B");
	} else {
	    bankBCol.attach(sampler(),"Bank_B");
	}
	ROTableColumn portACol(sampler(),"PORT_A");
	ROTableColumn portBCol(sampler(),"PORT_B");
	itsBankA.resize(bankACol.nrow());
	itsPortA.resize(portACol.nrow());
	itsBankB.resize(bankBCol.nrow());
	itsPortB.resize(portBCol.nrow());
	for (uInt i=0;i<bankACol.nrow();i++) {
	    itsBankA[i] = bankACol.asString(i);
	    itsPortA[i] = portACol.asInt(i);
	    itsBankB[i] = bankBCol.asString(i);
	    itsPortB[i] = portBCol.asInt(i);
	}
	ROTableColumn bankCol(port(),"BANK");
	ROTableColumn portCol(port(),"PORT");
	String thisBank = itsBankA[0];
	Int thisPort = itsPortA[0];
	uInt portRow = 0;
	Bool found = False;
	while(!found && portRow < port().nrow()) {
	    if (bankCol.asString(portRow) == thisBank && 
		portCol.asInt(portRow) == thisPort) {
		found = True;
		ROTableColumn levelCol(port(),"LEVEL");
		itsCachedLevel = levelCol.asInt(portRow);
	    } else {
		portRow++;
	    }
	}
	if (!found) {
	    // something bad happened
	    isValid_p = False;
	    LogIO os(LogOrigin("GBTACSTable","init"));
	    os << LogIO::SEVERE << WHERE
	       << "Can not find a match in the PORT table, this data is unusable."
	       << LogIO::POST;
	}
	itsNew = True;
    } else {
	if (sampler().tableDesc().isColumn("LEVEL")) {
	    ROTableColumn levelCol(sampler(), "LEVEL");
	    itsCachedLevel = levelCol.asInt(0);
	} else {
	    // the port table is just plain empty - something really 
	    // bad happened.
	    isValid_p = False;
	    LogIO os(LogOrigin("GBTACSTable","init"));
	    os << LogIO::SEVERE << WHERE
	       << "Empty PORT table in backend FITS file, this data is unusable."
	       << LogIO::POST;
	}
	// this should never be important for old data, just ensure 
	// things don't break
	itsBankA.resize(sampler().nrow());
	itsBankA = "unknown";
	itsBankB.resize(sampler().nrow());
	itsBankB = "unknown";
	itsPortA.resize(sampler().nrow());
	indgen(itsPortA);
	itsPortB.resize(sampler().nrow());
	indgen(itsPortB);
	itsNew = False;
    }

    itsIntegrat.attachToRecord(currentRow(), "INTEGRAT");
    handleField("INTEGRAT");

    // either DMJD or TIME-MID but not both
    if (currentRow().fieldNumber("DMJD") >= 0) {
	// new style, expect DJMD
	itsDMJD.attachToRecord(currentRow(), "DMJD");
	handleField("DJMD");
    } else {
	itsTimeMid.attachToRecord(currentRow(), "TIME-MID");
	handleField("TIME-MID");
    }

    // work out the spacing
    if (keywords().fieldNumber("CDELT1") >= 0) {
	itsSpacing = 1.0 / (keywords().asDouble("CDELT1")*nlags);
    } else {
	itsSpacing = 1.0;
    }

    // mark things as handled
    handleKeyword("CRPIX1");
    handleKeyword("CDELT1");
    handleKeyword("CRVAL1");

    // find the sampler axis
    itsSampAxis = 1;  // valid for old data
    if (keywords().fieldNumber("TDESC3") >= 0) {
	String tdesc = keywords().asString("TDESC3");
	Vector<String> axisType = stringToVector(tdesc);
	for (uInt i=0; i<axisType.nelements(); i++) {
	    if (axisType[i] == "SAMPLER") {
		itsSampAxis = i;
		break;
	    }
	}
    }

    if (isValid_p) {
	// And set up things for the data slice
	itsEnd = (*itsDataField).endPosition();
	itsStart = IPosition(itsEnd.nelements(), 0);

	if (keywords().fieldNumber("BANK") >= 0) {
	    itsBank = keywords().asString("BANK");
	    handleKeyword("BANK");
	} else if (sampler().keywordSet().fieldNumber("BANK") >= 0) {
	    itsBank = sampler().keywordSet().asString("BANK");
	} else if (sampler().keywordSet().fieldNumber("bank") >= 0) {
	    itsBank = sampler().keywordSet().asString("bank");
	} else {
	    itsBank = "unknown";
	}

	if (itsNew) {
	    // TDESC2 describes INTEGRAT
	    String tdesc2 = keywords().asString("TDESC2");
	    Vector<String> tintAxisType = stringToVector(tdesc2);
	    uInt tintStateAxis, tintSampAxis;
	    tintStateAxis = tintSampAxis = 0;
	    for (uInt i=0; i<tintAxisType.nelements();i++) {
		String thisType = tintAxisType[i];
		if (thisType == "ACT_STATE") {
		    tintStateAxis = i;
		} else if (thisType == "SAMPLER") {
		    tintSampAxis = i;
		} else {
		    throw(AipsError("Unrecognized INTEGRAT axis in ACS FITS file"));
		}
		itsNormalTintAxes = tintStateAxis > tintSampAxis;
	    }
	    handleKeyword("TDESC2");
	} else {
	    itsNormalTintAxes = False;
	}

	String bname = Path(name()).baseName();
	itsTimestamp = bname.before(Regex("[A-D].fits"));

	Int n = keywords().fieldNumber("SCAN");
	if (n >= 0) {
	    itsScan = keywords().asInt(n);
	} else {
	    itsScan = -1;
	}
    }

    return isValid_p;
}

void GBTACSTable::vanVleck()
{
    if (itsVanVleckCorr == NoVanVleck) return;

    Int nlevels = itsCachedLevel;
    Double actualBias = 0.0;
    Double actualThresh = 0.0;
    Int actualBiasFactor = 1;

    // promote it to a double
    convertArray(itsVVTemp, *itsCachedDataMatrix);

    if (itsVanVleckCorr == Schwab) {

	// set the size, if it differs
	if (itsVVsize > 0 && uInt(itsVVsize) != VanVleck::getsize()) {
	    VanVleck::size(uInt(itsVVsize));
	}

	// 9-level sampling needs to be multiplied by 16 first
	if (nlevels == 9) itsVVTemp *= 16.0;
	// temporary to hold each lags vector in turn.
	Vector<Double> lagsTemp(itsVVTemp.nrow());
	Int scan = 0;
	if (keywords().fieldNumber("SCAN") >= 0) {
	    scan = keywords().asInt("SCAN");
	}
	Vector<Double> thresholds(128,0.0), biases(128,0.0);
	Vector<Bool> badZeroLags(128,False);
	SimpleOrderedMap<String, Int> bankPortMap(0,2);
	Int counter=-1;
	// first pass, just do the autocorrelations and
	// remember threshold and bias values for each
	for (uInt i=0;i<itsVVTemp.ncolumn();i++) {
	    Int thisSamp = i % nsamp();

	    if (itsPortA[thisSamp] == itsPortB[thisSamp] && 
		itsBankA[thisSamp] == itsBankB[thisSamp]) {
		counter++;
		if (counter > Int(thresholds.nelements())) {
		    // double size
		    Int newsize = thresholds.nelements() * 2;
		    thresholds.resize(newsize,True);
		    biases.resize(newsize,True);
		    badZeroLags.resize(newsize,True);
		}
		String key = 
		    bankPortPhase(itsBankA[thisSamp],
				  itsPortA[thisSamp],(i/nsamp())%nstate());
		bankPortMap(key) = counter;
		if (itsCachedBadZeroLags[i]) {
		    // no correction, return to its original values
		    if (nlevels==9) {
			itsVVTemp.column(i) = itsVVTemp.column(i)/16.0;
		    }
		    badZeroLags[counter] = True;
		} else {
		    // things which are marked as bad data but not bad zero 
		    // lags are converted here, just in case the indication 
		    // that it is bad was wrong - the user can choose to 
		    // unflag it later and not loose anything.  Bad zero lags,
		    // though, can't be dealt with because there is no way to 
		    // get the van Vleck curve from it.
		    lagsTemp = itsVVTemp.column(i);
		    getACInfo(actualThresh, actualBias, actualBiasFactor, 
			      nlevels, lagsTemp);
		    vanVleckLags(lagsTemp, nlevels, actualThresh, actualThresh,
				 actualBias, actualBias);
		    itsVVTemp.column(i) = lagsTemp;
		    thresholds[counter] = actualThresh;
		    biases[counter] = actualBias;
		}
	    }
	}
	// second pass, cc's only
	for (uInt i=0;i<itsVVTemp.ncolumn();i++) {
	    Int thisSamp = i % nsamp();
	    if (itsPortA[thisSamp] != itsPortB[thisSamp] || 
		itsBankA[thisSamp] != itsBankB[thisSamp]) {
		Int thisPhase = i % nstate();
		String keyA = bankPortPhase(itsBankA[thisSamp],
					    itsPortA[thisSamp],thisPhase);
		String keyB = bankPortPhase(itsBankB[thisSamp],
					    itsPortB[thisSamp],thisPhase);
		Int indxA = bankPortMap(keyA);
		Int indxB = bankPortMap(keyB);
		if (itsHasBadData && 
		    (badZeroLags[indxA] || badZeroLags[indxB])) {
		    // no correction, return to its original values
		    if (nlevels==9) {
			itsVVTemp.column(i) = itsVVTemp.column(i)/16.0;
		    }
		} else {
		    // things which are marked as bad data but not bad zero 
		    // lags are converted here, just in case the indication 
		    // that its bad was wrong - the user can choose to unflag 
		    // it later and not loose anything.  Bad zero lags, 
		    // though, can't be dealt with because there's no way to 
		    // get the van Vleck curve from it.
		    lagsTemp = itsVVTemp.column(i);
		    vanVleckLags(lagsTemp, nlevels, thresholds[indxA], 
				 thresholds[indxB],biases[indxA], 
				 biases[indxB]);
		    itsVVTemp.column(i) = lagsTemp;
		}
	    }
	}
    } else {
	// do the powerlevel correction
	switch (itsCachedLevel) {
	case 3:
	    pow3lev();
	    vanvleck3lev();
	    break;
	case 9:
	    pow9lev();
	    vanvleck9lev();
	    break;
	default:
	    // emit some error?
	    // for now, do nothing
	    break;
	}
	// demote it to a float again
	// actually apply the power level array
	for (uInt i=0;i<itsVVTemp.ncolumn();i++) {
	    // only do this if the zero-lag is okay
	    if (!itsHasBadData || ! itsCachedBadZeroLags[i]) {
		itsVVTemp.column(i) = itsVVTemp.column(i) * itsPowerLevels(i);
	    }
	}
    }

    // Demote it to a float again
    convertArray(*itsCachedDataMatrix, itsVVTemp);
}

void GBTACSTable::getACInfo(Double &threshold, Double &dcbias, 
			    Int &biasfactor, Int nlevel,
			    Vector<Double> &aclags)
{
    // deduce the threshold
    dcbias = itsDCBias;
    threshold = 0.0;
    biasfactor = 0;
    if (itsUseDCBias || (dcbias == 0.0 && itsMinbiasfactor >= 0)) {
	// get the mean from the last 5% of the lags
	Int ntest = Int(aclags.nelements()*0.05);
	if (ntest < 1) ntest = 1;
	Int nstart = aclags.nelements()-ntest;
	dcbias = mean(aclags(Slice(nstart,ntest)));
	if (itsMinbiasfactor >= 0) {
	    if (dcbias < 0.0) {
		dcbias = abs(dcbias);
		biasfactor = Int(dcbias / itsTruncError);
		if (Double(biasfactor)*itsTruncError < dcbias) biasfactor++;
		biasfactor = max(itsMinbiasfactor, biasfactor);
	    } else {
		// always add in at least itsMinbiasfactor
		biasfactor = itsMinbiasfactor;
	    }
	    Double offset = Double(biasfactor) * itsTruncError;
	    aclags += offset;
	    dcbias = mean(aclags(Slice(nstart, ntest)));
	}
	if (!VanVleck::dcoff(dcbias, threshold, nlevel,
			     aclags[0], dcbias)) {
	    LogIO os(LogOrigin("GBTACSTable","data"));
	    os << LogIO::SEVERE << WHERE
	       << "The zerolag and bias at large lags are incompatible - "
	       << "default to 0.0 bias in vanVleck correction"
	       << LogIO::POST;
	}
    } else {
	threshold = VanVleck::thresh(nlevel, aclags[0]);
    }
}

void GBTACSTable::vanVleckLags(Vector<Double> &lags, Int nlevel, 
			       Double thresh1, Double thresh2, 
			       Double dcbias1, Double dcbias2)
{
    VanVleck::setEquiSpaced(thresh1, thresh2, dcbias1, dcbias2, nlevel);
	
    // set the result
    for (uInt i=0;i<lags.nelements();i++) lags[i] = VanVleck::r(lags[i]);

    // scale by square of ratio of lev to the optimum threshold for
    // this level
    Double scale = 1.0;
    if (nlevel == 3) {
	scale = 0.612003181/sqrt(thresh1*thresh2);
	scale = scale*scale;
    } else {
	// this assumes 9-level - is that a safe assumption?
	scale = 0.266911104/sqrt(thresh1*thresh2);
	scale = scale*scale;
    }
    lags *= scale;
}

void GBTACSTable::pow3lev()
{
    // if all of the zero lags are bad, just set itsPowerLevels to 1 and 
    // be done
    if (itsHasBadData && allEQ(itsCachedBadData,True)) {
	itsPowerLevels = 1.0;
    } else {
	// otherwise, just do it and then set the bad ones to 1.0 at the end
	// this is from Glen's vanvleck.c file ...
	// power level is 0.3745443672/(2*inverse_cerf(zero-lag)^2)
	// data is then scaled by that value
	// this converts the zeroLags into the inverse erfc for those values
	// the factor of 1.053 is an empiraclly determined fudge factor 
	// reported by Rick Fisher in EDIR No. 311.
	Vector<Double> erfData = (1.0 - itsVVTemp.row(0))/1.053;
	itsPowerLevels = (square(erfData) - 0.5625);
	Vector<Double> powLevsSq = square(itsPowerLevels);
	itsPowerLevels = erfData*(1.591863138+
				  (itsPowerLevels*(-2.442326820))+
				  (powLevsSq*0.37153461)) /
	    (1.467751692+(itsPowerLevels*(-3.013136362)) + (powLevsSq));
	// at this point, itsPowerLevels is the inverse complementary error 
	// function for the zero channel values for all states and samplers 
	// in this row.  Convert that to the power levels.
	itsPowerLevels = 0.3745443672 / (itsPowerLevels*itsPowerLevels*2.0);
	if (itsHasBadData) {
	    for (uInt i=0;i<itsCachedBadZeroLags.nelements();i++) {
		if (itsCachedBadZeroLags[i]) itsPowerLevels(i) = 1.0;
	    }
	}
    }
}

void GBTACSTable::pow9lev()
{
    // if all of the zero lags are bad, just set itsPowerLevels to 1
    if (itsHasBadData && allEQ(itsCachedBadData,True)) {
	itsPowerLevels = 1.0;
    } else {
	// otherwise, just do it and then set the bad ones to 1.0 at the end
    // From Rick
	Vector<Double> zeroLags(itsVVTemp.row(0));
	Vector<Double> zl = 10.0 * log10(zeroLags * 16.0 / 3.401);
	
	Double coef0 =  0.00907207669;
	Double coef1 = -0.204293646;
	Double coef2 = -0.0587269653;
	Double coef3 = -0.0116695172;
	Double coef4 = -0.00104146831;
	Double coef5 = -3.85638072e-05;
	
	itsPowerLevels = 
	    ((((coef5 * zl + coef4) * zl + coef3) * zl + coef2) * zl + coef1) *
	    zl + coef0;
	
	itsPowerLevels = pow(10.0,(-0.1 * itsPowerLevels));
	if (itsHasBadData) {
	    for (uInt i=0;i<itsCachedBadZeroLags.nelements();i++) {
		if (itsCachedBadZeroLags[i]) itsPowerLevels(i) = 1.0;
	    }
	}
    }
}

void GBTACSTable::vanvleck3lev()
{
    // adapted from Glen Langstong's copy of vanvleck.c

    //  Van Vleck Correction for 3-level sampling/correlation
    //   Samples {-1,0,1}
    //  Uses Zerolag to adjust correction
    //    data_array -> Points into autocorrelation function of at least 
    //                  'count' points

    // these should be cached internally by the ctor.

    Double loC00, loC01, loC02, loC03;
    Double loC10, loC11, loC12, loC13;
    Double loC20, loC21, loC22, loC23;

    Double hiC00, hiC01, hiC02, hiC03;
    Double hiC10, hiC11, hiC12, hiC13;
    Double hiC20, hiC21, hiC22, hiC23;
    Double hiC30, hiC31, hiC32, hiC33;
    Double hiC40, hiC41, hiC42, hiC43;

    loC00 =  0.939134371719;
    loC01 = -0.567722496249;
    loC02 =  1.02542540932;
    loC03 =  0.130740914912;
    loC10 = -0.369374472755;
    loC11 = -0.430065136734;
    loC12 = -0.06309459132;
    loC13 = -0.00253019992917;
    loC20 =  0.888607422108;
    loC21 = -0.230608118885;
    loC22 = 0.0586846424223;
    loC23 = 0.002012775510695;

    hiC00 = -1.83332160595;
    hiC01 =  0.719551585882;
    hiC02 =  1.214003774444;
    hiC03 =  7.15276068378e-5;
    hiC10 =  1.28629698818;
    hiC11 = -1.45854382672;
    hiC12 = -0.239102591283;
    hiC13 = -0.00555197725185;
    hiC20 = -7.93388279993;
    hiC21 =  1.91497870485;
    hiC22 =  0.351469403030;
    hiC23 =  0.00224706453982;
    hiC30 =  8.04241371651;
    hiC31 = -1.51590759772;
    hiC32 = -0.18532022393;
    hiC33 = -0.00342644824947;
    hiC40 =-13.076435520;
    hiC41 =  0.769752851477;
    hiC42 =  0.396594438775;
    hiC43 =  0.0164354218208;

    // do this for each spectra
    uInt nchan = itsVVTemp.nrow();
    uInt nspec = itsVVTemp.ncolumn();
    uInt nrest = nchan-1;
    for (uInt i=0;i<nspec;i++) {
	// skip bad zero lags
	if (itsHasBadData && itsCachedBadZeroLags[i]) continue;

	Double zho = itsVVTemp(0,i); // zero lag
	Double zho_3 = zho*zho*zho;   // zero lag cubed
	Vector<Double> restOfSpectra(itsVVTemp.column(i)(Slice(1,nrest)));
	LogicalVector hilogic(restOfSpectra > 0.199);
	if (anyEQ(hilogic, True)) {
	    MaskedArray<Double> himask = restOfSpectra(hilogic);
	    Double high_u0 = zho_3*zho_3*zho; // zero lag ^ 7
	    Double high_u1 = zho-(63.0/128.0);
	    Double high_u2 = zho*zho-(31.0/128.0);
	    Double high_u3 = zho_3-(61.0/512.0); 
	    Double high_u4 = zho-(63.0/128.0); 
	
	    Double high_h0 = zho_3*zho_3*zho*zho; // zero lag ^ 8
	    Double high_h1 = high_h0; //  zlag ^ 8
	    Double high_h2 = high_h0*zho_3*zho; // zero lag ^ 12 
	    Double high_h3 = high_h0*high_h0*zho;	// zlag ^ 17 
	    Double high_h4 = high_h3;	// zero lag ^ 17 
	    Double high_coefficient0 =
		(high_u0*(high_u0*(high_u0*hiC00 + hiC01) + hiC02)+hiC03)/
		high_h0;
	    Double high_coefficient1 =
		(high_u1*(high_u1*(high_u1*hiC10 + hiC11) + hiC12)+hiC13)/
		high_h1;
	    Double high_coefficient2 =
		(high_u2*(high_u2*(high_u2*hiC20 + hiC21) + hiC22)+hiC23)/
		high_h2;
	    Double high_coefficient3 =
		(high_u3*(high_u3*(high_u3*hiC30 + hiC31) + hiC32)+hiC33)/
		high_h3;
	    Double high_coefficient4 =
		(high_u4*(high_u4*(high_u4*hiC40 + hiC41) + hiC42)+hiC43)/
		high_h4;
	    Vector<Double> tempData(himask.getCompressedArray());
	    Vector<Double> tempDataCubed(tempData*tempData*tempData);
	    tempData = 
	      (tempData*(tempDataCubed*(tempDataCubed*
					(tempDataCubed*(tempDataCubed*
							high_coefficient3 +
							high_coefficient4) + 
					 high_coefficient2) + 
					high_coefficient1) + 
			 high_coefficient0));
	    himask.setCompressedArray(tempData);
	}
	if (anyEQ(hilogic, False)) {
	    MaskedArray<Double> lomask = restOfSpectra(!hilogic);
	    Double lo_u0 = zho;
	    Double lo_u1 = zho_3-(61.0/512.0);
	    Double lo_u2 = zho-(63.0/128.0);
	
	    Double lo_h0 = zho*zho;
	    Double lo_h1 = zho_3*zho_3*zho;         // zero lag ^ 7
	    Double lo_h2 = zho*lo_h1;               // zero lag ^ 8

	    Double lo_coefficient0 =
		(lo_u0*(lo_u0*(lo_u0*loC00+loC01)+loC02)+loC03)/lo_h0;
	    Double lo_coefficient1 =
		(lo_u1*(lo_u1*(lo_u1*loC10+loC11)+loC12)+loC13)/lo_h1;
	    Double lo_coefficient2 =
		(lo_u2*(lo_u2*(lo_u2*loC20+loC21)+loC22)+loC23)/lo_h2;

	    Vector<Double> tempData(lomask.getCompressedArray());
	    Vector<Double> tempDataSq = square(tempData);
	    tempData = 
		(tempData*(tempDataSq*(tempDataSq*lo_coefficient2+
				       lo_coefficient1)+lo_coefficient0));
	    lomask.setCompressedArray(tempData);
	}
	itsVVTemp(0,i) = 1.0;
    }
}

void GBTACSTable::vanvleck9lev()
{
    Double coef10 =  1.105842267;
    Double coef11 = -0.053258115;
    Double coef12 =  0.011830276;
    Double coef13 = -0.000916417;
    Double coef14 =  0.000033479;

    Double coef2rg4p50 =  0.111705575;
    Double coef2rg4p51 = -0.066425925;
    Double coef2rg4p52 =  0.014844439;
    Double coef2rg4p53 = -0.001369796;
    Double coef2rg4p54 =  0.000044119;

    Double coef2rl2p10 =  1.285303775;
    Double coef2rl2p11 = -1.472216011;
    Double coef2rl2p12 =  0.640885537;
    Double coef2rl2p13 = -0.123486209;
    Double coef2rl2p14 =  0.008817175;

    Double coef2rother0 =  0.519701391;
    Double coef2rother1 = -0.451046837;
    Double coef2rother2 =  0.149153116;
    Double coef2rother3 = -0.021957940;
    Double coef2rother4 =  0.001212970;
    
    Double coef3rg2p00 =  1.244495105;
    Double coef3rg2p01 = -0.274900651;
    Double coef3rg2p02 =  0.022660239;
    Double coef3rg2p03 = -0.000760938;
    Double coef3rg2p04 = -1.993790548;
    
    Double coef3rother0 =  1.249032787;
    Double coef3rother1 =  0.101951346;
    Double coef3rother2 = -0.126743165;
    Double coef3rother3 =  0.015221707;
    Double coef3rother4 = -2.625961708;
    
    Double coef4rg3p150 =  0.664003237;
    Double coef4rg3p151 = -0.403651682;
    Double coef4rg3p152 =  0.093057131;
    Double coef4rg3p153 = -0.008831547;
    Double coef4rg3p154 =  0.000291295;

    Double coef4rother0 =   9.866677289;
    Double coef4rother1 = -12.858153787;
    Double coef4rother2 =   6.556692205;
    Double coef4rother3 =  -1.519871179;
    Double coef4rother4 =   0.133591758;
    
    Double coef5rg4p00 =  0.033076469;
    Double coef5rg4p01 = -0.020621902;
    Double coef5rg4p02 =  0.001428681;
    Double coef5rg4p03 =  0.000033733;

    Double coef5rg2p20 =  -5.284269565;
    Double coef5rg2p21 =  6.571535249;
    Double coef5rg2p22 = -2.897741312;
    Double coef5rg2p23 =  0.443156543;

    Double coef5rother0 = -1.475903733;
    Double coef5rother1 =  1.158114934;
    Double coef5rother2 = -0.311659264;
    Double coef5rother3 =  0.028185170;

    // do this for each spectra
    uInt nchan = itsVVTemp.nrow();
    uInt nspec = itsVVTemp.ncolumn();
    uInt nrest = nchan-1;
    for (uInt i=0;i<nspec;i++) {
	// skip bad zero lags
	if (itsHasBadData && itsCachedBadZeroLags[i]) continue;

	Double zho = itsVVTemp(0,i); // zero lag
	Vector<Double> restOfSpectra(itsVVTemp.column(i)(Slice(1,nrest)));
	Double zl = zho*16;
	Double acoef0, acoef1, acoef2, acoef3, acoef4;
  
	acoef0 = ((((coef14*zl + coef13)*zl + coef12)*zl +coef11)*zl +coef10);

	if( zl > 4.50)
	    acoef1 =((((coef2rg4p54*zl + coef2rg4p53)*zl + coef2rg4p52)*zl + 
		      coef2rg4p51)*zl + coef2rg4p50);
	else if(zl < 2.10)  
	    acoef1 =((((coef2rl2p14*zl + coef2rl2p13)*zl + coef2rl2p12)*zl + 
		      coef2rl2p11)*zl + coef2rl2p10);
	else
	    acoef1 =((((coef2rother4*zl + coef2rother3)*zl + coef2rother2)*zl +
		      coef2rother1)*zl + coef2rother0);

	if( zl > 2.00)
	    acoef2 = coef3rg2p04/zl + (((coef3rg2p03*zl + coef3rg2p02)*zl + 
					coef3rg2p01)*zl + coef3rg2p00);
	else
	    acoef2 = coef3rother4/zl + (((coef3rother3*zl + coef3rother2)*zl +
					 coef3rother1)*zl + coef3rother0);  
  
	if( zl > 3.15)
	    acoef3 = ((((coef4rg3p154*zl + coef4rg3p153)*zl + coef4rg3p152)*zl
		       + coef4rg3p151)*zl + coef4rg3p150);
	else
	    acoef3 = ((((coef4rother4*zl + coef4rother3)*zl + coef4rother2)*zl
		       + coef4rother1)*zl + coef4rother0);  

	if( zl > 4.00)
	    acoef4 =(((coef5rg4p03*zl + coef5rg4p02)*zl + coef5rg4p01)*zl + 
		     coef5rg4p00);
	else if(zl < 2.2)  
	    acoef4 =(((coef5rg2p23*zl + coef5rg2p22)*zl + coef5rg2p21)*zl + 
		     coef5rg2p20);
	else
	    acoef4 =(((coef5rother3*zl + coef5rother2)*zl + coef5rother1)*zl +
		     coef5rother0);
  
	// normalize by the 0-channel value
	restOfSpectra /= zho;
	restOfSpectra = 
	    ((((acoef4*restOfSpectra + acoef3)*restOfSpectra + acoef2)*
	      restOfSpectra + 
	      acoef1)*restOfSpectra + acoef0)*restOfSpectra;
	itsVVTemp(0,i) = 1.0;
	// multiply everything by the 0-channel value to restore the 
	// original normalization
	itsVVTemp.column(i) = itsVVTemp.column(i) * zho;
    }
}

void GBTACSTable::applyFFT()
{
    uInt nlags = itsCachedDataMatrix->nrow();
    for (uInt i=0;i<itsCachedDataMatrix->ncolumn();i++) {
	// skip this if this lag-zero is bad
	if (itsHasBadData && itsCachedBadZeroLags[i]) continue;

	// fill itsFTTemp - place the origin at the pixel 0
	// put the (possibly) smoothed data in the first half
	Vector<Float> itsFirstHalf(itsFTTemp(Slice(0,nlags,1)));
	itsFirstHalf = itsCachedDataMatrix->column(i);

	// duplicate and reverse this 
	// I can't see any easy way to do this.  Slice requires positive 
	// increments.
	Float *ftdata, *inptr, *outptr;
	Bool deleteIt;
	ftdata = itsFTTemp.getStorage(deleteIt);
	// inptr comes from what we already set, starting at pixel 1
	inptr = ftdata + 1;
	// outptr is what we are going to set, starting at 1 pixel from the end
	outptr = ftdata + itsFTTemp.nelements() - 1;
	for (uInt j=1;j<nlags;j++) {
	    *outptr = *inptr;
	    outptr--;
	    inptr++;
	}
	itsFTTemp.putStorage(ftdata, deleteIt);
	// and set the nlags point

	// The first term is non-zero only for Hamming smoothing
	// the second term is non-zero because there may be a DC
	// bias in the sampler
	itsFTTemp[nlags] = itsNlagWeight * itsFTTemp[nlags-2] +
	    itsFTTemp[nlags-1];
	
	// do the transform
	itsFFTServer.fft0(itsFFTResult, itsFTTemp);
	// only half (+1) of the result is returned, which is what we want
	// The first element is the zero (DC) channel
	(*itsCachedZeroVector)[i] = fabs(itsFFTResult[0]);
	itsCachedDataMatrix->column(i) = 
	    Vector<Float>(real(itsFFTResult))(Slice(1,nlags,1));
    }
}

void GBTACSTable::smooth()
{
    Float a = 0.5;
    Float b = 0.5;
    switch (itsSmoothing) {
    case Hamming:
	a = 0.46;
	b = 0.54;
    case Hanning:
	if (itsSmoother.nelements() != (itsCachedDataMatrix->nrow())) {
	    Int nlags = itsCachedDataMatrix->nrow();
	    itsSmoother.resize(nlags);
	    indgen(itsSmoother);
	    itsSmoother *= Float(C::pi/nlags);
	    itsSmoother = cos(itsSmoother);
	    itsSmoother *= a;
	    itsSmoother += b;
	}
	    
	for (uInt i=0;i<itsCachedDataMatrix->ncolumn();i++) {
	    // skip columns with bad data
	    if (itsHasBadData && itsCachedBadZeroLags[i]) continue;
	    itsCachedDataMatrix->column(i) = 
		itsCachedDataMatrix->column(i) * itsSmoother;
	}
	// this is used later before FFTing to set the value of 
	// itsFTTemp(nlags) after things have been duplicated and 
	// reversed around nlags prior to FFTing
	itsNlagWeight = b-a;
	break;
    default:
	// do nothing
	itsNlagWeight = 0.0;
    }
}

Float GBTACSTable::integrat(uInt whichSampler, uInt whichState) const
{
    Float result = 1.0;
    if (itsNormalTintAxes) {
	result = (*itsIntegrat)(IPosition(2,whichSampler,whichState));
    } else {
	if (itsNew) {
	    result = (*itsIntegrat)(IPosition(2,whichState,whichSampler));
	} else {
	    result = (*itsIntegrat)(IPosition(3,0,whichSampler,whichState));
	}
    }
    // assume units of counts if >= 1e6, else seconds
    if (result >= 1e6) result /= 1e8;
    // watch for truly wonky INTEGRAT values
    if (result <= 1e-6 || result >= 1e6) result = 0.0;
    return result;
}

Bool GBTACSTable::checkForBadData(Int nspectra)
{
    delete itsCachedBadDataVector;
    itsCachedBadDataVector = 
	new Vector<Bool>(itsCachedBadData.reform(IPosition(1,nspectra)));
    AlwaysAssert(itsCachedBadDataVector, AipsError);

    itsCachedBadZeroLags.resize(itsCachedBadDataVector->nelements());
    itsCachedDiscontinuities.resize(itsCachedBadDataVector->nelements());
    itsCachedDiscontinuities = False;

    // first, check for bad zero lags
    // the GBT ACS always has an odd number of levels
    // The check on the upper limit of the zero-lag is only valid for that 
    // case.
    Float maxZeroLag = itsCachedLevel-1;
    maxZeroLag *= maxZeroLag;
    // check here is done before any multiplication by 16.0
    if (itsCachedLevel == 9) maxZeroLag /= 16.0;
    
    itsCachedBadZeroLags = itsCachedDataMatrix->row(0) <= Float(0.0) ||
	itsCachedDataMatrix->row(0) > maxZeroLag;

    // But if there are any x-corr terms, the zero-lag there is 
    // unimportant here
    for (Int i=0;i<nspectra;i++) {
	if (itsCachedBadZeroLags[i]) {
	    Int thisSamp = i % nsamp();
	    if (itsPortA[thisSamp] != itsPortB[thisSamp] || 
		itsBankA[thisSamp] != itsBankB[thisSamp]) {
		itsCachedBadZeroLags[i] = False;
	    }
	}
    }

    (*itsCachedBadDataVector) = itsCachedBadZeroLags;

    // second, check for discontinuities
    Bool badData = False;
    for (uInt spec=0;spec<itsCachedDataMatrix->ncolumn();spec++) {
	if (!(*itsCachedBadDataVector)[spec]) {
	    Int thisSamp = spec % nsamp();
	    Bool isXCorr = (itsPortA[thisSamp] != itsPortB[thisSamp]) || 
		(itsBankA[thisSamp] != itsBankB[thisSamp]);
	    itsCachedDiscontinuities[spec] = 
		checkForDiscontinuities(itsCachedDataMatrix->column(spec), 
					badData, spec, isXCorr);
	    (*itsCachedBadDataVector)[spec] = badData;
	} 
    }
    return anyEQ(*itsCachedBadDataVector, True);
}


Bool GBTACSTable::checkForDiscontinuities(Vector<Float> lags, Bool &badData, uInt spec, Bool isXcorr)
{
    // The goal is to do this with as few uses of the lags[i] method 
    // as possible for speed reasons.

    // result will be True whenver bad data is found, even if fixed
    Bool result = False;
    // badData will be True when a bad 1024-lag segement is found and
    // not fixed (remains bad on return)
    badData = False;

    if (!itsFixlags) {
	// never fix the lags, just find the bad ones
	// this is simpler because the check for single discontinuities
	// doesn't enter the picture.  Simplest to keep this code separate.

	Int nToCheck = Int(lags.nelements())/1024 - 1;
	if (nToCheck <= 0) return result; // too small to be bad

	for (Int i=0;i<nToCheck;i++) {
	    Int leftStart, testStart, rightStart;
	    Double goodSum, goodSumSq, testSum, nGood;
	    goodSum = goodSumSq = testSum = 0.0;
	    nGood = 512;
	    testStart = 1024*i + 1024;
	    leftStart = testStart - 512;
	    rightStart = testStart + 1024;
	    for (Int j=leftStart;j<testStart;j++) {
		Double thisLag = lags[j];
		goodSum += thisLag;
		goodSumSq += thisLag*thisLag;
	    }
	    // just test out the next 10 to see if it's worth doing all 1024
	    for (Int j=testStart;j<(testStart+10);j++) testSum += lags[j];
	    if (abs(testSum/10.0 - goodSum/nGood) > 
		itsSigmaFactor * sqrt((goodSumSq - goodSum*goodSum/nGood)/(nGood-1.0))) {
		// appears to be bad, time to make sure
		for (Int j=(testStart+10);j<rightStart;j++) testSum += lags[j];
		if (rightStart < Int(lags.nelements())) {
		    // make sure first 10 elements are not bad
		    Double rightSum, rightSumSq;
		    rightSum = rightSumSq = 0.0;
		    for (Int j=rightStart;j<(rightStart+10);j++) {
			Double thisLag = lags[j];
			rightSum += thisLag;
			rightSumSq += thisLag*thisLag;
		    }
		    if ((rightSum/10.0 - goodSum/nGood) > 
			itsSigmaFactor * sqrt((goodSumSq - goodSum*goodSum/nGood)/
					      (nGood-1.0))) {
			// no, that one is bad too, give up on this row
			badData = True;
			result = True;
			// no point in continuing
			break;
		    } else {
			goodSum += rightSum;
			goodSumSq += rightSumSq;
			for (Int j=(rightStart+10);j<(rightStart+512);j++) {
			    Double thisLag = lags[j];
			    goodSum += thisLag;
			    goodSumSq += thisLag*thisLag;
			}
			nGood += 512;
		    }
		}
		if (!badData) {
		    // final check
		    if (abs(testSum/1024.0 - goodSum/nGood) > 
			itsSigmaFactor * sqrt((goodSumSq - goodSum*goodSum/nGood)/
					      (nGood-1.0))) {
			// yup, it's bad
			badData = True;
			result = True;
			// no point in continuing
			break;
		    }
		}
	    }	    
	}	    
    } else {
	// Set up the spike checking values
	Double spikeSum, spikeSumSq, meanSpike, stddevSpike, nSpike;
	uInt testLoc, lastSpike;
	spikeSum = spikeSumSq = nSpike = 0.0;
	// start testing at itsSpikeStart
	testLoc = itsSpikeStart;
	lastSpike = 0;

	ostringstream badChanStream;
	uInt badCount = 0;

	Int spikeGroupStart = -1;

	// The 1024-lag discontinuity values
	Double goodSum, goodSumSq, testSum, testSumSq, nGood;
	goodSum = goodSumSq = testSum = testSumSq = nGood = 0.0;
	Double thisLag, testLag;

	if (isXcorr && lags.nelements() >= 1535) {
	    // need to check and possibly adjust the first 1024 lags
	    for (uInt i=512;i<1024;i++) {
		thisLag = lags[i];
		testSum += thisLag;
		testSumSq += thisLag*thisLag;
	    }
	    for (uInt i=1024;i<1536;i++) {
		thisLag = lags[i];
		goodSum += thisLag;
		goodSumSq += thisLag*thisLag;
	    }
	    Double testMean = testSum/512.0;
	    Double goodMean = goodSum/512.0;
	    Double testSigma = sqrt((testSumSq - testSum*testSum/512.0)/511.0);
	    Double goodSigma = sqrt((goodSumSq - goodSum*goodSum/512.0)/511.0);

	    if ((abs(testMean-goodMean) > itsSigmaFactor*goodSigma) && (abs(testMean) > abs(goodMean))) {
		// first 1024 lags appear to be bad - adjust
		Double scale = goodSigma/testSigma;
		for (uInt i=0;i<1024;i++) {
		    lags[i] = (lags[i]-testMean)*scale + goodMean;
		}
		result = True;
		// nothing else has been bad so far
		badChanStream << "0:1023";
		badCount++;
	    }

	    // reset the sums and count
	    goodSum = goodSumSq = testSum = testSumSq = nGood = 0.0;
	}
	// These are the next starting points for this test
	// These are updated once a test has been used
	uInt leftStart, rightStart, testStart;
	leftStart = 512;
	testStart = 1024;
	rightStart = 2048;

	thisLag = testLag = 0.0;

	Bool allBad = False;

	for (uInt i=(testLoc-100);i<lags.nelements();i++) {

	    thisLag = lags[i];
 
	    // if >= leftStart and < testStart add in to good*
	    if (i>=leftStart && i < testStart) {
		goodSum += thisLag;
		goodSumSq += thisLag*thisLag;
		nGood++;
	    } else if (i == testStart) {
		// when a bad region is encountered, we need to fix it
		// first before going on with the spike check, so, some
		// values are read an extra time, but we try and limit
		// that since it does slow down the process if not necessary

		// just test out the next 10 to see if it's worth doing 1024
		for (uInt j=testStart;j<(testStart+10);j++) {
		    Double testLag = lags[j];
		    testSum += testLag;
		    testSumSq += testLag*testLag;
		}
		if (abs(testSum/10.0 - goodSum/nGood) > 
		    itsSigmaFactor * sqrt((goodSumSq - goodSum*goodSum/nGood)/
					  (nGood-1.0))) {
		    // appears to be bad - get stats on the whole 1024
		    for (uInt j=(testStart+10);j<rightStart;j++) {
			Double testLag = lags[j];
			testSum += testLag;
			testSumSq += testLag*testLag;
		    }
		    if (rightStart < lags.nelements()) {
			// make sure first 10 elements are not also bad
			Double rightSum, rightSumSq;
			rightSum = rightSumSq = 0.0;
			for (uInt j=rightStart;j<(rightStart+10);j++) {
			    Double rightLag = lags[j];
			    rightSum += rightLag;
			    rightSumSq += rightLag*rightLag;
			}
			if (abs(rightSum/10.0 - goodSum/nGood) > 
			    itsSigmaFactor * sqrt((goodSumSq - goodSum*goodSum/nGood)/
						  (nGood-1.0))) {
			    // no, that one is bad too, give up on this row
			    // can not be fixed
			    badData = True;
			    result = True;
			    ostringstream allLags;
			    allLags << "0:" << lags.nelements();
			    reportBadLags(spec, String(allLags), "Adjacent bad 1024-blocks - entire row flagged as bad, not fixed.");
			    badCount = 0;
			    allBad = True;
			    break;
			} else {
			    // right side looks good to use, continue
			    goodSum += rightSum;
			    goodSumSq += rightSumSq;
			    for (uInt j=(rightStart+10);j<(rightStart+512);j++)
				{
				    Double rightLag = lags[j];
				    goodSum += rightLag;
				    goodSumSq += rightLag*rightLag;
				}
			    nGood += 512;
			}
		    }
		    // final check
		    Double meanTest = testSum/1024.0;
		    Double meanGood = goodSum/nGood;
		    Double stddevGood = 
			sqrt((goodSumSq - goodSum*meanGood)/(nGood-1.0));
		    if (abs(meanTest - meanGood) > itsSigmaFactor*stddevGood) {
			// yup, it's bad, try and fix it
			Double stddevTest = 
			    sqrt((testSumSq - testSum*meanTest)/1023.0);
			Double scale = stddevGood/stddevTest;
			for (uInt j=testStart;j<rightStart;j++) {
			    lags[j] = (lags[j]-meanTest)*scale + meanGood;
			}
			result = True;
			if (badCount > 0) badChanStream << ",";
			badChanStream << testStart << ":" << (rightStart-1);
			badCount++;
			// reset thisLag
			thisLag = lags[i];
		    }
		}
		// move regions to next segment
		leftStart += 1024;
		testStart += 1024;
		rightStart += 1024;
		goodSum = goodSumSq = testSum = testSumSq = nGood = 0.0;
		// if we reach the end, move leftStart so that this if/else 
		// block is never entered again
		if (testStart == lags.nelements()) {
		    leftStart = lags.nelements();
		}
	    }

	    if (allBad) break;

	    // okay to proceed with spike check
	    if (i < testLoc) {
		// still in first set, no checks, just build up stats
		spikeSum += thisLag;
		spikeSumSq += thisLag*thisLag;
		nSpike++;
	    } else if (i > testLoc) {
		// i==testLoc is skipped here
		if (nSpike < 200) {
		    // still adding values in, nothing to check
		    spikeSum += thisLag;
		    spikeSumSq += thisLag*thisLag;
		    nSpike++;
		    if (nSpike == 200) testLag = lags[testLoc];
		} else {
		    // static condition, add and subtract
		    spikeSum += thisLag;
		    spikeSumSq += thisLag*thisLag;
		    Double lastLag = lags[testLoc-100];
		    spikeSum -= lastLag;
		    spikeSumSq -= lastLag*lastLag;
		    // and the test location, this is where testLoc is 
		    // actually moved
		    spikeSum += testLag;
		    spikeSumSq += testLag*testLag;
		    testLoc++;
		    testLag = lags[testLoc];
		    spikeSum -= testLag;
		    spikeSumSq -= testLag*testLag;		    
		}
	    }
	    if (nSpike == 200) {
		// is testLoc a spike
		meanSpike = spikeSum/nSpike;
		stddevSpike = sqrt((spikeSumSq - spikeSum*meanSpike)/
				   (nSpike-1.0));
		if (abs(testLag - meanSpike) > itsSigmaFactor*stddevSpike) {
		    // yes, it is a spike or drop-out
		    if ((testLoc - lastSpike) > 1) {
			// not already next to a spike, is the next one bad?
			// no need to worry about end points here
			if (abs(lags[testLoc+1] - meanSpike) > itsSigmaFactor*stddevSpike) {
			    // adjacent spikes - start of group - do not fix
			    spikeGroupStart = testLoc;
			} else {
			    // no, must be isolated - fix and replace
			    lags[testLoc] = meanSpike;
			    testLag = meanSpike;
			    if (badCount > 0) badChanStream << ",";
			    badChanStream << testLoc;
			    badCount++;
			}
		    } // else must already be in a spike group, nothing to do yet
		    lastSpike = testLoc;
		} else {
		    // has a spike group just ended
		    if (spikeGroupStart >= 0) {
			ostringstream spikeGroup;
			spikeGroup << spikeGroupStart << ":" << lastSpike;
			reportBadLags(spec, String(spikeGroup), "Adjacent spikes - not fixed");
			spikeGroupStart = -1;
		    }
		}
	    }
	}        
	if (!allBad) {
	    // need to test from (testLoc+1) to the end
	    for (uInt i=Int(testLoc)+1;i<lags.nelements();i++) {
		// move from last location
		Double lastLag = lags[testLoc-100];
		spikeSum -= lastLag;
		spikeSumSq -= lastLag*lastLag;
		spikeSum += testLag;
		spikeSumSq += testLag*testLag;
		testLoc++;
		testLag = lags[testLoc];
		spikeSum -= testLag;
		spikeSumSq -= testLag*testLag;
		nSpike--;
		// is testLoc a spike
		meanSpike = spikeSum/nSpike;
		stddevSpike = sqrt((spikeSumSq - spikeSum*meanSpike)/
				   (nSpike-1.0));
		if (abs(testLag - meanSpike) > itsSigmaFactor*stddevSpike) {
		    // yes, it is a spike or drop-out
		    if ((testLoc - lastSpike) > 1) {
			// not already next to a spike, is the next one bad?
			if (i<(lags.nelements()-1) && abs(lags[testLoc+1] - meanSpike) > itsSigmaFactor*stddevSpike) {
			    // adjacent spikes - start of group - do not fix
			    spikeGroupStart = testLoc;
			} else {
			    // no, must be isolated - fix and replace
			    lags[testLoc] = meanSpike;
			    testLag = meanSpike;
			    if (badCount > 0) badChanStream << ",";
			    badChanStream << testLoc;
			    badCount++;
			}
		    } // else must already be in a spike group, nothing to do yet
		    lastSpike = testLoc;
		} else {
		    // has a spike group just ended
		    if (spikeGroupStart >= 0) {
			ostringstream spikeGroup;
			spikeGroup << spikeGroupStart << ":" << lastSpike;
			reportBadLags(spec, String(spikeGroup), "Adjacent spikes - not fixed");
			spikeGroupStart = -1;
		    }
		}
	    }
	    // finally, there might be a spike group that has not yet been reportedd
	    if (spikeGroupStart >= 0) {
		ostringstream spikeGroup;
		spikeGroup << spikeGroupStart << ":" << lastSpike;
		reportBadLags(spec, String(spikeGroup), "Adjacent spikes - not fixed");
		spikeGroupStart = -1;
	    }
	}
	if (badCount > 0) reportBadLags(spec,String(badChanStream));
    }
    return result;
}


String GBTACSTable::bankPortPhase(const String &bank, Int port, Int phase)
{
    ostringstream ostr;
    ostr << bank << port << phase;
    return String(ostr);
}

void GBTACSTable::reportBadLags(uInt spec, String chanStr, String comment)
{
    Int thisSamp = spec % nsamp();
    Int thisPhase = (spec/nsamp()) % nstate();

    String samplerA, samplerB, sampler;
    {
	ostringstream ostr;
	ostr << itsBankA[thisSamp] << itsPortA[thisSamp];
	samplerA = String(ostr);
    }
    {
	ostringstream ostr;
	ostr << itsBankB[thisSamp] << itsPortB[thisSamp];
	samplerB = String(ostr);
    }

    if (samplerA == samplerB) {
	sampler = samplerA;
    } else {
	sampler = samplerA + "x" + samplerB;
    }

    if (itsFixLagsLog.length() > 0) {
	ofstream badLagsLog(itsFixLagsLog.c_str(),fstream::app);
	if (!badLagsLog) { // open of log file failed
	    LogIO os(LogOrigin("GBTACSTable","reportBadLags"));
	    os << LogIO::SEVERE << WHERE
	       << "Cannot open fixed lags log file - unexpected, trying to continue ..."
	       << LogIO::POST;
	    itsFixLagsLog = "";
	    reportBadLags(spec,chanStr);
	} else {
	    if (comment.length() > 0) badLagsLog << "# ";
	    badLagsLog << itsTimestamp << " " << itsScan << " " << sampler << " " 
		       << rownr() << " " << thisPhase << " " << chanStr 
		       << " " << comment << endl;
	}
    } else {
	LogIO os(LogOrigin("GBTACSTable","reportBadLags"));
	os << LogIO::NORMAL << WHERE;
	if (comment.length() > 0) os << "# ";
	os << itsTimestamp << " " << itsScan << " " << sampler << " " 
	   << rownr() << " " << thisPhase << " " << chanStr << " " << comment << endl;	    
    }
}
