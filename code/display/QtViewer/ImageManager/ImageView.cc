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
#include "ImageView.qo.h"
#include <casa/BasicSL/String.h>
#include <casa/Containers/ValueHolder.h>
#include <display/QtViewer/ImageManager/ImageView.qo.h>
#include <display/QtViewer/ImageManager/DisplayLabel.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtPlotter/conversion/Converter.h>
#include <limits>

#include <QUuid>
#include <QDrag>
#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>


namespace casa {
	const QString ImageView::DROP_ID = "Image Name";
	ImageView::ImageView(QtDisplayData* data, QWidget *parent)
		: QFrame(parent),
		  viewAction( "View", this ),
		  closeAction( "Close", this ),
		  masterCoordinateSystemAction( "Coordinate System Master", this ),
		  masterCoordinateSystemUndoAction( "Clear Coordinate System Master", this ),
		  masterHueAction( "Hue Image", this ),
		  masterSaturationAction( "Saturation Image", this ),
		  rasterAction( "Raster", this ),
		  contourAction( "Contour", this ),
		  normalColor("#F0F0F0"),
		  masterCoordinateColor("#BABABA" ),
		  imageData( NULL ),
		  REST_FREQUENCY_KEY("axislabelrestvalue"),
		  VALUE_KEY( "value"),
		  SIZE_COLLAPSED( 50 ), SIZE_EXPANDED( 200 ),
		  VIEWED_BORDER_SIZE(5), NOT_VIEWED_BORDER_SIZE(2){

		ui.setupUi(this);

		initDisplayLabels();
		minimumSize = SIZE_COLLAPSED;
		spacerFirst = new QSpacerItem( 1, 1, QSizePolicy::MinimumExpanding );
		spacerLast = new QSpacerItem( 1, 1, QSizePolicy::MinimumExpanding );
		setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

		//Context Menu
		setContextMenuPolicy( Qt::CustomContextMenu );
		connect( this, SIGNAL( customContextMenuRequested( const QPoint&)), this, SLOT(showContextMenu( const QPoint&)) );

		if ( data != NULL ) {
			imageData = data;
			empty = false;
		}
		else {
			empty = true;
		}


		//Rest frequency/wavelength information
		initRestSettings();

		//Registration
		ui.selectCheckBox->setChecked( false );
		connect( ui.selectCheckBox, SIGNAL(toggled(bool)),
				this, SLOT(imageRegistrationChanged(bool)));

		//Color settings
		setBackgroundColor( normalColor );
		setAutoFillBackground( true );
		initColorModeSettings();

		//Data options
		connect( ui.dataOptionsButton, SIGNAL(clicked()), this, SLOT(showDataOptions()));

		//Title of the image
		setTitle();

		//Display type of the image
		initDisplayType();

		//Open close the image view.
		minimizeDisplay();
		connect( ui.openCloseButton, SIGNAL(clicked()), this, SLOT(openCloseDisplay()));

		rgbModeChanged();
	}


	void ImageView::setTitle(){
		QString name;
		if ( imageData == NULL ) {
			QUuid uid = QUuid::createUuid();
			name = uid.toString();
		}
		else {
			String imageName = imageData->name();
			name = imageName.c_str();
		}
		if ( isMasterCoordinate( ) ){
			name = "<b>"+name+"</b>";
		}
		ui.imageNameLabel->setText( name );
	}

