//# VisBuffGroup.h: class to store a group of VisBuffers
//# Copyright (C) 2011
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

#ifndef MSVIS_VISBUFFGROUP_H
#define MSVIS_VISBUFFGROUP_H

#include <casa/aips.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/Arrays/Vector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// A class to store a group of VisBuffers.
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> VisBuffer
// </prerequisite>
//
// <etymology>
//   It is a group of VisBuffers
// </etymology>
//
// <synopsis>
// This class stores a group of VisBuffers.  Unlike VisBuffGroupAcc or
// VisBuffAccumulator, the VisBuffers are copied and stored in a nearly
// pristine state so that their individual contents can be accessed later
// (i.e. without any averaging).  The only difference between a normal (or
// "fresh") VisBuffer and one retrieved from a VisBuffGroup is that
// VisBuffGroup detaches its VisBuffers from any underlying VisIter.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Some calculations or applications need more than one VisBuffer.  For
// example, one might want to estimate and subtract the continuum from the
// visibilities of an MS that has a broad line completely spanning spw 1, but
// spws 0 and 2 line-free, so the spws should be combined (combine='spw') for
// the continuum estimation.
//
// It is much more efficient if the group of necessary data can be read only
// once, worked on, and then written.  The CalTable approach is more flexible
// in that a CalTable can be applied to an MS with a different number or
// arrangement of rows from the input MS, but per chunk it requires two more
// reads (the CalTable and the _output_ MS) and an extra write (the CalTable).
// </motivation>
//
// <todo asof="2011/11/07">
//   <li> Allow retrieval by timestamp.
// </todo>

class VisBuffGroup
{
public:
  VisBuffGroup();

  // Null destructor
  ~VisBuffGroup();

  // Add a VisBuffer.  Returns True on success and False on failure.
  Bool store(const VisBuffer& vb);

  // Record the end of a chunk.
  // Doing so marks that if "playing back" a VisIter vi matching the order that
  // the VBs were stored in, vi.nextChunk() will be needed at this point
  // instead of ++vi.
  void endChunk();

  // (See endChunk())  Returns whether or not vi.nextChunk() should be used
  // when advancing past buffer number buf.
  Bool chunkEnd(const Int buf) const {return endChunk_p[buf];}

  // Replace the VisBuffer in slot buf with vb.
  // Returns True on success and False on failure.
  // Bool replace(const VisBuffer& vb, const uInt buf);

  // How many VisBuffers are contained herein?
  uInt nBuf() const {return nBuf_p;}

  // uInt nDDID() const {return nDDID_p;}
  // uInt nFld()  const {return nFld_p;}

  // Return a reference to the indexed VisBuffer.
  // Throws an exception if buf > nBuf().
  VisBuffer& operator()(const Int buf);

  // // Returns the buffer index corresponding to data description ID ddid and
  // // field ID fld.
  // //
  // // Returns -1 if there is no such buffer.
  // //
  // // Returns -(# of matching buffers) if there is > 1 match (use bufInds).
  // Int bufInd(const Int ddid, const Int fld);

  // // Returns the buffer indices corresponding to data description ID ddid and
  // // field ID fld.
  // Vector<Int> bufInds(const Int ddid, const Int fld)

  // The flagging approach to channel selection.
  // Sets chanmaskedflags to True wherever the channels in chanmask or flags in
  // vb.flagCube() are True, resizing if necessary.
  // Returns True/False on success/error (i.e. chanmask having a different # of
  // channels from vb.flagCube()).
  static Bool applyChanMask(Cube<Bool>& chanmaskedflags,
                            const Vector<Bool> *chanmask, const VisBuffer& vb);

private:
  // Prohibit public copying and assignment.
  VisBuffGroup(const VisBuffGroup&);
  VisBuffGroup& operator=(const VisBuffGroup&);

  // // Numbers of data description ids and fields
  // uInt nDDID_p, nFld_p, nBuf_p;

  // Number of buffers.
  uInt nBuf_p;

  // The list of buffers.
  PtrBlock<VisBuffer*> VB_p;
  
  Vector<Bool> endChunk_p;

  // // Map spw,fld to the buffer id
  // Matrix<Int> spwfldids_p;
};


} //# NAMESPACE CASA - END

#endif


