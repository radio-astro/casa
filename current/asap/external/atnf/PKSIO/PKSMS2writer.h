//#---------------------------------------------------------------------------
//# PKSMS2writer.h: Class to write Parkes Multibeam data to a measurementset.
//#---------------------------------------------------------------------------
//# livedata - processing pipeline for single-dish, multibeam spectral data.
//# Copyright (C) 2000-2009, Australia Telescope National Facility, CSIRO
//#
//# This file is part of livedata.
//#
//# livedata is free software: you can redistribute it and/or modify it under
//# the terms of the GNU General Public License as published by the Free
//# Software Foundation, either version 3 of the License, or (at your option)
//# any later version.
//#
//# livedata is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with livedata.  If not, see <http://www.gnu.org/licenses/>.
//#
//# Correspondence concerning livedata may be directed to:
//#        Internet email: mcalabre@atnf.csiro.au
//#        Postal address: Dr. Mark Calabretta
//#                        Australia Telescope National Facility, CSIRO
//#                        PO Box 76
//#                        Epping NSW 1710
//#                        AUSTRALIA
//#
//# http://www.atnf.csiro.au/computing/software/livedata.html
//# $Id: PKSMS2writer.h,v 19.14 2009-09-29 07:33:38 cal103 Exp $
//#---------------------------------------------------------------------------

#ifndef ATNF_PKSMS2WRITER_H
#define ATNF_PKSMS2WRITER_H

#include <atnf/PKSIO/PKSrecord.h>
#include <atnf/PKSIO/PKSwriter.h>

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>

#include <casa/namespace.h>

// <summary>
// Class to write Parkes Multibeam data to a measurementset.
// </summary>

class PKSMS2writer : public PKSwriter
{
  public:
    // Default constructor.
    PKSMS2writer();

    // Destructor.
    virtual ~PKSMS2writer();

    // Create the output MS and write static data.
    virtual Int create(
        const String msName,
        const String observer,
        const String project,
        const String antName,
        const Vector<Double> antPosition,
        const String obsMode,
        const String bunit,
        const Float  equinox,
        const String dopplerFrame,
        const Vector<uInt> nChan,
        const Vector<uInt> nPol,
        const Vector<Bool> haveXPol,
        const Bool   haveBase);

    // Write the next data record.
    virtual Int write(
        const PKSrecord &pksrec);

    // Close the MS, flushing all associated Tables.
    virtual void close();

  private:
    MFrequency::Types cDopplerFrame;

    // Measurementset main table and subtables.
    MeasurementSet *cPKSMS;
    MSAntenna cAntenna;
    MSDataDescription cDataDescription;
    MSDoppler cDoppler;
    MSFeed cFeed;
    MSField cField;
    MSFlagCmd cFlagCmd;
    MSHistory cHistory;
    MSObservation cObservation;
    MSPointing cPointing;
    MSPolarization cPolarization;
    MSProcessor cProcessor;
    MSSource cSource;
    MSSpectralWindow cSpectralWindow;
    MSState cState;
    MSSysCal cSysCal;
    MSWeather cWeather;

    // Access to measurementset table columns; we are forced to use pointers
    // here since none of these classes have default constructors.
    MSColumns *cMSCols;
    MSAntennaColumns *cAntennaCols;
    MSDataDescColumns *cDataDescCols;
    MSDopplerColumns *cDopplerCols;
    MSFeedColumns *cFeedCols;
    MSFieldColumns *cFieldCols;
    MSFlagCmdColumns *cFlagCmdCols;
    MSHistoryColumns *cHistoryCols;
    MSObservationColumns *cObservationCols;
    MSPointingColumns *cPointingCols;
    MSPolarizationColumns *cPolarizationCols;
    MSProcessorColumns *cProcessorCols;
    MSSourceColumns *cSourceCols;
    MSSpWindowColumns *cSpWindowCols;
    MSStateColumns *cStateCols;
    MSSysCalColumns *cSysCalCols;
    MSWeatherColumns *cWeatherCols;

    ArrayColumn<Float> *cCalFctrCol;
    ArrayColumn<Float> *cBaseLinCol;
    ArrayColumn<Float> *cBaseSubCol;
    ScalarColumn<Complex> *cXCalFctrCol;


    // Add an entry to the ANTENNA subtable.
    Int addAntennaEntry(
        const String antName,
        const Vector<Double> &antPosition);

    // Add an entry to the DATA_DESCRIPTION subtable.
    Int addDataDescriptionEntry(
        const Int iIF);

    // Add an entry to the DOPPLER subtable.
    Int addDopplerEntry();

    // Add an entry to the FEED subtable.
    Int addFeedEntry();

    // Add an entry to the FIELD subtable.
    Int addFieldEntry(
        const String fieldName,
        const Double mjd,
        const Vector<Double> direction,
        const Vector<Double> scanRate,
        const Int srcId);

    // Skip FLAG_CMD subtable.

    // Skip FREQ_OFFSET subtable.

    // Skip HISTORY subtable.

    // Add an entry to the OBSERVATION subtable.
    Int addObservationEntry(
        const String observer,
        const String project);

    // Add an entry to the POINTING subtable.
    Int addPointingEntry(
        const Double mjd,
        const Double interval,
        const String fieldName,
        const Vector<Double> direction,
        const Vector<Double> scanRate);

    // Add an entry to the POLARIZATION subtable.
    Int addPolarizationEntry(
        const Int iIF,
        const Int nPol);

    // Add an entry to the PROCESSOR subtable.
    Int addProcessorEntry();

    // Add an entry to the SOURCE subtable.
    Int addSourceEntry(
        const String name,
        const Vector<Double> direction,
        const Vector<Double> properMotion,
        const Double restFreq,
        const Double radialVelocity);

    // Add an entry to the SPECTRAL_WINDOW subtable.
    Int addSpectralWindowEntry(
        const Int iIF,
        const Int nChan,
        const Double refFreq,
        const Double bandwidth,
        const Double freqInc);

    // Add an entry to the STATE subtable.
    Int addStateEntry(
        const String obsMode);

    // Add an entry to the SYSCAL subtable.
    Int addSysCalEntry(
        const Int beamNo,
        const Int spWinId,
        const Double mjd,
        const Double interval,
        const Vector<Float> Tcal,
        const Vector<Float> Tsys);

    // Add an entry to the WEATHER subtable.
    Int addWeatherEntry(
        const Double mjd,
        const Double interval,
        const Double pressure,
        const Double humidity,
        const Double temperature);
};

#endif