	void ImageView::initColorModeSettings(){
		ui.colorLabel->setAutoFillBackground( true );
		colorMode = NO_COMBINATION;
		setColorCombinationMode( colorMode );
		//setButtonColor( normalColor );
		QButtonGroup* colorGroup = new QButtonGroup( this );
		colorGroup->addButton( ui.redRadio );
		colorGroup->addButton( ui.greenRadio );
		colorGroup->addButton( ui.blueRadio );
		//colorGroup->addButton( ui.otherRadio );
		//ui.otherRadio->setChecked( true );
		connect( ui.redRadio, SIGNAL(clicked()), this, SLOT(rgbChanged()));
		connect( ui.greenRadio, SIGNAL(clicked()), this, SLOT(rgbChanged()));
		connect( ui.blueRadio, SIGNAL(clicked()), this, SLOT(rgbChanged()));
		//connect( ui.otherRadio, SIGNAL(clicked()), this, SLOT(otherColorChanged()));
		//connect( ui.colorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));
		//otherColorChanged();

		//layout()->removeWidget( ui.colorLabel );
		//ui.colorLabel->setParent( NULL );
	}

	void ImageView::initDisplayLabel( QWidget* holder, DisplayLabel* label ){
		QVBoxLayout* verticalLayout = new QVBoxLayout(holder);
		verticalLayout->setContentsMargins( 0, 0, 0, 3 );
		verticalLayout->addWidget( label, Qt::AlignTop | Qt::AlignCenter );
		holder->setLayout( verticalLayout );
	}

	void ImageView::initDisplayLabels(){
		displayTypeLabel = new DisplayLabel( 1, NULL );
		initDisplayLabel( ui.displayTypeHolder, displayTypeLabel );

		coordinateMasterLabel = new DisplayLabel( 2, NULL );
		initDisplayLabel( ui.coordinateMasterHolder, coordinateMasterLabel );

		/*hueMasterLabel = new DisplayLabel( 1, this );
		initDisplayLabel( ui.hueMasterHolder, hueMasterLabel );

		saturationMasterLabel = new DisplayLabel( 1, this );
		initDisplayLabel( ui.saturationMasterHolder, saturationMasterLabel );*/
		hueMasterLabel = NULL;
		saturationMasterLabel = NULL;
		layout()->removeWidget( ui.hueMasterHolder );
		layout()->removeWidget( ui.saturationMasterHolder);
		ui.hueMasterHolder->setParent( NULL );
		ui.saturationMasterHolder->setParent( NULL );
	}

	void ImageView::initDisplayType() {
		displayGroup = new QButtonGroup( this );
		displayGroup->addButton( ui.contourRadio, DISPLAY_CONTOUR );
		displayGroup->addButton( ui.rasterRadio, DISPLAY_RASTER );
		displayGroup->addButton( ui.vectorRadio, DISPLAY_VECTOR );
		displayGroup->addButton( ui.markerRadio, DISPLAY_MARKER );
		displayTypeMap.insert( DISPLAY_CONTOUR, "C");
		displayTypeMap.insert( DISPLAY_RASTER, "R");
		displayTypeMap.insert( DISPLAY_VECTOR, "V");
		displayTypeMap.insert( DISPLAY_MARKER, "M");
		if ( imageData != NULL ) {
			if ( imageData->isContour() ) {
				ui.contourRadio->setChecked(true);
				displayTypeLabel->setText( displayTypeMap[DISPLAY_CONTOUR] );
				storedDisplay = DISPLAY_CONTOUR;
			} else if ( imageData->isRaster() ) {
				ui.rasterRadio->setChecked( true );
				storedDisplay = DISPLAY_RASTER;
				displayTypeLabel->setText( displayTypeMap[DISPLAY_RASTER] );
			} else if ( imageData->isMarker()) {
				ui.markerRadio->setChecked( true );
				storedDisplay = DISPLAY_MARKER;
				displayTypeLabel->setText( displayTypeMap[DISPLAY_MARKER] );
			} else if ( imageData->isVector()) {
				ui.vectorRadio->setChecked( true );
				storedDisplay = DISPLAY_VECTOR;
				displayTypeLabel->setText( displayTypeMap[DISPLAY_VECTOR] );
			}
		}
		connect( ui.contourRadio, SIGNAL(clicked()), this, SLOT(displayTypeChanged()));
		connect( ui.rasterRadio, SIGNAL(clicked()), this, SLOT(displayTypeChanged()));
		connect( ui.vectorRadio, SIGNAL(clicked()), this, SLOT(displayTypeChanged()));
		connect( ui.markerRadio, SIGNAL(clicked()), this, SLOT(displayTypeChanged()));
	}


