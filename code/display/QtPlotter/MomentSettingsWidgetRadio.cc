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


#include "MomentSettingsWidgetRadio.qo.h"
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <display/QtPlotter/ProfileTaskMonitor.h>
#include <display/QtPlotter/ThresholdingBinPlotDialog.qo.h>
#include <display/QtPlotter/conversion/Converter.h>
#include <display/QtPlotter/Util.h>
#include <imageanalysis/Regions/CasacRegionManager.h>
#include <ms/MeasurementSets/MS1ToMS2Converter.h>
#include <display/Display/Options.h>

#include <QFileDialog>
#include <QTime>
#include <QDebug>
#include <QTemporaryFile>

namespace casa {

	MomentCollapseThreadRadio::MomentCollapseThreadRadio( ImageAnalysis* imageAnalysis ):
		analysis( imageAnalysis ), stepSize( 10 ), collapseError(false) {
		imageAnalysis->setMomentsProgressMonitor( this );
	}

	bool MomentCollapseThreadRadio::isSuccess() const {
		bool success = false;
		if ( collapseResults.size() > 0 && !collapseError ) {
			success = true;
		}
		return success;
	}

	void MomentCollapseThreadRadio::setStepCount( int count ) {
		//We don't want to send too many notifications.
		const int STEP_LIMIT = 100;
		stepSize = count / STEP_LIMIT;
		const int BASE_STEP = 10;
		if ( stepSize < BASE_STEP ) {
			stepSize = BASE_STEP;
		}
		int adjustedCount = count / stepSize + 1;
		emit stepCountChanged( adjustedCount );
	}

	void MomentCollapseThreadRadio::halt() {
		stopImmediately = true;
	}

	void MomentCollapseThreadRadio::setStepsCompleted( int count ) {
		if ( count % stepSize == 0 ) {
			int adjustedCount = count / stepSize;
			emit stepsCompletedChanged( adjustedCount );
		}
	}

	void MomentCollapseThreadRadio::done() {
	}

	String MomentCollapseThreadRadio::getErrorMessage() const {
		return errorMsg;
	}

	void MomentCollapseThreadRadio::setData(const Vector<Int>& mments, const Int axis, Record& region,
	                                        const String& maskStr, const Vector<String>& methodVec,
	                                        const Vector<Int>& smoothaxesVec,
	                                        const Vector<String>& smoothtypesVec,
	                                        const Vector<Quantity>& smoothwidthsVec,
	                                        const Vector<Float>& includepixVec,
	                                        const Vector<Float>& excludepixVec,
	                                        const Double peaksnr, const Double stddev,
	                                        const String& doppler, const String& baseName) {
		moments.resize( mments.size());

		moments = mments;
		this->axis = axis;
		this->region = region;
		mask = maskStr;
		method = methodVec;
		smoothaxes = smoothaxesVec;
		smoothtypes = smoothtypesVec;
		smoothwidths = smoothwidthsVec;
		includepix = includepixVec;
		excludepix = excludepixVec;
		this->peaksnr = peaksnr;
		this->stddev = stddev;
		this->doppler = doppler;
		this->baseName = baseName;
	}

	std::vector<CollapseResult> MomentCollapseThreadRadio::getResults() const {
		return collapseResults;
	}

	void MomentCollapseThreadRadio::setChannelStr( String str ) {
		channelStr = str;
	}

	void MomentCollapseThreadRadio::setOutputFileName( QString name ) {
		outputFileName = name;
	}

	void MomentCollapseThreadRadio::setMomentNames( const Vector<QString>& momentNames ) {
		this->momentNames = momentNames;
	}

	bool MomentCollapseThreadRadio::getOutputFileName( String& outName,
	        int /*moment*/, const String& channelStr ) const {

		bool tmpFile = true;
		//Use a default base name
		if (outputFileName.isEmpty()) {
			outName = baseName;
		}
		//Use the user specified name
		else {
			outName = outputFileName.toStdString();
			tmpFile = false;
		}

		//Append the channel and moment used to make it descriptive.
		//outName = outName + "_" + String(momentNames[moment].toStdString());
		if ( channelStr != "") {
			outName = outName + "_"+channelStr;
		}
		if ( tmpFile ) {
			outName = viewer::options.temporaryPath( outName );
		}
		return tmpFile;
	}


