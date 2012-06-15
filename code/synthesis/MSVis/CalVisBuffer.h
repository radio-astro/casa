//# CalVisBuffer.h: A VisBuffer with extra stuff for calibration
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

#ifndef MSVIS_CALVISBUFFER_H
#define MSVIS_CALVISBUFFER_H

#include <casa/aips.h>
#include <synthesis/MSVis/VisBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//#forward

//<summary>CalVisBuffer extends VisBuffer to support storage and recall of associated residual and differentiated residual data. </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisBuffer">VisBuffer</linkto>
// </prerequisite>
//
// <etymology>
// CalVisBuffer is a VisBuffer for calibration solving
// </etymology>
//
//<synopsis>
// This class extends <linkto class="VisBuffer">VisBuffer to support 
// storage and recall of calibration solving-related information
// such as residuals (difference of trial-corrupted model data and
// observed data) and corresponding differentiated residuals.
// 
// (say something about synchronization with VisIter, etc.)
// 
//</synopsis>

//<todo>
// <li> write todo list
//</todo>
class CalVisBuffer : public VisBuffer
{
public:
  // Create empty VisBuffer you can assign to or attach.
  CalVisBuffer();
  // Construct VisBuffer for a particular VisibilityIterator
  // The buffer will remain synchronized with the iterator.
  CalVisBuffer(ROVisibilityIterator & iter);

  // Copy construct, looses synchronization with iterator: only use buffer for
  // current iteration (or reattach).
  CalVisBuffer(const CalVisBuffer& cvb);

  // Destructor (detaches from VisIter)
  ~CalVisBuffer(); 

  // Assignment, looses synchronization with iterator: only use buffer for 
  // current iteration (or reattach)
  CalVisBuffer& operator=(const VisBuffer& cvb);

  // Assignment, optionally without copying the data across; with copy=True
  // this is identical to normal assignment operator
  CalVisBuffer& assign(const VisBuffer& vb, Bool copy=True);

  // Update (simple) coord info 
  // (this OVERRIDES VisBuffer::updateCoordInfo(), which does more)
  void updateCoordInfo();

  // Apply amp-only or phase-only to data
  void enforceAPonData(const String& apmode);

  // Set the focus channel
  //  (forms references to focus-channel flag/data/model)
  void setFocusChan(const Int focusChan=-1);

  // Size/init/finalize the residuals workspaces
  void sizeResiduals(const Int& nPar,const Int& nDiff);
  void initResidWithModel();
  void finalizeResiduals();

  // Delete the workspaces
  void cleanUp();

  // <group>
  // Access functions
  //

  // Access to focus-channel slices of the flags, data, and model
  //  Cube<Bool>& infocusFlagCube() { return infocusFlagCube_p; }
  //  const Cube<Bool>& infocusFlagCube() const {return this->infocusFlagCube();}

  Matrix<Bool>& infocusFlag() { return infocusFlag_p; }
  const Matrix<Bool>& infocusFlag() const {return this->infocusFlag();}

  Cube<Complex>& infocusVisCube() { return infocusVisCube_p; }
  const Cube<Complex>& infocusVisCube() const {return this->infocusVisCube();}

  Cube<Complex>& infocusModelVisCube() { return infocusModelVisCube_p; }
  const Cube<Complex>& infocusModelVisCube() const {return this->infocusModelVisCube();}

  // Workspace for the residual visibilities
  Cube<Complex>& residuals() { return residuals_p; }
  const Cube<Complex>& residuals() const {return this->residuals();}

  // Workspace for flags of the residuals
  Matrix<Bool>& residFlag() { return residFlag_p; }
  const Matrix<Bool>& residFlag() const {return this->residFlag();}

  // Workspace for the differentiated residuals
  Array<Complex>& diffResiduals() { return diffResiduals_p; }
  const Array<Complex>& diffResiduals() const {return this->diffResiduals();}

  //</group>

private:

  // The current in-focus channel
  Int focusChan_p;

  // actual storage for the data
  //  Cube<Bool> infocusFlagCube_p;
  Matrix<Bool> infocusFlag_p;
  Cube<Complex> infocusVisCube_p;
  Cube<Complex> infocusModelVisCube_p;

  Cube<Complex> residuals_p;
  Matrix<Bool> residFlag_p;
  Array<Complex> diffResiduals_p;
};


} //# NAMESPACE CASA - END

#endif

