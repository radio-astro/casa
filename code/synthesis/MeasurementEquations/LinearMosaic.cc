/*
 * LinearMosaic.cc
 //# Copyright (C) 2015
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
 *  Created on: Feb 24, 2015
 *      Author: kgolap
 */

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <images/Images/TempImage.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageRegrid.h>
#include <images/Images/SubImage.h>
#include <images/Regions/WCBox.h>
#include <synthesis/MeasurementEquations/LinearMosaic.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN


	LinearMosaic::LinearMosaic (){


	}
	LinearMosaic::LinearMosaic(const String outim, const String outwgt, const MDirection& imcen, const Int nx, const Int ny, const Vector<ImageInterface<Float> >& ims,
				  	  const Vector<ImageInterface<Float> >& wgtims){
		Int nchan=ims[0].shape()[3];
		Int npol=ims[0].shape()[2];
		for (uInt k=0; k< ims.nelements(); ++k){
			if(nchan != ims[k].shape()[3] || nchan != wgtims.shape()[3])
				ThrowCc("images should have the same number of channels");
			if(npol != ims[k].shape()[2] || npol != wgtims.shape()[2])
				ThrowCc("images should have the same number of polarization planes");
		}
		CoordinateSystem outcs= ims[0].coordinates();
		Int dirAx=outcs.findCoordinate(Coordinate::DIRECTION);
		DirectionCoordinate dc=outcs.directionCoordinate(dirAx);
		String elunit=outcs.worldAxisUnits()[dirAx];
		Vector<Double> cenVec(2);
		cenVec[0]=Double(nx)/2.0; cenVec[1]=Double(ny)/2.0;
		dc.setReferencePixel(cenVec);
		MDirection::Types eltype;
		MDirection::getType(eltype, imcen.getRefString());
		dc.setReferenceValue(imcen.getAngle().getValue(elunit));
		dc.setReferenceFrame(eltype);
		outcs.replaceCoordinate(dc, dirAx);
		PagedImage<Float> outdiskim(IPosition(4, nx, ny, npol, nchan), outcs, outim);
		PagedImage<Float> outdiskwgt(IPosition(4, nx, ny, npol, nchan), outcs, outwgt);


	}
	Bool LinearMosaic::makeMosaic(ImageInterface<Float>& outim, ImageInterface<Float>& outwgt,
			  	  const Vector<ImageInterface<Float> >& ims,
			  	  const Vector<ImageInterface<Float> >& wgtims){

		Bool retval=True;
		if(ims.nelements() != wgtims.nelements())
			ThrowCc("Unequal number of images and weight images ");
		for (uInt k=0; k < ims.nelements(); ++k){
			retval=retval && addOnToImage(outim, outwgt, ims[k], wgtims[k], k>0, (k==ims.nelements()-1));
		}
		return retval;

	}
	Bool LinearMosaic::addOnToImage(ImageInterface<Float>& outim, ImageInterface<Float>& outwgt, const ImageInterface<Float>& inIm,
			  const ImageInterface<Float>& inWgt, Bool outImIsWeighted, Bool unWeightOutIm){

		Double meminMB=Double(HostInfo::memoryTotal(true))/1024.0;

		if(!outImIsWeighted){

			outim.copyData((LatticeExpr<Float>)(outim*outwgt));

		}

		CoordinateSystem incs=inIm.coordinates();



		IPosition iblc(inIm.shape().nelements(),0);
		IPosition itrc(inIm.shape());
		itrc=itrc-Int(1);



		LCBox lbox(iblc, itrc, inIm.shape());
		ImageRegion imagreg(WCBox(lbox, incs) );
		SubImage<Float> subOutIm;
		SubImage<Float> subOutWgt;
		try{
			subOutIm=SubImage<Float>(outim, imagreg, True);
			subOutWgt=SubImage<Float>(outwgt, imagreg, True);
		}
		catch(...){
			//Failed to make a subimage let us use the full image
			subOutIm=SubImage<Float>(outim, True);
			subOutWgt=SubImage<Float>(outwgt, True);

		}
		TempImage<Float> fullImage(subOutWgt.shape(), subOutIm.coordinates(), meminMB/8.0);
		TempImage<Float> fullWeight(subOutWgt.shape(), subOutIm.coordinates(), meminMB/8.0);
		{
			TempImage<Float> inWeightedIm(inIm.shape(), inIm.coordinates(), meminMB/8.0);
			inWeightedIm.copyData((LatticeExpr<Float>)(inIm*inWgt));
			ImageRegrid<Float> regridder;
			regridder.regrid( fullImage, Interpolate2D::LINEAR,
					IPosition(2,0,1), inWeightedIm);
			regridder.regrid( fullWeight, Interpolate2D::LINEAR,
								IPosition(2,0,1), inWgt);
		}

		subOutWgt.copyData((LatticeExpr<Float>)(subOutWgt+fullWeight));
		LatticeExprNode elmax = max( outwgt );
		Float wMax =  elmax.getFloat();
		subOutIm.copyData((LatticeExpr<Float>)(subOutIm+fullImage));
		if(wMax > 0.0){
			outim.copyData((LatticeExpr<Float>)(outim/wMax));
			outwgt.copyData((LatticeExpr<Float>)(outwgt/wMax));
		}
		if(unWeightOutIm){
			outim.copyData((LatticeExpr<Float>)(iif(outwgt > (0.0),
						       (outim/outwgt), 0)));

		}

		return True;
	}


} //end namespace casa
