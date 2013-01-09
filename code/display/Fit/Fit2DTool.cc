//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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
#include "Fit2DTool.qo.h"
#include <display/Fit/Gaussian2DFitter.h>
#include <display/Fit/RegionBox.h>
#include <display/Fit/ColorComboDelegate.h>
#include <display/RegionShapes/RegionShape.h>
#include <display/Display/Options.h>
#include <QMessageBox>
#include <QDebug>
#include <QDir>

namespace casa {

Fit2DTool::Fit2DTool(QWidget *parent)
    : QDialog(parent), DEFAULT_KEY(-1), REGION_LABEL("Region:  "),
      image( NULL), fitter(NULL), progressBar( this ),
      findSourcesDialog( this ), pixelRangeDialog( this ),
      logDialog( this ){
	ui.setupUi(this);
	currentRegionId = DEFAULT_KEY;
	const QString WINDOW_TITLE( "2D Fit Tool");
	this->setWindowTitle( WINDOW_TITLE );

	setImageFunctionalityEnabled( false );
	connect( &findSourcesDialog, SIGNAL(showOverlay(String)), this, SIGNAL(showOverlay(String)));
	connect( &findSourcesDialog, SIGNAL(removeOverlay(String)), this, SIGNAL(removeOverlay(String)));
	connect( &findSourcesDialog, SIGNAL(estimateFileSpecified(const QString&)), this, SLOT(estimateFileChanged( const QString&)));

	ui.channelLineEdit->setText( QString::number(0));
	QIntValidator* intValidator = new QIntValidator( 0, std::numeric_limits<int>::max(), this );
	ui.channelLineEdit->setValidator( intValidator );

	connect( ui.displayFitCheckBox, SIGNAL(toggled(bool)), this, SLOT(displayFitChanged(bool)));
	ui.viewButton->setEnabled( false );
	fitColorDelegate = new ColorComboDelegate( this );
	ui.fitColorCombo->setItemDelegate(  fitColorDelegate );
	const QStringList colorNames = QStringList()<<"yellow"<<"white"<<"red"<<
			"green"<<"blue"<<"cyan"<<"magenta"<<"black";
	fitColorDelegate->setSupportedColors( colorNames );
	for ( int i = 0; i < colorNames.size(); i++ ){
		ui.fitColorCombo->addItem(colorNames[i]);
	}

	QDoubleValidator* validator = new QDoubleValidator( std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), 7, this );
	ui.startLineEdit->setValidator( validator );
	ui.endLineEdit->setValidator( validator );
	connect( &pixelRangeDialog, SIGNAL(accepted()), this, SLOT(pixelRangeChanged()));
	QButtonGroup* buttonGroup = new QButtonGroup( this );
	buttonGroup->addButton( ui.includeRadioButton );
	buttonGroup->addButton( ui.excludeRadioButton );
	buttonGroup->addButton( ui.noneRadioButton );
	connect( ui.pixelRangeGroupBox, SIGNAL(toggled(bool)), this, SLOT(pixelRangeEnabledChanged(bool)));
	connect( ui.excludeRadioButton, SIGNAL(toggled(bool)), this, SLOT(pixelRangeNoneSelected(bool)));

	connect( ui.fitButton, SIGNAL(clicked()), this, SLOT(doFit()));
	connect( ui.findEstimatesButton, SIGNAL(clicked()), this, SLOT(showFindSourcesDialog()));
	connect( ui.closeButton, SIGNAL(clicked()), this, SLOT(finishedWork()));
	connect( ui.graphicalPixelRangeButton, SIGNAL(clicked()), this, SLOT(showPixelRangeDialog()));
	connect( ui.browseButton, SIGNAL(clicked()), this, SLOT(showFileDialog()));
	connect( ui.saveBrowseButton, SIGNAL(clicked()), this, SLOT(showSaveDialog()));
	connect( ui.residualBrowseButton, SIGNAL(clicked()), this, SLOT(showResidualDialog()));
	connect( ui.viewButton, SIGNAL(clicked()), this, SLOT(showResults()));
	connect( ui.residualHistogramButton, SIGNAL(clicked()), this, SLOT(showResidualHistogramDialog()));
	connect( ui.residualGroupBox, SIGNAL(toggled(bool)), this, SLOT(residualSupportChanged(bool)));
	progressBar.setWindowTitle( "Fit in Progress" );
	progressBar.setLabelText( "Fitting source(s)...");
	progressBar.setWindowModality( Qt::WindowModal );
	progressBar.setMinimum( 0 );
	progressBar.setMaximum( 0 );
	progressBar.setCancelButton( 0 );
}

