//#---------------------------------------------------------------------------
//# SDContainer.cc: A container class for single dish integrations
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id:
//#---------------------------------------------------------------------------

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Exceptions.h>
#include <casa/Utilities/Assert.h>
#include <tables/Tables/Table.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicMath/Math.h>
#include <casa/Quanta/MVTime.h>



#include <singledish/SDDefs.h>
#include <singledish/SDContainer.h>

using namespace casa;
using namespace asap;

void SDHeader::print() const {
  MVTime mvt(this->utc);
  mvt.setFormat(MVTime::YMD);
  cout << "Observer: " << this->observer << endl 
       << "Project: " << this->project << endl
       << "Obstype: " << this->obstype << endl
       << "Antenna: " << this->antennaname << endl
       << "Ant. Position: " << this->antennaposition << endl
       << "Equinox: " << this->equinox << endl
       << "Freq. ref.: " << this->freqref << endl
       << "Ref. frequency: " << this->reffreq << endl
       << "Bandwidth: "  << this->bandwidth << endl
       << "Time (utc): " 
       << mvt
       << endl;
  //setprecision(10) << this->utc << endl;
}


SDContainer::SDContainer(uInt nBeam, uInt nIF, uInt nPol, uInt nChan) 
  : nBeam_(nBeam),
    nIF_(nIF),
    nPol_(nPol),
    nChan_(nChan),
    spectrum_(IPosition(4,nBeam,nIF,nPol,nChan)),
    flags_(IPosition(4,nBeam,nIF,nPol,nChan)),
    tsys_(IPosition(4,nBeam,nIF,nPol,nChan)),
    freqidx_(nIF),
    restfreqidx_(nIF),
    direction_(IPosition(2,nBeam,2)) {
  uChar x = 0;
  flags_ = ~x;
  tcal.resize(2);
}

SDContainer::SDContainer(IPosition shp) 
  : nBeam_(shp(asap::BeamAxis)),
    nIF_(shp(asap::IFAxis)),
    nPol_(shp(asap::PolAxis)),
    nChan_(shp(asap::ChanAxis)),
    spectrum_(shp),
    flags_(shp),
    tsys_(shp),
    freqidx_(shp(asap::IFAxis)),
    restfreqidx_(shp(asap::IFAxis)) {
  IPosition ip(2,shp(asap::BeamAxis),2);
  direction_.resize(ip);
  uChar x = 0;
  flags_ = ~x;
  tcal.resize(2);
}

SDContainer::~SDContainer() {
}

Bool SDContainer::resize(IPosition shp) {
  nBeam_ = shp(asap::BeamAxis);
  nIF_ = shp(asap::IFAxis);
  nPol_ = shp(asap::PolAxis);
  nChan_ = shp(asap::ChanAxis);
  spectrum_.resize(shp);
  flags_.resize(shp);
  tsys_.resize(shp);
  freqidx_.resize(shp(asap::IFAxis));
  restfreqidx_.resize(shp(asap::IFAxis));
  IPosition ip(2,shp(asap::BeamAxis),2);
  direction_.resize(ip);
}

Bool SDContainer::putSpectrum(const Array<Float>& spec) {
  spectrum_ = spec;
}
Bool SDContainer::putFlags(const Array<uChar>& flag) {
  flags_ = flag;
}
Bool SDContainer::putTsys(const Array<Float>& tsys) {
  tsys_ = tsys;
}

Bool SDContainer::setSpectrum(const Matrix<Float>& spec,
                              const Vector<Complex>& cSpec,
			      uInt whichBeam, uInt whichIF) 
//
// spec is [nChan,nPol] 
// cspec is [nChan]
// spectrum_ is [,,,nChan]
//
// nPOl_ = 4  - xx, yy, real(xy), imag(xy)
//
{
  AlwaysAssert(nPol_==4,AipsError);

// Get slice  and check dim

  Bool tSys = False;
  Bool xPol = True;
  IPosition start, end;
  setSlice(start, end, spec.shape(), spectrum_.shape(),
            whichBeam, whichIF, tSys, xPol);

// Get a reference to the Pol/Chan slice we are interested in

  Array<Float> subArr = spectrum_(start,end);

// Iterate through pol-chan plane and fill

  ReadOnlyVectorIterator<Float> inIt(spec,0);
  VectorIterator<Float> outIt(subArr,asap::ChanAxis);
  while (!outIt.pastEnd()) {
     const IPosition& pos = outIt.pos();
     if (pos(asap::PolAxis)<2) {
        outIt.vector() = inIt.vector();
        inIt.next();
     } else if (pos(asap::PolAxis)==2) {
        outIt.vector() = real(cSpec);
     } else if (pos(asap::PolAxis)==3) {
        outIt.vector() = imag(cSpec);
     }
//
     outIt.next();
  }

  // unset flags for this spectrum, they might be set again by the
  // setFlags method

  Array<uChar> arr(flags_(start,end));
  arr = uChar(0);
//
  return True;
}


