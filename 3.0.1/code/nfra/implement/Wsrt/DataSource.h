//# DataSource.h : base class for access to local datasets
//# Copyright (C) 1996,1997,1999,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the termof the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef NFRA_DATASOURCE_H
#define NFRA_DATASOURCE_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays.h>
#include <measures/Measures/Stokes.h>


#include <casa/namespace.h>
// <summary> 
// Base class for access to local datasets.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <synopsis>
// This class is the base class for various data source classes
// for the filler.
// </synopsis>


class DataSource
{
public:
    // Constructor
    DataSource(const String& dsName);

    // Destructor
    virtual ~DataSource();

    // Return name of class
    inline String className() const;

    // Return name of Dataset
    inline const String& datasetName() const;

    // Is this a valid dataset?
    virtual Bool isValid();

    // Check the contents of the dataset
    virtual void show();

    // Fill the whole MeasurementSet in one go and return True,
    // or return False
    virtual Bool fill(MeasurementSet&) const;

    // Get the number of antennas; -1 if variable
    virtual Int numAnt() const;

    // Get the number of spectral bands
    virtual Int numBand() const;

    // Get the number of spectral channels; -1 if variable
    virtual Int numChan() const;

    // Get the number of correlations; -1 if variable
    virtual Int numCorr() const;

    // Get the number of baselines; -1 if variable
    virtual Int numBase() const;
 
    // Get the name of the observer(s)
    virtual String observer() const;

    // Get the project identification string
    virtual String project() const;

    // Get the field name
    virtual String fieldName() const;

    // Get the special observation type code
    virtual String fieldCode() const;

    // Get the directional information
    virtual Vector<Double> fieldDelayDir() const;
    virtual Vector<Double> fieldDelayDirRate() const;
    virtual Vector<Double> fieldPhaseDir() const;
    virtual Vector<Double> fieldPhaseDirRate() const;
    virtual Vector<Double> fieldPointDir() const;
    virtual Vector<Double> fieldPointDirRate() const;
    virtual Vector<Double> fieldReferenceDir() const;
    virtual Vector<Double> fieldReferenceDirRate() const;

    // Get the centre time and time interval in seconds
    virtual Vector<Double> timeInterval() const;

    // Get positions of antennas
    virtual Matrix<Double> antPosition() const;
 
    // Get antenna names
    virtual Vector<String> antName() const;

    // Get the frequency information
    virtual Double refFrequency() const;
    virtual Vector<Double> freqResolution() const;
//    virtual Vector<Double> freqResolution(Int bandnr) const;
    virtual Double restFrequency() const;
    virtual Double freqBandwidth() const;
    virtual Vector<Double> freqBandwidth(Int nrbands) const;
    virtual Vector<Double> chanFreq() const;
//    virtual Vector<Double> chanFreq(Int bandnr) const;

    virtual Double contChanFreq() const;
    virtual Double contFreqBandwidth() const;


    // Get the system temperatures
    virtual Matrix<Float> sysTemp() const;

    // Get the data array and flags for the current data block
    virtual Vector<Complex> msData(Vector<Bool>& flag);
    virtual Matrix<Complex> msData(Matrix<Bool>& flag, Int ico=-1);

    // Get the data cube for the continuuum band
    virtual Cube<Complex> continuumData() const;

    // Get the flag cube for the continuuum band
    virtual Cube<Bool> continuumFlag() const;

    // Get the correlation type (for the current data block)
    virtual Stokes::StokesTypes corrType();
    virtual Vector<Int> corrTypes() const;

    // Get the frequency channel number for the current data block
    virtual uInt chanNr();

    // Get the frequency band number for the current data block
    virtual uInt bandNr();

    // Get the antenna identifications for the MS main table
    virtual Vector<Int> msAntennaId(Int ico=0) const;

    // Get the times for the MS main table in seconds
    //times: exposure, interval, time
    virtual Vector<Double> msTime() const;

    // Move to next data block (return the number of datapoints)
    // Return zero if no more blocks available.
    virtual Int nextDataBlock();

    // Get UVW values for the MS main table (in meters)
    virtual Vector<Double> msUVW(const uInt sampleNr) const;

    // Get the TMS parameters
    // number of parameters
    virtual Int nrOfPars() const;
    // parameter names and values
    virtual Vector<String> parName() const;
    virtual Vector<String> parValue() const;

private:
    String itsDataset;
};

inline String DataSource::className() const
{
    return "DataSource";
}

inline const String& DataSource::datasetName() const
{
    return itsDataset;
}

#endif








