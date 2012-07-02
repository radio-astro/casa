//# DataSource.cc : base class for access to local datasets
//# Copyright (C) 1996,1997,1999,2000,2002
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

#include <nfra/Wsrt/DataSource.h>
#include <casa/iostream.h>


DataSource::DataSource(const String& dsName) : itsDataset(dsName)
{
    cout << className() << ": instantiated" << endl;
}

DataSource::~DataSource() 
{
    cout << className() << ": destructed" << endl;
}

Bool DataSource::isValid()
{
    cout << className() << ": no validity check implemented" << endl;
    return True;
}

void DataSource::show()
{
    cout << className() << ": cannot show" << endl;
}

Bool DataSource::fill(MeasurementSet&) const 
{
    return False;
}

Int DataSource::numAnt() const 
{
    return 0;
}

Int DataSource::numBand() const 
{
    return 0;
}

Int DataSource::numChan() const 
{
    return 0;
}

Int DataSource::numCorr() const 
{
    return 0;
}

Int DataSource::numBase() const 
{
    return 0;
}

 String DataSource::observer() const
{
    return "observer";
}
 
String DataSource::project() const
{
    return "project";
}

String DataSource::fieldName() const
{
    return "name";
}

String DataSource::fieldCode() const
{
    return "co";
}

Vector<Double> DataSource::fieldDelayDir() const
{
    Vector<Double> direction(2) ;
    direction = 0.0;
    return direction;
}

Vector<Double> DataSource::fieldDelayDirRate() const
{
    Vector<Double> rate(2) ;
    rate = 0.0;
    return rate;
}

Vector<Double> DataSource::fieldPhaseDir() const
{
    Vector<Double> direction(2) ;
    direction = 0.0;
    return direction;
}

Vector<Double> DataSource::fieldPhaseDirRate() const
{
    Vector<Double> rate(2) ;
    rate = 0.0;
    return rate;
}

Vector<Double> DataSource::fieldPointDir() const
{
    Vector<Double> direction(2) ;
    direction = 0.0;
    return direction;
}

Vector<Double> DataSource::fieldPointDirRate() const
{
    Vector<Double> rate(2) ;
    rate = 0.0;
    return rate;
}

Vector<Double> DataSource::fieldReferenceDir() const
{
    Vector<Double> direction(2) ;
    direction = 0.0;
    return direction;
}

Vector<Double> DataSource::fieldReferenceDirRate() const
{
    Vector<Double> rate(2) ;
    rate = 0.0;
    return rate;
}

Vector<String> DataSource::antName() const
{
    Vector<String> name(1);
    name = "unknown";
    return name;
}

Matrix<Double> DataSource::antPosition() const
{
    Matrix<Double> pos(1,3);
    pos = 0.0;
    return pos;
}

Vector<Double> DataSource::timeInterval() const
{
    Vector<Double> time(2);
    time = 0.0;
    return time;
}

Double DataSource::refFrequency() const
{
    return 0.0;
}

Vector<Double> DataSource::chanFreq() const
{
    Vector<Double> freq(1);
    freq = 0.0;
    return freq;
}

//Vector<Double> DataSource::chanFreq(Int bandnr) const
//{
//    Vector<Double> freq(1);
//    freq = 0.0;
//    return freq;
//}
//
Vector<Double> DataSource::freqResolution() const
{
    Vector<Double> resolution(1);
    resolution = 0.0;
    return resolution;
}

//Vector<Double> DataSource::freqResolution(Int bandnr) const
//{
//    Vector<Double> resolution(1);
//    resolution = 0.0;
//    return resolution;
//}
//
Double DataSource::restFrequency() const
{
    return 0.0;
}

Double DataSource::freqBandwidth() const
{
    return 0.0;
}

Vector<Double> DataSource::freqBandwidth(Int nrbands) const
{
    Vector<Double> bandwidth(nrbands);
    bandwidth = 0.0;
    return bandwidth;
}

Double DataSource::contChanFreq() const
{
    return 0.0;
}

Double DataSource::contFreqBandwidth() const
{
    return 0.0;
}

Matrix<Float> DataSource::sysTemp() const
{
    Matrix<Float> tSys(1,1);
    tSys = 0.0;
    return tSys;
}

Vector<Complex> DataSource::msData(Vector<Bool>& flag)
{
    uInt nRow = 1;
    Vector<Complex> data(nRow);
    data = Complex(0,0);
    //by default no useful data points
    flag = True;
    return data;
}

Matrix<Complex> DataSource::msData(Matrix<Bool>& flag, Int)
{
    //ignore 2nd argument ico
    uInt nRow = 1;
    uInt nCol = 1;
    Matrix<Complex> data(nRow,nCol);
    data = Complex(0,0);
    //by default no useful data points
    flag = True;
    return data;
}

Cube<Complex> DataSource::continuumData() const
{
    return Cube<Complex>();
}

Cube<Bool> DataSource::continuumFlag() const
{
    return Cube<Bool>();
}

Stokes::StokesTypes DataSource::corrType()
{
    return Stokes::Undefined;
}

Vector<Int> DataSource::corrTypes() const
{
    Vector<Int> types(1);
    types = Stokes::Undefined;
    return types;
}

uInt DataSource::chanNr()
{
    return 0;
}

uInt DataSource::bandNr()
{
    return 0;
}

Vector<Int> DataSource::msAntennaId(Int) const
{
    //ignore 2nd argument ico
    Vector<Int> id(2);
    id = -1;
    return id;
}

Vector<Double> DataSource::msTime() const
{
    Vector<Double> time(3);
    time = 0.0;
    return time;
}

Int DataSource::nextDataBlock()
{
    Int nRow = 0;
    return nRow;
}

Vector<Double> DataSource::msUVW(const uInt) const
{
    Vector<Double> UVW(3);
    UVW = 0.0;
    return UVW;
}

Int DataSource::nrOfPars() const
{
  Int npar = 0;
  return npar;
}

Vector<String> DataSource::parName() const
{
  Vector<String> name(nrOfPars());
  return name;
}

Vector<String> DataSource::parValue() const
{
  Vector<String> value(nrOfPars());
  return value;
}
