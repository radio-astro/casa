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
  class CoordinateSystem;
  // <summary> Class that contains functions needed for feathering</summary>

  class LinearMosaic{

  public:
	  LinearMosaic();
	  LinearMosaic(const String outim, const String outwgt, const MDirection& imcen, const Int nx, const Int ny,
			  const Quantity cellx=Quantity(0.0, "arcsec"), const Quantity celly=Quantity(0.0, "arcsec"), const Int linmostype=2);
	  //Coordinate System of output image along with the different pointing images and weight images
	  // linmostype 1 or 2.. ie. mosaic in PB or PB^2
	  LinearMosaic(const String outim, const String outwgt, const MDirection& imcen, const Int nx, const Int ny,
			  	  Vector<CountedPtr<ImageInterface<Float> > >& ims,
			  	  Vector<CountedPtr<ImageInterface<Float> > >& wgtims, const Int linmostype=2);
	  Bool makeMosaic(ImageInterface<Float>& outim, ImageInterface<Float>& outwgt,
			  Vector<CountedPtr<ImageInterface<Float> > >& ims,
			  Vector<CountedPtr<ImageInterface<Float> > >& wgtims);
	  Bool makeMosaic(Vector<CountedPtr<ImageInterface<Float> > >& ims,
		  	   Vector<CountedPtr<ImageInterface<Float> > >& wgtims);

	  // outim is weighted 0: flux correct, 1: flat noise, 2: noise optimal
	  void setOutImages(ImageInterface<Float>& outim, ImageInterface<Float>& outwgt, const Int imageWeightType=1, const Int weightType=2);
	  // Disk based image of the above
	  void setlinmostype(const Int linmostype);
	  void saultWeightImage(const String& outimname, const Float& fracPeakWgt);
	  void setOutImages(const String& outim, const String& outwgt, const Int imageWeightType=1, const Int weightType=2);
  private:
	  Bool addOnToImage(ImageInterface<Float>& outim, ImageInterface<Float>& outwgt, const ImageInterface<Float>& inIm,
			  const ImageInterface<Float>& inWgt, Bool unWeightOutImage=True);
	  void makeEmptyImage(const String imagename, const CoordinateSystem& cs, const MDirection& imcen, const Int nx, const Int ny, const Int npol, const Int nchan);
	  void createOutImages(const CoordinateSystem& cs, const Int npol, const Int nchan );
	  CountedPtr<ImageInterface<Float> > outImage_p, outWgt_p;
	  String outImName_p, outWgtName_p;
	  Int nx_p, ny_p;
	  MDirection imcen_p;
	  Quantity cellx_p, celly_p;

	  Int imageWeightType_p, weightType_p;
	  Int linmosType_p;
};



} //# NAMESPACE CASA - END



#endif /* SYNTHESISLINEARMOSAIC_H */
