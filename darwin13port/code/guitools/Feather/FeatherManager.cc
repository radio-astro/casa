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


#include "FeatherManager.qo.h"
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/SubImage.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <synthesis/Utilities/SpectralImageUtil.h>

#include <QDebug>

namespace casa {

FeatherManager::FeatherManager() :
		lowResImage( NULL ), highResImage( NULL ), dirtyImage( NULL ),
		highResFeedImage( NULL ), dirtyFeedImage( NULL ), lowResFeedImage( NULL ),
		 thread( NULL), logger( NULL){
	radialAxis = false;
	channelIndex = 0;
	channelsAveraged = true;
	success = true;
	featherWorker = NULL;
}

bool FeatherManager::loadImages( const QString& lowImagePath, const QString& highImagePath, LogIO* logger ){
	bool imagesGenerated = true;
	this->logger = logger;
	delete lowResImage;
	lowResImage = NULL;
	delete highResImage;
	highResImage = NULL;
	delete dirtyImage;
	dirtyImage = NULL;
	if ( featherWorker != NULL ){
		delete featherWorker;
	}
	featherWorker = new Feather();

	try {
		imagesGenerated = generateInputImage( lowImagePath, highImagePath);
	}
	catch( AipsError& error ){
		imagesGenerated = false;
	}
	return imagesGenerated;
}

bool FeatherManager::loadDirtyImage( const QString& dirtyImagePath ){
	bool imageGenerated = true;
	try {
		imageGenerated = generateDirtyImage( dirtyImagePath );
	}
	catch( AipsError& error ){
		imageGenerated = false;
	}
	return imageGenerated;
}


bool FeatherManager::setEffectiveDishDiameter( float xDiam, float yDiam ){
	bool valid = false;
	if ( lowResImage != NULL ){
		featherWorker->setINTImage( *highResFeedImage );
		featherWorker->setSDImage( *lowResFeedImage );
		valid = featherWorker->setEffectiveDishDiam( xDiam, yDiam );
	}
	return valid;
}

void FeatherManager::getEffectiveDishDiameter( Float& xDiam, Float& yDiam ){
	featherWorker->getEffectiveDishDiam( xDiam, yDiam );
}
void FeatherManager::setSDScale( float scale ){
	featherWorker->setSDScale( scale );
}

void FeatherManager::applyFeather( bool saveOutput, const QString& outputImagePath ){
	if ( thread != NULL ){
		delete thread;
		thread = NULL;
	}
	if ( highResFeedImage != NULL && lowResFeedImage != NULL ){
		thread = new FeatherThread();
		thread->setLogger( logger );
		thread->setFeatherWorker( featherWorker );
		thread->setImages( lowResFeedImage, highResFeedImage, dirtyFeedImage );
		connect( thread, SIGNAL( finished() ), this, SLOT(featherDone()));
		thread->setSaveOutput( saveOutput, outputImagePath );
		thread->setRadial( radialAxis );
		thread->start();
	}
	else {
		featherDone();
	}

}

void FeatherManager::featherDone(){
	success = thread->isSuccess();
	if ( !success ){
		errorMessage = thread->getErrorMessage();
	}
	emit featheringDone();
}

int FeatherManager::getPlaneCount( ImageInterface<float>* image ) const {
	int planeCount = 1;
	if ( image != NULL ){
		IPosition imgShape = highResImage->shape();
		CoordinateSystem coordinateSystem = highResImage->coordinates();
		if ( coordinateSystem.hasSpectralAxis()){
			int spectralIndex = coordinateSystem.spectralAxisNumber();
			planeCount = imgShape(spectralIndex);
		}
	}
	return planeCount;
}

int FeatherManager::getPlaneCount() const {
	return getPlaneCount( highResImage );
}

int FeatherManager::getChannelIndex() const {
	return channelIndex;
}

bool FeatherManager::isChannelsAveraged() const {
	return channelsAveraged;
}


bool FeatherManager::isFileSaved() const {
	bool fileSaved = false;
	if ( thread != NULL ){
		fileSaved = thread->fileSaved;
	}
	return fileSaved;
}

void FeatherManager::setRadial( bool radialPlot ){
	radialAxis = radialPlot;
}

bool FeatherManager::isRadial() const {
	return radialAxis;
}

void FeatherManager::setChannelsAveraged( bool averaged ){
	if ( channelsAveraged != averaged ){
		resetBasicFeedImages();
		resetDirtyFeedImage();
		channelsAveraged = averaged;
	}
}
void FeatherManager::setChannelIndex( int index ){
	if ( channelIndex != index ){
		resetBasicFeedImages();
		resetDirtyFeedImage();
		channelIndex = index;
	}

}

QString FeatherManager::getError() const {
	return errorMessage;
}

bool FeatherManager::generateInputImage( QString highResImagePath, QString lowResImagePath){
	bool success = true;
	try {
		if ( logger != NULL ){
			(*logger) << LogIO::NORMAL
				<< "\nLoading high and low resolution images.\n"
				<< LogIO::POST;
		}
		//Get initial images
		String highResLocation(highResImagePath.toStdString());
		String lowResLocation(lowResImagePath.toStdString());

		PagedImage<Float> highResImageTemp(highResLocation);
		PagedImage<Float> lowResImageTemp(lowResLocation);
		if(highResImageTemp.shape().nelements() != lowResImageTemp.shape().nelements()){
			String msg( "High and low resolution images do not have the same number of axes.");
			if ( logger != NULL ){
				(*logger) << LogIO::SEVERE  << msg << LogIO::EXCEPTION;
			}
			errorMessage = msg.c_str();
			success = false;
		}
		else {
			lowResImage = new PagedImage<Float>(lowResLocation);
			highResImage = new PagedImage<Float>(highResLocation);

			resetBasicFeedImages();
		}
	}
	catch (AipsError& x) {
		String msg = x.getMesg();
		if ( logger != NULL ){
			(*logger) << LogIO::SEVERE << "\nCaught exception: " << msg<< LogIO::EXCEPTION;
		}
		errorMessage = "There was a problem loading the low and high resolution images.";
		success = false;;
	}
	return success;
}

void FeatherManager::resetBasicFeedImages(){
	//Depending on whether the user has selected a single plane image
	//or averaging, we may need to take a slice of the image;
	//We clean up any previous slice here.
	if ( highResFeedImage != NULL ){
		delete highResFeedImage;
		highResFeedImage = NULL;
	}
	if ( lowResFeedImage != NULL ){
		delete lowResFeedImage;
		lowResFeedImage = NULL;
	}


	//Even though the thread will reset the images, we put these in initially
	//so we can get the effective dish diameter.
	highResFeedImage = getSinglePlaneImage( highResImage );
	lowResFeedImage = getSinglePlaneImage( lowResImage );

	if ( highResFeedImage != NULL ){
		featherWorker->setINTImage( *highResFeedImage );
	}
	if ( lowResFeedImage != NULL ){
		featherWorker->setSDImage( *lowResFeedImage );
	}
}

void FeatherManager::resetDirtyFeedImage(){
	//Depending on whether the user has selected a single plane image
	//or averaging, we may need to take a slice of the dirtyImage;
	//We clean up any previous slice here.
	if ( dirtyFeedImage != NULL ){
		delete dirtyFeedImage;
		dirtyFeedImage = NULL;
	}

	dirtyFeedImage = getSinglePlaneImage( dirtyImage );
}

bool FeatherManager::generateDirtyImage( QString dirtyImagePath ){

	bool success = true;

	try {
		if ( logger != NULL ){
			(*logger) << LogIO::NORMAL
				<< "\nLoading dirty image.\n"
				<< LogIO::POST;
		}
		//Get initial image
		String dirtyLocation(dirtyImagePath.toStdString());
		PagedImage<Float> dirtyImageTemp(dirtyLocation);
		if ( lowResImage == NULL ){
			success = false;
		}
		else if( lowResImage->shape().nelements() != dirtyImageTemp.shape().nelements()){
			String msg( "Dirty and low resolution images do not have the same number of axes.");
			if ( logger != NULL ){
				(*logger) << LogIO::SEVERE  << msg << LogIO::EXCEPTION;
			}
			errorMessage = msg.c_str();
			success = false;
		}
		else {
			dirtyImage = new PagedImage<Float>(dirtyLocation);

			resetDirtyFeedImage();
		}
	}
	catch (AipsError& x) {
		String msg = x.getMesg();
		if ( logger != NULL ){
			(*logger) << LogIO::SEVERE << "\nCaught exception: " << msg<< LogIO::EXCEPTION;
		}
		errorMessage = "There was a problem loading the low resolution image.";
		success = false;;
	}
	return success;
}

//------------------------------------------------------------------------------------------
//                                   Data
//------------------------------------------------------------------------------------------

FeatheredData FeatherManager::getSDOrig() {
	FeatheredData dataPair;
	/*if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::SD_ORIGINAL];
		qDebug() << "FeatherManager::getSDOrig empty="<<dataPair.isEmpty();
	}*/

	if ( lowResFeedImage != NULL ){
		dataPair = getConvolvedOrig( lowResFeedImage );
	}
	return dataPair;
}

FeatheredData FeatherManager::getIntOrig() {
	FeatheredData dataPair;
	/*if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::INT_ORIGINAL];
	}*/
	if ( highResFeedImage != NULL ){
		dataPair = getConvolvedOrig( highResFeedImage );
	}
	return dataPair;
}

FeatheredData FeatherManager::getDirtyOrig() {
	FeatheredData dataPair;
	/*if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::DIRTY_ORIGINAL];
	}*/
	if ( dirtyFeedImage != NULL ){
		dataPair = getConvolvedOrig( dirtyFeedImage );
	}
	return dataPair;
}

FeatheredData FeatherManager::getSDWeight() const {
	FeatheredData dataPair;
	if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::SD_WEIGHT];
	}
	return dataPair;
}


