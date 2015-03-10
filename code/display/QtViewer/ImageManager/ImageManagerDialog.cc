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
#include "ImageManagerDialog.qo.h"
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/DisplayDataHolder.h>
#include <display/QtViewer/ImageManager/ImageView.qo.h>
#include <display/QtViewer/ImageManager/ImageScroll.qo.h>
#include <display/QtViewer/ImageManager/DisplayOptionsDialog.h>
#include <display/Display/ColormapDefinition.h>
#include <display/Display/Colormap.h>
#include <images/Images/ImageStatistics.h>
#include <QDebug>
#include <QMessageBox>

namespace casa {

	ImageManagerDialog::ImageManagerDialog(QWidget *parent)
		: QDialog(parent),
		  allImages( NULL ),
		  displayedImages( NULL ),
		  SINGLE_COLOR_MAP( "Color Saturation Map"),
		  MASTER_COLOR_MAP( "Master Color Saturation Map"),
		  COLOR_MAP_SIZE(100){

		ui.setupUi(this);
		setWindowTitle( "Manage Images");
		setModal( false );
		displayOptionsDialog = NULL;

		//Scroll areas  open and registered images.
		initializeScrollArea();

		//All buttons
		connect( ui.unregisterButton, SIGNAL(clicked()), this, SLOT(unregisterImages()));
		connect( ui.registerButton, SIGNAL(clicked()), this, SLOT(registerImages()));
		connect( ui.closeButton, SIGNAL(clicked()), this, SLOT(closeAll()));

		//Color Restrictions
		QButtonGroup* colorRestrictionGroup = new QButtonGroup( this );
		colorRestrictionGroup->addButton( ui.noneRadio);
		colorRestrictionGroup->addButton( ui.colorPerImageRadio );
		colorRestrictionGroup->addButton( ui.colorMasterImageRadio );
		colorRestrictionsChanged();
		connect( ui.noneRadio, SIGNAL(clicked()), this, SLOT(colorRestrictionsChanged()));
		connect( ui.colorPerImageRadio, SIGNAL(clicked()), this, SLOT(colorRestrictionsChanged()));
		connect( ui.colorMasterImageRadio, SIGNAL(clicked()), this, SLOT(colorRestrictionsChanged()));
		connect( ui.applyButton, SIGNAL(clicked()), this, SLOT(applyColorChanges()));

		ui.colorMasterImageRadio->setVisible( false );
		layout()->removeWidget( ui.colorRestrictionsGroupBox );
		ui.colorRestrictionsGroupBox->setParent( NULL );

		connect( ui.closeDialogButton, SIGNAL( clicked()), this, SLOT(accept()));
	}

	void ImageManagerDialog::initializeScrollArea() {
		imageScroll = new ImageScroll( this );
		connect( imageScroll, SIGNAL(displayTypeChanged(ImageView*)),
		         this, SLOT(displayTypeChanged(ImageView*)) );
		connect( imageScroll, SIGNAL(displayDataRemoved( QtDisplayData*, bool)),
			     this, SLOT( closeImage( QtDisplayData*, bool)));
		connect( imageScroll, SIGNAL(imageOrderingChanged( QtDisplayData*, int, bool,bool)),
				 this, SLOT( reorderDisplayImages(QtDisplayData*, int, bool,bool)));
		connect( imageScroll, SIGNAL(masterCoordinateImageChanged( QtDisplayData*)),
				 this, SLOT( masterImageChanged(QtDisplayData*)));
		connect( imageScroll, SIGNAL(showDataDisplayOptions( QtDisplayData*)),
				 this, SLOT( showDataDisplayOptions( QtDisplayData* )));
		connect( imageScroll, SIGNAL(registrationChange(ImageView*)),
				this, SLOT(registrationChange(ImageView*)));
		connect( imageScroll, SIGNAL(animateToImage(int)),
				this, SIGNAL(animateToImage(int)));
		QHBoxLayout* holderLayout = new QHBoxLayout();
		holderLayout->setContentsMargins(0,0,0,0);
		holderLayout->addWidget( imageScroll );
		ui.imageHolder->setLayout( holderLayout );
	}

