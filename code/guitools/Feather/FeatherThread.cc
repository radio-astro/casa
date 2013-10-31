//# Copyright (C) 2005
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


#include "FeatherThread.h"
#include <images/Images/ImageInterface.h>
#include <images/Images/TempImage.h>
#include <synthesis/MeasurementEquations/Feather.h>
#include <synthesis/MeasurementEquations/Imager.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <images/Images/ImageUtilities.h>
#include <QDebug>

namespace casa {

FeatherThread::FeatherThread():
				featherWorker(NULL),
				lowImage(NULL), highImage(NULL), dirtyImage(NULL){
	saveOutput = false;
	fileSaved = true;
	success = true;
	featherWorker = new Feather();
	logger = NULL;
}

FeatherThread::~FeatherThread(){
}

void FeatherThread::setImages(ImageInterface<float>* lowImage, ImageInterface<float>* highImage, ImageInterface<float>* dirtyImage ){
	this->lowImage = lowImage;
	this->highImage = highImage;
	this->dirtyImage = dirtyImage;

}

QString FeatherThread::getErrorMessage() const {
	return errorMessage;
}

void FeatherThread::setLogger( LogIO* logger ){
	this->logger = logger;
}

void FeatherThread::setFeatherWorker( Feather* worker ){
	featherWorker = worker;
}

void FeatherThread::setRadial( bool radialPlot ){
	radial = radialPlot;
}

void FeatherThread::setSaveOutput( bool save, const QString& outputPath){
	saveOutput = save;
	if ( saveOutput ){
		saveFilePath = outputPath;
	}
}

bool FeatherThread::setWorkerImages( ImageInterface<float>* imageLow,
		ImageInterface<float>* imageHigh ){
	bool imagesLoaded = false;
	if ( imageLow != NULL && imageHigh != NULL ){
		try {
			//Note:  high resolution image must be defined before low resolution
			//image or we get an exception.
			featherWorker->setINTImage( *imageHigh );
			featherWorker->setSDImage( *imageLow );
			imagesLoaded = true;
		}
		catch( AipsError& errorRef ){
			if ( logger != NULL ){
				(*logger)<<LogIO::WARN<<
					 "\nFeatherThread:: Problem loading images: "<<
					 errorRef.getMesg().c_str()<<LogIO::POST;
				(*logger)<< LogIO::WARN<<
						"\nFeatherThread:: Single Dish="<<imageLow->name().c_str() <<
						" Interferometer="<< imageHigh->name().c_str()<<LogIO::POST;
			}
		}
	}
	return imagesLoaded;
}

void FeatherThread::run(){
	success = collectLowHighData();
	if ( success && dirtyImage != NULL ){
		success = collectLowDirtyData();
	}
}

bool FeatherThread::collectLowDirtyData(){

	bool lowDirtyLoaded = setWorkerImages( lowImage, dirtyImage );
	if ( lowDirtyLoaded ){
		featherWorker->clearWeightFlags();
		Vector<Float> uX;
		Vector<Float> uY;
		Vector<Float> vX;
		Vector<Float> vY;
		FeatheredData dirtyCut;
		featherWorker->getFeatheredCutINT( uX, uY, vX, vY, radial );
		dirtyCut.setU( uX, uY );
		dirtyCut.setV( vX, vY );
		dataMap.insert( DIRTY_CUT, dirtyCut );

		FeatheredData dirtyOriginal;
		featherWorker->getFTCutIntImage( uX, uY, vX, vY, radial );
		dirtyOriginal.setU( uX, uY );
		dirtyOriginal.setV( vX, vY );
		dataMap.insert( DIRTY_ORIGINAL, dirtyOriginal );

		try {
			ImageInfo lowInfo = lowImage->imageInfo();
			GaussianBeam beam = lowInfo.restoringBeam();
			featherWorker->convolveINT( beam );
			FeatheredData convolvedDirtyOriginal;
			featherWorker->getFTCutIntImage( uX, uY, vX, vY, radial );
			convolvedDirtyOriginal.setU( uX, uY );
			convolvedDirtyOriginal.setV( vX, vY );
			dataMap.insert( DIRTY_CONVOLVED_LOW, convolvedDirtyOriginal );
			FeatheredData convolvedDirtyCut;
			featherWorker->getFeatheredCutINT( uX, uY, vX, vY, radial );
			convolvedDirtyCut.setU( uX, uY );
			convolvedDirtyCut.setV( vX, vY );
			dataMap.insert( DIRTY_CONVOLVED_LOW_WEIGHTED, convolvedDirtyCut );
		}
		catch( AipsError& error ){
			lowDirtyLoaded = false;
			errorMessage = "Could not convolve dirty image with low beam.";
			if ( logger != NULL ){
				(*logger)<<LogIO::WARN << errorMessage.toStdString() <<
						"  "<<error.getMesg().c_str()<<LogIO::POST;
			}
		}


		/*ImageInterface<float>* newLow = makeConvolvedImage( lowImage, dirtyImage );
		if ( newLow != NULL ){
			featherWorker->setINTImage( *dirtyImage );
			featherWorker->setSDImage( *newLow );
			lowDirtyLoaded = collectConvolvedData(LOW_CONVOLVED_DIRTY, LOW_CONVOLVED_DIRTY_WEIGHTED );
			delete newLow;
		}*/

	}
	else {
		errorMessage = "There was a problem loading the dirty image data.";
	}
	return lowDirtyLoaded;
}




bool FeatherThread::collectConvolvedData( DataTypes original, DataTypes cut){
	//Now we compute the "LOW Convolved ______________" and the
	//"LOW Convolved with _______________, Weighted and Scaled".  The assumption is
	//that the SD beam has been replaced with the low data convolved with
	//the _________ beam.
	bool success = false;
	try {
		Vector<Float> uX;
		Vector<Float> uY;
		Vector<Float> vX;
		Vector<Float> vY;
		FeatheredData convolveIntOriginal;
		featherWorker->getFTCutSDImage(uX, uY, vX, vY, radial );
		convolveIntOriginal.setU( uX, uY );
		convolveIntOriginal.setV( vX, vY );
		dataMap.insert( original, convolveIntOriginal );

		FeatheredData convolveIntOriginalWeighted;
		featherWorker->getFeatheredCutSD( uX, uY, vX, vY, radial );
		convolveIntOriginalWeighted.setU( uX, uY );
		convolveIntOriginalWeighted.setV( vX, vY );
		dataMap.insert( cut, convolveIntOriginalWeighted );
		success = true;
	}
	catch( AipsError& error ){
		errorMessage = "Could not get data for low resolution data convolved with beam.";
		if ( logger != NULL ){
			(*logger)<<LogIO::WARN<< errorMessage.toStdString() << "  "<<error.getMesg().c_str()<<LogIO::POST;
		}
	}
	return success;
}

bool FeatherThread::isSuccess() const {
	return success;
}

bool FeatherThread::collectLowHighData(){
	bool lowHighLoaded = setWorkerImages( lowImage, highImage );
	if ( lowHighLoaded ){

		//Initialize the weight Data
		Vector<Float> uX;
		Vector<Float> uY;
		Vector<Float> vX;
		Vector<Float> vY;
		FeatheredData sdWeight;
		featherWorker->getFeatherSD( uX, uY, vX, vY, radial );
		sdWeight.setU( uX, uY );
		sdWeight.setV( vX, vY );
		dataMap.insert( SD_WEIGHT, sdWeight );
		FeatheredData intWeight;
		featherWorker->getFeatherINT( uX, uY, vX, vY, radial );
		intWeight.setU( uX, uY );
		intWeight.setV( vX, vY );
		dataMap.insert( INT_WEIGHT, intWeight );

		//Initialize the cut data
		FeatheredData sdCut;
		featherWorker->getFeatheredCutSD( uX, uY, vX, vY, radial );
		sdCut.setU( uX, uY );
		sdCut.setV( vX, vY );
		dataMap.insert( SD_CUT, sdCut );
		FeatheredData intCut;
		featherWorker->getFeatheredCutINT( uX, uY, vX, vY, radial );
		intCut.setU( uX, uY );
		intCut.setV( vX, vY );
		dataMap.insert( INT_CUT, intCut );

		//Initialize the original data
		FeatheredData sdOriginal;
		featherWorker->getFTCutSDImage( uX, uY, vX, vY, radial );
		sdOriginal.setU( uX, uY );
		sdOriginal.setV( vX, vY );
		dataMap.insert( SD_ORIGINAL, sdOriginal );
		FeatheredData intOriginal;
		featherWorker->getFTCutIntImage( uX, uY, vX, vY, radial );
		intOriginal.setU( uX, uY );
		intOriginal.setV( vX, vY );
		dataMap.insert( INT_ORIGINAL, intOriginal );

		if ( saveOutput ){
			fileSaved = featherWorker->saveFeatheredImage( saveFilePath.toStdString() );
		}

		try {
			ImageInfo lowInfo = lowImage->imageInfo();
			GaussianBeam beam = lowInfo.restoringBeam();
			featherWorker->convolveINT( beam );
			FeatheredData convolvedIntOriginal;
			featherWorker->getFTCutIntImage( uX, uY, vX, vY, radial );
			convolvedIntOriginal.setU( uX, uY );
			convolvedIntOriginal.setV( vX, vY );
			dataMap.insert( INT_CONVOLVED_LOW, convolvedIntOriginal );
			FeatheredData convolvedIntCut;
			featherWorker->getFeatheredCutINT( uX, uY, vX, vY, radial );
			convolvedIntCut.setU( uX, uY );
			convolvedIntCut.setV( vX, vY );
			dataMap.insert( INT_CONVOLVED_LOW_WEIGHTED, convolvedIntCut );
		}
		catch( AipsError& error ){
			lowHighLoaded = false;
			errorMessage = "Could not convolve high image with low beam.";
			if ( logger != NULL ){
				(*logger)<<LogIO::WARN<< errorMessage.toStdString() << "  "<<error.getMesg().c_str()<<LogIO::POST;
			}
		}

		ImageInterface<float>* convolvedImage = FeatherThread::makeConvolvedImage( lowImage, highImage );
		if ( convolvedImage != NULL ){
			featherWorker->setINTImage( *highImage );
			featherWorker->setSDImage( *convolvedImage );
			lowHighLoaded = collectConvolvedData(LOW_CONVOLVED_HIGH, LOW_CONVOLVED_HIGH_WEIGHTED );
			delete convolvedImage;
		}
		else {
			errorMessage = "Could not convolve low data with high beam.";
			lowHighLoaded = false;
		}
	}
	else {
		errorMessage = "There was a problem loading the single dish/interferometer data.";
	}
	return lowHighLoaded;
}

ImageInterface<float>* FeatherThread::addMissingAxes( ImageInterface<float>* firstImage ){
	TempImage<float>* convolvedImage = NULL;
	if ( firstImage != NULL ){
		CoordinateSystem cSys = firstImage->coordinates();
		CountedPtr<ImageInterface<Float> > firstCopy;
		firstCopy=new TempImage<Float>(firstImage->shape(), cSys);
		(*firstCopy).copyData(*firstImage);
		ImageUtilities::copyMiscellaneous(*firstCopy, *firstImage);
		if(firstImage->getDefaultMask() != "")
			Imager::copyMask(*firstCopy, *firstImage,  firstImage->getDefaultMask());
		PtrHolder<ImageInterface<Float> > copyPtr;
		PtrHolder<ImageInterface<Float> > copyPtr2;
		Vector<Stokes::StokesTypes> stokesvec;
		if(CoordinateUtil::findStokesAxis(stokesvec, cSys) <0){
			CoordinateUtil::addIAxis(cSys);
			ImageUtilities::addDegenerateAxes (*logger, copyPtr, *firstCopy, "",
					False, False,"I", False, False,
					True);
			firstCopy=CountedPtr<ImageInterface<Float> >(copyPtr.ptr(), False);
		}
		if(CoordinateUtil::findSpectralAxis(cSys) <0){
			CoordinateUtil::addFreqAxis(cSys);
			ImageUtilities::addDegenerateAxes (*logger, copyPtr2, *firstCopy, "",
					False, True,
					"", False, False,
					True);
			firstCopy=CountedPtr<ImageInterface<Float> >(copyPtr2.ptr(), False);
		}


		convolvedImage=new TempImage<Float>(firstCopy->shape(), cSys);

		convolvedImage->copyData(*firstCopy);
		ImageUtilities::copyMiscellaneous(*convolvedImage, *firstCopy);
		String maskname=firstCopy->getDefaultMask();
		if(maskname != ""){
			Imager::copyMask(*convolvedImage, *firstCopy, maskname);

		}
	}
	return convolvedImage;
}

ImageInterface<float>* FeatherThread::makeConvolvedImage(ImageInterface<float>* firstImage, ImageInterface<float>* secondImage ){
	ImageInterface<float>* convolvedImage = NULL;
	if ( secondImage != NULL && firstImage != NULL ){
		try {
			ImageInfo lowInfo= secondImage->imageInfo();
			GaussianBeam beam=lowInfo.restoringBeam();
			convolvedImage = addMissingAxes( firstImage );
			StokesImageUtil::Convolve(*convolvedImage, beam, True);
		}
		catch( AipsError& error ){
			if ( logger != NULL ){
				(*logger)<<LogIO::WARN<< "Error making convolved image: "<<error.getMesg().c_str()<<LogIO::POST;
			}
		}
	}
	return convolvedImage;
}



} /* namespace casa */