FeatheredData FeatherManager::getIntWeight() const {
	FeatheredData dataPair;
	if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::INT_WEIGHT];
	}
	return dataPair;
}
FeatheredData FeatherManager::getSDCut() const {
	FeatheredData dataPair;
	if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::SD_CUT];
	}
	return dataPair;
}

FeatheredData FeatherManager::getIntCut() const {
	FeatheredData dataPair;
	if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::INT_CUT];
	}
	return dataPair;
}

FeatheredData FeatherManager::getDirtyCut() const {
	FeatheredData dataPair;
	if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::DIRTY_CUT];
	}
	return dataPair;
}
FeatheredData FeatherManager::getIntConvolvedSDOrig() {
	FeatheredData dataPair;
	if ( highResFeedImage != NULL ){
		ImageInterface<float>* highConvolved =
			thread->makeConvolvedImage( highResFeedImage, lowResFeedImage );
		if ( highConvolved != NULL ){
			//ImageInterface <Float>* highConvolvedFeed = getSinglePlaneImage( highConvolved );
			//dataPair = getConvolvedOrig( highConvolvedFeed );
			dataPair = getConvolvedOrig( highConvolved );
			delete highConvolved;
			//delete highConvolvedFeed;
		}
	}
	return dataPair;
}

FeatheredData FeatherManager::getConvolvedOrig( ImageInterface<float>* image ) const {
	FeatheredData dataPair;
	Vector<Float> ux;
	Vector<Float> xamp;
	Vector<Float> uy;
	Vector<Float> yamp;
	if ( !radialAxis ){
		Feather::getCutXY(ux, xamp, uy, yamp, *image);
	}
	else {
		Feather::getRadialCut( ux, xamp, *image);
	}
	dataPair.setU( ux, xamp );
	dataPair.setV( uy, yamp );
	return dataPair;
}

