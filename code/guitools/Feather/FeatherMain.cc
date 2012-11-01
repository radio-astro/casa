#include "FeatherMain.qo.h"
#include <guitools/Feather/FeatherPlotWidget.qo.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageUtilities.h>
#include <casa/Utilities/PtrHolder.h>
#include <QDebug>
#include <QMap>
#include <QMessageBox>

namespace casa {

const int FeatherMain::DISH_DIAMETER_DEFAULT = -1;

FeatherMain::FeatherMain(QWidget *parent)
    : QMainWindow(parent), fileLoader( this ),
      preferences( this ), preferencesColor( this ),
      origXWidget(NULL),xWidget(NULL),origYWidget(NULL),yWidget(NULL),
      logger(LogOrigin("CASA", "Feather")){

	ui.setupUi(this);

	//Put a validator on the effective dish diameter.
	QDoubleValidator* doubleValidator = new QDoubleValidator( 0, std::numeric_limits<double>::max(), 10, this );
	ui.dishDiameterXLineEdit->setValidator( doubleValidator );
	ui.dishDiameterYLineEdit->setValidator( doubleValidator );
	connect( ui.dishDiameterXLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(dishDiameterXChanged(const QString&)));

	//Takeout
	/*ui.highResolutionLabel->setText( "/home/uniblab/casa/active/test/orion_vlamem.im");
	ui.lowResolutionLabel->setText( "/home/uniblab/casa/active/test/orion_gbt.im");
	ui.outputLabel->setText("/home/uniblab/casa/active/test/feather.image");
	*/
	ui.featherButton->setEnabled( false );

	//Add the plots to the frames
	origXWidget = new FeatherPlotWidget( "Original Slice Cuts X", true, this);
	xWidget = new FeatherPlotWidget( "Feathered Slice Cuts X", false, this );
	origYWidget = new FeatherPlotWidget("Original Slice cuts Y", true, this);
	yWidget = new FeatherPlotWidget( "Feathered Slice Cuts Y", false,  this );
	functionColorsChanged();
	preferencesChanged();
	layoutPlotWidgets();

	connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(openPreferences()));
	connect(ui.actionImageFiles, SIGNAL(triggered()), this, SLOT(openFileLoader()));
	connect(ui.actionPreferencesColor, SIGNAL(triggered()), this, SLOT(openPreferencesColor()));
	connect( &fileLoader, SIGNAL(imageFilesChanged()), this, SLOT(imageFilesChanged()));
	connect( &preferencesColor,SIGNAL(colorsChanged()), this, SLOT(functionColorsChanged()));
	connect(ui.featherButton, SIGNAL(clicked()), this, SLOT( featherImages()));
	connect( ui.yGroupBox, SIGNAL(toggled(bool)), this, SLOT(ySupportChanged(bool)));
	connect(&preferences, SIGNAL(preferencesChanged()), this, SLOT(preferencesChanged()));
}

void FeatherMain::ySupportChanged( bool ySupport ){
	if ( !ySupport ){
		ui.dishDiameterYLineEdit->setText( ui.dishDiameterXLineEdit->text());
	}
	layoutPlotWidgets();
}
void FeatherMain::dishDiameterXChanged( const QString& xDiameter ){
	if ( !ui.dishDiameterYLineEdit->isEnabled() ){
		ui.dishDiameterYLineEdit->setText( xDiameter );
	}
}
void FeatherMain::removeWidget(QWidget* widget, QLayout* layout){
	layout->removeWidget( widget );
	widget->setParent( NULL );
}