Bool SDContainer::setSpectrum(const Matrix<Float>& spec,
			      uInt whichBeam, uInt whichIF) 
//
// spec is [nChan,nPol] 
// spectrum_ is [,,,nChan]
// How annoying.
{

// Get slice and check dim

  IPosition start, end;
  setSlice(start, end, spec.shape(), spectrum_.shape(),
            whichBeam, whichIF, False, False);

// Get a reference to the Pol/Chan slice we are interested in

  Array<Float> subArr = spectrum_(start,end);

// Iterate through it and fill

  ReadOnlyVectorIterator<Float> inIt(spec,0);
  VectorIterator<Float> outIt(subArr,asap::ChanAxis);
  while (!outIt.pastEnd()) {
     outIt.vector() = inIt.vector();
//
     inIt.next();
     outIt.next();
  }

  // unset flags for this spectrum, they might be set again by the
  // setFlags method

  Array<uChar> arr(flags_(start,end));
  arr = uChar(0);
//
  return True;
}

Bool SDContainer::setFlags(const Matrix<uChar>& flags,
			   uInt whichBeam, uInt whichIF,
			   Bool hasXPol)
//
// flags is [nChan,nPol] 
// flags_ is [nBeam,nIF,nPol,nChan]
//
// Note that there are no separate flags for the XY cross polarization so we make
// them up from the X and Y which is all the silly code below.  This means
// that nPol==2 on input but 4 on output
//
{
  if (hasXPol) AlwaysAssert(nPol_==4,AipsError);

// Get slice and check dim

  IPosition start, end;
  setSlice(start, end, flags.shape(), flags_.shape(),
            whichBeam, whichIF, False, hasXPol);

// Get a reference to the Pol/Chan slice we are interested in

  Array<uChar> subArr = flags_(start,end);

// Iterate through pol/chan plane  and fill

  ReadOnlyVectorIterator<uChar> inIt(flags,0);
  VectorIterator<uChar> outIt(subArr,asap::ChanAxis);
//
  Vector<uChar> maskPol0;
  Vector<uChar> maskPol1;
  Vector<uChar> maskPol01;
  while (!outIt.pastEnd()) {
     const IPosition& pos = outIt.pos();
     if (pos(asap::PolAxis)<2) {
        outIt.vector() = inIt.vector();
//
        if (hasXPol) {
           if (pos(asap::PolAxis)==0) {
              maskPol0 = inIt.vector();
           } else if (pos(asap::PolAxis)==1) {
              maskPol1 = inIt.vector();
//
              maskPol01.resize(maskPol0.nelements());
              for (uInt i=0; i<maskPol01.nelements(); i++) maskPol01[i] = maskPol0[i]&maskPol1[i];
           }
        }
        inIt.next();
     } else if (pos(asap::PolAxis)==2) {
        if (hasXPol) {
           outIt.vector() = maskPol01;
        } else {
           outIt.vector() = inIt.vector();
           inIt.next();
        }

     } else if (pos(asap::PolAxis)==3) { 
        if (hasXPol) {
           outIt.vector() = maskPol01;
        } else {
           outIt.vector() = inIt.vector();
           inIt.next();
        }
     }
     outIt.next();
  }
//
  return True;
}


Bool SDContainer::setTsys(const Vector<Float>& tsys,
			  uInt whichBeam, uInt whichIF,
                          Bool hasXpol)
