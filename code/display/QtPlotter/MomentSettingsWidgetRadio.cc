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
#include <images/Images/ImageAnalysis.h>

#include <display/QtPlotter/ProfileTaskMonitor.h>
#include <display/QtPlotter/MomentCollapseThreadRadio.h>
#include <display/QtPlotter/ThresholdingBinPlotDialog.qo.h>
#include <display/QtPlotter/Util.h>
#include <imageanalysis/Regions/CasacRegionManager.h>
#include <ms/MeasurementSets/MS1ToMS2Converter.h>

#include <QFileDialog>
#include <QTime>
#include <QTemporaryFile>

namespace casa {

MomentSettingsWidgetRadio::MomentSettingsWidgetRadio(QWidget *parent)
    : QWidget(parent), imageAnalysis( NULL ), collapseThread( NULL ), thresholdingBinDialog( NULL )
{
	ui.setupUi(this);

	momentOptions << "Maximum Value" << "Mean Value" <<"Median Coordinate"
			<< "Minimum Value" << "Root Mean Square" << "Standard Deviation about the Mean" <<
			"Integrated Value" << "Absolute Mean Deviatian";
	for ( int i = 0; i < static_cast<int>(END_INDEX); i++ ){
		QListWidgetItem* listItem = new QListWidgetItem( momentOptions[i], ui.momentList);
		if ( i == static_cast<int>(INTEGRATED) ){
			ui.momentList->setCurrentItem( listItem );
		}
	}
	momentMap[MAX]=8;
	momentMap[MEAN]=-1;
	momentMap[MEDIAN]=4;
	momentMap[MIN]=10;
	momentMap[RMS]=6;
	momentMap[STDDEV]=5;
	momentMap[INTEGRATED]= 0;
	momentMap[ABS_MEAN_DEV]=7;

	ui.channelTable->setColumnCount( 2 );
	QStringList tableHeaders =(QStringList()<< "   Min   " << "   Max   ");
	ui.channelTable->setHorizontalHeaderLabels( tableHeaders );
	ui.channelTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.channelTable->setSelectionMode( QAbstractItemView::SingleSelection );
	ui.channelTable->setColumnWidth(0, 70);
	ui.channelTable->setColumnWidth(1,70);

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

	//Make sure the min and max thresholds accept only doubles.
	const QDoubleValidator* validator = new QDoubleValidator( this );
	ui.minThresholdLineEdit->setValidator( validator );
	ui.maxThresholdLineEdit->setValidator( validator );

	//For random number generation.
	QTime time = QTime::currentTime();
	qsrand( (uint) time.msec());
}

String MomentSettingsWidgetRadio::makeChannelInterval( float startChannelIndex,
		float endChannelIndex ) const {
	String channelStr=String::toString( startChannelIndex)+"~"+String::toString(endChannelIndex);
	return channelStr;
}

String MomentSettingsWidgetRadio::populateChannels(uInt* nSelectedChannels ){
	int channelIntervalCount = ui.channelIntervalCountSpinBox->value();
	String channelStr;
	for ( int i = 0; i < channelIntervalCount; i++ ){
		QTableWidgetItem* minItem = ui.channelTable->item(i, 0);
		QString startStr;
		if ( minItem != NULL ){
			startStr = minItem->text();
		}
		QTableWidgetItem* maxItem  = ui.channelTable->item( i, 1 );
		QString endStr;
		if ( maxItem != NULL ){
			endStr = maxItem->text();
		}
		if ( isValidChannelRangeValue( startStr, "Start" ) && isValidChannelRangeValue( endStr, "End" )){
			// convert input values to Float
			float startChanVal=startStr.toFloat();
			float endChanVal  =endStr.toFloat();
			if ( endChanVal < startChanVal ){
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
			if ( i > 0 ){
				channelStr = channelStr + ",";
			}
			channelStr = channelStr + channelIntervalStr;
		}
	}
	return channelStr;
}

bool MomentSettingsWidgetRadio::populateThresholds( Vector<Float>& includeThreshold,
		Vector<Float>& excludeThreshold ){
	bool validThresholds = true;
	if ( ui.includeRadioButton->isChecked() ){
		validThresholds = populateThreshold( includeThreshold );
	}
	else if ( ui.excludeRadioButton->isChecked() ){
		validThresholds = populateThreshold( excludeThreshold );
	}
	return validThresholds;
}

bool MomentSettingsWidgetRadio::populateThreshold( Vector<Float>& threshold ){

	bool validThreshold = true;

	//Neither threshold should be blank
	QString minThresholdStr = ui.minThresholdLineEdit->text();
	QString maxThresholdStr = ui.maxThresholdLineEdit->text();
	if ( minThresholdStr.isEmpty() || maxThresholdStr.isEmpty() ){
		validThreshold = false;
		QString msg = "Please specify thresholding limit(s).";
		Util::showUserMessage( msg, this );
	}
	else {
		double minThreshold = minThresholdStr.toDouble();
		double maxThreshold = maxThresholdStr.toDouble();

		//Minimum should be less than the maximum
		if ( minThreshold > maxThreshold ){
			validThreshold = false;
			QString msg = "Minimum threshold should be less than the maximum threshold.";
			Util::showUserMessage( msg, this );
		}
		else {
			threshold.resize( 2 );
			threshold[0] = minThreshold;
			threshold[1] = maxThreshold;
		}
	}
	return validThreshold;
}

Vector<Int> MomentSettingsWidgetRadio::populateMoments(){
	//Set up which moments we want
	QList<QListWidgetItem*> selectedItems = ui.momentList->selectedItems();
	int momentCount = selectedItems.length();
	Vector<Int> whichMoments(momentCount);
	if ( momentCount == 0 ){
		QString msg = "Please select at least one moment.";
		Util::showUserMessage( msg, this );
	}
	else {
		for( int i = 0; i < momentCount; i++ ){
			QString selectedText = selectedItems[i]->text();
			int index = momentOptions.indexOf( selectedText );
			if ( index >= 0 ){
				int momentIndex = static_cast<int>(momentMap[static_cast<SummationIndex>(index)]);
				whichMoments[i] = momentIndex;
			}
		}
	}
	return whichMoments;
}



void MomentSettingsWidgetRadio::collapseImage(){

	// Get the spectral axis number.
	// TODO: Generalize this to any hidden axis
	const ImageInterface<float>* image = taskMonitor->getImage();
	CoordinateSystem cSys = image -> coordinates();
	int spectralAxisNumber = cSys.spectralAxisNumber();

	Vector<String> method;

	//Note default SNRPEAK is 3.  Must be nonnegative.
	Double peaksnr = 3;
	//Note default stddev is 0. Must be nonnegative.
	Double stddev = 0;

	//Initialize the include/exclude pixels
	Vector<Float> excludepix;
	Vector<Float> includepix;
	bool validThresholds = populateThresholds( includepix, excludepix );
	if ( !validThresholds ){
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
	String stokesStr;
	CasacRegionManager crm( cSys );
	String diagnostics;
	String pixelBox="";
	Record region = crm.fromBCS( diagnostics, nSelectedChannels, stokesStr,
		NULL, regionName, channelStr, CasacRegionManager::USE_ALL_STOKES,
		pixelBox, pos, infile);

	//Calculate the moments
	if ( imageAnalysis == NULL ){
		imageAnalysis = new ImageAnalysis( image );
	}

	delete collapseThread;
	collapseThread = new MomentCollapseThreadRadio( imageAnalysis );
	connect( collapseThread, SIGNAL( finished() ), this, SLOT(collapseDone()));

	//Do a collapse image for each of the moments.
	Vector<Int> moments = populateMoments();
	Vector<QString> momentNames( moments.size());
	for ( int i = 0; i < static_cast<int>(moments.size()); i++ ){
		int sIndex = momentMap.key( i );
		momentNames[i] = momentOptions[sIndex];
	}
	String baseName( taskMonitor->getFileName().toStdString());
	collapseThread->setData(moments, spectralAxisNumber, region,
			    	"", method, smoothaxes, smoothtypes, smoothwidths,
			        includepix, excludepix, peaksnr, stddev,
			        "RADIO", baseName);
	collapseThread-> setMomentNames( momentNames );
	collapseThread->setChannelStr( channelStr );
	collapseThread->start();
}

void MomentSettingsWidgetRadio::collapseDone(){
	//Update the viewer with the collapsed image.
	if ( collapseThread != NULL && collapseThread->isSuccess()){
		std::vector<CollapseResult> results = collapseThread->getResults();
		for ( int i = 0; i < static_cast<int>(results.size()); i++ ){
			String outName = results[i].getOutputFileName();
			bool outputTemporary = results[i].isTemporaryOutput();
			ImageInterface<Float>* newImage = results[i].getImage();
			taskMonitor->imageCollapsed(outName, "image", "raster", True, outputTemporary, newImage );
		}
	}
	else {
		QString msg( "Moment calculation failed.");
		Util::showUserMessage( msg, this );
	}
}


void MomentSettingsWidgetRadio::setUnits( QString unitStr ){
	int bracketIndex = unitStr.indexOf( "[]");
	if ( bracketIndex > 0 ){
		unitStr = "";
	}
	ui.channelUnitLabel->setText( unitStr );
}

void MomentSettingsWidgetRadio::setTableValue(int row, int col, float val ){
	QTableWidgetItem* peakItem = new QTableWidgetItem();
	peakItem -> setText( QString::number( val ) );
	ui.channelTable->setItem( row, col, peakItem );
}

void MomentSettingsWidgetRadio::setRange( float min, float max ){
	if (max < min){
		ui.channelIntervalCountSpinBox->setValue( 0 );
		ui.channelTable->setRowCount( 0 );
	}
	else {
		QList<QTableWidgetSelectionRange> selectionRanges = ui.channelTable->selectedRanges();
		int selectionCount = selectionRanges.length();
		if ( selectionCount == 0 ){
			if ( ui.channelTable->isVisible() && ui.channelIntervalCountSpinBox->value() > 0 ){
				QString msg( "Please select a row in the channel table before specifying the estimate.");
				Util::showUserMessage( msg, this );
			}
		}
		else if ( selectionCount == 1 ){
			QTableWidgetSelectionRange selectionRange = selectionRanges[0];
			int tableRow = selectionRange.bottomRow();
			setTableValue( tableRow, 0, min );
			setTableValue( tableRow, 1, max );
		}
	}
}

void MomentSettingsWidgetRadio::reset(){
	if ( imageAnalysis != NULL ){
		delete imageAnalysis;
		imageAnalysis = NULL;
	}
	delete collapseThread;
	collapseThread = NULL;
	if ( taskMonitor != NULL ){
		ImageInterface<Float>* img = const_cast<ImageInterface <Float>* >(taskMonitor->getImage());
		imageAnalysis = new ImageAnalysis(img);
	}
}

void MomentSettingsWidgetRadio::thresholdingChanged( ){
	bool enabled = false;
	if ( ui.includeRadioButton->isChecked() || ui.excludeRadioButton->isChecked() ){
		enabled = true;
	}

	ui.maxThresholdLineEdit->setEnabled( enabled );
	ui.symmetricIntervalCheckBox->setEnabled( enabled );
	if ( !ui.symmetricIntervalCheckBox->isChecked() ){
		ui.minThresholdLineEdit->setEnabled( enabled );
	}
	if ( !enabled ){
		ui.minThresholdLineEdit->clear();
		ui.maxThresholdLineEdit->clear();
	}
	//Until we get to qwt6
	ui.graphThresholdButton->setEnabled( false );
}


void MomentSettingsWidgetRadio::adjustTableRows( int count ){
	ui.channelTable -> setRowCount( count );

}

void MomentSettingsWidgetRadio::setCollapsedImageFile(){
	string homedir = getenv("HOME");
	QFileDialog fd( this, tr("Specify a root file for the collapsed image(s)."),
			QString(homedir.c_str()), "");
	fd.setFileMode( QFileDialog::AnyFile );
	if ( fd.exec() ){
		QStringList fileNames = fd.selectedFiles();
		if ( fileNames.size() > 0 ){
			outputFileName = fileNames[0];
		}
	}

}

void MomentSettingsWidgetRadio::symmetricThresholdChanged( int checkedState ){
	if ( checkedState == Qt::Checked ){
		ui.minThresholdLineEdit->setEnabled( false );
		//Copy the next from the max to the min.
		QString maxText = ui.maxThresholdLineEdit->text();
		thresholdTextChanged( maxText );
	}
	else {
		ui.minThresholdLineEdit->setEnabled( true );
	}
}

void MomentSettingsWidgetRadio::thresholdTextChanged( const QString& text ){
	if ( ui.symmetricIntervalCheckBox->isChecked() ){
		Bool validDouble = false;
		if ( !text.isEmpty() ){
			double thresholdValue = text.toDouble( &validDouble );
			if ( validDouble ){
				thresholdValue = -1 * thresholdValue;
				QString oppositeText = QString::number( thresholdValue );
				ui.minThresholdLineEdit->setText( oppositeText );
			}
			else {
				//Shouldn't get here, but just in case let the user know there is
				//a problem.
				QString msg( "Please specify a valid number for the threshold.");
				Util::showUserMessage( msg, this );
			}
		}
	}
}

void MomentSettingsWidgetRadio::graphicalThreshold(){
	if ( thresholdingBinDialog == NULL ){
		thresholdingBinDialog = new ThresholdingBinPlotDialog( this );
	}
	ImageInterface<Float>* image = const_cast<ImageInterface<Float>* >(taskMonitor->getImage());
	thresholdingBinDialog->setImage( image );

	thresholdingBinDialog->show();
}

MomentSettingsWidgetRadio::~MomentSettingsWidgetRadio(){
	if ( imageAnalysis != NULL ){
		delete imageAnalysis;
	}
}



}
