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
#include <casa/Arrays/Vector.h>
#include <measures/Measures/MDirection.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <scimath/Mathematics/FFTServer.h>
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
   * Separate side bands
   **/
  void separate(string outname);

  /**
   * Set IFNO and frequency tolerance to select data to process
   **/
  void setFrequency(const int ifno, const string freqtol,
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
  void setLO1(const string lo1, const string frame="TOPO",
	      const double reftime=-1, string refdir="");
  void setLO1Root(const string name);

private:
  /** Initialize member variables **/
  void init();
  void initshift();

  /** Return if the path exists (optionally, check file type) **/
  Bool checkFile(const string name, string type="");

  /** **/
  unsigned int setupShift();
  bool getFreqInfo(const CountedPtr<Scantable> &stab, const unsigned int &ifno,
		   double &freq0, double &incr, unsigned int &nchan);

  /** Grid scantable **/
  ScantableWrapper gridTable();
  void mapExtent(vector< CountedPtr<Scantable> > &tablist,
		 Double &xmin, Double &xmax,
		 Double &ymin, Double &ymax);

  /** 
   * Shift TIME in gridded scantable for future imaging
   * 
   * STGrid sets the identical time for all rows in scantable
   * which is reasonable thing to do in position based averaging.
   * However, this prevents CASA from finding proper pointing
   * per spectra once the gridded scantable is converted to
   * measurement set (MS). It is because MS does not
   * have ability to store per spectra pointing information.
   * MS stores pointing information in a subtable, POINTING,
   * with corresponding TIME when an antenna pointed the direction.
   * The pointing direction corresponding to a spectra is resolved
   * in MS by interpolating DIRECTION in POINTING subtable in TIME
   * the spectra is observed. If there are multiple match,
   * the first match is adopted. Therefore, gridded table (whose TIME
   * is set to a single value) is misunderstood in MS that all data
   * come from a single pointing.
   * The function workarounds this defect by artificially shifting
   * TIME by INTERVAL in each row.
   **/
  void shiftTimeInGriddedST(const CountedPtr<Scantable> &stab);
  /**
   * Actual calculation of frequencies of image sideband
   **/
  void solveImageFrequency();

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
  //  bool getSpectraToSolve(const int polId, const int beamId,
  //			 const double dirX, const double dirY,
  //			 Matrix<float> &specmat, vector<uInt> &tabIdvec);
  bool getSpectraToSolve(const int polId, const int beamId,
			 const double dirX, const double dirY,
			 Matrix<float> &specMat, Matrix<bool> &flagMat,
			 vector<uInt> &tabIdvec);

  vector<float> solve(const Matrix<float> &specMat,
		      const vector<uInt> &tabIdvec,
		      const bool signal = true);

  Vector<bool> collapseFlag(const Matrix<bool> &flagMat,
			    const vector<uInt> &tabIdvec,
			    const bool signal = true);

  void shiftSpectrum(const Vector<float> &invec, double shift,
		     Vector<float> &outvec);

  void shiftFlag(const Vector<bool> &invec, double shift,
		     Vector<bool> &outvec);

  void deconvolve(Matrix<float> &specmat, const vector<double> shiftvec,
		  const double threshold, Matrix<float> &outmat);

  void aggregateMat(Matrix<float> &inmat, vector<float> &outvec);

  void subtractFromOther(const Matrix<float> &shiftmat,
			 const vector<float> &invec,
			 const vector<double> &shift,
			 vector<float> &outvec);



  /** Member variables **/
  // input tables
  vector<string> infileList_;
  vector< CountedPtr<Scantable> > intabList_;
  unsigned int ntable_;
  // frequency and direction setup to select data.
  int sigIfno_;
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
  double lo1Freq_; // in Hz
  MFrequency::Types loFrame_;
  double loTime_;
  string loDir_;
  string asdmName_, asisName_;

  //CountedPtr<Scantable> imgTab_p, sigTab_p;
  CountedPtr<Scantable> imgTab_p, sigTab_p;
  Table::TableType tp_;
  FFTServer<Float, Complex> fftsf, fftsi;

}; // class

} // namespace

#endif