	void ImageManagerDialog::setImageHolders( DisplayDataHolder* registeredImages,
	        DisplayDataHolder* images ) {
		//Store the list of all images and add them to the
		//master image combo selection box.
		allImages = images;
		displayedImages = registeredImages;
		if ( allImages != NULL && displayedImages != NULL ){
			allImages->setImageTracker( this );

			//Add the images to the scroll.
			for ( DisplayDataHolder::DisplayDataIterator iter = allImages->beginDD();
				iter != allImages->endDD(); iter++ ){
				bool registered = displayedImages->exists( *iter );
				bool coordinateMaster = allImages->isCoordinateMaster( *iter );
				imageAdded( *iter, -1, registered, coordinateMaster, false, false );

			}
		}
	}

	void ImageManagerDialog::setViewedImage( int registrationIndex ){
		imageScroll->setViewedImage( registrationIndex );
	}


//----------------------------------------------------------------
//                Reordering Images
//----------------------------------------------------------------
	void ImageManagerDialog::reorderDisplayImages( QtDisplayData* displayData,
			int dropIndex, bool registered, bool masterCoordinate){
		if ( displayedImages == NULL || allImages == NULL ){
			return;
		}

		//First remove it from the registered images, it it is preset.
		if ( registered ){
			displayedImages->discardDD( displayData );
		}

		if ( masterCoordinate ){
			displayedImages->setDDControlling( NULL );
		}
		//Now remove it from all the images.
		allImages->discardDD( displayData);

		//We handle adding it to the scroll first, because by the time
		//we get the callbacks, the insert order is lost.
		imageScroll->removeDisplayDataLayout( displayData );
		imageScroll->addDisplayDataLayout( displayData, dropIndex);

		//Now insert it at the correct position in all the images.
		allImages->insertDD( displayData, dropIndex, registered );

		//If it was registered, figure out what it's new index is in the
		//registered images.
		if ( registered ){
			int registeredIndex = imageScroll->getRegisteredIndex( dropIndex );
			displayedImages->insertDD( displayData, registeredIndex,
					registered );
			displayedImages->registrationOrderChanged();
		}

		if ( masterCoordinate ){
			displayedImages->setDDControlling( displayData );
		}


	}

	//-------------------------------------------------------------------------
	//             Master Coordinate Image
	//-------------------------------------------------------------------------

	void ImageManagerDialog::masterImageChanged(QtDisplayData* newMaster ) {
		if ( allImages != NULL && displayedImages != NULL ){

			//See if there was an old master
			QtDisplayData* oldMaster = displayedImages->getDDControlling();

			//So the QtDisplayPanelGui is updated.
			allImages->setDDControlling( newMaster );

			//So the QtDisplayPanel gets notified (does the real work)
			displayedImages->setDDControlling( newMaster );

			emit masterCoordinateChanged( oldMaster, newMaster );
		}
	}

	//-------------------------------------------------------------------
	//           Show Data Display Options Dialog
	//--------------------------------------------------------------------
	void ImageManagerDialog::showDataDisplayOptions( QtDisplayData* imageData ){
		if ( displayOptionsDialog == NULL ){
			displayOptionsDialog = new DisplayOptionsDialog( this );
		}
		displayOptionsDialog->showOptions( imageData );
		displayOptionsDialog->show();
	}

	//----------------------------------------------------------------------
	//            Raster, Contour,Vector,Marker,etc
	//----------------------------------------------------------------------