FeatheredData FeatherManager::getIntConvolvedSDCut() const {
	FeatheredData dataPair;
	if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::INT_CONVOLVED_LOW_WEIGHTED];
	}
	return dataPair;
}


bool FeatherManager::isReady() const {
	bool imagesAvailable = false;
	if ( lowResImage != NULL && highResImage != NULL ){
		imagesAvailable = true;
	}
	return imagesAvailable;
}

bool FeatherManager::isSuccess() const {
	return success;
}

ImageInterface<float>* FeatherManager::getSinglePlaneImage( ImageInterface<float>* image ) const {
	ImageInterface<float>* feedImage = NULL;
	if ( image != NULL ){
		ImageInterface<float>* imageCopy = thread->addMissingAxes(image);
		if ( !channelsAveraged  ){
			int channelCount = getPlaneCount( image );
			bool channelsExceeded = false;
			if ( channelIndex >= channelCount ){
				channelsExceeded = true;
				if ( logger != NULL ){
					(*logger)<< LogIO::WARN<<"\nAveraging channels for image "<<image->name().c_str()<<
							" because channel index="<<channelIndex<<" is larger than maximum="<<channelCount
							<<LogIO::POST;
				}
			}

			if ( !channelsExceeded ){
				try {

					SubImage<Float>* channeledImage = SpectralImageUtil::getChannel(*imageCopy, channelIndex);
					if ( channeledImage != NULL ){
						feedImage = channeledImage;
					}
					else {
						qDebug() << "Could not obtain image plane plainIndex="<<channelIndex;
					}
					delete imageCopy;
				}
				catch( AipsError& error ){
					channelsExceeded = true;
					if ( logger != NULL ){
						(*logger) << LogIO::WARN<<"\nAveraging channels for image "<<image->name()<<".  Error:"<<error.getMesg()
								<<LogIO::POST;
					}
				}
			}
		}
		else {
			feedImage = imageCopy;
		}
	}
	return feedImage;
}