	//-----------------------------------------------------------------------------
	//              Getters
	//-----------------------------------------------------------------------------

	bool ImageView::isControlEligible() const {
		bool controlEligible = false;
		if ( imageData != NULL ) {
			if ( imageData->isImage() ) {
				if ( imageData->dd()->isDisplayable() && imageData->imageInterface() != NULL ) {
					controlEligible = true;
				}
			}
		}
		return controlEligible;
	}

	bool ImageView::isRegistered() const {
		return ui.selectCheckBox->isChecked();
	}

	QtDisplayData* ImageView::getData() const {
		return imageData;
	}

	QString ImageView::getDataDisplayTypeName() const {
		QAbstractButton* button = displayGroup->checkedButton();
		return button->text();
	}

	bool ImageView::isMasterHue() const {
		bool masterHue = false;
		if ( hueMasterLabel != NULL ){
			masterHue = !hueMasterLabel->isEmpty();
		}
		return masterHue;
	}

	bool ImageView::isMasterSaturation() const {
		bool masterSaturation = false;
		if ( saturationMasterLabel != NULL ){
			masterSaturation = !saturationMasterLabel->isEmpty();
		}
		return masterSaturation;
	}

	bool ImageView::isEmpty() const {

		return empty;
	}

	bool ImageView::isMasterCoordinate() const {
		bool masterCoordinate = !coordinateMasterLabel->isEmpty();
		return masterCoordinate;
	}

	bool ImageView::isViewed() const {
		bool imageViewed = false;
		int borderWidth = lineWidth();
		if ( borderWidth == VIEWED_BORDER_SIZE ){
			imageViewed = true;
		}
		return imageViewed;
	}

	QString ImageView::getName() const {
		QString nameStr = ui.imageNameLabel->text();
		nameStr = nameStr.replace( "<b>", "");
		nameStr = nameStr.replace( "</b>", "");
		return nameStr;
	}



	bool ImageView::isRaster() const {
		bool rasterImage = false;
		if ( displayTypeLabel != NULL ){
			QString displayString = displayTypeLabel->getText();
			if ( displayString == displayTypeMap[DISPLAY_RASTER]){
				rasterImage = true;
			}
		}
		return rasterImage;
	}


	//---------------------------------------------------------------------------------
	//                     Rest Frequency/Wavelength
	//---------------------------------------------------------------------------------

	void ImageView::initRestSettings(){
		frequencyUnits = (QStringList() << "Hz"<<"MHz"<<"GHz");
		wavelengthUnits = (QStringList() << "mm"<<"um"<<"nm"<<"Angstrom");

		QButtonGroup* buttonGroup = new QButtonGroup( this );
		buttonGroup->addButton( ui.frequencyRadio );
		buttonGroup->addButton( ui.wavelengthRadio );
		connect( ui.frequencyRadio, SIGNAL(clicked()), this, SLOT(restChanged()));
		connect( ui.wavelengthRadio, SIGNAL(clicked()), this, SLOT(restChanged()));

		double maxValue = std::numeric_limits<double>::max();
		double minValue = 0;
		QValidator* restValidator = new QDoubleValidator( minValue, maxValue, 8, this);
		ui.restLineEdit->setValidator( restValidator );

		Record options = imageData->getOptions();
		if ( options.isDefined( REST_FREQUENCY_KEY)){
			Record restRecord = options.asRecord( REST_FREQUENCY_KEY);
			String restValueStr = restRecord.asString( VALUE_KEY);
			QString restUnits;
			String restValue;
			int unitsIndex = -1;
			for ( int i = 0; i < frequencyUnits.size(); i++ ){
				unitsIndex = restValueStr.index( frequencyUnits[i].toStdString());
				if ( unitsIndex >= 0 ){
					restValue = restValueStr.before(unitsIndex );
					restUnits = frequencyUnits[i];
					ui.frequencyRadio->setChecked( true );
					restType = REST_FREQUENCY;
					break;
				}
			}
			if ( unitsIndex < 0 ){
				for ( int i = 0; i < wavelengthUnits.size(); i++ ){
					unitsIndex = restValueStr.index( wavelengthUnits[i].toStdString());
					if ( unitsIndex >= 0 ){
						restValue = restValueStr.before( unitsIndex );
						restUnits = wavelengthUnits[i];
						ui.wavelengthRadio->setChecked( true );
						restType = REST_WAVELENGTH;
						break;
					}
				}
			}
			if ( unitsIndex >= 0 ){
				ui.restLineEdit->setText( restValue.c_str());
				int index = ui.restUnitsCombo->findText( restUnits );
				ui.restUnitsCombo->setCurrentIndex( index );
			}
		}
		//cout << options << endl;
		connect( ui.restLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(restFrequencyChanged()));
		connect( ui.restUnitsCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(restUnitsChanged()));
		restChanged();
	}

