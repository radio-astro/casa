//# GBTACSTable.h: GBT backend table for the ACS.
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
//#
//# $Id$

#ifndef NRAO_GBTACSTABLE_H
#define NRAO_GBTACSTABLE_H

#include <nrao/FITS/GBTFITSBase.h>
#include <nrao/FITS/GBTBackendTable.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/RecordField.h>
#include <casa/BasicSL/Complex.h>
#include <scimath/Mathematics/FFTServer.h>
#include <scimath/Mathematics/VanVleck.h>

#include <casa/namespace.h>
// forward declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
} //# NAMESPACE CASA - END


// <summary>
// A GBTBackendTable for the ACS
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// GBTACSTable is a GBTBackendTable with some additional
// methods specific to the ACS.  
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>

class GBTACSTable : public GBTBackendTable
{
public:
    // define the vanVleck correction to used.
    enum VanVleckCorr {
	// No vanVleck correction will be done.
	NoVanVleck,
	// The correction from Schwab as implemented in the VanVleck class.
	Schwab,
	// The old correction from the Arecibo code.
	Old,
	// the default correction
	DefaultVanVleck = Schwab
    };
    // define the smoothing to apply after the data have been corrected
    enum Smoothing {
	// No smoothing
	NoSmoothing,
	// hanning smoothing
	Hanning,
	// hamming smoothing
	Hamming,
	// the default smoothing
	DefaultSmoothing = Hanning
    };

    // Construct it from a file name.  Use the indicated vanVleck correction
    // and smoothing in the data method.  The vvsize is the table size
    // used when the Schwab correction (the default) is requested.  If
    // useDCBias is True, an attemp is made to use the DCBias (the average
    // of the autocorrrelations over the last 5% of the lags) in the vanVleck
    // correction if the Schwab correction is specified.  If usebias is False
    // then dcbias is used as the bias.  If minbiasfactor is >= 0 then the
    // nearest integer >= minbiasfactor is found that, when multiplied by
    // 0.5/65536, raises the mean of the last 5% of the lags to just > 0.0.
    // This is an attempt to correct for a truncation error introduced when
    // the lags are converted to 32-bit values.
    GBTACSTable(const String &fileName, VanVleckCorr vanVleckCorr=DefaultVanVleck,
		Smoothing smoothing = DefaultSmoothing, Int vvsize=65,
		Bool useDCBias = False, Double dcbias=0.0, Int minbiasfactor=-1,
		Bool fixlags = False, String fixlagslog="",
		Double sigmaFactor = 6.0, Int spikeStart = 200);
    ~GBTACSTable();

    // Attach this GBTACSTable to a new file name.
    // when resync is True, if fileName is the same as the
    // currently opened file, if there is one, then this just
    // closes and reopens the FITS file setting things right
    // back to the next row in the table, if there is one
    virtual Bool reopen(const String &fileName, Bool resync);

    // the default reopen from FITSTabular uses resync=True
    virtual Bool reopen(const String &fileName) 
    {return reopen(fileName, True);}

    // get the raw data from the specific sampler
    virtual Array<Float> rawdata(uInt whichSampler);

    // get the data from the specific sampler,
    // applying the vanVleck correction and smoothing specified
    virtual Array<Float> data(uInt whichSampler);

    // is the data bad for the specific sampler
    virtual Array<Bool> badData(uInt whichSampler);

    // the zero channel for the specific sampler
    virtual Array<Float> zeroChannel(uInt whichSampler);
			     
    // the channel spacing as deduced from the CDELT keyword and number of lags
    virtual Double spacing() {return itsSpacing;}

    // separate isValid needed here to watch for problems in init
    virtual Bool isValid() const {return isValid_p;}

    // Do the switching signals in ACT_STATE follow the
    // documentation?  Prior to FITSVER 1.2 they had the
    // opposite sense.
    virtual Bool switchOK() const
    { return (deviceVersion() > 1 || baseVersion() > 1); }

    // return the number of sampler levels being used at the moment
    virtual Int nlevels() const { return itsCachedLevel;}

    // The value of the INTEGRAT field in the current row for
    // the given sampler and state
    virtual Float integrat(uInt whichSampler, uInt whichState) const;

    // The DMJD value from the current row
    virtual Double dmjd() {return *itsDMJD;}

    // The value of the TIME-MID field in the current row
    virtual Double timeMid() {return *itsTimeMid;}

    // A string giving the bank here
    virtual String bank() {return itsBank;}

private:
    // remember what the cached data cube currently has
    VanVleckCorr itsVanVleckCorr;
    Smoothing itsSmoothing;
    Int itsVVsize, itsMinbiasfactor, itsSpikeStart;
    Bool itsUseDCBias, itsNormalTintAxes, itsNew, itsFixlags;
    Double itsDCBias, itsTruncError, itsSigmaFactor;

    // used in reporting 
    String itsTimestamp;
    Int itsScan;