void Fit2DTool::residualSupportChanged( bool enable ){
	if ( enable ){
		if ( this->fitMarkers.isEmpty() ){
			ui.residualHistogramButton->setEnabled( false );
		}
	}
}

void Fit2DTool::showFileChooserDialog(const QString& title,
		QFileDialog::FileMode mode, QLineEdit* destinationLineEdit ){
	QFileDialog dialog( this );
	QDir homeDir = QDir::home();
	QString homePath = homeDir.absolutePath();
	dialog.setFileMode(mode);
	dialog.setWindowTitle( title );
	dialog.setDirectory( homePath );
	if ( dialog.exec() ){
		QStringList fileNames = dialog.selectedFiles();
		QString fileName = fileNames[0];
		destinationLineEdit->setText( fileName );
	}
}

void Fit2DTool::showFileDialog(){
	showFileChooserDialog( "Select Fit2D Estimate File",
		QFileDialog::ExistingFile, ui.estimateFileLineEdit);
}

void Fit2DTool::showSaveDialog(){
	showFileChooserDialog( "Select Save Directory",
		QFileDialog::DirectoryOnly, ui.saveDirectoryLineEdit);
}

void Fit2DTool::showResidualDialog(){
	showFileChooserDialog( "Select a Directory for the Residual Image",
			QFileDialog::DirectoryOnly, ui.residualDirectoryLineEdit);
}

void Fit2DTool::displayFitChanged( bool display ){
	if ( display ){
		addViewerFitMarkers();
	}
	else {
		removeViewerFitMarkers();
	}
	ui.fitColorCombo->setEnabled( display );
}

void Fit2DTool::pixelRangeEnabledChanged( bool enabled ){
	if ( !enabled ){
		ui.noneRadioButton->setChecked( true );
	}
}

void Fit2DTool::pixelRangeNoneSelected( bool selected ){
	ui.startLineEdit->setEnabled( !selected );
	ui.endLineEdit->setEnabled( !selected );
	ui.graphicalPixelRangeButton->setEnabled( !selected );
}

void Fit2DTool::pixelRangeChanged(){
	pair<double,double> range = pixelRangeDialog.getInterval();
	QString startRange = QString::number( range.first );
	QString endRange = QString::number( range.second );
	ui.startLineEdit->setText( startRange );
	ui.endLineEdit->setText( endRange );
}

void Fit2DTool::showPixelRangeDialog(){
	pixelRangeDialog.setImage( image );
	pixelRangeDialog.show();
}

void Fit2DTool::showResidualHistogramDialog(){
	if ( residualImagePath.length() > 0 ){
		bool imageSet = residualHistogramDialog.setImage( residualImagePath );
		if ( imageSet ){
			residualHistogramDialog.show();
		}
		else {
			QString msg( "There was an error generated the histogram for the residual Image");
			QMessageBox::warning( this, "Residual Histogram Error", msg );
		}
	}
}

void Fit2DTool::populateIncludeExclude(Vector<Float>& range ) const {
	QString startRangeStr = ui.startLineEdit->text();
	float startRange = startRangeStr.toFloat();
	QString endRangeStr = ui.endLineEdit->text();
	float endRange = endRangeStr.toFloat();
	if ( endRange < startRange ){
		float tmp = endRange;
		startRange = endRange;
		endRange = tmp;
	}
	if ( startRange < endRange ){
		range.resize( 2 );
		range[0] = startRange;
		range[1] = endRange;
	}
}