	void ImageManagerDialog::displayTypeChanged( ImageView* imageView ) {
		//We are going to keep this imageView but replace the data that
		//is stored inside of it.  So that we can recreate the data with
		//the correct parameters, we store the information before deleting
		//the data.

		QColor singleColor = imageView->getDisplayedColor();
		int dropIndex = imageScroll->getIndex( imageView );
		bool registered = imageView->isRegistered();
		bool masterCoordinate = imageView->isMasterCoordinate();
		bool masterSaturation = imageView->isMasterSaturation();
		bool masterHue        = imageView->isMasterHue();

		//Get the data type
		QtDisplayData* imageData = imageView->getData();
		if ( imageData != NULL ){
			String ddType(imageData->dataType());

			//Get the path
            SHARED_PTR<ImageInterface<float> > image = imageData->imageInterface();
			if ( image ) {
				String path = image->name();

				//Get the display type
				QString displayTypeName = imageView->getDataDisplayTypeName();
				String displayType( displayTypeName.toLower().toStdString());

				//Close the dd
				//imageScroll->removeImageView( imageData );
				QtDisplayData* dd = imageView->getData();
				emit closeImage( dd, masterCoordinate );

				//Reopen it with the new specifications
				emit ddOpened( path, ddType, displayType,
						/*, singleColor*/ dropIndex,
						registered, masterCoordinate, masterSaturation,
						masterHue );
			}
		}
	}


//----------------------------------------------------------------------
//                             Color Changes
//----------------------------------------------------------------------

	void ImageManagerDialog::applyColorChanges() {
		//Revert the color changes
		//qDebug() << "Applying color changes sizeof QRGB="<<sizeof(QRgb);
		for ( DisplayDataHolder::DisplayDataIterator iter = allImages->beginDD();
			iter !=allImages->endDD(); iter++ ) {
			(*iter)->removeColorMap( SINGLE_COLOR_MAP );
			(*iter)->removeColorMap( MASTER_COLOR_MAP );
		}
		if ( ui.colorPerImageRadio->isChecked()) {
			QList<ImageView*> views = imageScroll->getViews();

			int count = views.size();
			QtDisplayData* redImage = NULL;
			QtDisplayData* blueImage = NULL;
			QtDisplayData* greenImage = NULL;
			int assignedCount = 0;
			//qDebug() << "Coloring by image count="<<count;
			for( int i = 0; i < count; i++ ){
				QtDisplayData* dd = views[i]->getData();
				if ( dd != NULL && dd->isRaster() ){
					QColor displayedColor = views[i]->getDisplayedColor();
					if ( displayedColor == Qt::red && redImage == NULL){
						redImage = dd;
						assignedCount++;
					}
					else if ( displayedColor == Qt::green && greenImage == NULL ){
						greenImage = dd;
						assignedCount++;
					}
					else if ( displayedColor == Qt::blue && blueImage == NULL ){
						blueImage = dd;
						assignedCount++;
					}
				}
			}
			//At least two of the colored images should be not NULL;
			//qDebug() << "Assigned color count="<<assignedCount;
			if ( assignedCount >= 2 ){
				QtDisplayData* controllingDD = this->displayedImages->getDDControlling();
				if ( controllingDD == NULL ){
					if ( redImage != NULL ){
						controllingDD = redImage;
					}
					else if ( greenImage != NULL ){
						controllingDD = greenImage;
					}
					else {
						controllingDD = blueImage;
					}

				}
				emit createRGBImage( controllingDD, redImage, greenImage, blueImage );
			}
			else {
				//Post a warning
				QMessageBox::warning( this, "Check Image Colors", "Please check that you have assigned colors to at least two images using 'Options...'.");
			}

		}
		else if ( ui.noneRadio->isChecked()) {

		} else if ( ui.colorMasterImageRadio->isChecked()) {
			QString masterColorError;
			bool colorApplied = applyMasterColor( masterColorError );
			if ( !colorApplied ){
				QMessageBox::warning( this, "Master Hue/Saturation Image Problem", masterColorError );
			}
		}
	}

	/*float ImageManagerDialog::getTransparency() const {
		int alphaInt = ui.transparencySpinBox->value();
		float alpha = (alphaInt * 1.0f) / ui.transparencySpinBox->maximum();
		return alpha;
	}*/