	bool ImageView::isCategoryMatch( const QString& newUnits, const QString& oldUnits ) const {
		bool categoryMatch = false;
		if ( frequencyUnits.indexOf( newUnits )>= 0 && frequencyUnits.indexOf( oldUnits )>= 0 ){
			categoryMatch = true;
		}
		else if ( wavelengthUnits.indexOf( newUnits )>= 0 && wavelengthUnits.indexOf( newUnits )>= 0 ){
			categoryMatch = true;
		}
		return categoryMatch;
	}

	void ImageView::restUnitsChanged(){
		//Only use this if changing withen units withen a given category.
		//I.e. Frequency in Hz->GHz

		QString newUnits = ui.restUnitsCombo->currentText();
		bool categoryMatch = isCategoryMatch( newUnits, restUnits );
		if ( categoryMatch && restUnits != newUnits &&
				newUnits.trimmed().length() > 0 && restUnits.trimmed().length() > 0 ){
			QString oldValueStr = ui.restLineEdit->text();
			if ( oldValueStr.trimmed().length() > 0 ){
				double oldValue = oldValueStr.toDouble();
				double newValue = oldValue;
				Converter* converter = Converter::getConverter( restUnits, newUnits );
				if ( converter != NULL ){
					newValue = converter->convert( oldValue );
				}
				delete converter;
				if ( newValue != oldValue && ! isnan( newValue )){
					ui.restLineEdit->setText( QString::number( newValue ));
				}
			}
			restUnits = newUnits;
		}
	}

	void ImageView::restFrequencyChanged(){
		QString valueStr = ui.restLineEdit->text();
		QString unitStr = ui.restUnitsCombo->currentText();
		if ( valueStr.length() > 0 ){
			String comboStr(valueStr.toStdString());
			comboStr.append( unitStr.toStdString());
			Record dataOptions = imageData->getOptions();
			Record restRecord = dataOptions.asRecord( REST_FREQUENCY_KEY );
			restRecord.define( VALUE_KEY, comboStr );
			dataOptions.defineRecord( REST_FREQUENCY_KEY, restRecord );
			//An AipsError occurs if we do not remove region and mask;
			const String REGION_RECORD = "region";
			if ( dataOptions.isDefined( REGION_RECORD)){
				dataOptions.removeField( REGION_RECORD);
			}
			const String MASK_RECORD = "mask";
			if ( dataOptions.isDefined( MASK_RECORD)){
				dataOptions.removeField( MASK_RECORD);
			}
			imageData->setOptions( dataOptions );
		}
	}


	//--------------------------------------------------------------------
	//                    Setters
	//--------------------------------------------------------------------

