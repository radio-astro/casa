//# tImageFitter.cc:  test the PagedImage class
//# Copyright (C) 1994,1995,1998,1999,2000,2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or(at your option)
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
//#
//# $Id: $

#include <imageanalysis/IO/FitterEstimatesFileParser.h>

#include <casa/Utilities/Assert.h>
#include <casa/OS/File.h>
#include <components/ComponentModels/GaussianShape.h>
#include <images/Images/FITSImage.h>
#include <images/Regions/WCEllipsoid.h>
#include <images/Regions/WCExtension.h>
#include <imageanalysis/ImageAnalysis/ImageProfileFitter.h>
#include <imageanalysis/Annotations/AnnEllipse.h>

#include <casa/namespace.h>

void test1(){
	String pixelBox("100,100,115,115");
	String channelStr( "17~27");
	int spectralAxisNumber = 3;
	int nGauss = 1;
	SpectralList spectralList;

	SHARED_PTR<ImageInterface<Float> > image( new PagedImage<Float>( "/home/achillea/casa/trunk/test/titan2/titanline-small2.image"));
	ImageProfileFitter* fitter = new ImageProfileFitter( image, "", 0, pixelBox,
											 channelStr, "", "",
											 spectralAxisNumber,
											 static_cast<uInt>(nGauss), "",
											 spectralList);
	fitter->setPolyOrder( 0 );
	fitter->setLogfile( "/tmp/fitTest.txt" );
	Record results = fitter->fit();
	cout << "Results: "<<results;
	cout << "ok" << endl;
}


ImageRegion* getEllipsoid(const CoordinateSystem& cSys,
		const Vector<Double>& x, const Vector<Double>& y){
	Vector<Quantity> center(2);
	Vector<Quantity> radius(2);
	const String radUnits( "rad");
	center[0] = Quantity( (x[0]+x[1])/2, radUnits );
	center[1] = Quantity( (y[0]+y[1])/2, radUnits );

	MDirection::Types type = MDirection::N_Types;
	int directionIndex = cSys.findCoordinate( Coordinate::DIRECTION );
	uInt dirIndex = static_cast<uInt>(directionIndex);
	type = cSys.directionCoordinate(dirIndex).directionType(true);

	Vector<Double> qCenter(2);
	qCenter[0] = center[0].getValue();
	qCenter[1] = center[1].getValue();
	MDirection mdcenter( Quantum<Vector<Double> >(qCenter,radUnits), type );

	Vector<Double> blc_rad_x(2);
	blc_rad_x[0] = x[0];
	blc_rad_x[1] = center[1].getValue();
	MDirection mdblc_x( Quantum<Vector<Double> >(blc_rad_x,radUnits),type );

	Vector<Double> blc_rad_y(2);
	blc_rad_y[0] = center[0].getValue();
	blc_rad_y[1] = y[0];
	MDirection mdblc_y( Quantum<Vector<Double> >(blc_rad_y,radUnits),type );


	double xdistance = mdcenter.getValue( ).separation(mdblc_x.getValue( ));
	double ydistance = mdcenter.getValue( ).separation(mdblc_y.getValue( ));
	radius[0] = Quantity(xdistance, radUnits );
	radius[1] = Quantity(ydistance, radUnits );

	Vector<Int> pixax(2);
	Vector<Int> dirPixelAxis = cSys.pixelAxes(directionIndex);
	pixax(0) = dirPixelAxis[0];
	pixax(1) = dirPixelAxis[1];


	WCEllipsoid ellipsoid( center, radius, IPosition(dirPixelAxis), cSys);
	ImageRegion* imageRegion = new ImageRegion(ellipsoid);
	return imageRegion;
}



Record getEllipticalRegion3D( const CoordinateSystem& cSys,
			const Vector<Double>& x, const Vector<Double>& y,
			int channelMin, int channelMax, int spectralAxisIndex){
	Record regionRecord;
	ImageRegion* ellipsoid = getEllipsoid( cSys, x, y);
	if ( ellipsoid != NULL ){
		//Make an extension box in the spectral direction.
		bool spectralAxis = cSys.hasSpectralAxis();
		if ( spectralAxis ) {
			IPosition pixelAxes(1);
			if (spectralAxisIndex > 0) {
				pixelAxes[0] = spectralAxisIndex;
				int pixelAxesCount = pixelAxes.size();
				Vector<Int> absRel(pixelAxesCount, RegionType::Abs);
				Vector<Quantity> minRange(pixelAxesCount);
				Vector<Quantity> maxRange(pixelAxesCount);
				minRange[0]=Quantity( channelMin, "pix" );
				maxRange[0] = Quantity( channelMax, "pix");
				WCBox wbox(minRange, maxRange, pixelAxes, cSys, absRel);
				WCExtension extension( *ellipsoid, wbox);
				ImageRegion extendedRegion (extension);
				regionRecord = extendedRegion.toRecord("");
			}
		}
	}
	return regionRecord;
}

void test2(){

	//Testing params
	int nGauss = 1;
	//Dave, Please Note:  If you change 0,40 to 17,27 the fit will no longer converge.
	int channelMin = 17;
	int channelMax = 27;

	//Load the image.
	SHARED_PTR<ImageInterface<Float> > image( new PagedImage<Float>( "/home/achillea/casa/trunk/test/titan2/titanline-small2.image"));
    CoordinateSystem cSys = image->coordinates();
    int spectralAxisNumber = cSys.spectralAxisNumber();

	//Add an estimate for the Gauss fit
	SpectralList spectralList;
	double peakVal = 1.47455;
	double centerValPix = 21.9734;
	double fwhmValPix = 3.87114;
	GaussianSpectralElement* estimate = new GaussianSpectralElement( peakVal, centerValPix, fwhmValPix);
	spectralList.add( *estimate );

	//Elliptical region bounding box
	Vector<double> regionXValues(2);
	Vector<double> regionYValues(2);
	regionXValues[0] = 3.525016142;
	regionXValues[1] = 3.525010009;
	regionYValues[0] = -0.111230432;
	regionYValues[1] = -0.1112240063;

	//Make the 3D elliptical region.
	Record regionRecord = getEllipticalRegion3D( cSys,
											regionXValues, regionYValues,
											channelMin, channelMax, spectralAxisNumber);

	//Initialize the fitter
	ImageProfileFitter* fitter = new ImageProfileFitter( image, "", &regionRecord, "",
								 "", "", "", spectralAxisNumber, static_cast<uInt>(nGauss), "",
								 spectralList);

	fitter->setPolyOrder( 0 );
	fitter->setLogfile( "/tmp/fitTest.txt" );
	Record results = fitter->fit();
	cout << "Results: "<<results;
	cout << "ok" << endl;
}

int main() {
	test2();
    return 0;
}