	bool ImageManagerDialog::applyMasterColor( QString& /*errorMessage*/ ) {
		bool success = true;
		/*QtDisplayData* colorDD = imageScroll->getHueMaster();
		QtDisplayData* saturationDD = imageScroll->getSaturationMaster();

		if ( colorDD != NULL && saturationDD != NULL ) {
			Colormap* hueMap = colorDD->getColorMap();
			if ( hueMap != NULL ) {
				ColormapDefinition* hueDefinition = hueMap->definition();
				double baseColorMin;
				double baseColorMax;
                SHARED_PTR<ImageInterface<float> > hueInterface = colorDD->imageInterface();
				getIntensityMinMax( hueInterface, &baseColorMin, &baseColorMax );

				for ( DisplayDataHolder::DisplayDataIterator iter = allImages->beginDD();
				        iter != allImages->endDD(); iter++ ) {
					Colormap* ddMap = NULL;
					if ( (*iter) != saturationDD ){
						double intensityMin;
						double intensityMax;
                        SHARED_PTR<ImageInterface<float> > ddInterface = (*iter)->imageInterface();
						getIntensityMinMax( ddInterface, &intensityMin, &intensityMax );
						ddMap = generateMasterDefinition( hueDefinition,
								baseColorMin, baseColorMax, intensityMin, intensityMax );
					}
					else {
						//Saturation image will be on a gray scale.
						ddMap = generateColorMap( QColor("#d3d3d3") );
					}
					ddMap->setName( MASTER_COLOR_MAP );
					ddMap->setAlpha( getTransparency());
					(*iter)->setColorMap( ddMap );
				}
			}
			else {
				success = false;
				errorMessage ="The master hue and/or saturation image is missing a color map.";
			}
		}
		else {
			success = false;
			QString missing = "";
			if ( colorDD == NULL ){
				missing.append( "hue");
			}
			if ( saturationDD == NULL ){
				if ( missing.length() > 0 ){
					missing.append( " and ");
				}
				missing.append( "saturation" );
			}
			errorMessage ="Please specify a master ";
			errorMessage.append( missing );
			errorMessage.append(" image by right clicking the mouse on an image.");
		}*/
		return success;
	}


	float ImageManagerDialog::getColorFraction( float value, double minValue, double maxValue ){
		float fraction = 0;
		if ( value < minValue ){
			fraction = 0;
		}
		else if ( value > maxValue ){
			fraction = 1;
		}
		else {
			double span = maxValue - minValue;
			if ( span > 0 ){
				fraction = ( value - minValue ) / span;
			}
		}
		return fraction;
	}


	Colormap* ImageManagerDialog::generateMasterDefinition( ColormapDefinition* baseMap,
			double colorMin, double colorMax,
			double intensityMin, double intensityMax ){
		Vector<Float> reds(COLOR_MAP_SIZE);
		Vector<Float> greens(COLOR_MAP_SIZE);
		Vector<Float> blues(COLOR_MAP_SIZE);
		float intensity = intensityMin;
		float intensitySpan = intensityMax - intensityMin;
		float intensityIncrement = intensitySpan / COLOR_MAP_SIZE;
		for ( int i = 0; i < COLOR_MAP_SIZE; i++ ) {
			//First decide on the color.
			float colorFraction = getColorFraction( intensity, colorMin, colorMax );
			Float redAmount;
			Float blueAmount;
			Float greenAmount;
			baseMap->getValue( colorFraction, redAmount, greenAmount, blueAmount );

			//Now convert back to rgb for colormap
			reds[i]   = static_cast<int>(redAmount);
			greens[i] = static_cast<int>(greenAmount);
			blues[i]  = static_cast<int>(blueAmount);
			intensity = intensity + intensityIncrement;
		}
		ColormapDefinition* hueRamp = new ColormapDefinition(MASTER_COLOR_MAP, reds, greens, blues);
		Colormap* hueMap = new Colormap();
		hueMap->setColormapDefinition( hueRamp );
		return hueMap;
	}