	void ImageView::setMasterCoordinateImage( bool masterImage ){
		if ( masterImage ){
			setBackgroundColor( masterCoordinateColor );
			coordinateMasterLabel->setText( "MC");
		}
		else {
			setBackgroundColor( normalColor );
			coordinateMasterLabel->setText( "");
		}
		setTitle();
	}

	void ImageView::setMasterHueImage( bool masterImage ){
		if ( masterImage && hueMasterLabel != NULL ){
			hueMasterLabel->setText( "H");
		}
		else {
			hueMasterLabel->setText( "");
		}
	}
	void ImageView::setMasterSaturationImage( bool masterImage ){
		if ( masterImage && saturationMasterLabel != NULL ){
			saturationMasterLabel->setText( "S");
		}
		else {
			saturationMasterLabel->setText( "");
		}
	}

	void ImageView::setRegistered( bool selected ) {
		ui.selectCheckBox->setChecked( selected );
	}

	void ImageView::setColorCombinationMode( ColorCombinationMode mode ){
		if ( colorMode != mode ){
			colorMode = mode;
			rgbModeChanged();
		}
	}

	bool ImageView::isRGBImage() const {
		QString imageName = getName();
		int RGBIndex = imageName.indexOf( "RGB");
		bool rgbImage = false;
		if ( RGBIndex == 0 ){
			rgbImage = true;
		}
		return rgbImage;
	}

	void ImageView::rgbModeChanged(){
		bool rgbMode = false;
		if (colorMode == RGB ){
			rgbMode = true;
		}
		bool colorLabelVisible = rgbMode && !isRGBImage();
		ui.colorLabel->setVisible( colorLabelVisible );
	}

	void ImageView::setViewedImage( bool viewed ){
		if ( viewed ){
			setFrameStyle( QFrame::Box | QFrame::Sunken );
			setLineWidth( VIEWED_BORDER_SIZE );
		}
		else {
			setFrameStyle( QFrame::Plain );
			setLineWidth( NOT_VIEWED_BORDER_SIZE );
		}
	}

	void ImageView::setData( QtDisplayData* other ){
		if ( empty && other != NULL ){
			empty = false;
			imageData = other;
			setTitle();
			restoreSnapshot();
		}
	}


	//-----------------------------------------------------------------------
	//             Context Menu
	//-----------------------------------------------------------------------

	void ImageView::showContextMenu( const QPoint& location ) {

		//Set-up the context
		QPoint showLocation = mapToGlobal( location );
		contextMenu.clear();
		bool masterCoordinateImage = isMasterCoordinate();
		if ( !masterCoordinateImage && isControlEligible() ){
			contextMenu.addAction( &masterCoordinateSystemAction );
		}
		else if (masterCoordinateImage ){
			contextMenu.addAction( &masterCoordinateSystemUndoAction );
		}
		/*if ( !isMasterHue() ){
			contextMenu.addAction( &masterHueAction );
		}
		if ( !isMasterSaturation() ){
			contextMenu.addAction( &masterSaturationAction );
		}*/
		if ( !ui.rasterRadio->isChecked() ){
			contextMenu.addAction( &rasterAction );
		}
		if ( !ui.contourRadio->isChecked() ){
			contextMenu.addAction( &contourAction );
		}
		if ( !isViewed() && ui.rasterRadio->isChecked() && isRegistered() ){
			contextMenu.addAction( &viewAction );
		}
		contextMenu.addAction( &closeAction );

		//Act on the user's selection.
		QAction* selectedAction = contextMenu.exec( showLocation );
		if ( selectedAction == &closeAction ){
			emit close( this );
		}
		else if ( selectedAction == &viewAction ){
			emit viewImage( this );
		}
		else if ( selectedAction == &masterCoordinateSystemAction ){
			setMasterCoordinateImage( true );
			emit masterCoordinateImageSelected( this );
		}
		else if ( selectedAction == &masterCoordinateSystemUndoAction ){
			setMasterCoordinateImage( false );
			emit masterCoordinateImageClear();
		}
		else if ( selectedAction == &rasterAction ){
			ui.rasterRadio->setChecked( true );
			displayTypeChanged();
		}
		else if ( selectedAction == &contourAction ){
			ui.contourRadio->setChecked( true );
			displayTypeChanged();
		}
		/*else if ( selectedAction == &masterHueAction ){
			setMasterHueImage( true );
			emit masterHueImageSelected( this );
		}
		else if ( selectedAction == &masterSaturationAction ){
			setMasterSaturationImage( true );
			emit masterSaturationImageSelected( this );
		}*/
	}


