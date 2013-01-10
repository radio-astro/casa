// C++ Interface: STSideBandSep
//
// Description:
//    A class to invoke sideband separation of Scantable
//
// Author: Kanako Sugimoto <kana.sugi@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSIDEBANDSEP_H
#define ASAPSIDEBANDSEP_H

// STL
#include <iostream>
#include <string>
#include <vector>
// casacore
#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <measures/Measures/MDirection.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
// asap
#include "ScantableWrapper.h"
#include "Scantable.h"

using namespace std;
using namespace casa;

namespace asap {

class STSideBandSep {
public:
  /**
   * constructors and a destructor
   **/
  STSideBandSep() { throw( AipsError("No data set to process") ); };
  explicit STSideBandSep(const vector<string> &names);
  explicit STSideBandSep(const vector<ScantableWrapper> &tables);
  virtual ~STSideBandSep();

  /**
   * Set IFNO and frequency tolerance to select data to process
   **/
  void setFrequency(const unsigned int ifno, const string freqtol,
		    const string frame="");

  /**
   * Set direction tolerance to group spectra.
   * The spectra within this range will be averaged before procesing.
   **/
  void setDirTolerance(const vector<string> dirtol);

  /**
   * Set the number of channels shifted in image side band 
   * of each of scantable.
   **/
  void setShift(const vector<double> &shift);

  /**
   * Set rejection limit of solution.
   **/
  void setThreshold(const double limit);

  /**
   * Resolve both image and signal sideband when true is set.
   **/
  void solveBoth(const bool flag) { doboth_ = flag; };

  /**
   * Obtain spectra by subtracting the solution of the other sideband.
   **/
  void solvefromOther(const bool flag) { otherside_ = flag; };

  /**
   * Set scantable to fill frequencies of image sideband (temporal)
   **/
  void setImageTable(const ScantableWrapper &s);
  void setScanTb0(const ScantableWrapper &s);

  /**
   * Set additional information to fill frequencies of image sideband
   **/
  void setLO1(const double lo1, const string frame="TOPO",
	      const double reftime=-1, string refdir="");
  void setLO1Root(const string name);

  /**
   * Actual calculation of frequencies of image sideband
   **/
  void solveImageFreqency();

private:
  /** Initialize member variables **/
  void init();
  void initshift();

  /** Return if the path exists (optionally, check file type) **/
  Bool checkFile(const string name, string type="");

  /** 
   * Get LO1 frequency to solve the frequencies of image side band
   **/
  bool getLo1FromAsdm(const string asdmname,
		      const double refval, const double refpix,
		      const double increment, const int nChan);
  bool getLo1FromAsisTab(const string msname,
			 const double refval, const double refpix,
			 const double increment, const int nChan);
  bool getLo1FromScanTab(casa::CountedPtr< Scantable > &scantab,
			 const double refval, const double refpix,
			 const double increment, const int nChan);

  /** Member variables **/
  // input tables
  vector<string> infileList_;
  vector< CountedPtr<Scantable> > intabList_;
  unsigned int ntable_;
  // frequency and direction setup to select data.
  unsigned int sigIfno_;
  Quantum<Double> ftol_;
  MFrequency::Types solFrame_;
  vector<double> sigShift_, imgShift_;
  unsigned int nshift_, nchan_;
  vector< CountedPtr<Scantable> > tableList_;
  Double xtol_, ytol_;
  // solution parameters
  bool otherside_, doboth_;
  double rejlimit_;
  // LO1
  double lo1Freq_;
  MFrequency::Types loFrame_;
  double loTime_;
  string loDir_;
  string asdmName_, asisName_;

  CountedPtr<Scantable> imgTab_p, sigTab_p;
  // TEMPORAL member
  CountedPtr<Scantable> st0_;

}; // class

} // namespace

#endif