//
// TSys shape is [nPol]
// Tsys does not depend upon channel but is replicated
// for simplicity of use.
// There is no Tsys measurement for the cross polarization
// so I have set TSys for XY to sqrt(Tx*Ty)
//
{

// Get slice and check dim

  IPosition start, end;
  setSlice(start, end, tsys.shape(), tsys_.shape(),
            whichBeam, whichIF, True, hasXpol);

// Get a reference to the Pol/Chan slice we are interested in

  Array<Float> subArr = tsys_(start,end);

// Iterate through it and fill

  VectorIterator<Float> outIt(subArr,asap::ChanAxis);
  uInt i=0;
  while (!outIt.pastEnd()) {
     const IPosition& pos = outIt.pos();
//
     if (pos(asap::PolAxis)<2) {
       outIt.vector() = tsys(i++);
     } else {
       if (hasXpol) {
          outIt.vector() = sqrt(tsys[0]*tsys[1]);
       } else {
          outIt.vector() = tsys(i++);
       }
     }
//
     outIt.next();
  }
}

Array<Float> SDContainer::getSpectrum(uInt whichBeam, uInt whichIF)
//
// non-const function because of Array(start,end) slicer
//
// Input  [nBeam,nIF,nPol,nChan]
// Output [nChan,nPol]
//
{

// Get reference to slice and check dim

  IPosition start, end;
  setSlice(start, end, spectrum_.shape(), whichBeam, whichIF);
//
  Array<Float> dataIn = spectrum_(start,end);
  Array<Float> dataOut(IPosition(2, nChan_, nPol_));
//
  ReadOnlyVectorIterator<Float> itIn(dataIn, asap::ChanAxis);
  VectorIterator<Float> itOut(dataOut, 0);
  while (!itOut.pastEnd()) {
     itOut.vector() = itIn.vector();
//
     itIn.next();
     itOut.next();
  }
//
  return dataOut.copy(); 
}

Array<uChar> SDContainer::getFlags(uInt whichBeam, uInt whichIF)
//
// non-const function because of Array(start,end) slicer
//
// Input  [nBeam,nIF,nPol,nChan]
// Output [nChan,nPol]
//
{

// Get reference to slice and check dim

  IPosition start, end;
  setSlice(start, end, flags_.shape(), whichBeam, whichIF);
//
  Array<uChar> dataIn = flags_(start,end);
  Array<uChar> dataOut(IPosition(2, nChan_, nPol_));
//
  ReadOnlyVectorIterator<uChar> itIn(dataIn, asap::ChanAxis);
  VectorIterator<uChar> itOut(dataOut, 0);
  while (!itOut.pastEnd()) {
     itOut.vector() = itIn.vector();
//
     itIn.next();
     itOut.next();
  }
//
  return dataOut.copy();
}

Array<Float> SDContainer::getTsys(uInt whichBeam, uInt whichIF) 
//
// Input  [nBeam,nIF,nPol,nChan]
// Output [nPol]   (drop channel dependency and select first value only)
// 
{
// Get reference to slice and check dim

  IPosition start, end;
  setSlice(start, end, spectrum_.shape(), whichBeam, whichIF);
//
  Array<Float> dataIn = tsys_(start,end);
  Vector<Float> dataOut(nPol_);
//
  ReadOnlyVectorIterator<Float> itIn(dataIn, asap::ChanAxis);
  VectorIterator<Float> itOut(dataOut, 0); 
  uInt i = 0;
  while (!itIn.pastEnd()) {
    dataOut[i++] = itIn.vector()[0];
    itIn.next();
  }
//
  return dataOut.copy();
}



Array<Double> SDContainer::getDirection(uInt whichBeam) const 
//
// Input [nBeam,2]
// Output [nBeam]
//
{
  Vector<Double> dataOut(2);
  dataOut(0) = direction_(IPosition(2,whichBeam,0));
  dataOut(1) = direction_(IPosition(2,whichBeam,1));
  return dataOut.copy();
}


Bool SDContainer::setFrequencyMap(uInt freqID, uInt whichIF) {
  freqidx_[whichIF] = freqID;
  return True;
}

Bool SDContainer::putFreqMap(const Vector<uInt>& freqs) {
  freqidx_.resize();
  freqidx_ = freqs;
  return True;
}

Bool SDContainer::setRestFrequencyMap(uInt freqID, uInt whichIF) {
  restfreqidx_[whichIF] = freqID;
  return True;
}

Bool SDContainer::putRestFreqMap(const Vector<uInt>& freqs) {
  restfreqidx_.resize();
  restfreqidx_ = freqs;
  return True;
}

Bool SDContainer::setDirection(const Vector<Double>& point, uInt whichBeam) 
//
// Input [2]
// Output [nBeam,2]
//
{
  if (point.nelements() != 2) return False;
//
  Vector<Double> dataOut(2);
  direction_(IPosition(2,whichBeam,0)) = point[0];
  direction_(IPosition(2,whichBeam,1)) = point[1];
  return True;
}