	Colormap* ImageManagerDialog::generateColorMap( QColor baseColor ) {
		Colormap* saturationMap = NULL;
		ColormapDefinition* saturationDefinition = generateSaturationMap( baseColor );
		//Make it into a color map.
		saturationMap = new Colormap();
		String title( SINGLE_COLOR_MAP );
		saturationMap->setName( title );
		saturationMap->setColormapDefinition( saturationDefinition );
		//saturationMap->setAlpha( getTransparency() );
		return saturationMap;
	}



	ColormapDefinition* ImageManagerDialog::generateSaturationMap( QColor baseColor ) {

		Vector<Float> reds(COLOR_MAP_SIZE);
		Vector<Float> greens(COLOR_MAP_SIZE);
		Vector<Float> blues(COLOR_MAP_SIZE);

		int redAmount = baseColor.red();
		int greenAmount = baseColor.green();
		int blueAmount = baseColor.blue();
		//Based on the HSP Color system.
		double saturation = qSqrt(qPow(redAmount,2) * .299
				+ qPow(greenAmount,2)*.587 + qPow(blueAmount,2)*.114);
		if ( saturation == redAmount && saturation == blueAmount && saturation == greenAmount ){
			//Dealing with a shade of gray.  Just go from medium gray to white
			float startIncrement = 0.5f;
			float baseIncrement = (1.0f - startIncrement) / COLOR_MAP_SIZE;
			for ( int i = 0; i < COLOR_MAP_SIZE; i++ ){
				reds[i] = baseIncrement * i + startIncrement;
				greens[i] = baseIncrement * i + startIncrement;
				blues[i] = baseIncrement * i + startIncrement;
			}
		}
		else {
			float maxChange = 2.0f;
			float baseIncrement = maxChange / COLOR_MAP_SIZE;
			for ( int i = 0; i < COLOR_MAP_SIZE; i++ ) {
				float change = maxChange - i * baseIncrement;
				reds[i]   = saturation + (redAmount - saturation) * change;
				greens[i] = saturation + (greenAmount - saturation) * change;
				blues[i]  = saturation + (blueAmount - saturation) * change;
			}
		}
		ColormapDefinition* saturationRamp = new ColormapDefinition("saturationRamp", reds, greens, blues);
		return saturationRamp;
	}


	bool ImageManagerDialog::getIntensityMinMax( SHARED_PTR<ImageInterface<float> > img,
	        double* intensityMin, double* intensityMax ) {
		ImageStatistics<Float> stats(*img, False);
		bool success = true;

		int axisCount = img->ndim();
		Vector<Int> cursorAxes( axisCount );
		for ( int i = 0; i < axisCount; i++ ) {
			cursorAxes[i] = i;
		}

		if(!stats.setAxes(cursorAxes)) {
			success = false;
		} else {
			// This tells stats to compute just one set of statistics
			// for the entire (sub-)image.

			// Gather robust stats (for now, this is omitted as
			// too time-consuming for more than 10 megapixels).

			Array<Double> minVals;
			stats.getStatistic(minVals, LatticeStatsBase::MIN);
			Array<Double> maxVals;
			stats.getStatistic( maxVals, LatticeStatsBase::MAX);
			if ( minVals.size() > 0 ) {
				Vector<Double> minVector = minVals;
				*intensityMin = minVector[0];
			}
			if ( maxVals.size() > 0 ) {
				Vector<Double> maxVector = maxVals;
				*intensityMax = maxVector[0];
			}
		}
		return success;
	}

	ImageView::ColorCombinationMode ImageManagerDialog::getColorCombinationMode() const {
		ImageView::ColorCombinationMode mode = ImageView::NO_COMBINATION;
		if ( ui.colorPerImageRadio->isChecked()){
			mode = ImageView::RGB;
		}
		else if ( ui.colorMasterImageRadio->isChecked()){
			mode = ImageView::HUE_SATURATION;
		}
		return mode;
	}