	void MomentCollapseThreadRadio::run() {
		try {
			//casa::utilj::ThreadTimes t1;
			stopImmediately = false;
			for ( int i = 0; i < static_cast<int>(moments.size()); i++ ) {
				if ( stopImmediately ){
					collapseError = true;
					break;
				}
				Vector<int> whichMoments(1);
				whichMoments[0] = moments[i];

				//Output file
				String outFile;
				bool outputFileTemporary = getOutputFileName( outFile, i, channelStr );
				/*const String& doppler = "RADIO",  const String& outfile = "",
				        const String& smoothout="", const String& plotter="/NULL",
				        const Int nx=1, const Int ny=1,  const Bool yind=False,
				        const Bool overwrite=False, const Bool drop=True,*/

				ImageInterface<Float>* newImage = analysis->moments( whichMoments, axis, region,
				                                  mask, method,
				                                  smoothaxes, smoothtypes, smoothwidths,
				                                  includepix,excludepix,
				                                  peaksnr, stddev, "RADIO", outFile, "", "",
				                                  1,1,False,False,False);
				if ( newImage != NULL ) {
					CollapseResult result( outFile, outputFileTemporary, newImage );
					collapseResults.push_back( result );
				}
			}
			//casa::utilj::ThreadTimes t2;
			//casa::utilj::DeltaThreadTimes dt = t2 - t1;
			//qDebug() << "Elapsed time moment="<<moments[0]<< " elapsed="<<dt.elapsed()<<" cpu="<<dt.cpu();
		} catch( AipsError& error ) {
			errorMsg = error.getLastMessage();
			collapseError = true;
		}
	}

	MomentCollapseThreadRadio::~MomentCollapseThreadRadio() {
	}


	MomentSettingsWidgetRadio::MomentSettingsWidgetRadio(QWidget *parent)
		: QWidget(parent), imageAnalysis( NULL ), collapseThread( NULL ),
		  thresholdingBinDialog( NULL ), progressBar( parent ) {
		ui.setupUi(this);

		//Initialize the progress bar
		progressBar.setWindowModality( Qt::ApplicationModal );
		Qt::WindowFlags flags = Qt::Dialog;
		flags |= Qt::FramelessWindowHint;
		progressBar.setWindowFlags( flags);
		progressBar.setWindowTitle( "Collapse/Moments");
		progressBar.setLabelText( "Calculating moments...");
		connect( this, SIGNAL( updateProgress(int)), &progressBar, SLOT( setValue( int )));
		connect( this, SIGNAL( momentsFinished()), &progressBar, SLOT(cancel()));
		connect( &progressBar, SIGNAL(canceled()), this, SLOT(stopMoments()));

		momentOptions << "(-1) Mean Value, Mean Intensity" <<
		              "(0) Integrated Value, Sum" <<
		              "(1) Weighted Mean, Velocity Field"<<
		              "(2) Intensity-Weighted Dispersion of Spectral Coordinate, Velocity Dispersion" <<
		              "(3) Median Value, Median Intensity" <<
		              "(4) Spectral Coordinate of Median, Median Velocity Field" <<
		              "(5) Standard Deviation About Mean, Noise, Intensity Scatter" <<
		              "(6) Root Mean Square Intensity"<<
		              "(7) Absolute Mean Deviation" <<
		              "(8) Maximum Intensity, MaximumValue" <<
		              "(9) Spectral Coordinate of Maximum, Velocity of Maximum"<<
		              "(10) Minimum Intensity, MinimumValue" <<
		              "(11) Spectral Coordinate of Minimum, Velocity of Minimum";
		for ( int i = 0; i < static_cast<int>(END_INDEX); i++ ) {
			QListWidgetItem* listItem = new QListWidgetItem( momentOptions[i], ui.momentList);
			if ( i == static_cast<int>(INTEGRATED) ) {
				ui.momentList->setCurrentItem( listItem );
			}
		}
		int columnWidth = ui.momentList->sizeHintForColumn(0);

		ui.momentList->setMinimumWidth( 2*columnWidth/3 );
		ui.momentList->setMaximumWidth( columnWidth );

		//Right now, there is not a clear need for the moment map, but if some
		//moments are no longer used in the display, it will be needed.
		momentMap[MEAN] = -1;
		momentMap[INTEGRATED] = 0;
		momentMap[WEIGHTED_MEAN] = 1;
		momentMap[DISPERSION] = 2;
		momentMap[MEDIAN] = 3;
		momentMap[MEDIAN_VELOCITY] = 4;
		momentMap[STDDEV] = 5;
		momentMap[RMS] = 6;
		momentMap[ABS_MEAN_DEV] = 7;
		momentMap[MAX] = 8;
		momentMap[MAX_VELOCITY] = 9;
		momentMap[MIN] = 10;
		momentMap[MIN_VELOCITY] = 11;

		ui.channelTable->setColumnCount( 2 );
		QStringList tableHeaders =(QStringList()<< "Min" << "Max");
		ui.channelTable->setHorizontalHeaderLabels( tableHeaders );
		ui.channelTable->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.channelTable->setSelectionMode( QAbstractItemView::SingleSelection );
		ui.channelTable->setColumnWidth(0, 125);
		ui.channelTable->setColumnWidth(1, 125);

		connect( ui.collapseButton, SIGNAL(clicked()), this, SLOT( collapseImage()));
		connect( ui.channelIntervalCountSpinBox, SIGNAL( valueChanged(int)), this, SLOT(adjustTableRows(int)));
		connect( ui.includeRadioButton, SIGNAL(clicked()), this, SLOT(thresholdingChanged()));
		connect( ui.excludeRadioButton, SIGNAL(clicked()), this, SLOT(thresholdingChanged()));
		connect( ui.noneRadioButton, SIGNAL(clicked()), this, SLOT(thresholdingChanged()));
		connect( ui.outputButton, SIGNAL(clicked()), this, SLOT( setCollapsedImageFile()));
		connect( ui.graphThresholdButton, SIGNAL(clicked()), this, SLOT( graphicalThreshold()));
		connect( ui.symmetricIntervalCheckBox, SIGNAL(stateChanged(int)), this, SLOT(symmetricThresholdChanged(int)));
		connect( ui.maxThresholdLineEdit, SIGNAL(textChanged( const QString&)), this, SLOT(thresholdTextChanged( const QString&)));

		thresholdingChanged();
		ui.channelIntervalCountSpinBox->setValue( 1 );

		//Make sure the min and max thresholds accept only doubles.
		const QDoubleValidator* validator = new QDoubleValidator( this );
		ui.minThresholdLineEdit->setValidator( validator );
		ui.maxThresholdLineEdit->setValidator( validator );
	}

