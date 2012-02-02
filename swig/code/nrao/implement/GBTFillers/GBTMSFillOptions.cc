//# GBTMSFillOptions.cc:  Defines GBTMSFillOptions
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
//# $Id$

//# Includes

#include <nrao/GBTFillers/GBTMSFillOptions.h>

GBTMSFillOptions::GBTMSFillOptions()
    : itsLags(False), itsRawFocus(False), itsRawPointing(False), itsUseHighCal(False),
      itsCompressCalCols(True), itsUseBias(False), itsOneacsms(True), itsVVsize(65),
      itsMinbiasfactor(-1), itsSpikeStart(200), itsDCBias(0.0), itsSigmaFactor(6.0),
      itsVVCorr(GBTACSTable::DefaultVanVleck),
      itsSmooth(GBTACSTable::DefaultSmoothing)
{;}

GBTMSFillOptions::GBTMSFillOptions(const GBTMSFillOptions &other)
{
  *this = other;
}

GBTMSFillOptions &GBTMSFillOptions::operator=(const GBTMSFillOptions &other)
{
  if (this != &other) {
    itsLags = other.itsLags;
    itsRawFocus = other.itsRawFocus;
    itsRawPointing = other.itsRawPointing;
    itsUseHighCal = other.itsUseHighCal;
    itsCompressCalCols = other.itsCompressCalCols;
    itsUseBias = other.itsUseBias;
    itsOneacsms = other.itsOneacsms;
    itsFixbadlags = other.itsFixbadlags;
    itsMinbiasfactor = other.itsMinbiasfactor;
    itsSpikeStart =other.itsSpikeStart;
    itsDCBias = other.itsDCBias;
    itsSigmaFactor = other.itsSigmaFactor;
    itsVVCorr = other.itsVVCorr;
    itsSmooth = other.itsSmooth;
  }
  return *this;
}

String GBTMSFillOptions::vanVleckCorrString() const
{
    String result;
    switch (itsVVCorr) {
    case GBTACSTable::NoVanVleck:
	result = "None";
	break;
    case GBTACSTable::Schwab:
	result = "Schwab";
	break;
    default:
	result = "Old";
	break;
    }
    return result;
}

Bool GBTMSFillOptions::setVanVleckCorr(const String &vanVleckCorr)
{
    // make this case-insensitive
    String vvcorr = vanVleckCorr;
    vvcorr.downcase();
    Bool result = True;
    if (vvcorr == "none") {
	setVanVleckCorr(GBTACSTable::NoVanVleck);
    } else if (vvcorr == "schwab") {
	setVanVleckCorr(GBTACSTable::Schwab);
    } else if (vvcorr == "old") {
	setVanVleckCorr(GBTACSTable::Old);
    } else if (vvcorr == "default") {
	setVanVleckCorr(GBTACSTable::DefaultVanVleck);
    } else {
	result = False;
	setVanVleckCorr(GBTACSTable::DefaultVanVleck);
    }
    return result;
}

String GBTMSFillOptions::smoothingString() const
{
    String result;
    switch (itsSmooth) {
    case GBTACSTable::NoSmoothing:
	result = "None";
	break;
    case GBTACSTable::Hanning:
	result = "Hanning";
	break;
    default:
	result = "Hamming";
	break;
    }
    return result;
}

Bool GBTMSFillOptions::setSmoothing(const String &smoothing)
{
    // make this case-insensitive
    String smooth = smoothing;
    smooth.downcase();
    Bool result = True;
    if (smooth == "none") {
	setSmoothing(GBTACSTable::NoSmoothing);
    } else if (smooth == "hanning") {
	setSmoothing(GBTACSTable::Hanning);
    } else if (smooth == "hamming") {
	setSmoothing(GBTACSTable::Hamming);
    } else if (smooth == "default") {
	setSmoothing(GBTACSTable::DefaultSmoothing);
    } else {
	result = False;
	setSmoothing(GBTACSTable::DefaultSmoothing);
    }
    return result;
}

Bool GBTMSFillOptions::setvvSize(const Int vvsize)
{
    Bool result = vvsize > 0 && vvsize%2 != 0;
    if (result) {
	itsVVsize = vvsize;
    }
    return result;
}
