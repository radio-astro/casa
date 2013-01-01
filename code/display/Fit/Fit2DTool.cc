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
#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <QFileDialog>

namespace casa {

Fit2DTool::Fit2DTool(QWidget *parent)
    : QDialog(parent), image( NULL), fitter(NULL), progressBar( this ),
      findSourcesDialog( this ), pixelRangeDialog( this ),
      logDialog( this ){
	ui.setupUi(this);
	const QString WINDOW_TITLE( "2D Fit Tool");
	this->setWindowTitle( WINDOW_TITLE );

	setImageFunctionalityEnabled( false );
	connect( &findSourcesDialog, SIGNAL(showOverlay(String, String, String)),
		this, SIGNAL(showOverlay(String,String,String)));
	connect( &findSourcesDialog, SIGNAL(removeOverlay(String)), this, SIGNAL(removeOverlay(String)));
	connect( &findSourcesDialog, SIGNAL(estimateFileSpecified(const QString&)), this, SLOT(estimateFileChanged( const QString&)));

	ui.channelLineEdit->setText( QString::number(0));
	QIntValidator* intValidator = new QIntValidator( 0, std::numeric_limits<int>::max(), this );
	ui.channelLineEdit->setValidator( intValidator );

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

	progressBar.setWindowTitle( WINDOW_TITLE );
	progressBar.setLabelText( "Fitting source(s)...");
	progressBar.setWindowModality( Qt::WindowModal );
	progressBar.setMinimum( 0 );
	progressBar.setMaximum( 0 );
	progressBar.setCancelButton( 0 );
}

void Fit2DTool::showFileDialog(){
	QFileDialog dialog( this );
	QDir homeDir = QDir::home();
	QString homePath = homeDir.absolutePath();
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setWindowTitle("Select Fit2D Estimate File" );
	dialog.setDirectory( homePath );
	if ( dialog.exec() ){
		QStringList fileNames = dialog.selectedFiles();
		QString fileName = fileNames[0];
		ui.estimateFileLineEdit->setText( fileName );
	}
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

	String pixelBox = populatePixelBox();
	Vector<Float> includeVector = populateInclude();
	Vector<Float> excludeVector = populateExclude();
	QString channelStr = ui.channelLineEdit->text();
	int channel = channelStr.toInt();

	delete fitter;
	fitter = new Gaussian2DFitter();
	fitter->setFitParameters( image, pixelBox, channel, estimatesFileName, includeVector, excludeVector );
	connect( fitter, SIGNAL( finished() ), this, SLOT(fitDone()));
	fitter->start();
	progressBar.show();
}

void Fit2DTool::frameChanged( int frame ){
	ui.channelLineEdit->setText( QString::number( frame ));
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
	}
}

void Fit2DTool::finishedWork(){
	this->close();
}

void Fit2DTool::estimateFileChanged( const QString& fullPath ){
	ui.estimateFileLineEdit->setText( fullPath );
}

void Fit2DTool::showFindSourcesDialog(){
	if ( image != NULL ){
		String pixelBox = populatePixelBox();
		findSourcesDialog.setPixelBox( pixelBox );
		QString channelStr = ui.channelLineEdit->text();
		findSourcesDialog.setChannel(channelStr.toInt());
		findSourcesDialog.setImage( image );
		findSourcesDialog.show();
	}
	else {
		QMessageBox::warning( this, "Missing Image", "Please load an image that can be used for finding source estimates.");
	}
}

void Fit2DTool::resetRegion( const QList<int>& pixelX, const QList<int>& pixelY ){
	if ( pixelX.size() == 2 && pixelY.size() == 2 ){
		regionBoxBLC.clear();
		regionBoxTRC.clear();
		regionBoxBLC.append(pixelX[0]);
		regionBoxBLC.append(pixelY[0]);
		regionBoxTRC.append(pixelX[1]);
		regionBoxTRC.append(pixelY[1]);
	}
}

String Fit2DTool::populatePixelBox() const {
	QString pixelStr( "");
	const QString COMMA_STR( ",");
	if ( regionBoxBLC.size() == 2 && regionBoxTRC.size() == 2 ){
		pixelStr.append( QString::number(regionBoxBLC[0]) + COMMA_STR );
		pixelStr.append( QString::number(regionBoxBLC[1]) + COMMA_STR );

		pixelStr.append( QString::number(regionBoxTRC[0]) + COMMA_STR );
		pixelStr.append( QString::number(regionBoxTRC[1]));
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
}

void Fit2DTool::setImage( ImageInterface<Float>* image ){
	this->image = image;
	bool enableFunctionality = true;
	if ( image == NULL ){
		enableFunctionality = false;
	}
	setImageFunctionalityEnabled( enableFunctionality );
}

void Fit2DTool::newRegion( int /*id*/, const QString &/*shape*/, const QString &/*name*/,
		const QList<double> &/*world_x*/, const QList<double> &/*world_y*/,
		const QList<int> &pixel_x, const QList<int> &pixel_y,
		const QString &/*linecolor*/, const QString &/*text*/, const QString &/*font*/,
		int /*fontsize*/, int /*fontstyle*/ ) {
	resetRegion( pixel_x, pixel_y );
}

void Fit2DTool::updateRegion( int /*id*/, viewer::Region::RegionChanges /*changes*/,
		const QList<double> &/*world_x*/, const QList<double> &/*world_y*/,
		const QList<int> &pixel_x, const QList<int> &pixel_y ){
	resetRegion( pixel_x, pixel_y );
}

Fit2DTool::~Fit2DTool(){
	delete fitter;
}
}