	String MomentSettingsWidgetRadio::makeChannelInterval( float startChannelIndex,
	        float endChannelIndex ) const {
		String channelStr=String::toString( startChannelIndex)+"~"+String::toString(endChannelIndex);
		return channelStr;
	}

	String MomentSettingsWidgetRadio::populateChannels(uInt* nSelectedChannels ) {
		int channelIntervalCount = ui.channelIntervalCountSpinBox->value();
		String channelStr;
		for ( int i = 0; i < channelIntervalCount; i++ ) {
			QString startStr;
			QString endStr;
			getChannelMinMax( i, startStr, endStr );
			if ( isValidChannelRangeValue( startStr, "Start" ) && isValidChannelRangeValue( endStr, "End" )) {
				// convert input values to Float
				float startChanVal=startStr.toFloat();
				float endChanVal  =endStr.toFloat();
				if ( endChanVal < startChanVal ) {
					//Switch them around - the code expects the startVal
					//to be less than the endVal;
					float tempVal = startChanVal;
					startChanVal = endChanVal;
					endChanVal = tempVal;
				}

				int startChannelIndex = -1;
				int endChannelIndex = -1;
				Vector<float> z_xval = taskMonitor->getXValues();
				findChannelRange( startChanVal, endChanVal, z_xval, startChannelIndex, endChannelIndex );
				*nSelectedChannels = *nSelectedChannels + (endChannelIndex - startChannelIndex + 1);

				String channelIntervalStr = makeChannelInterval( startChannelIndex, endChannelIndex );
				if ( i > 0 ) {
					channelStr = channelStr + ",";
				}
				channelStr = channelStr + channelIntervalStr;
			}
		}
		return channelStr;
	}

