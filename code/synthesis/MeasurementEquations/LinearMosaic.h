/*
 * LinearMosaic.h  Linear mosaicing code
//# Copyright (C) 2015
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

 *
 *
 *  Created on: Feb 24, 2015
 *      Author: kgolap
 */

#ifndef SYNTHESIS_LINEARMOSAIC_H
#define SYNTHESIS_LINEARMOSAIC_H

namespace casa { //# NAMESPACE CASA - BEGIN
  //Forward declaration
  template<class T> class ImageInterface;
  template<class T> class Vector;
  // <summary> Class that contains functions needed for feathering</summary>

  class LinearMosaic{

  public:
	  LinearMosaic();
	  //Coordinate System of output image along with the different pointing images and weight images
	  LinearMosaic(const String outim, const String outwgt, const MDirection& imcen, const Int nx, const Int ny,
			  	  const Vector<ImageInterface<Float> >& ims,
			  	  const Vector<ImageInterface<Float> >& wgtims);
	  Bool makeMosaic(ImageInterface<Float>& outim, ImageInterface<Float>& outwgt,
			  	  const Vector<ImageInterface<Float> >& ims,
			  	  const Vector<ImageInterface<Float> >& wgtims);



  private:
	  Bool addOnToImage(ImageInterface<Float>& outim, ImageInterface<Float>& outwgt, const ImageInterface<Float>& inIm,
			  const ImageInterface<Float>& inWgt, Bool outimIsWeighted=False, Bool unWeightOutImage=True);
};



} //# NAMESPACE CASA - END



#endif /* SYNTHESISLINEARMOSAIC_H */
