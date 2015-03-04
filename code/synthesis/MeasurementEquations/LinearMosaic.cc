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


	LinearMosaic::LinearMosaic (): outImage_p(NULL), outWgt_p(NULL), outImName_p(""), outWgtName_p(""), weightType_p(1){


	}
	LinearMosaic::LinearMosaic(const String outim, const String outwgt, const MDirection& imcen, const Int nx,
			const Int ny, const Quantity cellx, const Quantity celly) :outImage_p(NULL), outWgt_p(NULL) {
		outImName_p=outim;

		outWgtName_p= outwgt=="" ? outim+String(".weight") : outwgt;
		nx_p=nx;
		ny_p=ny;
		imcen_p=imcen;
		cellx_p=cellx;
		celly_p=celly;

	 }
	LinearMosaic::LinearMosaic(const String outim, const String outwgt, const MDirection& imcen, const Int nx, const Int ny,
				const Vector<CountedPtr<ImageInterface<Float> > >& ims,
				const Vector<CountedPtr<ImageInterface<Float> > >& wgtims){
		Int nchan=ims[0]->shape()[3];
		Int npol=ims[0]->shape()[2];
		for (uInt k=0; k< ims.nelements(); ++k){
			if(nchan != ims[k]->shape()[3] || nchan != wgtims[k]->shape()[3])
				ThrowCc("images should have the same number of channels");
			if(npol != ims[k]->shape()[2] || npol != (wgtims[k])->shape()[2])
				ThrowCc("images should have the same number of polarization planes");
		}
		CoordinateSystem cs=ims[0]->coordinates();
		makeEmptyImage(outim, cs, imcen, nx, ny,npol, nchan);
		makeEmptyImage(outwgt, cs, imcen, nx, ny,npol, nchan);
		PagedImage<Float> outdiskim(outim);
		PagedImage<Float> outdiskwgt(outwgt);
		setOutImages(outdiskim, outdiskwgt, 2);
		makeMosaic(ims, wgtims);

	}
	Bool LinearMosaic::makeMosaic(ImageInterface<Float>& outim, ImageInterface<Float>& outwgt,
			  	  const Vector<CountedPtr<ImageInterface<Float> > >& ims,
			  	  const Vector<CountedPtr<ImageInterface<Float> > >& wgtims){

		Bool retval=True;
		if(ims.nelements() != wgtims.nelements())
			ThrowCc("Unequal number of images and weight images ");
		for (uInt k=0; k < ims.nelements(); ++k){
			retval=retval && addOnToImage(outim, outwgt, *(ims[k]), *(wgtims[k]), k>0, (k==ims.nelements()-1));
		}
		return retval;

	}
	Bool LinearMosaic::makeMosaic(
				  	  const Vector<CountedPtr<ImageInterface<Float> > >& ims,
				  	  const Vector<CountedPtr<ImageInterface<Float> > >& wgtims){

			Bool retval=True;
			if(outImage_p.null() && outImName_p=="")
				ThrowCc("No output image or weight image defined");
			if(outImage_p.null())
				createOutImages(ims[0]->coordinates(), ims[0]->shape()[2], ims[0]->shape()[3] );
			if(ims.nelements() != wgtims.nelements())
				ThrowCc("Unequal number of images and weight images ");
			for (uInt k=0; k < ims.nelements(); ++k){
				retval=retval && addOnToImage(*outImage_p, *outWgt_p, *(ims[k]), *(wgtims[k]), k>0, (k==ims.nelements()-1));
			}
			return retval;

		}
	void LinearMosaic::setOutImages(ImageInterface<Float>& outim, ImageInterface<Float>& outwgt, const Int weightType){
		outImage_p=CountedPtr<ImageInterface<Float> >(&outim, False);
		outWgt_p = CountedPtr<ImageInterface<Float> >(&outwgt, False);
		weightType_p=weightType;
	}
	void LinearMosaic::setOutImages(const String& outim, const String& outwgt, const Int weightType){
			outImage_p=new PagedImage<Float>(outim);
			outWgt_p = new PagedImage<Float>(outwgt);
			weightType_p=weightType;
	}
	Bool LinearMosaic::addOnToImage(ImageInterface<Float>& outim, ImageInterface<Float>& outwgt, const ImageInterface<Float>& inIm,
			  const ImageInterface<Float>& inWgt, Bool outImIsWeighted, Bool unWeightOutIm){

		Double meminMB=Double(HostInfo::memoryTotal(true))/1024.0;

		if(!outImIsWeighted){

			if(weightType_p==1)
				outim.copyData((LatticeExpr<Float>)(outim*outwgt));
			else if( weightType_p==0)
				outim.copyData((LatticeExpr<Float>)(outim*outwgt*outwgt));
			weightType_p=2;

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

	void LinearMosaic::createOutImages(const CoordinateSystem& cs, const Int npol, const Int nchan ){
		makeEmptyImage(outImName_p, cs, imcen_p, nx_p, ny_p, npol, nchan );
		makeEmptyImage(outWgtName_p, cs, imcen_p, nx_p, ny_p, npol, nchan );
		outImage_p=new PagedImage<Float>(outImName_p);
		outWgt_p= new PagedImage<Float>(outWgtName_p);
	}
	void LinearMosaic::makeEmptyImage(const String imagename, const CoordinateSystem& cs, const MDirection& imcen, const Int nx, const Int ny,
				const Int npol, const Int nchan){
		CoordinateSystem outcs=cs;
		Int dirAx=outcs.findCoordinate(Coordinate::DIRECTION);
		DirectionCoordinate dc=outcs.directionCoordinate(dirAx);
		Vector<Double> incr=dc.increment();
		String elunit=dc.worldAxisUnits()[0];
		if(cellx_p.getValue() !=  0.0)
			incr[0]=cellx_p.getValue(elunit);
		if(celly_p.getValue() != 0.0)
			incr[1]=celly_p.getValue(elunit);
		dc.setIncrement(incr);
		Vector<Double> cenVec(2);
		cenVec[0]=Double(nx)/2.0; cenVec[1]=Double(ny)/2.0;
		dc.setReferencePixel(cenVec);
		MDirection::Types eltype;
		MDirection::getType(eltype, imcen.getRefString());
		dc.setReferenceValue(imcen.getAngle().getValue(elunit));
		dc.setReferenceFrame(eltype);
		outcs.replaceCoordinate(dc, dirAx);
		PagedImage<Float> outdiskim(IPosition(4, nx, ny, npol, nchan), outcs, imagename);
		outdiskim.set(0.0);
	}

} //end namespace casa
