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
#include <display/Fit/ColorComboDelegate.h>
#include <display/RegionShapes/RegionShape.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/Display/Options.h>

#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <limits>

namespace casa {

	Fit2DTool::Fit2DTool(QWidget *parent)
		: QDialog(parent),  REGION_LABEL("Region:  "),
		  image(), fitter(NULL), progressBar( this ),
		  findSourcesDialog( this, false ), pixelRangeDialog( this ),
		  logDialog( this ) {
		ui.setupUi(this);

		const QString WINDOW_TITLE( "2D Fit Tool");
		this->setWindowTitle( WINDOW_TITLE );
		QPalette warningPalette = ui.warningLabel->palette();
		warningPalette.setColor(ui.warningLabel->foregroundRole(), Qt::red);
		ui.warningLabel->setPalette(warningPalette);

		QButtonGroup* modeGroup = new QButtonGroup( this );
		modeGroup->addButton( ui.imageRadioButton );
		modeGroup->addButton( ui.regionRadioButton );
		ui.imageRadioButton->setChecked( true );
		connect( ui.imageRadioButton, SIGNAL(toggled(bool)), this, SLOT(imageModeChanged(bool)));


		setImageFunctionalityEnabled( false );
		connect( &findSourcesDialog, SIGNAL(showOverlay(String, const QString& )), this, SIGNAL(showOverlay(String, const QString&)));
		connect( &findSourcesDialog, SIGNAL(removeOverlay(String)), this, SIGNAL(removeOverlay(String)));
		connect( &findSourcesDialog, SIGNAL(estimateFileSpecified(const QString&)), this, SLOT(estimateFileChanged( const QString&)));

		ui.channelLineEdit->setText( QString::number(0));
		QIntValidator* intValidator = new QIntValidator( 0, std::numeric_limits<int>::max(), this );
		ui.channelLineEdit->setValidator( intValidator );

		connect( ui.displayFitCheckBox, SIGNAL(toggled(bool)), this, SLOT(displayFitChanged(bool)));
		ui.viewButton->setEnabled( false );
		fitColorDelegate = new ColorComboDelegate( this );
		ui.fitColorCombo->setItemDelegate(  fitColorDelegate );
		fitColorDelegate->setSupportedColors( FindSourcesDialog::colorNames );
		for ( int i = 0; i < FindSourcesDialog::colorNames.size(); i++ ) {
			ui.fitColorCombo->addItem(FindSourcesDialog::colorNames[i]);
		}
		connect( ui.fitColorCombo, SIGNAL(currentIndexChanged(const QString&)),
		         this, SLOT(fitColorChanged(const QString&)));
		findSourcesDialog.setOverlayColor( FindSourcesDialog::colorNames[0] );

		double maxValue =std::numeric_limits<double>::max();
		double minValue = -1 * maxValue;
		QDoubleValidator* validator = new QDoubleValidator( minValue, maxValue, 7, this );
		ui.startLineEdit->setValidator( validator );
		ui.endLineEdit->setValidator( validator );
		connect( &pixelRangeDialog, SIGNAL(accepted()), this, SLOT(pixelRangeChanged()));
		QButtonGroup* buttonGroup = new QButtonGroup( this );
		buttonGroup->addButton( ui.includeRadioButton );
		buttonGroup->addButton( ui.excludeRadioButton );
		buttonGroup->addButton( ui.noneRadioButton );
		connect( ui.pixelRangeGroupBox, SIGNAL(toggled(bool)), this, SLOT(pixelRangeEnabledChanged(bool)));
		connect( ui.noneRadioButton, SIGNAL(toggled(bool)), this, SLOT(pixelRangeNoneSelected(bool)));

		connect( ui.fitButton, SIGNAL(clicked()), this, SLOT(doFit()));
		connect( ui.findEstimatesButton, SIGNAL(clicked()), this, SLOT(showFindSourcesDialog()));
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

	void Fit2DTool::fitColorChanged( const QString& colorName ) {
		if ( !fitMarkers.isEmpty()) {
			removeViewerFitMarkers();
			clearFitMarkers();
			fitMarkers = fitter->toDrawingDisplay( image, colorName );
			addViewerFitMarkers();
		}
		findSourcesDialog.setOverlayColor( colorName );
	}

	void Fit2DTool::imageModeChanged( bool enabled ){
		findSourcesDialog.setImageMode( enabled );
		pixelRangeDialog.setImageMode( enabled );
	}

	void Fit2DTool::residualSupportChanged( bool enable ) {
		if ( enable ) {
			if ( this->fitMarkers.isEmpty() ) {
				ui.residualHistogramButton->setEnabled( false );
			}
		}
	}

	void Fit2DTool::showFileChooserDialog(const QString& title,
	                                      QFileDialog::FileMode mode, QLineEdit* destinationLineEdit ) {
		QFileDialog dialog( this );
		QDir homeDir = QDir::current();
		QString homePath = homeDir.absolutePath();
		dialog.setFileMode(mode);
		dialog.setWindowTitle( title );
		dialog.setDirectory( homePath );
		if ( dialog.exec() ) {
			QStringList fileNames = dialog.selectedFiles();
			QString fileName = fileNames[0];
			destinationLineEdit->setText( fileName );
		}
	}

	void Fit2DTool::showFileDialog() {
		showFileChooserDialog( "Select Fit2D Estimate File",
		                       QFileDialog::ExistingFile, ui.estimateFileLineEdit);
	}

	void Fit2DTool::showSaveDialog() {
		showFileChooserDialog( "Select Save Directory",
		                       QFileDialog::DirectoryOnly, ui.saveDirectoryLineEdit);
	}

	void Fit2DTool::showResidualDialog() {
		showFileChooserDialog( "Select a Directory for the Residual Image",
		                       QFileDialog::DirectoryOnly, ui.residualDirectoryLineEdit);
	}

	void Fit2DTool::displayFitChanged( bool display ) {
		if ( display ) {
			addViewerFitMarkers();
		} else {
			removeViewerFitMarkers();
		}
		ui.fitColorCombo->setEnabled( display );
	}

	void Fit2DTool::pixelRangeEnabledChanged( bool enabled ) {
		if ( !enabled ) {
			ui.noneRadioButton->setChecked( true );
		}
		bool noneChecked = ui.noneRadioButton->isChecked();
		pixelRangeNoneSelected( noneChecked );
	}

	void Fit2DTool::pixelRangeNoneSelected( bool selected ) {
		ui.startLineEdit->setEnabled( !selected );
		ui.endLineEdit->setEnabled( !selected );
		ui.graphicalPixelRangeButton->setEnabled( !selected );
	}

	void Fit2DTool::pixelRangeChanged() {
		pair<double,double> range = pixelRangeDialog.getInterval();
		QString startRange = QString::number( range.first );
		QString endRange = QString::number( range.second );
		ui.startLineEdit->setText( startRange );
		ui.endLineEdit->setText( endRange );
	}

	void Fit2DTool::showPixelRangeDialog() {
		pixelRangeDialog.setImage( image );
		pixelRangeDialog.show();
	}

	void Fit2DTool::showResidualHistogramDialog() {
		if ( residualImagePath.length() > 0 ) {
			bool imageSet = residualHistogramDialog.setImage( residualImagePath );
			if ( imageSet ) {
				residualHistogramDialog.show();
			} else {
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
		if ( endRange < startRange ) {
			float tmp = endRange;
			startRange = endRange;
			endRange = tmp;
		}
		if ( startRange < endRange ) {
			range.resize( 2 );
			range[0] = startRange;
			range[1] = endRange;
		}
	}

	Vector<Float> Fit2DTool::populateInclude() const {
		Vector<Float> includeVector(0);
		if ( ui.includeRadioButton->isChecked() ) {
			populateIncludeExclude( includeVector );
		}
		return includeVector;
	}

	Vector<Float> Fit2DTool::populateExclude() const {
		Vector<Float> excludeVector(0);
		if ( ui.excludeRadioButton->isChecked() ) {
			populateIncludeExclude( excludeVector );
		}
		return excludeVector;
	}

	bool Fit2DTool::populateSaveFile( String& saveFile ) {
		bool saveOk = true;
		if ( ui.saveGroupBox->isChecked() ) {
			saveOk = validateFile( ui.saveDirectoryLineEdit, ui.saveFileLineEdit,
			                       saveFile, "saving fit output" );
		}
		return saveOk;
	}

	bool Fit2DTool::populateResidualFile( String& saveFile ) {
		bool residualOk = true;
		if ( ui.residualGroupBox->isChecked() ) {
			residualOk = validateFile( ui.residualDirectoryLineEdit, ui.residualFileLineEdit,
			                           saveFile, "saving residual image");
		}
		return residualOk;
	}

	bool Fit2DTool::validateFile( QLineEdit* directoryLineEdit, QLineEdit* fileLineEdit,
	                              String& saveFile, const QString& purpose ) {
		QString directoryStr = directoryLineEdit->text();
		bool fileOk = true;
		if ( directoryStr.trimmed().length() == 0 ) {
			QString msg("Please specify a directory for ");
			msg.append( purpose );
			msg.append( "." );
			QMessageBox::warning( this, "Missing Directory", msg );
			fileOk = false;
		} else {
			QFile file( directoryStr );
			if ( !file.exists() ) {
				QString msg("Please check that the directory for ");
				msg.append( purpose );
				msg.append( " is correctly specified.");
				QMessageBox::warning( this, "Path Error", msg );
				fileOk = false;
			} else {
				if ( !directoryStr.endsWith( QDir::separator() ) ) {
					directoryStr = directoryStr + QDir::separator();
				}
				QString fileName = fileLineEdit->text();
				if ( fileName.trimmed().length() == 0 ) {
					QString msg("Please specify the name of a file for ");
					msg.append( purpose );
					msg.append( "." );
					QMessageBox::warning( this, "Missing File Name", msg );
					fileOk = false;
				} else {
					QString path = directoryStr + fileName;
					saveFile = path.toStdString();
				}
			}
		}
		return fileOk;
	}


	void Fit2DTool::doFit() {

		String estimatesFileName;
		if ( ui.estimateGroupBox->isChecked()) {
			QString fileName = ui.estimateFileLineEdit->text().trimmed();
			if ( fileName.length() == 0 ) {
				QMessageBox::warning( this, "Missing Estimate File", "Please specify a file containing fit estimates.");
				return;
			} else {
				estimatesFileName = fileName.toStdString();
				bool errorWritingFile = false;
				estimatesFileName = findSourcesDialog.getScreenedEstimatesFile( estimatesFileName, &errorWritingFile );
				if ( errorWritingFile ) {
					QMessageBox::warning( this, "Error Writing Estimates File",
					                      "There was an error writing the source estimates file.");
					return;
				}
			}
		}

		String pixelBox = findSourcesDialog.getPixelBox();
		if ( pixelBox.length() == 0 ) {
			if ( ui.imageRadioButton->isChecked()){
				pixelBox = findSourcesDialog.getImagePixelBox().toStdString();
			}
			if ( pixelBox.length() == 0 ){
				QMessageBox::warning( this, "Invalid Pixel Box", "Please check that the region is valid in the image.");
				return;
			}
		}

		Vector<Float> includeVector = populateInclude();
		Vector<Float> excludeVector = populateExclude();
		QString channelStr = ui.channelLineEdit->text();
		int channel = channelStr.toInt();
		String saveFile;
		if ( ! populateSaveFile( saveFile )) {
			return;
		}
		String residualFile;
		if ( !populateResidualFile( residualFile )) {
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
		if ( saveFile.length() > 0 ) {
			fitter->setFilePath( saveFile );
		}
		fitter->setWriteLogFile( ui.logOutputCheckBox->isChecked());
		fitter->start();
		progressBar.show();
	}

	void Fit2DTool::frameChanged( int frame ) {
		ui.channelLineEdit->setText( QString::number( frame ));
		findSourcesDialog.setChannel( frame );
		pixelRangeDialog.setChannelValue( frame );
	}

	void Fit2DTool::updateFrame(){
		QString channelStr = ui.channelLineEdit->text();
		Bool ok = false;
		int channel = channelStr.toInt( &ok );
		if ( ok ){
			findSourcesDialog.setChannel( channel );
			pixelRangeDialog.setChannelValue( channel );
		}
	}

	bool Fit2DTool::setImageRegion( ImageRegion* imageRegion, int id ){
		bool result = pixelRangeDialog.setImageRegion( imageRegion, id );
		return result;
	}

	void Fit2DTool::deleteImageRegion( int id ){
		pixelRangeDialog.deleteImageRegion( id );
	}

	void Fit2DTool::imageRegionSelected( int id ){
		pixelRangeDialog.imageRegionSelected( id );
	}

	void Fit2DTool::clearFitMarkers() {
		while ( !fitMarkers.isEmpty() ) {
			RegionShape* marker = fitMarkers.takeLast();
			delete marker;
		}
	}

	void Fit2DTool::addViewerFitMarkers() {
		if ( fitMarkers.size() > 0 ) {
			if ( ui.displayFitCheckBox->isChecked()) {
				emit add2DFitOverlay( fitMarkers );
			}
		}
	}

	void Fit2DTool::removeViewerFitMarkers() {
		if ( fitMarkers.size() > 0 ) {
			emit remove2DFitOverlay( fitMarkers );
		}
	}

	void Fit2DTool::fitDone() {
		progressBar.hide();
		bool successfulFit = fitter->isFitSuccessful();
		if ( !successfulFit ) {
			QString errorMsg = fitter->getErrorMessage();
			QMessageBox::warning( this, "Fit Error", errorMsg );
		} else {
			QString logFilePath = fitter->getLogFilePath();
			bool logGenerated = logDialog.setLog( logFilePath );
			if ( logGenerated ) {
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
			if ( ui.residualGroupBox->isChecked() ) {
				ui.residualHistogramButton->setEnabled(true);
				residualImagePath=(fitter->getResidualImagePath().toStdString());
				emit addResidualFitImage( residualImagePath );
			}

			if ( ui.saveGroupBox->isChecked() ) {
				if ( ui.regionOutputCheckBox->isChecked() ) {
					bool success = fitter->writeRegionFile();
					if ( !success ) {
						QMessageBox::warning( this, "Error Writing Fit in Region Format",
						                      "Please check the output file path." );
					}
				}
			}
		}
	}

	void Fit2DTool::showResults() {
		logDialog.show();
	}



	void Fit2DTool::estimateFileChanged( const QString& fullPath ) {
		ui.estimateFileLineEdit->setText( fullPath );
	}

	void Fit2DTool::showFindSourcesDialog() {
		if ( image != NULL ) {
			QString channelStr = ui.channelLineEdit->text();
			findSourcesDialog.setChannel(channelStr.toInt());
			findSourcesDialog.show();
		} else {
			QMessageBox::warning( this, "Missing Image", "Please load an image that can be used for finding source estimates.");
		}
	}



	void Fit2DTool::setImageFunctionalityEnabled( bool enable ) {
		ui.fitButton->setEnabled( enable );
		ui.estimateGroupBox->setEnabled( enable );
		ui.pixelRangeGroupBox->setEnabled( enable );
		ui.channelLineEdit->setEnabled( enable );
		ui.fitImageLabel->setVisible( enable );
		if ( !enable ) {
			ui.fitRegionLabel->setVisible( enable );
		}
	}

	void Fit2DTool::setImage( shared_ptr<const ImageInterface<Float> > image ) {
		if ( image.get() != this->image.get() ){
			this->image = image;
			QString unitStr;
			bool enableFunctionality = true;
			if ( image.get() == NULL ) {
				enableFunctionality = false;
				findSourcesDialog.clearImage();
				image.reset();
			}
			else {
				String imageName = image->name(false);
				QString imageNameStr( imageName.c_str());
				ui.fitImageLabel->setText( "Image:" + imageNameStr );
				findSourcesDialog.setImage( image );
				Unit unit = this->image->units();
				unitStr = unit.getName().c_str();
			}
			ui.unitsLabel->setText( unitStr );
			//if ( pixelRangeDialog.isVisible() ){
				pixelRangeDialog.setImage( this->image );
			//}
			updateFrame();
			setImageFunctionalityEnabled( enableFunctionality );
		}
	}



	void Fit2DTool::newRegion( int id, const QString & shape, const QString &name,
	                           const QList<double> & world_x, const QList<double> & world_y,
	                           const QList<int> &pixel_x, const QList<int> &pixel_y,
	                           const QString & linecolor, const QString & text, const QString & font,
	                           int fontsize, int fontstyle ) {
		bool regionSet = false;
		if ( image.get() != NULL ) {
			regionSet = findSourcesDialog.newRegion( id, shape, name, world_x, world_y, pixel_x,
			            pixel_y, linecolor, text, font, fontsize, fontstyle );
			if ( regionSet ) {
				ui.fitRegionLabel->setVisible( true );
				QString regionStr = findSourcesDialog.getRegionString();
				QString boxSpec( regionStr);
				ui.fitRegionLabel->setText( REGION_LABEL+boxSpec);
			}
		}
	}



	void Fit2DTool::updateRegion( int id, viewer::region::RegionChanges changes,
	                              const QList<double> & world_x, const QList<double> & world_y,
	                              const QList<int> &pixel_x, const QList<int> &pixel_y ) {
		if ( image.get() != NULL ) {
			bool regionUpdate = findSourcesDialog.updateRegion( id, changes, world_x, world_y,
			                    pixel_x, pixel_y);
			if ( regionUpdate ) {
				QString regionStr = findSourcesDialog.getRegionString();
				if ( regionStr.length() == 0 ) {
					ui.fitRegionLabel->setVisible( false );
				} else {
					ui.fitRegionLabel->setText( REGION_LABEL + regionStr);
				}
			}
		}
	}


	Fit2DTool::~Fit2DTool() {
		delete fitter;
		clearFitMarkers();

	}
}