Vector<Float> Fit2DTool::populateInclude() const {
	Vector<Float> includeVector(0);
	if ( ui.includeRadioButton->isChecked() ){
		populateIncludeExclude( includeVector );
	}
	return includeVector;
}

Vector<Float> Fit2DTool::populateExclude() const {
	Vector<Float> excludeVector(0);
	if ( ui.excludeRadioButton->isChecked() ){
		populateIncludeExclude( excludeVector );
	}
	return excludeVector;
}

bool Fit2DTool::populateSaveFile( String& saveFile ){
	bool saveOk = true;
	if ( ui.saveGroupBox->isChecked() ){
		saveOk = validateFile( ui.saveDirectoryLineEdit, ui.saveFileLineEdit,
				saveFile, "saving fit output" );
	}
	return saveOk;
}

bool Fit2DTool::populateResidualFile( String& saveFile ){
	bool residualOk = true;
	if ( ui.residualGroupBox->isChecked() ){
		residualOk = validateFile( ui.residualDirectoryLineEdit, ui.residualFileLineEdit,
				saveFile, "saving residual image");
	}
	return residualOk;
}

bool Fit2DTool::validateFile( QLineEdit* directoryLineEdit, QLineEdit* fileLineEdit,
		String& saveFile, const QString& purpose ){
	QString directoryStr = directoryLineEdit->text();
	bool fileOk = true;
	if ( directoryStr.trimmed().length() == 0 ){
		QString msg("Please specify a directory for ");
		msg.append( purpose );
		msg.append( "." );
		QMessageBox::warning( this, "Missing Directory", msg );
		fileOk = false;
	}
	else {
		QFile file( directoryStr );
		if ( !file.exists() ){
			QString msg("Please check that the directory for ");
			msg.append( purpose );
			msg.append( " is correctly specified.");
			QMessageBox::warning( this, "Path Error", msg );
			fileOk = false;
		}
		else {
			if ( !directoryStr.endsWith( QDir::separator() ) ){
				directoryStr = directoryStr + QDir::separator();
			}
			QString fileName = fileLineEdit->text();
			if ( fileName.trimmed().length() == 0 ){
				QString msg("Please specify the name of a file for ");
				msg.append( purpose );
				msg.append( "." );
				QMessageBox::warning( this, "Missing File Name", msg );
				fileOk = false;
			}
			else {
				QString path = directoryStr + fileName;
				saveFile = path.toStdString();
			}
		}
	}
	return fileOk;
}

String Fit2DTool::getScreenedEstimatesFile( const String& estimatesFileName, bool* errorWritingFile ){
	QString screenedEstimatesFileName(estimatesFileName.c_str());
	if ( screenedEstimatesFileName.length() > 0 ){
		if ( !regions.isEmpty() ){
			RegionBox* screenBox = regions[currentRegionId ];
			if ( screenBox != NULL ){
				int separatorIndex = screenedEstimatesFileName.lastIndexOf( QDir::separator());
				if ( separatorIndex > 0 ){
					screenedEstimatesFileName = screenedEstimatesFileName.right(screenedEstimatesFileName.length() - separatorIndex-1);
				}
				String outName= viewer::options.temporaryPath( screenedEstimatesFileName.toStdString() );
				QString outNameStr( outName.c_str() );
				bool success = findSourcesDialog.writeEstimateFile( outNameStr, true, screenBox );
				if ( success ){
					screenedEstimatesFileName = outNameStr;
				}
				else {
					*errorWritingFile = true;
					qDebug() << "Could not write estimates to temp file: "<<outNameStr;
				}
			}
		}
	}
	return screenedEstimatesFileName.toStdString();
}