	void ImageManagerDialog::colorRestrictionsChanged() {
		ImageView::ColorCombinationMode mode = getColorCombinationMode();
		imageScroll->setColorCombinationMode( mode );
	}

//-------------------------------------------------------------------
//                   Closing & Registration
//------------------------------------------------------------------
	void ImageManagerDialog::closeImageView( QtDisplayData* image ){
		imageScroll->removeImageView( image );
	}

	void ImageManagerDialog::closeImage( QtDisplayData* image, bool controlling ) {
		if ( allImages != NULL && allImages->exists(image)) {
			//If it was the image used to set the master coordinate system,
			//we notify that the master coordinate system image is NULL
			if ( controlling ){
				allImages->setDDControlling( NULL );
				displayedImages->setDDControlling( NULL );
			}
			//Close the dd
			emit ddClosed( image );
		}
		updateAllButtons();
	}


	void ImageManagerDialog::closeAll() {
		if ( allImages != NULL && displayedImages != NULL ){
			//Notify that there will be no master image used to set the
			//coordinate system.
			allImages->setDDControlling( NULL );
			displayedImages->setDDControlling( NULL );

			//Close all the images.
			imageScroll->closeImages();
		}
	}

	void ImageManagerDialog::updateAllButtons(){
		int imageCount = imageScroll->getImageCount();
		int registeredCount = imageScroll->getRegisteredCount();

		bool enableClose = true;
		bool enableRegister = true;
		bool enableUnregister = true;
		if ( imageCount == 0 ){
			enableClose = false;
			enableRegister = false;
			enableUnregister = false;
		}
		else {
			if ( registeredCount == 0 ){
				enableUnregister = false;
			}
			else if ( imageCount == registeredCount ){
				enableRegister = false;
			}
		}

		ui.closeButton->setEnabled( enableClose );
		ui.registerButton->setEnabled( enableRegister );
		ui.unregisterButton->setEnabled( enableUnregister );
	}

	void ImageManagerDialog::registrationChange( ImageView* imageView ){
		bool registerImage = imageView->isRegistered();

		QtDisplayData* displayData = imageView->getData();

		if ( registerImage ){
			int overallIndex = imageScroll->getIndex( imageView );
			int registerPosition = imageScroll->getRegisteredIndex(overallIndex);
			emit registerDD( displayData, registerPosition );
		}
		else {
			emit unregisterDD( displayData );
		}
		updateAllButtons();
	}

	void ImageManagerDialog::registerImages(){
		imageScroll->setRegisterAll( true );
		emit registerAll();
		updateAllButtons();
	}

	void ImageManagerDialog::unregisterImages(){
		imageScroll->setRegisterAll( false );
		emit unregisterAll();
		updateAllButtons();
	}


//------------------------------------------------------------------
//              Image Tracker Interface
//-------------------------------------------------------------------


	//Called by the display data holder when a new image is opened.
	void ImageManagerDialog::imageAdded( QtDisplayData* image, int position,
			bool autoRegister, bool masterCoordinate,
			bool masterSaturation, bool masterHue ) {

		if ( allImages == NULL && displayedImages == NULL ){
			return;
		}


		imageScroll->addImageView( image, autoRegister,
				getColorCombinationMode(), position,
				masterCoordinate, masterSaturation, masterHue);

		if ( masterCoordinate ){
			allImages->setDDControlling( image );
			displayedImages->setDDControlling( image );
		}
		updateAllButtons();
	}



	void ImageManagerDialog::masterImageSelected( QtDisplayData* image ) {
		//Called by the displayDataHolder if we have a new master image.
		//Update the view to display the new master image
		if ( image != NULL ) {
			QString imageName( image->name().c_str());
			imageScroll->setMasterCoordinateImage( imageName );
		}
	}


	ImageManagerDialog::~ImageManagerDialog() {
		//Note:  allImages and displayedImages are deleted in the QtDisplayPanelGui
		//and QtDisplayPanel, respectively.  This is because someone may not even use
		//the image manager.
		delete ui.colorRestrictionsGroupBox;
	}
}