Bool SDContainer::putDirection(const Array<Double>& dir) {
  direction_.resize();
  direction_ = dir;
  return True;
}

Bool SDContainer::putFitMap(const Array<Int>& arr) {
  fitIDMap_.resize();
  fitIDMap_ = arr;
  return True;
}

void SDContainer::setSlice(IPosition& start, IPosition& end,
			   const IPosition& shpIn, const IPosition& shpOut,
			   uInt whichBeam, uInt whichIF, Bool tSys,
			   Bool xPol) const
//
// tSYs
//   shpIn [nPol]
// else
//   shpIn [nCHan,nPol]
//
// if xPol present, the output nPol = 4 but
// the input spectra are nPol=2 (tSys) or nPol=2 (spectra)
//
{
  AlwaysAssert(asap::nAxes==4,AipsError);
  uInt pOff = 0;
  if (xPol) pOff += 2;
  if (tSys) {
     AlwaysAssert(shpOut(asap::PolAxis)==shpIn(0)+pOff,AipsError);     // pol
  } else {
     AlwaysAssert(shpOut(asap::ChanAxis)==shpIn(0),AipsError);       // chan
     AlwaysAssert(shpOut(asap::PolAxis)==shpIn(1)+pOff,AipsError);   // pol
  }
//
  setSlice(start, end, shpOut, whichBeam, whichIF);
}


void SDContainer::setSlice(IPosition& start, IPosition& end, 
			   const IPosition& shape, 
			   uInt whichBeam, uInt whichIF) const
{
  AlwaysAssert(asap::nAxes==4,AipsError);
  //
  start.resize(asap::nAxes);
  start = 0;
  start(asap::BeamAxis) = whichBeam;
  start(asap::IFAxis) = whichIF;
//
  end.resize(asap::nAxes);
  end = shape-1;
  end(asap::BeamAxis) = whichBeam;
  end(asap::IFAxis) = whichIF;
}


uInt SDFrequencyTable::addFrequency(Double refPix, Double refVal, Double inc) 
{
  if (length() > 0) {
    for (uInt i=0; i< length();i++) {
      if (near(refVal,refVal_[i]) && 
          near(refPix,refPix_[i]) && 
          near(inc,increment_[i])) {
         return i;
      }
    }
  }

// Not found - add it

  nFreq_ += 1;
  refPix_.resize(nFreq_,True);
  refVal_.resize(nFreq_,True);
  increment_.resize(nFreq_,True);
  refPix_[nFreq_-1] = refPix;
  refVal_[nFreq_-1] = refVal;
  increment_[nFreq_-1] = inc;
  return nFreq_-1;
}

uInt SDFrequencyTable::addRestFrequency(Double val)
{
  uInt nFreq = restFreqs_.nelements();
  if (nFreq>0) {
    for (uInt i=0; i<nFreq;i++) {
      if (near(restFreqs_[i],val)) {
         return i;
      }
    }
  }

// Not found - add it

  nFreq += 1;
  restFreqs_.resize(nFreq,True);
  restFreqs_[nFreq-1] = val;
  return nFreq-1;
}


void SDFrequencyTable::restFrequencies(Vector<Double>& rfs, 
				       String& rfunit ) const
{
  rfs.resize(restFreqs_.nelements());
  rfs = restFreqs_;
  rfunit = restFreqUnit_;
}

// SDDataDesc

uInt SDDataDesc::addEntry(const String& source, uInt ID, 
			  const MDirection& dir, uInt secID)
{

// See if already exists

  if (n_ > 0) {
    for (uInt i=0; i<n_; i++) {
      if (source==source_[i] && ID==ID_[i]) {
         return i;
      }
    }
  }

// Not found - add it

  n_ += 1;
  source_.resize(n_,True);
  ID_.resize(n_,True);
  secID_.resize(n_,True);
  secDir_.resize(n_,True,True);
//
  source_[n_-1] = source;
  ID_[n_-1] = ID;
  secID_[n_-1] = secID;
  secDir_[n_-1] = dir;
//
  return n_-1;
}

void SDDataDesc::summary() const
{
   if (n_>0) {
      cerr << "Source    ID" << endl;   
      for (uInt i=0; i<n_; i++) {
         cout << setw(11) << source_(i) << ID_(i) << endl;
      }
   }
}