	//-----------------------------------------------------------
	//            Color
	//-----------------------------------------------------------

	void ImageView::setBackgroundColor( QColor color ) {
		QPalette pal = palette();
		pal.setColor( QPalette::Background, color );
		setPalette( pal );
		displayTypeLabel->setEmptyColor( color );
		if ( coordinateMasterLabel != NULL ){
			coordinateMasterLabel->setEmptyColor( color );
		}
		if ( hueMasterLabel != NULL ){
			hueMasterLabel->setEmptyColor( color );
		}
		if ( saturationMasterLabel != NULL ){
			saturationMasterLabel->setEmptyColor( color );
		}
	}

	QColor ImageView::getBackgroundColor() const {
		QPalette pal = palette();
		return pal.color( QPalette::Background );
	}

	/*QColor ImageView::getButtonColor() const {
		QPalette p = ui.colorButton->palette();
		QBrush brush = p.brush(QPalette::Button );
		QColor backgroundColor = brush.color();
		return backgroundColor;
	}*/

	/*void ImageView::setButtonColor( QColor color ){
		QPalette p = ui.colorButton->palette();
		p.setBrush(QPalette::Button, color);
		ui.colorButton->setPalette( p );
	}*/

	/*void ImageView::showColorDialog(){
		QColor initialColor = getButtonColor();
		QColor selectedColor = QColorDialog::getColor( initialColor, this );
		if ( selectedColor.isValid() ){
			setButtonColor( selectedColor );
			rgbChanged();
		}
	}*/

	QColor ImageView::getDisplayedColor() const {
		QColor displayedColor = Qt::black;
		//const int MAX_COLOR = 255;
		/*if ( ui.otherRadio->isChecked()){
			displayedColor = getButtonColor();
		}
		else*/
		bool rgbImage = isRGBImage();
		if ( !rgbImage ){
			if ( ui.redRadio->isChecked()){
				displayedColor = Qt::red;
			}
			else if ( ui.greenRadio->isChecked()){
				displayedColor = Qt::green;
			}
			else if ( ui.blueRadio -> isChecked()){
				displayedColor = Qt::blue;
			}
		}
		return displayedColor;
	}

	void ImageView::setDisplayedColor( QColor rgbColor ){
		if ( rgbColor == Qt::red ){
			ui.redRadio->setChecked( true );
		}
		else if (rgbColor == Qt::blue){
			ui.blueRadio->setChecked( true );
		}
		else if ( rgbColor == Qt::green){
			ui.greenRadio->setChecked( true );
		}
		rgbChanged();
	}

	void ImageView::rgbChanged(){
		QColor baseColor = getDisplayedColor();
		QPalette pal = ui.colorLabel->palette();
		pal.setColor( QPalette::Background, baseColor );
		ui.colorLabel->setPalette( pal );
	}

	/*void ImageView::otherColorChanged(){
		ui.colorButton->setEnabled( ui.otherRadio->isChecked());
		rgbChanged();
	}*/

	//----------------------------------------------------------------
	//            Slots
	//----------------------------------------------------------------