void Fit2DTool::doFit(){

	String estimatesFileName;
	if ( ui.estimateGroupBox->isChecked()){
		QString fileName = ui.estimateFileLineEdit->text().trimmed();
		if ( fileName.length() == 0 ){
			QMessageBox::warning( this, "Missing Estimate File", "Please specify a file containing fit estimates.");
			return;
		}
		else {
			estimatesFileName = fileName.toStdString();
		}
	}
	bool errorWritingFile = false;
	estimatesFileName = getScreenedEstimatesFile( estimatesFileName, &errorWritingFile );
	if ( errorWritingFile ){
		return;
	}

	String pixelBox = populatePixelBox();
	Vector<Float> includeVector = populateInclude();
	Vector<Float> excludeVector = populateExclude();
	QString channelStr = ui.channelLineEdit->text();
	int channel = channelStr.toInt();
	String saveFile;
	if ( ! populateSaveFile( saveFile )){
		return;
	}
	String residualFile;
	if ( !populateResidualFile( residualFile )){
		return;
	}

	//If we have an overlay of the 2D fit, remove it before
	//we start a new fit.
	removeViewerFitMarkers();

	//Initialize the thread that does the fit.
	delete fitter;
	fitter = new Gaussian2DFitter();
	fitter->setFitParameters( image, pixelBox, channel, estimatesFileName,
			residualFile, includeVector, excludeVector );
	connect( fitter, SIGNAL( finished() ), this, SLOT(fitDone()));
	if ( saveFile.length() > 0 ){
		fitter->setLogFilePath( saveFile );
	}
	fitter->start();
	progressBar.show();
}

void Fit2DTool::frameChanged( int frame ){
	ui.channelLineEdit->setText( QString::number( frame ));
	findSourcesDialog.setChannel( frame );
}

void Fit2DTool::clearFitMarkers(){
	while ( !fitMarkers.isEmpty() ){
		RegionShape* marker = fitMarkers.takeLast();
		delete marker;
	}
}

void Fit2DTool::addViewerFitMarkers(){
	if ( fitMarkers.size() > 0 ){
		if ( ui.displayFitCheckBox->isChecked()){
			emit add2DFitOverlay( fitMarkers );
		}
	}
}

void Fit2DTool::removeViewerFitMarkers(){
	if ( fitMarkers.size() > 0 ){
		emit remove2DFitOverlay( fitMarkers );
	}
}

void Fit2DTool::fitDone(){
	progressBar.hide();
	bool successfulFit = fitter->isFitSuccessful();
	if ( !successfulFit ){
		QString errorMsg = fitter->getErrorMessage();
		QMessageBox::warning( this, "Fit Error", errorMsg );
	}
	else {
		QString logFilePath = fitter->getLogFilePath();
		bool logGenerated = logDialog.setLog( logFilePath );
		if ( logGenerated ){
			logDialog.show();
		}
		ui.viewButton->setEnabled( logGenerated );

		//Fit Markers
		clearFitMarkers();
		int colorIndex = ui.fitColorCombo->currentIndex();
		QString colorName = fitColorDelegate->getNamedColor( colorIndex );
		fitMarkers = fitter->toDrawingDisplay( image, colorName );
		addViewerFitMarkers();

		//Residual image
		if ( ui.residualGroupBox->isChecked() ){
			ui.residualHistogramButton->setEnabled(true);
			residualImagePath=(fitter->getResidualImagePath().toStdString());
			emit addResidualFitImage( residualImagePath );
		}
	}
}

void Fit2DTool::showResults(){
	logDialog.show();
}

void Fit2DTool::finishedWork(){
	this->close();
}

void Fit2DTool::estimateFileChanged( const QString& fullPath ){
	ui.estimateFileLineEdit->setText( fullPath );
}

void Fit2DTool::showFindSourcesDialog(){
	if ( image != NULL ){
		QString channelStr = ui.channelLineEdit->text();
		findSourcesDialog.setChannel(channelStr.toInt());
		findSourcesDialog.show();
	}
	else {
		QMessageBox::warning( this, "Missing Image", "Please load an image that can be used for finding source estimates.");
	}
}