FeatheredData FeatherManager::getDirtyConvolvedSDOrig() {
	FeatheredData dataPair;
	if ( dirtyFeedImage != NULL ){
		ImageInterface<float>* dirtyConvolved =
			thread->makeConvolvedImage( dirtyFeedImage, lowResFeedImage );
		if ( dirtyConvolved != NULL ){
			dataPair = getConvolvedOrig( dirtyConvolved );
			delete dirtyConvolved;
		}
	}
	return dataPair;
}
FeatheredData FeatherManager::getDirtyConvolvedSDCut() const {
	FeatheredData dataPair;
	if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::DIRTY_CONVOLVED_LOW_WEIGHTED];
	}
	return dataPair;
}

FeatheredData FeatherManager::getSDConvolvedIntOrig(){
	FeatheredData dataPair;
	if ( lowResFeedImage != NULL ){
		ImageInterface<float>* sdConvolved =
			thread->makeConvolvedImage( lowResFeedImage, highResFeedImage );
		if ( sdConvolved != NULL ){
			dataPair = getConvolvedOrig( sdConvolved );
			delete sdConvolved;
		}
	}
	return dataPair;
}
FeatheredData FeatherManager::getSDConvolvedIntCut() const{
	FeatheredData dataPair;
	if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::LOW_CONVOLVED_HIGH_WEIGHTED];
	}
	return dataPair;
}
/*FeatheredData FeatherManager::getSDConvolvedDirtyOrig(){
	FeatheredData dataPair;
	if ( lowResImage != NULL ){
		ImageInterface<float>* sdConvolved = FeatherThread::makeConvolvedImage( lowResImage, dirtyImage );
		if ( sdConvolved != NULL ){
			dataPair = getConvolvedOrig( FeatherThread::LOW_CONVOLVED_DIRTY, sdConvolved );
			delete sdConvolved;
		}
	}
	return dataPair;
}*/
/*FeatheredData FeatherManager::getSDConvolvedDirtyCut() const{
	FeatheredData dataPair;
	if ( thread != NULL ){
		dataPair = thread->dataMap[FeatherThread::LOW_CONVOLVED_DIRTY_WEIGHTED];
	}
	return dataPair;
}*/

FeatherManager::~FeatherManager() {
	delete lowResImage;
	delete highResImage;
	delete dirtyImage;
	delete highResFeedImage;
	delete dirtyFeedImage;
	delete lowResFeedImage;
	delete thread;
	delete featherWorker;
}

} /* namespace casa */