void FeatherMain::layoutPlotWidgets(){
	QLayout* layout = ui.plotHolderWidget->layout();
	if ( layout == NULL ){
		layout = new QGridLayout();
	}
	QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(layout);
	removeWidget( xWidget, gridLayout );
	removeWidget( yWidget, gridLayout );
	removeWidget( origXWidget, gridLayout );
	removeWidget( origYWidget, gridLayout );
	bool displayOriginalFunctions = preferences.isDisplayOriginalFunctions();
	bool displayOutputFunctions = preferences.isDisplayOutputFunctions();
	bool displayYGraphs = ui.yGroupBox->isChecked();
	int rowCount = 0;
	if ( displayOutputFunctions ){
		rowCount++;
		gridLayout->addWidget( xWidget, 0, 0 );
		if ( displayYGraphs ){
			gridLayout->addWidget( yWidget, 0, 1 );
		}
	}
	if ( displayOriginalFunctions ){
		rowCount++;
		gridLayout->addWidget( origXWidget, 1, 0 );
		if ( displayYGraphs ){
			gridLayout->addWidget( origYWidget, 1, 1);
		}
	}
	ui.plotHolderWidget->setLayout( gridLayout );

	//Adjust the minimum height of the dialog to fit
	//the number of graphs we are displaying.
	const int BASE_WIDTH = 800;
	const int BASE_HEIGHT = 400;
	const int HEIGHT_INCREMENT = 200;
	int height = BASE_HEIGHT + HEIGHT_INCREMENT * rowCount;
	setMinimumSize( BASE_WIDTH, height );
	adjustSize();
}


void FeatherMain::preferencesChanged(){
	//Line thickness
	int lineThickness = preferences.getLineThickness();
	xWidget->setLineThickness( lineThickness );
	yWidget->setLineThickness( lineThickness );
	origXWidget->setLineThickness( lineThickness );
	origYWidget->setLineThickness( lineThickness );

	//Legend visibility
	bool origLegendVisible = preferences.isDisplayOriginalLegend();
	origXWidget->setLegendVisibility( origLegendVisible );
	origYWidget->setLegendVisibility( origLegendVisible );
	bool outputLegendVisible = preferences.isDisplayOutputLegend();
	xWidget->setLegendVisibility( outputLegendVisible );
	yWidget->setLegendVisibility( outputLegendVisible );

	layoutPlotWidgets();
}

void FeatherMain::functionColorsChanged(){
	QMap<PreferencesColor::FunctionColor,QColor> colorMap = preferencesColor.getFunctionColors();
	QColor scatterPlotColor = preferencesColor.getScatterPlotColor();
	xWidget->setPlotColors(colorMap[PreferencesColor::SD_OUTPUT_COLOR],
			colorMap[PreferencesColor::INT_OUTPUT_COLOR],
			colorMap[PreferencesColor::SD_WEIGHT_COLOR],
			colorMap[PreferencesColor::INT_WEIGHT_COLOR], scatterPlotColor );
	yWidget->setPlotColors(colorMap[PreferencesColor::SD_OUTPUT_COLOR],
			colorMap[PreferencesColor::INT_OUTPUT_COLOR],
			colorMap[PreferencesColor::SD_WEIGHT_COLOR],
			colorMap[PreferencesColor::INT_WEIGHT_COLOR], scatterPlotColor );
	origXWidget->setPlotColors(colorMap[PreferencesColor::SD_ORIGINAL_COLOR],
			colorMap[PreferencesColor::INT_ORIGINAL_COLOR],
			colorMap[PreferencesColor::SD_WEIGHT_COLOR],
			colorMap[PreferencesColor::INT_WEIGHT_COLOR], scatterPlotColor );
	origYWidget->setPlotColors(colorMap[PreferencesColor::SD_ORIGINAL_COLOR],
			colorMap[PreferencesColor::INT_ORIGINAL_COLOR],
			colorMap[PreferencesColor::SD_WEIGHT_COLOR],
			colorMap[PreferencesColor::INT_WEIGHT_COLOR], scatterPlotColor );
}

