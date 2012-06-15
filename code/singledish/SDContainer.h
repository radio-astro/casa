//#---------------------------------------------------------------------------
//# SDContainer.h: A container class for single dish integrations
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
#ifndef SDCONTAINER_H
#define SDCONTAINER_H

#include <vector>

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <measures/Measures/MDirection.h>

//template<class T> class casa::Matrix;

namespace asap {


struct SDHeader {
  casa::Int nchan;
  casa::Int npol;
  casa::Int nif;
  casa::Int nbeam;
  casa::String observer;
  casa::String project;
  casa::String obstype;
  casa::String antennaname;
  casa::Vector<casa::Double> antennaposition;
  casa::Float equinox;
  casa::String freqref;
  casa::Double reffreq;
  casa::Double bandwidth;
  casa::Double utc;
  casa::String fluxunit;
  casa::String epoch;
  void print() const ;
};

class SDFrequencyTable {

public:

  SDFrequencyTable() : nFreq_(0) {;}
  virtual ~SDFrequencyTable() {;}

  // Add a new entry or match an existing one. Returns the index into
  // the table
  casa::uInt addFrequency(casa::Double refPix, casa::Double refVal,
                          casa::Double inc);

  casa::Int length() const { return nFreq_;}  // # of stored Frequencies
  void setLength(casa::uInt length) {nFreq_ = length;}

  // Get attributes
  casa::Double referencePixel(casa::uInt which) const { return refPix_[which];}
  casa::Double referenceValue(casa::uInt which) const { return refVal_[which];}
  casa::Double increment(casa::uInt which) const { return increment_[which];}
  casa::Float equinox() const { return equinox_; }
  casa::String refFrame() const { return refFrame_; }
  casa::String baseRefFrame() const { return baseRefFrame_; }
  casa::String unit() const { return unit_; }

  void restFrequencies(casa::Vector<casa::Double>& rfs,
                       casa::String& rfunit ) const ;

  // Set attributes
  void setEquinox(casa::Float eq) { equinox_ = eq; }
  void setRefFrame(const casa::String& reff) { refFrame_ = reff; }
  void setBaseRefFrame(const casa::String& reff) { baseRefFrame_ = reff; }
  void setUnit(const casa::String& un) { unit_= un; }

  void deleteRestFrequencies() {restFreqs_.resize(0);}
  casa::uInt addRestFrequency(casa::Double);
  void setRestFrequencyUnit(const casa::String& theunit)
  { restFreqUnit_ = theunit;}

private:
  casa::uInt nFreq_;
  casa::Vector<casa::Double> refPix_;
  casa::Vector<casa::Double> refVal_;           // Hz
  casa::Vector<casa::Double> increment_;        // Hz
  casa::Float equinox_;
  casa::String unit_;
  casa::String refFrame_;
  casa::String baseRefFrame_;
  casa::Vector<casa::Double> restFreqs_;        // Hz
  casa::String restFreqUnit_;
};


class SDContainer {

public:
  SDContainer(casa::uInt nBeam, casa::uInt nIF, casa::uInt nPol,
              casa::uInt nChan);
  SDContainer(casa::IPosition shp);

  virtual ~SDContainer();

  casa::Bool resize(casa::IPosition shp);

  casa::Bool setSpectrum(const casa::Matrix<casa::Float>& spec,
                   casa::uInt whichBeam, casa::uInt whichIF);
  casa::Bool setSpectrum(const casa::Matrix<casa::Float>& spec,
                         const casa::Vector<casa::Complex>& cSpec,
                         casa::uInt whichBeam, casa::uInt whichIF);
  casa::Bool putSpectrum(const casa::Array<casa::Float>& spec);

  casa::Bool setFlags(const casa::Matrix<casa::uChar>& flgs,
                      casa::uInt whichBeam, casa::uInt whichIF,
                      casa::Bool hasXPol=casa::False);
  casa::Bool putFlags(const casa::Array<casa::uChar>& spec);

  casa::Bool setTsys(const casa::Vector<casa::Float>& ts,
               casa::uInt whichBeam, casa::uInt whichIF,
               casa::Bool hasXpol);
  casa::Bool putTsys(const casa::Array<casa::Float>& spec);

  casa::Bool setDirection(const casa::Vector<casa::Double>& point,
                          casa::uInt whichBeam);
  casa::Bool putDirection(const casa::Array<casa::Double>& dir);