	bool MomentSettingsWidgetRadio::populateThresholds( Vector<Float>& includeThreshold,
	        Vector<Float>& excludeThreshold ) {
		bool validThresholds = true;
		if ( ui.includeRadioButton->isChecked() ) {
			validThresholds = populateThreshold( includeThreshold );
		} else if ( ui.excludeRadioButton->isChecked() ) {
			validThresholds = populateThreshold( excludeThreshold );
		}
		return validThresholds;
	}

	bool MomentSettingsWidgetRadio::populateThreshold( Vector<Float>& threshold ) {

		bool validThreshold = true;

		//Neither threshold should be blank
		QString minThresholdStr = ui.minThresholdLineEdit->text();
		QString maxThresholdStr = ui.maxThresholdLineEdit->text();
		const int ALL_THRESHOLD = -1;
		double minThreshold = ALL_THRESHOLD;
		double maxThreshold = ALL_THRESHOLD;
		if ( ! minThresholdStr.isEmpty() ) {
			minThreshold = minThresholdStr.toDouble();
		}
		if ( ! maxThresholdStr.isEmpty() ) {
			maxThreshold = maxThresholdStr.toDouble();
		}

		//Minimum should be less than the maximum
		if ( minThreshold > maxThreshold ) {
			validThreshold = false;
			QString msg = "Minimum threshold should be less than the maximum threshold.";
			Util::showUserMessage( msg, this );
		} else {

			//Initialize the vector.
			if ( minThreshold == ALL_THRESHOLD && maxThreshold == ALL_THRESHOLD ) {
				threshold.resize( 1 );
				threshold[0] = ALL_THRESHOLD;
			} else {
				threshold.resize( 2 );
				threshold[0] = minThreshold;
				threshold[1] = maxThreshold;
			}
		}
		return validThreshold;
	}

	Vector<Int> MomentSettingsWidgetRadio::populateMoments( Vector<QString>& momentNames ) {
		//Set up which moments we want
		QList<QListWidgetItem*> selectedItems = ui.momentList->selectedItems();
		int momentCount = selectedItems.length();
		momentNames.resize( momentCount );
		Vector<Int> whichMoments(momentCount);
		if ( momentCount == 0 ) {
			QString msg = "Please select at least one moment.";
			Util::showUserMessage( msg, this );
		} else {
			for( int i = 0; i < momentCount; i++ ) {
				QString selectedText = selectedItems[i]->text();
				momentNames[i] = selectedText;
				int index = momentOptions.indexOf( selectedText );
				if ( index >= 0 ) {
					int momentIndex = static_cast<int>(momentMap[static_cast<SummationIndex>(index)]);
					whichMoments[i] = momentIndex;
				}
			}
		}
		return whichMoments;
	}



