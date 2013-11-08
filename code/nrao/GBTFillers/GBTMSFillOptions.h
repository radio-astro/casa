//# GBTMSFillOptions.h: class to hold the filler options
//# Copyright (C) 2002,2003
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

#ifndef NRAO_GBTMSFILLOPTIONS_H
#define NRAO_GBTMSFILLOPTIONS_H

#include <casa/aips.h>
#include <nrao/FITS/GBTACSTable.h>

#include <casa/namespace.h>

//# Forward Declarations

// <summary>
// This class holds the various fill options set through the filler DO
// </summary>

// <use visibility=internal>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class=gbtmsfiller>gbtmsfiller</linkto> distributed object
// </prerequisite>
//
// <etymology>
// This holds the options for the gbtmsfiller DO.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Some options are common to all backends while others only apply to
// one backend.  By putting the options here, an instance of this class
// can be passed to each filler via the fill command so that each 
// backend filler doesn't need to know about options that don't affect
// it and the fill arguments don't need to be changed for each backend
// whenever a particular filler needs a new option.
// </motivation>
//
// <todo asof="yyyy/mm/dd">
//   <li> Everything
// </todo>

class GBTMSFillOptions
{
public:
    // initialized with the default values for each option
    GBTMSFillOptions();

    // copy constructor - copy semantics
    GBTMSFillOptions(const GBTMSFillOptions &other);

    ~GBTMSFillOptions() {;}

    // Assignment operator, copy semantics
    GBTMSFillOptions &operator=(const GBTMSFillOptions &other);

    // The following are used by all backend fillers
    // <group>
    // Fill the raw pointing values to the appropriate sub-table?
    Bool fillRawPointing() const {return itsRawPointing;}
    // Set the fillRawPointing option
    void setFillRawPointing(Bool fillRawPointing) 
    {itsRawPointing = fillRawPointing;}
    // Fill the raw focus values to the appropriate sub-table?
    Bool fillRawFocus() const {return itsRawFocus;}
    // Set the fillRawFocus option
    void setFillRawFocus(Bool fillRawFocus) 
    {itsRawFocus = fillRawFocus;}
    // Which CAL_TEMP to use when filling TCAL, LOW or HIGH?
    // Eventually this will be handled through the IF file and these two
    // methods will be eliminated. If this returns True, then the HIGH
    // cal will be used, otherwise the LOW cal is used.
    Bool useHighCal() const {return itsUseHighCal;}
    // Set the useHighCal option. This can be used
    // to override the default behavior of using LOW_CAL_TEMP.
    void setUseHighCal(Bool useHighCal)
    {itsUseHighCal = useHighCal;}
    // Compress the calibration columns.
    Bool compressCalCols() const {return itsCompressCalCols;}
    // Set the flag to control whether calibration columns are compressed.
    void setCompressCalCols(Bool compressCalCols)
    {itsCompressCalCols = compressCalCols;}
    // </group>

    // The following options only apply to the ACS
    // <group>
    // Fill the LAGS in addition to FLOAT_DATA?
    Bool fillLags() const {return itsLags;}
    // Set the fillLags option
    void setFillLags(Bool fillLags) {itsLags = fillLags;}
    // The vanVleck correction to use
    GBTACSTable::VanVleckCorr vanVleckCorr() const {return itsVVCorr;}
    // Set the vanVleck correction to use
    void setVanVleckCorr(GBTACSTable::VanVleckCorr vvCorr)
    { itsVVCorr = vvCorr;}
    // The vanVleck correction to use, as a string
    String vanVleckCorrString() const;
    // Set the vanVleck correction from a string, returns False if the
    // value is unrecognized and sets the vanVleck correction type to the default
    Bool setVanVleckCorr(const String &vanVleckCorr);
    // The smoothing (windowing) to apply to the corrected data prior to 
    // fourier transforming the data to the spectral domain
    GBTACSTable::Smoothing smoothing() const {return itsSmooth;}
    // Set the smoothing (windowing) function to use
    void setSmoothing(GBTACSTable::Smoothing smoothing)
    {itsSmooth = smoothing;}
    // The smoothing type as a string
    String smoothingString() const;
    // Set the smoothing correction from a string, returns False if
    // The value is unrecognized and set the vanVleck correction type to
    // the default
    Bool setSmoothing(const String &smoothing);
    // Get the vv table size for the schwab correction 
    Int vvSize() const 
    {return itsVVsize;}
    // Set the vv table size for the schwab correction - must be an odd number
    Bool setvvSize(const Int vvsize);
    // Use a derived bias approximation in the van Vleck correction
    // (Schwab correction only)
    void setUseBias(Bool usebias) {itsUseBias = usebias;}
    // Get the boolean that signals whether or not the bias should be used.
    Bool useBias() const {return itsUseBias;}
    // Set to fill multi bank ACS data to a single MS
    void setOneacsms(Bool oneacsms) {itsOneacsms = oneacsms;}
    // Should the filler fill multi bank ACS data to a single MS
    Bool oneacsms() const {return itsOneacsms;}
    // Set a specific dcbias to use.
    Bool setDCBias(Double dcbias) { itsDCBias = dcbias; return True;}
    // Returns the specific dcbias to use.  Ignored if usebias is True.
    Double dcbias() const {return itsDCBias;}
    // Set a minimum bias factor to subtract from the lags.
    Bool setMinbiasfactor(Int minbiasfactor) { itsMinbiasfactor = minbiasfactor; return True;}
    // The minimum bias factor to subtract (actual may be larger).  If < 0, nothing is subtracted from the lags.
    Int minbiasfactor() const {return itsMinbiasfactor;}
    // Toggle correcting for offset ACS lags
    void setFixbadlags(Bool fixbadlags) {itsFixbadlags = fixbadlags;}
    // Should the ACS lags be fixed if possible
    Bool fixbadlags() const {return itsFixbadlags;}
    // Set the sigma factor used when fixbadlags() is True
    void setSigmaFactor(Double sigmafactor) {itsSigmaFactor = sigmafactor;}
    // What is the value of the sigma factor used when fixbadlags() is True
    Double sigmaFactor() const {return itsSigmaFactor;}
    // Set the starting lag for spike fixing, used when fixbadlags() is True
    void setSpikeStart(Int spikestart) {itsSpikeStart = spikestart;}
    // What is the value of the spike starting location
    Int spikeStart() const {return itsSpikeStart;}
    // </group>

private:
    Bool itsLags, itsRawFocus, itsRawPointing, itsUseHighCal, 
	itsCompressCalCols, itsUseBias, itsOneacsms, itsFixbadlags;
    Int itsVVsize, itsMinbiasfactor, itsSpikeStart;
    Double itsDCBias, itsSigmaFactor;

    GBTACSTable::VanVleckCorr itsVVCorr;
    GBTACSTable::Smoothing itsSmooth;
};
#endif