    // the last row that was corrected and FFTed and is now
    // in itsCachedData.
    Int itsCachedDataRow, itsCachedLagRow;

    // cache the spacing when (re)opening the file
    Double itsSpacing;

    // itsCachedData is what is returned by data().
    Array<Float> itsCachedData;
    // itsCachedDataMatrix references the same values, but with a 2D shape
    // to make it trivial to iterate through each spectra.
    Matrix<Float> *itsCachedDataMatrix;
    // itsCachedZeroChannel is the vector of corrected zero channels
    // associated with itsCachedData - one per sampler
    Array<Float> itsCachedZeroChannel;
    Vector<Float> *itsCachedZeroVector;
    // itsCachedBadData is the vector of booleans indicating which 
    // lag sequences are bad - one per sampler
    Array<Bool> itsCachedBadData;
    Vector<Bool> *itsCachedBadDataVector;
    Vector<Bool> itsCachedBadZeroLags, itsCachedDiscontinuities;

    Array<Float> itsCachedLag;

    FFTServer<Float, Complex> itsFFTServer;

    // remember the sampler level - used in a few places when processing the data
    Int itsCachedLevel;

    // van-vleck corr is done as a double for accuracy - this is a temp used for that
    Matrix<Double> itsVVTemp;

    // fourier transform is 2x size in first dimension - keep transients here
    // one spectra at a time
    Vector<Float> itsFTTemp;
    Vector<Complex> itsFFTResult;

    // weight to give the nlags elements in itsFTTemp when that is constructed
    // set by smooth but defaults to 0.0
    Float itsNlagWeight;

    // smoothing vector - only depends on the number of lags and type of smoothing
    Vector<Float> itsSmoother;

    // the power level for each spectra in a row is kept here, when requested
    Vector<Double> itsPowerLevels;

    RORecordFieldPtr<Array<Float> > itsDataField, itsIntegrat;
    RORecordFieldPtr<Double> itsDMJD, itsTimeMid;

    // A flag to indicate that at least one bad zero lag was detected.
    // This is set to true and an error message emited when a bad zero
    // lag is seen and reset to false in reopen.  In that way, the error
    // message is only seen once per input file.
    Bool itsHasBadData, itsErrorEmitted;
    // similarly for discontinuities
    Bool itsHasDiscontinuities, itsDiscErrorEmitted;

    // Is everything okay (appart from bad zero-lags)
    Bool isValid_p;

    // used in the data slicing
    IPosition itsStart, itsEnd;
    uInt itsSampAxis;

    String itsBank;

    // The BANK_{A,B} and  PORT_{A,B} column values from sampler()
    Vector<String> itsBankA, itsBankB;
    Vector<Int> itsPortA, itsPortB;

    String itsFixLagsLog;

    void vanVleck();

    // get information from an AC lag vector
    void getACInfo(Double &threshold, Double &dcbias, Int &biasfactor, Int nlevel, 
		   Vector<Double> &aclags);

    // This actually does the correction - in place
    void vanVleckLags(Vector<Double> &lags, Int nlevel, Double thresh1, Double thresh2,
		      Double dcbias1, Double dcbias2);

    // Functions related to the old vv correction
    // these are all in-place operations on itsCachedData
    // 3-level sampling
    void pow3lev();
    void vanvleck3lev();
    // 9-level sampling
    void pow9lev();
    void vanvleck9lev();

    void applyFFT();

    // smoothing functions
    void smooth();
    void hamming();
    void hanning();

    // check for bad data, sets itsCachedBadData and returns True if
    // there was any bad data.  
    Bool checkForBadData(Int nspectra);
    // Checks a lag set for any discontinuties in 1024-lag chunks.
    // A discontinuity is when a 1024-lag chunk is more than nsigma
    // from the mean of the 512-lags on either side of that chunk.
    // This will also fix it when one is found if itsFixLags is true.  
    // When fixed, that information is logged to the named logfile.  
    // If not fixed, then this should be flagged as bad.  Returns True 
    // if any discontinuities were found.  badData is set to True when 
    // the discontinuity was not fixed.  The first 1024-lag segment is 
    // not checked here.  If that is offset then it should get flagged
    // elsewhere because the zero lag will be unphysical.  The first
    // 1024 segement can not be fixed.
    Bool checkForDiscontinuities(Vector<Float> lags, Bool &badData, uInt spec, Bool isXcorr);

    // Generates a report for a given bad lag 
    void reportBadLags(uInt spec, String badChanStr, String comment="");

    // initializes shapes and the cachedLevel, clears the booleans and
    // the cached row number - does not read any data.
    Bool init();

    // utility functions
    String bankPortPhase(const String &bank, Int port, Int phase);

    // Undefined and inaccessible.
    GBTACSTable();
    GBTACSTable(const GBTACSTable &);
    GBTACSTable &operator=(const GBTACSTable &);
};


#endif