	void MomentSettingsWidgetRadio::collapseImage() {

		// Get the spectral axis number.
		// TODO: Generalize this to any hidden axis
		std::tr1::shared_ptr<const ImageInterface<float> > image = taskMonitor->getImage();
		DisplayCoordinateSystem cSys = image -> coordinates();
		int spectralAxisNumber = cSys.spectralAxisNumber();
		if ( spectralAxisNumber < 0 ){
			spectralAxisNumber = Util::getTabularFrequencyAxisIndex( image );
		}
		Vector<String> method;

		//Note default SNRPEAK is 3.  Must be nonnegative.
		Double peaksnr = 3;
		//Note default stddev is 0. Must be nonnegative.
		Double stddev = 0;

		//Initialize the include/exclude pixels
		Vector<Float> excludepix;
		Vector<Float> includepix;
		bool validThresholds = populateThresholds( includepix, excludepix );
		if ( !validThresholds ) {
			return;
		}

		Vector<Int> smoothaxes;
		Vector<String> smoothtypes;
		Vector<Quantity> smoothwidths;
		QString fileName = taskMonitor->getImagePath();
		String infile(fileName.toStdString());

		//Initialize the channels
		uInt nSelectedChannels;
		String channelStr = populateChannels( &nSelectedChannels );

		//Get the region
		IPosition pos = image->shape();
		String regionName;

		String stokesStr = "";
		CasacRegionManager crm( cSys );
		String diagnostics;
		String pixelBox="";
		Record region = crm.fromBCS( diagnostics, nSelectedChannels, stokesStr,
		                             NULL, regionName, channelStr, CasacRegionManager::USE_FIRST_STOKES,
		                             pixelBox, pos, infile);
		//Set up the imageAnalysis
		if ( imageAnalysis == NULL ) {
			// a cast might work, but I'm just going to clone it to be safe - dmehring shared_ptr refactor
			// cloning was what the old ImageAnalysis constructor did anyway
			std::tr1::shared_ptr<ImageInterface<float> > image2(image->cloneII());
			imageAnalysis = new ImageAnalysis( image2 );

		}

		//Set up the thread that will do the work.
		delete collapseThread;
		collapseThread = new MomentCollapseThreadRadio( imageAnalysis );
		connect( collapseThread, SIGNAL( finished() ), this, SLOT(collapseDone()));
		connect( collapseThread, SIGNAL(stepCountChanged(int)), this, SLOT(setStepCount(int)));
		connect( collapseThread, SIGNAL(stepsCompletedChanged(int)), this, SLOT(setStepsCompleted(int)));

		//Do a collapse image for each of the moments.
		Vector<QString> momentNames;
		Vector<Int> moments = populateMoments( momentNames );
		momentCount = moments.size();
		collapseThread-> setMomentNames( momentNames );
		String baseName( taskMonitor->getFileName().toStdString());
		collapseThread->setData(moments, spectralAxisNumber, region,
		                        "", method, smoothaxes, smoothtypes, smoothwidths,
		                        includepix, excludepix, peaksnr, stddev,
		                        "RADIO", baseName);
		if ( !outputFileName.isEmpty() ) {
			collapseThread->setOutputFileName( outputFileName );
		}

		collapseThread->setChannelStr( channelStr );
		previousCount = 0;
		cycleCount = 0;
		if ( moments.size() == 1 ){
			progressBar.setCancelButtonText( QString() );
		}
		else {
			progressBar.setCancelButtonText( "Cancel");
		}
		progressBar.show();
//#warning "Revert to THREADING"
		collapseThread->start();
		//collapseThread->run();
	}

	void MomentSettingsWidgetRadio::collapseDone() {
		//Update the viewer with the collapsed image.
		emit momentsFinished();
		if ( collapseThread != NULL && collapseThread->isSuccess()) {
			std::vector<CollapseResult> results = collapseThread->getResults();
			for ( int i = 0; i < static_cast<int>(results.size()); i++ ) {
				String outName = results[i].getOutputFileName();
				bool outputTemporary = results[i].isTemporaryOutput();
				ImageInterface<Float>* newImage = results[i].getImage();
				taskMonitor->imageCollapsed(outName, "image", "raster", True, outputTemporary, newImage );
			}
			taskMonitor->setPurpose(ProfileTaskMonitor::MOMENTS_COLLAPSE );
		} else {

			QString msg( "Moment calculation failed.");
			String errorMsg = collapseThread->getErrorMessage();
			if ( ! errorMsg.empty() ) {
				msg.append( "\n");
				msg.append( errorMsg.c_str() );
			}
			Util::showUserMessage( msg, this );
		}
	}

	void MomentSettingsWidgetRadio::getChannelMinMax( int channelIndex, QString& minStr, QString& maxStr ) const {
		QTableWidgetItem* minItem = ui.channelTable->item( channelIndex, 0 );
		if ( minItem != NULL ) {
			minStr = minItem->text();
		}
		QTableWidgetItem* maxItem  = ui.channelTable->item( channelIndex, 1 );
		if ( maxItem != NULL ) {
			maxStr = maxItem->text();
		}
	}

	void MomentSettingsWidgetRadio::convertChannelValue( const QString& channelStr,
	        const QString& channelIdentifier, Converter* converter, int row, int col,
	        bool toPixels ) {
		if ( isValidChannelRangeValue( channelStr, channelIdentifier )) {
			float chanVal = channelStr.toFloat();
			if ( ! toPixels ) {
				chanVal = converter->convert( chanVal );
			} else {
				chanVal = converter->toPixel( chanVal );
			}
			setTableValue( row, col, chanVal );
		}
	}