  casa::Bool setFrequencyMap(casa::uInt freqslot, casa::uInt whichIF);
  casa::Bool putFreqMap(const casa::Vector<casa::uInt>& freqs);

  casa::Bool setRestFrequencyMap(casa::uInt freqslot, casa::uInt whichIF);
  casa::Bool putRestFreqMap(const casa::Vector<casa::uInt>& freqs);

  casa::Array<casa::Float> getSpectrum(casa::uInt whichBeam,
                                       casa::uInt whichIF);
  casa::Array<casa::uChar> getFlags(casa::uInt whichBeam,
                                    casa::uInt whichIF);
  casa::Array<casa::Float> getTsys(casa::uInt whichBeam,
                                   casa::uInt whichIF);
  casa::Array<casa::Double> getDirection(casa::uInt whichBeam) const;

  const casa::Array<casa::Float>& getSpectrum() const { return spectrum_; }
  const casa::Array<casa::uChar>& getFlags() const { return flags_; }
  const casa::Array<casa::Float>& getTsys() const { return tsys_; }
  const casa::Array<casa::Double>& getDirection() const { return direction_; }

  const casa::Vector<casa::uInt>& getFreqMap() const { return freqidx_; }
  const casa::Vector<casa::uInt>& getRestFreqMap() const
  { return restfreqidx_; }

  casa::Bool putFitMap(const casa::Array<casa::Int>& arr);

  const casa::Array<casa::Int>& getFitMap() const { return fitIDMap_; }


  casa::Double timestamp;
  //Double bandwidth;
  casa::String sourcename;
  casa::String fieldname;
  casa::Double interval;
  casa::Int scanid;
  casa::Vector<casa::Float> tcal;
  casa::String tcaltime;
  casa::Float azimuth;
  casa::Float elevation;
  casa::Float parangle;
  casa::Int refbeam;

private:
  casa::uInt nBeam_,nIF_,nPol_,nChan_;

  // (nBeam,nIF,nPol,nChannel)
  casa::Array<casa::Float>    spectrum_;
  casa::Array<casa::uChar>    flags_;

  // (nBeam,nIF,nPol,[nChannel]) Tsys is not really a function of
  // channel, but this makes it easier to work with at the expense of
  // a little memory
  casa::Array<casa::Float>    tsys_;
  casa::Array<casa::Float>    tcal_;

  //(nIF) indx into "global" frequency table
  casa::Vector<casa::uInt>    freqidx_;

 // (nIF) indx into "global" rest frequency table
  casa::Vector<casa::uInt>    restfreqidx_;

  //(nBeam,2) maybe use Measures here...
  casa::Array<casa::Double>   direction_;
  casa::Array<casa::Int> fitIDMap_;

  void setSlice(casa::IPosition& start, casa::IPosition& end,
                const casa::IPosition& shpIn, const casa::IPosition& shpOut,
                casa::uInt whichBeam, casa::uInt whichIF, casa::Bool checkPol,
                casa::Bool xPol) const;
  void setSlice(casa::IPosition& start, casa::IPosition& end,
                const casa::IPosition& shape,
                casa::uInt whichBeam, casa::uInt whichIF) const;
};



class SDDataDesc {

public:

  // Constructor
  SDDataDesc() : n_(0) {;}
  ~SDDataDesc() {;}

  // Add an entry if source name and Integer ID (can be anything you
  // like, such as FreqID) are unique.  You can add secondary entries
  // direction and another integer index which are just stored along
  // with the the primary entries
  casa::uInt addEntry(const casa::String& source, casa::uInt ID,
                      const casa::MDirection& secDir, casa::uInt secID);

  // Number of entries
  casa::Int length() const { return n_;}

  // Get attributes
  casa::String source(casa::uInt which) const {return source_[which];}
  casa::uInt ID(casa::uInt which) const {return ID_[which];}
  casa::uInt secID(casa::uInt which) const {return secID_[which];}
  casa::MDirection secDir(casa::uInt which) const {return secDir_[which];}

  // Summary
  void summary() const;

private:
  casa::uInt n_;
  casa::Vector<casa::String> source_;
  casa::Vector<casa::uInt> ID_, secID_;
  casa::Block<casa::MDirection> secDir_;

  SDDataDesc(const SDDataDesc& other);

};


} // namespace
#endif