	void ImageView::restChanged(){
		ui.restUnitsCombo->clear();
		if ( ui.frequencyRadio->isChecked()){
			for ( int i = 0; i < frequencyUnits.size(); i++ ){
				ui.restUnitsCombo->addItem( frequencyUnits[i]);
			}
		}
		else {
			for ( int i = 0; i < wavelengthUnits.size(); i++ ){
				ui.restUnitsCombo->addItem( wavelengthUnits[i]);
			}
		}

		QString restValueStr = ui.restLineEdit->text();
		if ( restValueStr.trimmed().length() > 0 ){
			double restValue = restValueStr.toDouble();
			double convertedRestValue = restValue;
			QString newUnits = ui.restUnitsCombo->currentText();
			//Moving between frequency/wavelenth units
			if (  restUnits.length() > 0 && newUnits.length() > 0 ){
				convertedRestValue = wavelengthFrequencyConversion( restValue, restUnits, newUnits );
			}
			//Store the old rest type and units for next time.
			restUnits = newUnits;
			if ( ui.frequencyRadio->isChecked()){
				restType = REST_FREQUENCY;
			}
			else {
				restType = REST_WAVELENGTH;
			}


			//Put the converted value back into the text field.
			if ( restValue != convertedRestValue ){
				ui.restLineEdit->setText( QString::number(convertedRestValue) );
			}

		}
	}


	void ImageView::displayTypeChanged() {
		//Decide if there has been a change to Raster/Color/Contour/Vector
		DisplayType guiDisplay = static_cast<DisplayType>(displayGroup->checkedId());
		if ( storedDisplay != guiDisplay ){
			storedDisplay = guiDisplay;

			//Update the quick-look label
			displayTypeLabel->setText( displayTypeMap[guiDisplay] );

			saveSnapshot();
			empty = true;

			//Change the underlying data type
			emit displayTypeChanged( this );
		}
	}

	void ImageView::saveSnapshot(){
		//Store a snapshot of the display options.
		Record displayOptions = imageData->getOptions();
		displayOptionsSnapshot.merge( displayOptions, RecordInterface::OverwriteDuplicates );
	}

	void ImageView::restoreSnapshot(){
		//Get the image data
		Record imageOptions = imageData->getOptions();

		//Overwrite any fields we may have stored in the snapshot.
		int fieldCount = imageOptions.nfields();
		for ( int i = 0; i < fieldCount; i++ ){
			String fieldName = imageOptions.name( i );
			int snapIndex = displayOptionsSnapshot.fieldNumber( fieldName );
			if ( snapIndex >= 0 ){
				ValueHolder fieldRecord = displayOptionsSnapshot.asValueHolder( snapIndex );
				imageOptions.defineFromValueHolder( i, fieldRecord );
			}
		}

		//Remove region and mask records because they cause an
		//exception and have nothing to do with display options.
		bool regionDefined = imageOptions.isDefined( "region" );
		if ( regionDefined ){
			int fieldId = imageOptions.fieldNumber( "region");
			imageOptions.removeField( fieldId );
		}
		bool maskDefined = imageOptions.isDefined( "mask" );
		if ( maskDefined ){
			int fieldId = imageOptions.fieldNumber( "mask" );
			imageOptions.removeField( fieldId );
		}

		//Put the update options back in.
		imageData->setOptions( imageOptions );
	}

	void ImageView::imageRegistrationChanged( bool /*selected*/ ) {
		emit imageSelected( this );
	}


	void ImageView::showDataOptions(){
		emit showDataDisplayOptions( imageData );
	}




	//-----------------------------------------------------------------------------
	//                 Opening/Closing
	//-----------------------------------------------------------------------------

	QSize ImageView::minimumSizeHint() const {
		QSize frameSize = QFrame::size();
		QSize hint ( 200, minimumSize );
		return hint;
	}

	void ImageView::openCloseDisplay() {
		if ( minimumSize == SIZE_COLLAPSED ) {
			maximizeDisplay();
		} else {
			minimizeDisplay();
		}

		updateGeometry();
	}