	void MomentSettingsWidgetRadio::convertChannelRanges( const QString& oldUnits, const QString& newUnits ) {
		int channelIntervalCount = ui.channelIntervalCountSpinBox->value();
		bool toPixels = false;
		if ( newUnits.isEmpty() ) {
			toPixels = true;
		}
		Converter* converter = Converter::getConverter( oldUnits, newUnits );
		for ( int i = 0; i < channelIntervalCount; i++ ) {
			QString startStr;
			QString endStr;
			getChannelMinMax( i, startStr, endStr );
			convertChannelValue( startStr, "Start", converter, i, 0, toPixels );
			convertChannelValue( endStr, "End", converter, i, 1, toPixels );
		}
		delete converter;
	}

	void MomentSettingsWidgetRadio::setUnits( QString unitStr ) {
		int bracketIndex = unitStr.indexOf( "[]");
		if ( bracketIndex > 0 ) {
			unitStr = "";
		}

		QString prevUnit = ui.channelGroupBox->title();
		int startIndex = unitStr.indexOf( "[");
		int endIndex = unitStr.indexOf( "]");
		if ( startIndex > 0 && endIndex > 0 ) {
			unitStr = unitStr.mid(startIndex, endIndex + 1 - startIndex);
		}
		ui.channelGroupBox->setTitle( unitStr );
		if ( prevUnit != "Channels" && prevUnit != unitStr ) {
			QString oldUnits = Util::stripBrackets( prevUnit );
			QString newUnits = Util::stripBrackets( unitStr );
			convertChannelRanges( oldUnits, newUnits );
		}
	}

	void MomentSettingsWidgetRadio::setTableValue(int row, int col, float val ) {
		QTableWidgetItem* peakItem = new QTableWidgetItem();
		peakItem -> setText( QString::number( val ) );
		ui.channelTable->setItem( row, col, peakItem );
	}

	void MomentSettingsWidgetRadio::setRange( double min, double max ) {
		if (max < min) {
			ui.channelIntervalCountSpinBox->setValue( 0 );
			ui.channelTable->setRowCount( 0 );
		} else {
			QList<QTableWidgetSelectionRange> selectionRanges = ui.channelTable->selectedRanges();
			int selectionCount = selectionRanges.length();
			if ( selectionCount == 0 ) {
				if ( ui.channelTable->isVisible() && ui.channelIntervalCountSpinBox->value() > 0 ) {
					QString msg( "Please select a row in the channel table before specifying the estimate.");
					Util::showUserMessage( msg, this );
				}
			} else if ( selectionCount == 1 ) {
				QTableWidgetSelectionRange selectionRange = selectionRanges[0];
				int tableRow = selectionRange.bottomRow();
				setTableValue( tableRow, 0, min );
				setTableValue( tableRow, 1, max );
			}
		}
	}

	void MomentSettingsWidgetRadio::reset() {
		if ( imageAnalysis != NULL ) {
			delete imageAnalysis;
			imageAnalysis = NULL;
		}
		delete collapseThread;
		collapseThread = NULL;
		if ( taskMonitor != NULL ) {
			std::tr1::shared_ptr<ImageInterface<Float> > img = std::tr1::const_pointer_cast<ImageInterface <Float> >(taskMonitor->getImage());
			imageAnalysis = new ImageAnalysis(img);
		}
	}

	void MomentSettingsWidgetRadio::thresholdingChanged( ) {
		bool enabled = false;
		if ( ui.includeRadioButton->isChecked() || ui.excludeRadioButton->isChecked() ) {
			enabled = true;
		}

		ui.maxThresholdLineEdit->setEnabled( enabled );
		ui.symmetricIntervalCheckBox->setEnabled( enabled );
		ui.graphThresholdButton->setEnabled( enabled );
		if ( !ui.symmetricIntervalCheckBox->isChecked() ) {
			ui.minThresholdLineEdit->setEnabled( enabled );
		}
		if ( !enabled ) {
			ui.minThresholdLineEdit->clear();
			ui.maxThresholdLineEdit->clear();
		}
	}


	void MomentSettingsWidgetRadio::adjustTableRows( int count ) {
		ui.channelTable -> setRowCount( count );
		//Select the last row of the table
		QTableWidgetSelectionRange selectionRange(count-1,0,count-1,1);
		ui.channelTable->setRangeSelected( selectionRange, true );
	}