void FeatherMain::imageFilesChanged(){
	QString lowResolutionPath = fileLoader.getFilePathLowResolution();
	QString highResolutionPath = fileLoader.getFilePathHighResolution();
	QString outputPath = fileLoader.getFilePathOutput();
	ui.lowResolutionLabel->setText( lowResolutionPath );
	ui.highResolutionLabel->setText( highResolutionPath );
	ui.outputLabel->setText( outputPath );
	ui.featherButton->setEnabled( true );
}

void FeatherMain::openFileLoader(){
	fileLoader.exec();
}

void FeatherMain::openPreferences(){
	preferences.exec();
}

void FeatherMain::openPreferencesColor(){
	preferencesColor.exec();
}

void FeatherMain::featherImages() {
	String lowResImagePath = ui.lowResolutionLabel->text().toStdString();
	String highResImagePath = ui.highResolutionLabel->text().toStdString();
	QString outputImagePath = ui.outputLabel->text();
	ImageInterface<Float>* lowResImage = NULL;
	ImageInterface<Float>* highResImage = NULL;
	bool imagesGenerated = generateInputImage( lowResImagePath, highResImagePath, lowResImage, highResImage );
	if ( imagesGenerated ){
		qDebug() << "Successfully generated images";
		Feather featherWorker( *lowResImage, *highResImage );
		pair<float,float> dishDiameters = populateDishDiameters();
		qDebug() << "First dish diameter="<<dishDiameters.first<<" second="<<dishDiameters.second;
		bool dishDiametersValid = featherWorker.setEffectiveDishDiam( dishDiameters.first, dishDiameters.second );
		if ( dishDiametersValid ){

			//First get the feathering functions
			Vector<Float> sDx;
			Vector<Float> sDxAmp;
			Vector<Float> sDy;
			Vector<Float> sDyAmp;
			featherWorker.getFeatherSD( sDx, sDxAmp, sDy, sDyAmp );
			xWidget->setSingleDishWeight( sDx, sDxAmp );
			yWidget->setSingleDishWeight( sDy, sDyAmp );

			Vector<Float> intx;
			Vector<Float> intxAmp;
			Vector<Float> inty;
			Vector<Float> intyAmp;
			featherWorker.getFeatherINT( intx, intxAmp, inty, intyAmp );
			xWidget->setInterferometerWeight( intx, intxAmp );
			yWidget->setInterferometerWeight( intx, intxAmp );

			//Now get the feathered data
			sDx.resize( 0 );
			sDxAmp.resize( 0 );
			sDy.resize( 0 );
			sDyAmp.resize( 0 );
			featherWorker.getFeatheredCutSD( sDx, sDxAmp, sDy, sDyAmp );
			xWidget->setSingleDishData( sDx, sDxAmp );
			yWidget->setSingleDishData( sDy, sDyAmp );

			intx.resize( 0 );
			intxAmp.resize( 0 );
			inty.resize( 0 );
			intyAmp.resize( 0 );
			featherWorker.getFeatheredCutINT( intx, intxAmp, inty, intyAmp );
			xWidget->setInterferometerData( intx, intxAmp );
			yWidget->setInterferometerData( inty, intyAmp );

			//Finally, get the original data
			sDx.resize( 0 );
			sDxAmp.resize( 0 );
			sDy.resize( 0 );
			sDyAmp.resize( 0 );
			featherWorker.getFTCutSDImage( sDx, sDxAmp, sDy, sDyAmp );
			origXWidget->setSingleDishData( sDx, sDxAmp );
			origYWidget->setSingleDishData( sDy, sDyAmp );

			intx.resize( 0 );
			intxAmp.resize( 0 );
			inty.resize( 0 );
			intyAmp.resize( 0 );
			featherWorker.getFeatheredCutINT(intx, intxAmp, inty, intyAmp );
			origXWidget->setInterferometerData( intx, intxAmp );
			origYWidget->setInterferometerData( inty, intyAmp );

			if ( fileLoader.isOutputSaved() ){
				bool savedFile = featherWorker.saveFeatheredImage( outputImagePath.toStdString() );
				if ( !savedFile ){
					QString msg( "There was a problem saving the image to "+outputImagePath);
					QMessageBox::warning( this, "Problem Saving Feathered Image", msg);
				}
			}

		}
		else {
			QString msg("Effective dish diameter had a finer resolution than the original data.");
			QMessageBox::warning( this, "Warning", msg );
		}
	}
	else {
		qDebug() << "Could not generate images";
	}
}