String Fit2DTool::populatePixelBox() const {
	QString pixelStr( "");
	const QString COMMA_STR( ",");
	if ( currentRegionId != DEFAULT_KEY ){
		pixelStr = regions[currentRegionId]->toString();
	}
	else {
		//No regions so just use the image as bounds.
		const QString ZERO_STR( "0");
		pixelStr.append( ZERO_STR + COMMA_STR );
		pixelStr.append( ZERO_STR + COMMA_STR );
		IPosition imageShape = image->shape();
		pixelStr.append( QString::number( imageShape(0) - 1) + COMMA_STR);
		pixelStr.append( QString::number( imageShape(1) - 1));
	}
	String pixStr( pixelStr.toStdString());
	return pixStr;
}

void Fit2DTool::setImageFunctionalityEnabled( bool enable ){
	ui.fitButton->setEnabled( enable );
	ui.estimateGroupBox->setEnabled( enable );
	ui.pixelRangeGroupBox->setEnabled( enable );
	ui.channelLineEdit->setEnabled( enable );
	ui.fitImageLabel->setVisible( enable );
	if ( !enable ){
		ui.fitRegionLabel->setVisible( enable );
	}
}

void Fit2DTool::setImage( ImageInterface<Float>* image ){
	this->image = image;
	bool enableFunctionality = true;
	if ( image == NULL ){
		enableFunctionality = false;
	}
	else {
		String imageName = image->name(false);
		QString imageNameStr( imageName.c_str());
		ui.fitImageLabel->setText( "Image:" + imageNameStr );
		findSourcesDialog.setImage( image );

	}
	setImageFunctionalityEnabled( enableFunctionality );
}

void Fit2DTool::newRegion( int id, const QString &/*shape*/, const QString &/*name*/,
		const QList<double> &/*world_x*/, const QList<double> &/*world_y*/,
		const QList<int> &pixel_x, const QList<int> &pixel_y,
		const QString &/*linecolor*/, const QString &/*text*/, const QString &/*font*/,
		int /*fontsize*/, int /*fontstyle*/ ) {
	RegionBox* regionBox = regions[id];
	if ( regionBox == NULL ){
		regionBox = new RegionBox( pixel_x, pixel_y );
		regions[id] = regionBox;
	}
	else {
		regionBox->update( pixel_x, pixel_y );
	}
	ui.fitRegionLabel->setVisible( true );
	QString boxSpec( regionBox->toStringLabelled());
	ui.fitRegionLabel->setText( REGION_LABEL+boxSpec);
	currentRegionId = id;

	String pixelBox = populatePixelBox();
	findSourcesDialog.setPixelBox( pixelBox );
}

void Fit2DTool::clearRegions(){
	QList<int> keys = regions.keys();
	for ( int i = 0; i < keys.size(); i++ ){
		RegionBox* box = regions.take( keys[i] );
		delete box;
	}
}

void Fit2DTool::updateRegion( int id, viewer::Region::RegionChanges changes,
		const QList<double> &/*world_x*/, const QList<double> &/*world_y*/,
		const QList<int> &pixel_x, const QList<int> &pixel_y ){
	if ( changes != viewer::Region::RegionChangeDelete ){
		RegionBox* box = regions[id];
		if ( box != NULL ){
			box->update( pixel_x, pixel_y );
			currentRegionId = id;
			ui.fitRegionLabel->setText( REGION_LABEL + regions[currentRegionId]->toStringLabelled());
			String pixelBox = populatePixelBox();
			findSourcesDialog.setPixelBox( pixelBox );
		}
		else {
			qDebug() << "Fit2DTool::updateRegion unrecognized id="<<id;
		}
	}
	else {
		RegionBox* regionToRemove = regions.take(id);
		delete regionToRemove;
		if ( regions.isEmpty() ){
			currentRegionId = DEFAULT_KEY;
			ui.fitRegionLabel->setVisible( false );
		}
		else {
			QList<int> keys = regions.keys();
			currentRegionId = keys[0];
			ui.fitRegionLabel->setText( REGION_LABEL + regions[currentRegionId]->toStringLabelled());
		}
		String pixelBox = populatePixelBox();
		findSourcesDialog.setPixelBox( pixelBox );
	}
}


Fit2DTool::~Fit2DTool(){
	delete fitter;
	clearFitMarkers();
	clearRegions();
}
}