	void ImageView::minimizeDisplay() {
		ui.widgetLayout->removeWidget( ui.displayGroupBox );
		ui.widgetLayout->removeWidget( ui.colorGroupBox);
		ui.widgetLayout->removeWidget( ui.restGroupBox );
		ui.widgetLayout->removeItem( spacerFirst );
		ui.widgetLayout->removeItem( spacerLast );
		ui.displayOptionsLayout->removeWidget( ui.dataOptionsButton );

		ui.displayGroupBox->setParent( NULL );
		ui.colorGroupBox->setParent( NULL );
		ui.restGroupBox->setParent( NULL );
		ui.dataOptionsButton->setParent( NULL );
		minimumSize = SIZE_COLLAPSED;
		ui.openCloseButton->setText( "Options...");
	}

	void ImageView::maximizeDisplay() {
		ui.widgetLayout->addSpacerItem( spacerFirst );
		ui.widgetLayout->addWidget( ui.displayGroupBox );

		ui.widgetLayout->addWidget( ui.restGroupBox );
		/*bool rgbImage = isRGBImage();
		if ( !rgbImage ){
			ui.widgetLayout->addWidget( ui.colorGroupBox );
		}*/
		ui.widgetLayout->addSpacerItem( spacerLast );
		ui.displayOptionsLayout->insertWidget( 1, ui.dataOptionsButton );
		minimumSize = SIZE_EXPANDED;
		ui.openCloseButton->setText( "Close" );
	}


	//**********************************************************************
	//                   Drag and Drop
	//**********************************************************************

	QImage* ImageView::makeDragImage(){
		QSize viewSize = size();
		QImage* dragImage = new QImage( viewSize.width(), viewSize.height(), QImage::Format_ARGB32_Premultiplied );
		const int GRAY_SHADE = 200;
		dragImage->fill(qRgba(GRAY_SHADE,GRAY_SHADE,GRAY_SHADE,255));
		QPainter painter;
		painter.begin(dragImage);
		const int PEN_WIDTH = 2;
		QPen pen;
		pen.setStyle(Qt::DashLine );
		pen.setWidth( PEN_WIDTH );
		pen.setColor( Qt::black );
		painter.setPen( pen );
		int width = dragImage->width() - PEN_WIDTH;
		int height = dragImage->height() - PEN_WIDTH;
		painter.drawRect( QRect(1,1, width, height));
		QFontMetrics metrics = painter.fontMetrics();
		QString title = getName();
		int x = ( width - metrics.width( title )) / 2;
		int y = height / 2 + 5;
		painter.drawText(x, y, title );
		painter.end();
		return dragImage;
	}

	void ImageView::makeDrag( QMouseEvent* event ) {
		QDrag* drag = new QDrag( this );
		QMimeData* mimeData = new QMimeData();
		QByteArray itemId = getName().toAscii();
		mimeData->setData( DROP_ID, itemId );
		QImage* dragImage = makeDragImage();
		drag->setPixmap( QPixmap::fromImage(*dragImage));
		drag->setMimeData( mimeData );
		QPoint hotSpot = event->pos();
		QByteArray itemData;
		QDataStream dataStream( &itemData, QIODevice::WriteOnly );
		dataStream << hotSpot;
		mimeData->setData( "application/manage_images", itemData );
		drag->setHotSpot( hotSpot );
		drag->start();
	}

	void ImageView::mouseMoveEvent( QMouseEvent* event ) {
		makeDrag( event );
	}

	double ImageView::wavelengthFrequencyConversion( double value,
			QString oldUnits, QString newUnits ) const {
		Converter* converter = Converter::getConverter( oldUnits, newUnits );
		double result = value;
		if ( converter != NULL ){
			result = converter->convert( value );
		}
		delete converter;
		return result;
	}

	ImageView::~ImageView() {
		//If we are minimized, we have to delete the orphaned
		//widgets ourselves.
		if ( minimumSize == SIZE_COLLAPSED ){
			delete spacerFirst;
			delete spacerLast;
		}
	}
}
