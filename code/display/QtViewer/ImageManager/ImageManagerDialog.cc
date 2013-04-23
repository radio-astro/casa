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
#include <display/QtViewer/ImageManager/ImageScrollWidget.qo.h>
#include <display/Display/ColormapDefinition.h>
#include <display/Display/Colormap.h>
#include <images/Images/ImageStatistics.h>
#include <QDebug>

namespace casa {

ImageManagerDialog::ImageManagerDialog(QWidget *parent)
    : QDialog(parent), openHolder( NULL ), allImages( NULL )
{
	ui.setupUi(this);
	setWindowTitle( "Manage Images");

	//Scroll areas containing open and registered images.
	initializeScrollArea( ui.openHolder, openScroll );
	initializeScrollArea( ui.displayedHolder, displayedScroll );
	connect( displayedScroll, SIGNAL(displayDataRemoved( QtDisplayData*)),
			this, SLOT( unDisplayImage( QtDisplayData*)));
	connect( displayedScroll, SIGNAL(displayDataAdded( QtDisplayData*)),
				this, SLOT( displayImage( QtDisplayData*)));

	//Master image
	connect( ui.masterImageComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(masterImageChanged(const QString&)));

	//Color Restrictions
	QButtonGroup* colorRestrictionGroup = new QButtonGroup( this );
	colorRestrictionGroup->addButton( ui.noneRadio);
	colorRestrictionGroup->addButton( ui.singleColorRadio );
	colorRestrictionGroup->addButton( ui.singleColorSaturationRadio );
	colorRestrictionsChanged();
	connect( ui.noneRadio, SIGNAL(clicked()), this, SLOT(colorRestrictionsChanged()));
	connect( ui.singleColorRadio, SIGNAL(clicked()), this, SLOT(colorRestrictionsChanged()));
	connect( ui.singleColorSaturationRadio, SIGNAL(clicked()), this, SLOT(colorRestrictionsChanged()));

	connect( ui.openToRegisteredButton, SIGNAL(clicked()), this, SLOT(openToDisplayed()));
	connect( ui.registerToOpenButton, SIGNAL(clicked()), this, SLOT(displayedToOpen()));
	connect( ui.closeImageButton, SIGNAL(clicked()), this, SLOT(closeImage()));
}

//----------------------------------------------------------------
//                 Master Image
//----------------------------------------------------------------

void ImageManagerDialog::masterImageChanged(const QString& imageName){
	QtDisplayData* newMaster = allImages->getDD( imageName.toStdString());
	if ( newMaster != NULL && allImages != NULL ){

		//So the QtDisplayPanelGui is updated.
		allImages->setDDControlling( newMaster );

		//So the QtDisplayPanel gets notified (does the real work)
		displayedScroll->setControllingDD( newMaster );


	}
}

bool ImageManagerDialog::isControlEligible( QtDisplayData* qdd ) const {
	bool controlEligible = false;
	if ( qdd != NULL ){
		if ( qdd->isImage() ){
			if ( qdd->dd()->isDisplayable() && qdd->imageInterface() != NULL ){
				controlEligible = true;
			}
		}
	}
	return controlEligible;
}

void ImageManagerDialog::updateSelectedMaster( const QString& previousMaster ){
	//Now select the one that is the master image.
	if ( allImages != NULL ){
		bool selectionMade = false;
		//First see if a controlling dd exists
		QtDisplayData* display = allImages->getDDControlling();
		if ( display != NULL ){
			QString controlName(display->name().c_str());
			int masterIndex = ui.masterImageComboBox->findText(controlName);
			if ( masterIndex >= 0 ){
				setComboIndex( ui.masterImageComboBox, masterIndex );
				selectionMade = true;
			}
		}
		//Okay, no controlling DD.  Try to use the previous master if there was one.
		if ( !selectionMade && previousMaster.length() > 0 ){
			int previousMasterIndex = ui.masterImageComboBox->findText( previousMaster );
			if ( previousMasterIndex >= 0 ){
				setComboIndex( ui.masterImageComboBox, previousMasterIndex );
				selectionMade = true;
			}
		}

		//Just use the first one
		if ( !selectionMade ){
			setComboIndex( ui.masterImageComboBox, 0 );
		}
	}
}

void ImageManagerDialog::setComboIndex( QComboBox* combo, int index ){
	combo->blockSignals( true );
	combo->setCurrentIndex( index );
	combo->blockSignals( false );
}

void ImageManagerDialog::updateMasterList(){
	QString oldMaster = ui.masterImageComboBox->currentText();
	updateImageList( ui.masterImageComboBox );
	updateSelectedMaster( oldMaster );
}

void ImageManagerDialog::updateSaturationList(){
	updateImageList( ui.saturationImageCombo );
	//updateSelectedMaster();
}

void ImageManagerDialog::updateColorList(){
	updateImageList( ui.colorImageCombo );
}

void ImageManagerDialog::updateImageList(QComboBox* combo){
	combo->clear();
	for ( DisplayDataHolder::DisplayDataIterator iter = allImages->beginDD();
		iter != allImages->endDD(); iter++ ){
		if ( isControlEligible( *iter) ){
			QString imageName( (*iter)->name().c_str());
			combo->addItem( imageName );
		}
	}
}


void ImageManagerDialog::setImageHolders( DisplayDataHolder* displayedImages,
		DisplayDataHolder* images ){
	//Store the list of all images and add them to the
	//master image combo selection box.
	allImages = images;
	allImages->setImageTracker( this );
	updateMasterList();
	updateSaturationList();
	updateColorList();

	//Give the displayed images to the display scroll so it
	//can create GUI's
	displayedScroll->setImageHolder( displayedImages );

	//Create a data holder for those images that are open,
	//but not displayed.  Give this data holder to the openScroll.
	openHolder = new DisplayDataHolder();
	for ( DisplayDataHolder::DisplayDataIterator iter = allImages->beginDD();
			iter != allImages->endDD(); iter++ ){
		if ( ! displayedImages->exists( (*iter)) ){
			openHolder->addDD( *iter);
		}
	}
	openScroll->setImageHolder( openHolder );
}

void ImageManagerDialog::initializeScrollArea( QWidget* holder, ImageScrollWidget*& scrollArea ){
	scrollArea = new ImageScrollWidget( this );
	connect( scrollArea, SIGNAL(displayTypeChanged(ImageView*)),
			this, SLOT(displayTypeChanged(ImageView*)) );
	connect( scrollArea, SIGNAL(displayColorsChanged(ImageView*)),
			this, SLOT(displayColorsChanged(ImageView*)));
	QHBoxLayout* holderLayout = new QHBoxLayout();
	holderLayout->setContentsMargins(0,0,0,0);
	holderLayout->addWidget( scrollArea );
	holder->setLayout( holderLayout );
}

void ImageManagerDialog::displayTypeChanged( ImageView* imageView ){
	//We are going to be removing this imageView so store what we
	//are going to need.
	//Get the single image color
	QColor singleColor = imageView->getDisplayedColor();

	//Get the data type
	QtDisplayData* imageData = imageView->getData();
	String ddType(imageData->dataType());

	//Get the path
	ImageInterface<float>* image = imageData->imageInterface();
	if ( image != NULL ){
		String path = image->name();

		//Get the display type
		QString displayTypeName = imageView->getDataDisplayTypeName();
		String displayType( displayTypeName.toLower().toStdString());

		//Close the dd
		emit ddClosed( imageData );

		//Reopen it with the new specifications
		emit ddOpened( path, ddType, displayType/*, singleColor*/ );
	}
}


void ImageManagerDialog::displayColorsChanged( ImageView* imageView ){
	if ( ui.singleColorRadio->isChecked()){

		//Find the min and max intensity
		double intensityMin = 0;
		double intensityMax = 0;
		QtDisplayData* imageData = imageView->getData();
		ImageInterface<float>* img = imageData->imageInterface();
		bool intensityRangeFound = getIntensityMinMax( img, &intensityMin, & intensityMax );
		if ( intensityRangeFound ){
			//Generate a color definition.
			QColor baseColor = imageView->getDisplayedColor();
			ColormapDefinition* saturationDefinition = generateSaturationMap( intensityMin, intensityMax,
					baseColor );

			//Make it into a color map.
			Colormap* saturationMap = new Colormap();
			saturationMap->setName( "saturationMap");
			saturationMap->setColormapDefinition( saturationDefinition );

			//Add it to the QtDisplayData and tell it to use it.
			imageData->setColorMap( saturationMap );


		}
	}
}

ColormapDefinition* ImageManagerDialog::generateSaturationMap( double /*minIntensity*/, double /*maxIntensity*/,
		QColor baseColor ){
	float hue = baseColor.hueF();
	float value = baseColor.valueF();
	const int VALUE_COUNT = 40;
	Vector<Float> reds(VALUE_COUNT);
	Vector<Float> greens(VALUE_COUNT);
	Vector<Float> blues(VALUE_COUNT);
	float saturation = 0;
	float saturationIncrement = 1.0f / VALUE_COUNT;
	for ( int i = 0; i < VALUE_COUNT; i++ ){
		QColor valueColor;
		valueColor.setHsvF( hue, saturation, value );
		reds[i]   = valueColor.redF();
		greens[i] = valueColor.greenF();
		blues[i]  = valueColor.blueF();
		saturation = saturation + saturationIncrement;
	}
	ColormapDefinition* saturationRamp = new ColormapDefinition("saturationRamp", reds, greens, blues);
	return saturationRamp;
}


bool ImageManagerDialog::getIntensityMinMax( ImageInterface<float>* img,
		double* intensityMin, double* intensityMax ){
	ImageStatistics<Float> stats(*img, False);
	bool success = true;

	int axisCount = img->ndim();
	Vector<Int> cursorAxes( axisCount );
	for ( int i = 0; i < axisCount; i++ ){
		cursorAxes[i] = i;
	}

	if(!stats.setAxes(cursorAxes)){
		success = false;
	}
	else {
		// This tells stats to compute just one set of statistics
		// for the entire (sub-)image.

		// Gather robust stats (for now, this is omitted as
		// too time-consuming for more than 10 megapixels).

		Array<Double> minVals;
		stats.getStatistic(minVals, LatticeStatsBase::MIN);
		Array<Double> maxVals;
		stats.getStatistic( maxVals, LatticeStatsBase::MAX);
		if ( minVals.size() > 0 ){
			Vector<Double> minVector = minVals;
			*intensityMin = minVector[0];
		}
		if ( maxVals.size() > 0 ){
			Vector<Double> maxVector = maxVals;
			*intensityMax = maxVector[0];
		}
	}
	return success;
}

void ImageManagerDialog::openToDisplayed(){
	QList<ImageView*> movedImages = openScroll->getSelectedViews();
	openScroll->removeImageViews( movedImages );
	displayedScroll->addImageViews( movedImages );
}

void ImageManagerDialog::displayedToOpen(){
	QList<ImageView*> movedImages = displayedScroll->getSelectedViews();
	displayedScroll->removeImageViews( movedImages );
	openScroll->addImageViews( movedImages );
}


void ImageManagerDialog::closeImage(){
	openScroll->closeImages();
	displayedScroll->closeImages();
}

//-----------------------------------------------------------------
//          Notification from the "Displayed" scroll when an
//       image is added/removed.  Update the "Open" scroll data holder.
//-----------------------------------------------------------------

void ImageManagerDialog::unDisplayImage( QtDisplayData* image ){
	if ( allImages->exists(image)){
		openHolder->addDD( image );
	}
}

void ImageManagerDialog::displayImage( QtDisplayData* image ){
	openHolder->removeDD( image );
}

//------------------------------------------------------------------
//              Image Tracker Interface
//-------------------------------------------------------------------

void ImageManagerDialog::imageAdded( QtDisplayData* image ){
	bool displayedImage = openScroll->isManaged( image );
	if ( !displayedImage ){
		openHolder->addDD( image );
	}
	updateMasterList();
	updateColorList();
	updateSaturationList();
}

void ImageManagerDialog::imageRemoved( QtDisplayData* image ){
	openHolder->removeDD( image );
	updateMasterList();
	updateColorList();
	updateSaturationList();
}

void ImageManagerDialog::masterImageSelected( QtDisplayData* image ){
	//Called by the displayDataHolder if we have a new master image.
	//Update the combo box to display the new master.
	if ( image != NULL ){
		QString imageName = image->name().c_str();
		QString selectedImageName = ui.masterImageComboBox->currentText();
		if ( imageName != selectedImageName ){
			//We are supposed to feed it the previous master, but it should
			//select the new one from the underlying data.
			updateSelectedMaster( selectedImageName);
		}
	}
}

//-----------------------------------------------------------
//                    Color Restrictions
//-----------------------------------------------------------

void ImageManagerDialog::colorRestrictionsChanged(){

	//Enable/Disable the color buttons on individual images
	bool masterColorEnabled = false;
	if ( ui.singleColorRadio->isChecked() ||
			ui.singleColorSaturationRadio->isChecked()){
		masterColorEnabled = true;
	}
	openScroll->setImageColorsEnabled( masterColorEnabled );
	displayedScroll->setImageColorsEnabled( masterColorEnabled );

	//Enable/Disable the saturation/color image selection
	//combos.
	bool masterImages = false;
	if ( ui.singleColorSaturationRadio->isChecked()){
		masterImages = true;
	}
	ui.colorImageCombo->setEnabled( masterImages );
	ui.saturationImageCombo->setEnabled( masterImages );
}



ImageManagerDialog::~ImageManagerDialog(){
	delete allImages;
}
}
