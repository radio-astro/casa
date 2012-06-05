#include "SpecFitSettingsWidgetRadio.qo.h"
#include <imageanalysis/ImageAnalysis/ImageProfileFitter.h>
#include <images/Images/ImageFit1D.h>
#include <components/SpectralComponents/GaussianSpectralElement.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/SpecFitLogDialog.qo.h>
#include <display/QtPlotter/SpecFitSettingsFixedTableCell.qo.h>
#include <display/QtPlotter/Util.h>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QList>
#include <QDebug>
namespace casa {

SpecFitSettingsWidgetRadio::SpecFitSettingsWidgetRadio(QWidget *parent)
    : QWidget(parent), fitter( NULL )
{
	ui.setupUi(this);

	//Until we get the code written
	ui.loadButton->setEnabled( false );

	//Text fields
	QValidator *validator = new QDoubleValidator(-1.0e+32, 1.0e+32,10,this);
	ui.minLineEdit->setValidator(validator);
	ui.maxLineEdit->setValidator(validator);

	ui.estimateTable->setColumnCount( END_COLUMN );
	QStringList tableHeaders =(QStringList()<< "  Peak  " << " Center " << "  FWHM  " << "          Fixed          ");
	ui.estimateTable->setHorizontalHeaderLabels( tableHeaders );
	ui.estimateTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.estimateTable->setSelectionMode( QAbstractItemView::SingleSelection );

	int state = ui.polyFitCheckBox->checkState();
	polyFitChanged( state );
	int saveOutput = ui.saveOutputCheckBox->checkState();
	saveOutputChanged( saveOutput );

	connect( ui.polyFitCheckBox, SIGNAL(stateChanged(int)), this, SLOT(polyFitChanged(int)) );
	connect( ui.gaussCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(adjustTableRowCount(int)));
	connect( ui.fitButton, SIGNAL(clicked()), this, SLOT(specLineFit()));
	connect( ui.cleanButton, SIGNAL(clicked()), this, SLOT(clean()));
	connect( ui.saveButton, SIGNAL(clicked()), this, SLOT(setOutputLogFile()));
	connect( ui.viewButton, SIGNAL(clicked()), this, SLOT(viewOutputLogFile()));
	connect( ui.saveOutputCheckBox, SIGNAL(stateChanged(int)), this, SLOT(saveOutputChanged(int)));
}

void SpecFitSettingsWidgetRadio::setCanvas( QtCanvas* canvas ){
	ProfileTaskFacilitator::setCanvas( canvas );
	connect( pixelCanvas, SIGNAL(specFitEstimateSpecified(double,double,bool)),
			this, SLOT(specFitEstimateSpecified(double,double,bool)));
}

void SpecFitSettingsWidgetRadio::setEstimateValue( int row, int col, double val ){
	QTableWidgetItem* peakItem = new QTableWidgetItem();
	peakItem -> setText( QString::number( val ) );
	ui.estimateTable->setItem( row, col, peakItem );
}


void SpecFitSettingsWidgetRadio::specFitEstimateSpecified(double xValue,
		double yValue, bool centerPeak ){

	QList<QTableWidgetSelectionRange> selectionRanges = ui.estimateTable->selectedRanges();
	int selectionCount = selectionRanges.length();
	if ( selectionCount == 0 ){
		if ( ui.estimateTable->isVisible()){
			QString msg( "Please select a row in the initial Gaussian estimates table before specifying the estimate.");
			Util::showUserMessage( msg, this );
		}
	}
	else if ( selectionCount == 1 ){
		QTableWidgetSelectionRange selectionRange = selectionRanges[0];
		//We should have a single row selected
		int rowCount = selectionRange.rowCount();
		if ( rowCount == 1 ){
			int row = selectionRange.bottomRow();
			if ( centerPeak ){
				setEstimateValue( row, PEAK, yValue );
				setEstimateValue( row, CENTER, xValue );
				pixelCanvas->setProfileFitMarkerCenterPeak( row, xValue, yValue );
			}
			else {
				//Take the distance between the center and xValue, then
				//double it to get the fwhm.
				QTableWidgetItem* centerItem = ui.estimateTable->item( row, CENTER );
				QString centerStr = centerItem->text();
				bool validNum;
				double val = centerStr.toDouble( &validNum );
				if ( validNum ){
					double fwhm = 2 * abs(val - xValue);
					setEstimateValue( row, FWHM, fwhm);
					pixelCanvas -> setProfileFitMarkerFWHM( row, fwhm, yValue );
				}
			}
			//Update the profile fit marker in this row.
		}

	}

}

void SpecFitSettingsWidgetRadio::reset(){
	if ( fitter != NULL ){
		delete fitter;
		fitter = NULL;
	}
}

void SpecFitSettingsWidgetRadio::clean(){
	plotMainCurve();
}


void SpecFitSettingsWidgetRadio::setUnits( QString units ){
	ui.rangeGroupBox->setTitle( units );
}

void SpecFitSettingsWidgetRadio::adjustTableRowCount( int count ){
	int oldRowCount = ui.estimateTable->rowCount();
	ui.estimateTable -> setRowCount( count );
	//We are adding rows.
	if ( oldRowCount < count ){
		for ( int i = oldRowCount; i < count; i++ ){
			SpecFitSettingsFixedTableCell* cellWidget = new SpecFitSettingsFixedTableCell();
			ui.estimateTable->setCellWidget( i, FIXED, cellWidget );
			ui.estimateTable->resizeRowToContents( i );

		}
	}
	ui.estimateTable->resizeColumnToContents( FIXED );
}

void SpecFitSettingsWidgetRadio::setRange(float xmin, float xmax ){
	if (xmax < xmin){
		ui.minLineEdit->clear();
		ui.maxLineEdit->clear();
	}
	else {
		QString startStr;
		QString endStr;
		startStr.setNum(xmin);
		endStr.setNum(xmax);
		ui.minLineEdit->setText(startStr);
		ui.maxLineEdit->setText(endStr);
	}
}

void SpecFitSettingsWidgetRadio::polyFitChanged( int state ){
	if ( state == Qt::Checked ){
		ui.polyOrderSpinBox -> setEnabled( true );
	}
	else {
		ui.polyOrderSpinBox -> setEnabled( false );
	}
}

void SpecFitSettingsWidgetRadio::saveOutputChanged( int state ){
	bool saveOutput = false;
	if ( state == Qt::Checked ){
		saveOutput = true;
	}
	else {
		outputLogPath="";
		ui.viewButton->setEnabled( false );
	}
	ui.saveButton->setEnabled( saveOutput );
}

bool SpecFitSettingsWidgetRadio::isValidFitSpecification( int gaussCount, bool polyFit ){
	bool valid = true;
	if (gaussCount <= 0 && !polyFit){
		String msg("Please indicate the type of fit:  Gaussian and/or Polynomial.");
		logWarning( msg );
		postStatus( msg );
		valid = false;
	}
	return valid;
}




bool SpecFitSettingsWidgetRadio::isValidEstimate( QString& peakStr,
		QString& centerStr, QString& fwhmStr,
		QString& fixedStr, int rowIndex ) {
	bool validEstimate = true;
	QDoubleValidator validator;
	int pos = 0;
	QString msg( "Row "+QString::number(rowIndex)+" of the initial estimate table is invalid.\n");
	if ( ! validator.validate(peakStr, pos) == QValidator::Acceptable ){
		validEstimate = false;
		msg.append( "Please check that the following numbers are valid:\n");
		msg.append( "  peak");
	}
	if ( !validator.validate(centerStr, pos) == QValidator::Acceptable ){
		validEstimate = false;
		msg.append( "\n  center");
	}
	if ( !validator.validate( fwhmStr, pos) == QValidator::Acceptable ){
		validEstimate = false;
		msg.append( "\n  FWHM");
	}
	//Check that the fixed string contains
	//zero or one instance of the characters "pcf" in any order.
	QRegExp fixedRe("p?c?f?|p?f?c?|c?p?f?|c?f?p?|f?c?p?|f?p?c?");
	QRegExpValidator validatorFixed(fixedRe, 0 );
	if ( !validatorFixed.validate(fixedStr, pos)  == QValidator::Acceptable ){
		validEstimate = false;
		msg.append( "\nPlease check that the fixed string contains only the characters 'p', 'c', and 'f' at most once in any order.");
	}
	if ( !validEstimate ){
		Util::showUserMessage( msg, this );
	}

	return validEstimate;
}

SpectralList SpecFitSettingsWidgetRadio::buildSpectralList( int nGauss, Bool& validEstimates ){
	SpectralList spectralList;
	validEstimates = true;
	if ( nGauss > 0 ){
		int potentialEstimateCount = ui.estimateTable->rowCount();
		if ( nGauss == potentialEstimateCount ){
			for ( int i = 0; i < potentialEstimateCount; i++  ){
				QTableWidgetItem* peakItem = ui.estimateTable->itemAt(i, PEAK );
				QString peakStr;
				if ( peakItem != NULL ){
					peakStr = peakItem->text();
				}
				QTableWidgetItem* centerItem = ui.estimateTable->itemAt( i, CENTER );
				QString centerStr;
				if ( centerItem != NULL ){
					centerStr = centerItem->text();
				}
				QTableWidgetItem* fwhmItem = ui.estimateTable->itemAt( i, FWHM );
				QString fwhmStr;
				if ( fwhmItem != NULL ){
					fwhmStr = ui.estimateTable->itemAt(i, FWHM )->text();
				}
				SpecFitSettingsFixedTableCell* fixedItem = dynamic_cast<SpecFitSettingsFixedTableCell*>(ui.estimateTable->cellWidget(i,FIXED));
				QString fixedStr;
				if ( fixedItem != NULL ){
					fixedStr = fixedItem->getFixedStr();
				}
				if ( isValidEstimate( peakStr, centerStr, fwhmStr, fixedStr, i+1 ) ){
					if ( !peakStr.isEmpty() && !centerStr.isEmpty() && !fwhmStr.isEmpty()){
						double peakVal = peakStr.toDouble();
						double centerVal = centerStr.toDouble();
						double fwhmVal = fwhmStr.toDouble();
						GaussianSpectralElement* estimate = new GaussianSpectralElement( peakVal, centerVal, fwhmVal);
						estimate->fixByString( fixedStr.toStdString());
						spectralList.add( *estimate );
					}

				}
				else {
					validEstimates = false;
					break;
				}
			}
		}

	}
	return spectralList;
}

void SpecFitSettingsWidgetRadio::doFit( float startVal, float endVal, uint nGauss, bool fitPoly, int polyN ){
	Vector<Float> z_xval = getXValues();
	Vector<Float> z_yval = getYValues();
	Vector<Float> z_eval = getZValues();
	reset();
	const ImageInterface<float>* image = getImage();
	const String pixelBox = getPixelBox();

	Bool validSpectralList;
	SpectralList spectralList = buildSpectralList( nGauss, validSpectralList );
	if ( !validSpectralList ){
		return;
	}

	int startChannelIndex = -1;
	int endChannelIndex = -1;
	findChannelRange( startVal, endVal, z_xval, startChannelIndex, endChannelIndex );
	if ( startChannelIndex >= 0 && endChannelIndex >= 0 ){
		const String channelStr = String::toString( startChannelIndex )+ "~"+String::toString( endChannelIndex);
		fitter = new ImageProfileFitter( image, "", 0, pixelBox,
				channelStr, "", "", 0, static_cast<uInt>(nGauss), "", spectralList);
		if ( fitPoly ){
			fitter->setPolyOrder( polyN );
		}
		if ( !outputLogPath.isEmpty() ){
			fitter->setLogfile( outputLogPath.toStdString() );
		}
		if ( ui.multiFitCheckBox->isChecked() ){
			fitter->setDoMultiFit( true );
		}

		Record results = fitter->fit();
		Vector<Bool> succeeded = results.asArrayBool(ImageProfileFitter::_SUCCEEDED );
		if ( ! succeeded.size() == 1 || ! succeeded[0]){
			String msg = String("Data could not be fitted!");
			QString msgStr(msg.c_str());
			postStatus(msg);
			Util::showUserMessage( msgStr, this);
		}
		else{
			String xaxisUnit = getXAxisUnit();
			QString yUnit = getYUnit();
			QString yUnitPrefix = getYUnitPrefix();
			Vector<Bool> converged = results.asArrayBool( ImageProfileFitter::_CONVERGED );
			if ( converged.size() == 1 && converged[0]){
				Vector<Int> iterationCounts = results.asArrayInt(ImageProfileFitter::_ITERATION_COUNT);
				String msg( "Fit converged in "+String::toString(iterationCounts[0])+" iterations.");
				postStatus( msg );
			}
			else {
				//Post message saying that we didn't get convergence.
				QString msg("Profile fit did not converge.");
				Util::showUserMessage( msg, this);
			}
		}
	}
}


void SpecFitSettingsWidgetRadio::specLineFit(){
	*logger << LogOrigin("SpecFitOptical", "specLineFit");

	if ( isValidChannelRangeValue( ui.minLineEdit->text(), "Start" ) &&
			isValidChannelRangeValue( ui.maxLineEdit->text(), "End" )){
		// convert input values to Float
		float startVal=ui.minLineEdit->text().toFloat();
		float endVal  =ui.maxLineEdit->text().toFloat();
		if ( endVal < startVal ){
			//Switch them around - the code expects the startVal
			//to be less than the endVal;
			float tempVal = startVal;
			startVal = endVal;
			endVal = tempVal;
		}

		//Determine what combination of Gauss & Polynomial fits we
		//are doing.
		int gaussCount = ui.gaussCountSpinBox->value();
		bool polyFit = false;
		if ( ui.polyFitCheckBox->isChecked() ){
			polyFit = true;
		}
		// make sure something should be fitted at all
		bool valid = isValidFitSpecification( gaussCount, polyFit);
		if ( valid ){
			int polyN = 0;
			if ( polyFit ){
				polyN = ui.polyOrderSpinBox->value();
			}
			doFit( startVal, endVal, gaussCount, polyFit, polyN );
		}
	}
}

void SpecFitSettingsWidgetRadio::setOutputLogFile(){
	string homedir = getenv("HOME");
	QFileDialog fd( this, tr("Specify File for Output Logging"),
			QString(homedir.c_str()), "");
	fd.setFileMode( QFileDialog::AnyFile );
	if ( fd.exec() ){
		QStringList fileNames = fd.selectedFiles();
		if ( fileNames.size() > 0 ){
			outputLogPath = fileNames[0];
			bool validPath = !outputLogPath.isEmpty();
			ui.viewButton->setEnabled( validPath );
		}
	}

}

void SpecFitSettingsWidgetRadio::viewOutputLogFile(){

	if ( !outputLogPath.isEmpty()){
		QFile outputLogFile(outputLogPath);
		if ( !outputLogFile.exists() ){
			QString msg("The output file did not exist.\n  Please check it has been specified correctly.");
			Util::showUserMessage ( msg, this);
		}
		else {
			if ( !outputLogFile.open( QIODevice::ReadOnly )){
				QString msg("Could not open output file for reading: "+outputLogPath+"\n  Please check it has proper permissions.");
				Util::showUserMessage( msg, this);
			}
			else {
				QTextStream ts( &outputLogFile );
				QString logResults;
				while ( ! ts.atEnd() ){
					QString line = ts.readLine();
					line.append( "\n");
					logResults.append( line );
				}
				SpecFitLogDialog* dialog = new SpecFitLogDialog( this );
				dialog -> setLogResults( logResults );
				dialog -> exec();
			}
		}
	}
	else {
		QString msg("Please specify a file for output logging.");
		Util::showUserMessage( msg, this);
	}
}

SpecFitSettingsWidgetRadio::~SpecFitSettingsWidgetRadio()
{
	delete fitter;
}
}
