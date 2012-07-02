//# WSRTDataSource.h : class for access to WSRT datasets
//# Copyright (C) 1996,1997,1998,1999,2000
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
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

#ifndef NFRA_WSRTDATASOURCE_H
#define NFRA_WSRTDATASOURCE_H

#include <nfra/Wsrt/DataSource.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/DataConversion.h>
#include <casa/IO/RegularFileIO.h>


#include <casa/namespace.h>
// <summary> 
// Class for access to WSRT datasets.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <synopsis>
// This class is the concrete class to use the filler for a WSRT dataset.
// </synopsis>


class WSRTDataSource : public DataSource
{
public:
    // Constructor; open and read input file
    WSRTDataSource(const String& dsName, Bool noscale);

    // Destructor; close input file
    ~WSRTDataSource();

    // Return name of class
    inline String className() const;

    // Check the contents of the dataset
    virtual void show();

    // Fill the whole MeasurementSet in one go, if possible
    virtual inline Bool fill(MeasurementSet& ms) const;

    // Get the number of antennas
    virtual inline Int numAnt() const;

    // Get the number of spectral bands
    virtual inline Int numBand() const;

    // Get the number of spectral channels
    virtual inline Int numChan() const;

    // Get the number of correlations
    virtual inline Int numCorr() const;

    // Get the number of baselines
    virtual inline Int numBase() const;

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

    // Get antenna names
    virtual Vector<String> antName() const;

    // Get positions of antennas
    virtual Matrix<Double> antPosition() const;

    // Get the centre time and time interval in seconds
    virtual Vector<Double> timeInterval() const;

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

    // Get the data vector for the current data block
    virtual inline Vector<Complex> msData(Vector<Bool>& flag);

    // Get the data cube for the continuum band
    virtual Cube<Complex> continuumData() const;

    // Get the flag cube for the continuum band
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
    // Return zero, if no more blocks available.
    virtual Int nextDataBlock();

    // Get UVW values for the MS main table (in meters)
    virtual Vector<Double> msUVW(const uInt sampleNr) const;

private:
    // The ByteIO object and the appropriate data conversion object
    RegularFileIO itsFile;
    DataConversion* itsDataConversion;

    // Do we need to scale the data?
    Bool itsNoScale;

    // Buffers for the individual blocks
    Char* itsFDBuffer;
    Char* itsOHBuffer;
    Char* itsSCBuffer;
    Char* itsSHBuffer;
    Char* itsIHBuffer;
    Char* itsDBBuffer;

    // Buffer for continuum data and flags in spectral-line datasets
    Cube<Complex> itsContinuumData;
    Cube<Bool>    itsContinuumFlag;
    // baseline to row assignment for continuum filling
    Matrix<Int>   itsContinuumRow;
    uInt          itsNextContinuumRow;

    // Various other members for internal use
    Bool  itsFirstDataBlock;
    Char  itsBECode;

    // Size information for the dataset
    Int itsNumAnt;
    Int itsNumBand;
    Int itsNumChan;
    Int itsNumCorr;
    Int itsNumBase;

    // Layout of the dataset
    Short itsLRCRD;   //# record length in nr of bytes
    Short itsPHBLL;   //# physical block length in nr of records
    Int   itsNBL;     //# total nr of physical blocks in the dataset
    Int   itsNrOfOH, itsNrOfSC, itsNrOfSH, itsNrOfIH, itsNrOfDB;
                      //# nr of logical blocks of each type
    Int   itsFDlen;
    Short itsOHlen,  itsSClen,  itsSHlen,  itsIHlen,  itsDBlen;
                      //# logical block lengths in nr of records
    Int   itsOHoff, itsSCoff, itsSHoff, itsIHoff, itsDBoff;
                      //# "current" block offsets

    // Private functions to read the (next) block
    Bool readFD();
    Bool readOH();
    Bool readSC();
    Bool readSH();
    Bool readIH();
    Bool readDB();
    
    // Cache the continuum data and flags for a continuum data block
    void cacheContinuum(Int nRow);
};

inline String WSRTDataSource::className() const
{
    return "WSRTDataSource";
}
	
inline Bool WSRTDataSource::fill(MeasurementSet& ms) const
{
    return DataSource::fill(ms);
}

inline Int WSRTDataSource::numAnt() const
{
    return itsNumAnt;
}

inline Int WSRTDataSource::numBand() const
{
    return itsNumBand;
}

inline Int WSRTDataSource::numChan() const
{
    return itsNumChan;
}

inline Int WSRTDataSource::numCorr() const
{
    return itsNumCorr;
}

inline Int WSRTDataSource::numBase() const 
{
    return itsNumBase;
}

#endif