pair<float,float> FeatherMain::populateDishDiameters() const {
	QString xDiameterStr = ui.dishDiameterXLineEdit->text().trimmed();
	QString yDiameterStr = ui.dishDiameterYLineEdit->text().trimmed();
	float xDiameter = DISH_DIAMETER_DEFAULT;
	float yDiameter = DISH_DIAMETER_DEFAULT;
	if ( xDiameterStr.length() > 0 ){
		xDiameter = xDiameterStr.toFloat();
	}
	if ( yDiameterStr.length() > 0 ){
		yDiameter = yDiameterStr.toFloat();
	}
	pair<float,float> diameters( xDiameter, yDiameter );
	return diameters;
}


bool FeatherMain::generateInputImage( const String& lowResImagePath, const String& highResImagePath,
		ImageInterface<Float>* lowResImage, ImageInterface<Float>* highResImage ){
	bool success = true;
	try {
		bool noStokes=False;
		String outLowRes = lowResImagePath;
		String outHighRes = highResImagePath;

		/*if ( ! doVP_p ) {
			this->unlock();
			os << LogIO::SEVERE <<
					"Must invoke setvp() first in order to apply the primary beam"
					<< LogIO::EXCEPTION;
			return False;
		}*/

		logger << LogIO::NORMAL
				<< "\nFeathering together high and low resolution images.\n"
				<< LogIO::POST;

		//Get initial images
		//Deal with images that don't have stokes.
		PagedImage<Float> highResImageTemp(highResImagePath);
		PagedImage<Float> lowResImageTemp(lowResImagePath);
		if(highResImageTemp.shape().nelements() != lowResImageTemp.shape().nelements()){
			String msg( "High and low resolution images do not have the same number of axes.");
			logger << LogIO::SEVERE  << msg << LogIO::EXCEPTION;
			QMessageBox::warning( this, "Warning!", msg.c_str());
			success = false;
		}
		else {
			if ( (highResImageTemp.coordinates().findCoordinate(Coordinate::STOKES) < 0) &&
					(lowResImageTemp.coordinates().findCoordinate(Coordinate::STOKES) < 0)){
				noStokes=True;
				String msg("Making some temporary images as the inputs have no Stokes axis.\n");
				logger << LogIO::NORMAL << msg << LogIO::POST;

				PtrHolder<ImageInterface<Float> > outImage1;
				outHighRes= highResImagePath+"_stokes";
				ImageUtilities::addDegenerateAxes (logger, outImage1, highResImageTemp, outHighRes,
						False, False,
						"I", False, False,
						False);

				PtrHolder<ImageInterface<Float> > outImage2;
				outLowRes= lowResImagePath+"_stokes";
				ImageUtilities::addDegenerateAxes (logger, outImage2, lowResImageTemp, outLowRes,
						False, False,
						"I", False, False,
						False);

			}

			lowResImage = new PagedImage<Float>(outLowRes);
			highResImage = new PagedImage<Float>(outHighRes);

			if(noStokes){
				Table::deleteTable(outHighRes);
				Table::deleteTable(outLowRes);
			}

		}
	}
	catch (AipsError x) {
		String msg = x.getMesg();
		logger << LogIO::SEVERE << "Caught exception: " << msg<< LogIO::EXCEPTION;
		success = false;;
	}


	return success;
}


FeatherMain::~FeatherMain()
{
	delete origXWidget;
	delete xWidget;
	delete origYWidget;
	delete yWidget;
}
}