	void MomentSettingsWidgetRadio::setCollapsedImageFile() {
		string homedir = getenv("HOME");
		QFileDialog fd( this, tr("Specify a root file for the collapsed image(s)."),
		                QString(homedir.c_str()), "");
		fd.setFileMode( QFileDialog::AnyFile );
		if ( fd.exec() ) {
			QStringList fileNames = fd.selectedFiles();
			if ( fileNames.size() > 0 ) {
				outputFileName = fileNames[0];
			}
		}

	}

	void MomentSettingsWidgetRadio::symmetricThresholdChanged( int checkedState ) {
		if ( checkedState == Qt::Checked ) {
			ui.minThresholdLineEdit->setEnabled( false );
			//Copy the next from the max to the min.
			QString maxText = ui.maxThresholdLineEdit->text();
			thresholdTextChanged( maxText );
		} else {
			ui.minThresholdLineEdit->setEnabled( true );
		}
	}

	void MomentSettingsWidgetRadio::thresholdTextChanged( const QString& text ) {
		if ( ui.symmetricIntervalCheckBox->isChecked() ) {
			Bool validDouble = false;
			if ( !text.isEmpty() ) {
				double thresholdValue = text.toDouble( &validDouble );
				if ( validDouble ) {
					thresholdValue = -1 * thresholdValue;
					QString oppositeText = QString::number( thresholdValue );
					ui.minThresholdLineEdit->setText( oppositeText );
				} else {
					//Shouldn't get here, but just in case let the user know there is
					//a problem.
					QString msg( "Please specify a valid number for the threshold.");
					Util::showUserMessage( msg, this );
				}
			}
		}
	}

	void MomentSettingsWidgetRadio::thresholdSpecified() {
		pair<double,double> minMaxValues = thresholdingBinDialog->getInterval();
		QString maxValueStr = QString::number( minMaxValues.second);
		ui.maxThresholdLineEdit->setText( maxValueStr );
		if ( ! ui.symmetricIntervalCheckBox->isChecked() ) {
			ui.minThresholdLineEdit->setText( QString::number(minMaxValues.first) );
		} else {
			thresholdTextChanged( maxValueStr );
		}
	}



	void MomentSettingsWidgetRadio::graphicalThreshold() {
		if ( thresholdingBinDialog == NULL ) {
			QString yUnits = this->getYUnit();
			thresholdingBinDialog = new ThresholdingBinPlotDialog( yUnits, this );
			connect( thresholdingBinDialog, SIGNAL(accepted()), this, SLOT(thresholdSpecified()));
		}
		// ImageInterface<Float>* image = const_cast<ImageInterface<Float>* >(taskMonitor->getImage().get());
		std::tr1::shared_ptr<ImageInterface<Float> > image(std::tr1::const_pointer_cast<ImageInterface<Float> >(taskMonitor->getImage()));
		thresholdingBinDialog->setImage( image );
		thresholdingBinDialog->show();
		QString minValueStr = ui.minThresholdLineEdit->text();
		QString maxValueStr = ui.maxThresholdLineEdit->text();
		double minValue = minValueStr.toDouble();
		double maxValue = maxValueStr.toDouble();
		thresholdingBinDialog->setInterval( minValue, maxValue );
	}
//*************************************************************************
//       Methods from the ImageMomentsProgressMonitor interface
//*************************************************************************

//Note:  because the moments computation is run in a background thread,
//and progress updates must occur in the GUI thread, communication between
//the background thread and the progress bar is via signal/slots.

	void MomentSettingsWidgetRadio::setStepCount( int count ) {
		progressBar.setMinimum( 0 );
		progressBar.setMaximum( count );
		baseIncrement = count / momentCount;
	}

	void MomentSettingsWidgetRadio::setStepsCompleted( int count ) {
		//Cycling over again with a new moment.
		if ( count < previousCount ) {
			cycleCount++;
		}
		int taskCount = cycleCount * baseIncrement + count / momentCount;
		previousCount = count;
		emit updateProgress( taskCount );
	}

	void MomentSettingsWidgetRadio::stopMoments(){
		if ( collapseThread != NULL && collapseThread->isRunning()){
			collapseThread->halt();
		}
	}

	MomentSettingsWidgetRadio::~MomentSettingsWidgetRadio() {
		if ( imageAnalysis != NULL ) {
			delete imageAnalysis;
		}
	}



}
