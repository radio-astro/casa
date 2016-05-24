//# SolveDataBuffer.h: A container for data and residuals for solving
//# Copyright (C) 2008
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
//# $Id: VisBuffer.h,v 19.14 2006/02/28 04:48:58 mvoronko Exp $

#ifndef SYNTHESIS_SOLVEDATABUFFER_H
#define SYNTHESIS_SOLVEDATABUFFER_H

#include <casa/aips.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
namespace casa { //# NAMESPACE CASA - BEGIN

//#forward

//<summary>SolveDataBuffer is a container for VisBuffer2 data and related residual and differentiation results related to generic calibration solving </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisBuffer">VisBuffer</linkto>
// </prerequisite>
//
// <etymology>
// SolveDataBuffer is a Buffer for Solving Data
// </etymology>
//
//<synopsis>
// 
// 
//</synopsis>

//<todo>
// <li> write todo list
//</todo>



class SolveDataBuffer 
{
public:
  // Generic ctor
  SolveDataBuffer();

  // Create from a VisBuffer2
  SolveDataBuffer(const vi::VisBuffer2& vb);

  // Copy construct
  // current iteration (or reattach).
  SolveDataBuffer(const SolveDataBuffer& sdb);

  // Destructor 
  ~SolveDataBuffer(); 

  // Assignment
  SolveDataBuffer& operator=(const SolveDataBuffer& sdb);

  // Apply amp-only or phase-only to data
  void enforceAPonData(const String& apmode);

  // Zero flagged weights, and optionally the cross-hand weights
  void enforceSolveWeights(const Bool pHandOnly);

  // Set the focus channel
  //  (forms references to focus-channel flag/data/model)
  void setFocusChan(const Int focusChan=-1);

  // Size/init/finalize the residuals workspaces
  void sizeResiduals(const Int& nPar,const Int& nDiff);
  void initResidWithModel();
  void finalizeResiduals();

  // Delete the workspaces
  void cleanUp();


  // VB2-like data access methods (mostly const)
  Int nRows() const { return vb_->nRows(); };
  const Vector<Int>& arrayId() const { return vb_->arrayId(); };
  const Vector<Int>& antenna1() const { return vb_->antenna1(); };
  const Vector<Int>& antenna2() const { return vb_->antenna2(); };
  const Vector<Int>& dataDescriptionIds() const { return vb_->dataDescriptionIds(); };
  const Vector<Int>& spectralWindow() const { return vb_->spectralWindows(); };
  const Vector<Int>& scan() const { return vb_->scan(); };
  const Vector<Double>& time() const { return vb_->time(); };
  const Vector<Int>& fieldId() const { return vb_->fieldId(); };
  Int nChannels() const { return vb_->nChannels(); };
  Int nCorrelations() const { return vb_->nCorrelations(); };
  const Cube<Complex>& visCubeModel() const { return vb_->visCubeModel(); };
  const Cube<Complex>& visCubeCorrected() const { return vb_->visCubeCorrected(); };
  // These are not const, because we will generally amend them:
  Vector<Bool>& flagRow() {fR_.reference(vb_->flagRow());return fR_;};
  Cube<Bool>& flagCube() {fC_.reference(vb_->flagCube());return fC_;};
  Cube<Float>& weightSpectrum() {wS_.reference(vb_->weightSpectrum());return wS_;};

  // <group>

  // Access functions
  //
  // Access to focus-channel slices of the flags, data, and model
  Cube<Bool>& infocusFlagCube() { return infocusFlagCube_p; }
  const Cube<Bool>& infocusFlagCube() const {return this->infocusFlagCube();}

  Cube<Complex>& infocusVisCube() { return infocusVisCube_p; }
  const Cube<Complex>& infocusVisCube() const {return this->infocusVisCube();}

  Cube<Float>& infocusWtSpec() { return infocusWtSpec_p; }
  const Cube<Float>& infocusWtSpec() const {return this->infocusWtSpec();}

  Cube<Complex>& infocusModelVisCube() { return infocusModelVisCube_p; }
  const Cube<Complex>& infocusModelVisCube() const {return this->infocusModelVisCube();}

  // Workspace for the residual visibilities
  Cube<Complex>& residuals() { return residuals_p; }
  const Cube<Complex>& residuals() const {return this->residuals();}

  // Workspace for flags of the residuals
  Cube<Bool>& residFlagCube() { return residFlagCube_p; }
  const Cube<Bool>& residFlagCube() const {return this->residFlagCube();}

  // Workspace for the differentiated residuals
  Array<Complex>& diffResiduals() { return diffResiduals_p; }
  const Array<Complex>& diffResiduals() const {return this->diffResiduals();}

  //</group>

protected:

  // Handle copy from the input VB2
  void initFromVB(const vi::VisBuffer2& vb);


private:

  // The underlying VisBuffer2
  vi::VisBuffer2* vb_;
  
  // Array reference objects for things in the vb we need to mess with
  Vector<Bool> fR_;
  Cube<Bool> fC_;
  Cube<Float> wS_;

  // The current in-focus channel
  Int focusChan_p;

  // actual storage for the data
  Cube<Bool> infocusFlagCube_p;
  Cube<Float> infocusWtSpec_p;
  Cube<Complex> infocusVisCube_p;
  Cube<Complex> infocusModelVisCube_p;

  Cube<Complex> residuals_p;
  Cube<Bool> residFlagCube_p;
  Array<Complex> diffResiduals_p;
};


class SDBList 
{
public:

  // Construct empty list
  SDBList();

  // Destructor
  ~SDBList();

  // How many SDBs?
  Int nSDB() const { return nSDB_; };

  // Generate a new SDB from an input VB2
  void add(const vi::VisBuffer2& vb);

  // Access an SDB by index
  SolveDataBuffer& operator()(Int i);

  // Aggregated methods
  void enforceAPonData(const String& apmode);
  void enforceSolveWeights(const Bool pHandOnly);
  void sizeResiduals(const Int& nPar, const Int& nDiff);
  void initResidWithModel();
  void finalizeResiduals();

private:

  // How many SDBs contained herein
  Int nSDB_;

  // Keep SDBs as a list of pointers
  PtrBlock<SolveDataBuffer*> SDB_;

};



} //# NAMESPACE CASA - END

#endif

