//# SetJyGridFT.h: Definition for GridFT
//# Copyright (C) 2012
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_SETJYGRIDFT_H
#define SYNTHESIS_SETJYGRIDFT_H

#include <synthesis/TransformMachines/GridFT.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>  An FTMachine for Gridded Fourier transforms specializing in setjy frequency scaling </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class=FTMachine>GridFT</linkto> module
//   <li> <linkto class=SkyEquation>SkyEquation</linkto> module
//   <li> <linkto class=VisBuffer>VisBuffer</linkto> module
// </prerequisite>
//
// <etymology>
// Inheriting from GridFT which does
// Grid-based Fourier transforms.
// special case for setjy style frequency scaling
// </etymology>
//
// <synopsis> 

// </synopsis> 
//
// <example>
// </example>
//
// <motivation>
// Define an interface to allow efficient processing of chunks of 
// visibility data
// </motivation>
//
// <todo asof="2012/05/08">
//
// </todo>

class SetJyGridFT : public GridFT {
public:

  // Constructor: cachesize is the size of the cache in words
  // (e.g. a few million is a good number), tilesize is the
  // size of the tile used in gridding (cannot be less than
  // 12, 16 works in most cases), and convType is the type of
  // gridding used (SF is prolate spheriodal wavefunction,
  // and BOX is plain box-car summation). mLocation is
  // the position to be used in some phase rotations. If
  // mTangent is specified then the uvw rotation is done for
  // that location iso the image center.
  // <group>

  SetJyGridFT(Long cachesize, Int tilesize, String convType,
	 MPosition mLocation, MDirection mTangent, Float passing=1.0,
	 Bool usezero=True, Bool useDoublePrec=False,
	      const Vector<Double>& freqscale=Vector<Double>(1, 0.0), const Vector<Double>& scale=Vector<Double>(1, 1.0));
  // </group>

  // Construct from a Record containing the GridFT state
  SetJyGridFT(const RecordInterface& stateRec);

  // Copy constructor
  SetJyGridFT(const SetJyGridFT &other);

  // Assignment operator
  SetJyGridFT &operator=(const SetJyGridFT &other);

  virtual ~SetJyGridFT();

  //clone FTM
  virtual FTMachine* cloneFTM();
  // Initialize transform to Visibility plane using the image
  // as a template. The image is loaded and Fourier transformed.
  virtual void initializeToVis(ImageInterface<Complex>& image,
		       const VisBuffer& vb);
  
  // Finalize transform to Visibility plane: flushes the image
  // cache and shows statistics if it is being used.
  //void finalizeToVis();

  // Get actual coherence from grid by degridding
  void get(VisBuffer& vb, Int row=-1);

  
  // Save and restore the GridFT to and from a record
  virtual Bool toRecord(String& error, RecordInterface& outRec, 
			Bool withImage=False, const String diskimage="");
  virtual Bool fromRecord(String& error, const RecordInterface& inRec);

  virtual void setScale(const Vector<Double>& freq, const Vector<Double>& scale);

  virtual String name() const;


protected:




  //Prepare the grid for degridding
  //void prepGridForDegrid();

  Vector<Double> freqscale_p;
  Vector<Double> scale_p;
  Vector<Double> interpscale_p;



};

} //# NAMESPACE CASA - END

#endif
