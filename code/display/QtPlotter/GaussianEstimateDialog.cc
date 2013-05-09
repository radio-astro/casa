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

#include "GaussianEstimateDialog.qo.h"
#include <display/QtPlotter/GaussianEstimateWidget.qo.h>
#include <display/QtPlotter/conversion/Converter.h>
#include <display/QtPlotter/conversion/ConverterChannel.h>
#include <QtCore/qmath.h>
#include <QScrollArea>
#include <QDebug>

namespace casa {
	GaussianEstimateDialog::GaussianEstimateDialog(QWidget *parent)
		: QDialog(parent),/* searchDialog( this ),*/ plotsCoordinated( false )
  /* selectEstimateDialog( this ),*/ { /* clearPlotDialog( this )*/
		ui.setupUi(this);
		this->setWindowTitle( "Initial Gaussian Estimates");

		plotHolderWidget = new QWidget( this );
		ui.scrollArea -> setWidget( plotHolderWidget );

		//Initialize the units
		QStringList supportedUnits (QStringList() << "Hz" << "MHz" << "GHz" <<
		                            "Angstrom" << "nm" << "um" << "mm");
		for ( int i = 0; i < supportedUnits.size(); i++ ) {
			ui.axisUnitsComboBox->addItem( supportedUnits[i]);
		}
		ui.axisUnitsComboBox->setCurrentIndex( 2 );
		unitStr = ui.axisUnitsComboBox->currentText();
		connect( ui.axisUnitsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(unitsChanged(int)));

		//Signals/slots
		//connect( ui.searchButton, SIGNAL(clicked()), this, SLOT(searchMolecules()));
		connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		//connect( ui.clearButton, SIGNAL(clicked()), this, SLOT(clearGraphs()));
		connect( ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
		//connect(&searchDialog, SIGNAL(moleculesSelected()), this, SLOT(molecularLinesChanged()));
		connect( ui.coordinateCheckBox, SIGNAL(stateChanged(int)), this, SLOT(plotsCoordinatedChanged(int)));
		//connect(&selectEstimateDialog, SIGNAL(accepted()), this, SLOT(updateMolecularLines()));
		//connect(&clearPlotDialog, SIGNAL(accepted()), this, SLOT(clearSelectedPlots()));

		//Searching was moved to main screen
		//ui.searchButton->setVisible( false );
		//ui.clearButton->setVisible( false );
	}

//--------------------------------------------------------------------------------
//               Adjusting the number of displayed plots
//--------------------------------------------------------------------------------

	void GaussianEstimateDialog::clearPlots( int removeCount ) {
		QVBoxLayout* plotLayout =
		    dynamic_cast<QVBoxLayout*>(plotHolderWidget->layout());
		if ( plotLayout != NULL ) {
			for ( int i = 0; i < plots.size(); i++ ) {
				plotLayout->removeWidget( plots[i]);
			}
			delete plotLayout;
			plotLayout = NULL;
			int index = 0;
			while ( index < removeCount ) {
				GaussianEstimateWidget* plot = plots.takeLast();
				delete plot;
				index++;
			}
		}
	}

	void GaussianEstimateDialog::setGaussCount( int count ) {

		//Clear out the old plots
		int removeCount = 0;
		if ( count < plots.size() ) {
			removeCount = plots.size() - count;
		}
		clearPlots(removeCount);

		//Readjust the layout
		QVBoxLayout* plotLayout = new QVBoxLayout();
		plotLayout->setSpacing( 15 );

		//Add the plots to the layout, using existing plots if there
		//are some and otherwise making new ones.
		for ( int i = 0; i < count; i++ ) {

			GaussianEstimateWidget* plot = NULL;
			if ( i >= plots.size() ) {
				plot = new GaussianEstimateWidget( this );
				connect( plot, SIGNAL(coordinatedValuesChanged(float)), this, SLOT( coordinatedValuesChanged(float)));
				plots.append( plot );
			} else {
				plot = plots[i];
			}
			plots[i]->setTitle( "Estimate "+QString::number(i+1));
			plotLayout->addWidget( plot );
		}
		plotHolderWidget->setLayout( plotLayout );

		//Initialize the plots
		setCurveData();
		setCurveColor();
		setDisplayYUnits();
		resetEstimates();
	}

//----------------------------------------------------------------------------------
//                          Initialization
//----------------------------------------------------------------------------------

	void GaussianEstimateDialog::setEstimates( QList<SpecFitGaussian>& estimates ) {
		QString currentUnits = ui.axisUnitsComboBox->currentText();
		bool newUnits = false;
		if ( currentUnits != specUnitStr ) {
			newUnits = true;
		}
		for ( int i = 0; i < plots.size(); i++ ) {
			plots[i]->setEstimate( estimates[i]);
			if ( newUnits ) {
				plots[i]->unitsChanged(specUnitStr, currentUnits);
			}

		}
	}

	void GaussianEstimateDialog::resetEstimates() {
		for ( int i = 0; i < plots.size(); i++ ) {
			plots[i]->updateUIBasedOnEstimate();
		}
	}

	void GaussianEstimateDialog::setSpecFitUnits( const QString& specUnits ) {
		this->specUnitStr = specUnits;
	}



	void GaussianEstimateDialog::setCurveData( const Vector<float>& xValues,
	        const Vector<float>& yValues) {

		Vector<float> translatedXValues( xValues.size() );
		translatedXValues = xValues;

		//We got pixels.  Translate to world units first.
		/*if ( specUnitStr.length() == 0 ){
			Converter* pixelConverter = new ConverterChannel( &spectralCoordinate );
			translatedXValues = translateDataUnits( xValues, pixelConverter );
			specUnitStr = pixelConverter->getNewUnits();
			delete pixelConverter;
		}*/


		QString newUnits = ui.axisUnitsComboBox->currentText();
		if ( newUnits != specUnitStr ) {
			Converter* converter = Converter::getConverter( specUnitStr, newUnits );
			xVals = translateDataUnits(translatedXValues, converter );
			delete converter;
		} else {
			xVals = translatedXValues;
		}

		yVals = yValues;
		setCurveData();
	}

	void GaussianEstimateDialog::setCurveData() {

		Float xMin = 0;
		Float xMax = 0;
		Float yMin = 0;
		Float yMax = 0;

		initializeLimits( xVals, &xMin, &xMax );
		initializeLimits( yVals, &yMin, &yMax );

		for ( int i = 0; i < plots.size(); i++ ) {
			plots[i]->setCurveData( xVals, yVals );
			plots[i]->setRangeX( xMin, xMax );
			plots[i]->setRangeY( yMin, yMax );
		}

		//searchDialog.setRange( xMin, xMax, ui.axisUnitsComboBox->currentText());
	}

	void GaussianEstimateDialog::setCurveColor( QColor color ) {
		curveColor = color;
		setCurveColor();
	}

	void GaussianEstimateDialog::setCurveColor() {
		for ( int i = 0; i < plots.size(); i++ ) {
			plots[i]->setCurveColor( curveColor );
		}
	}

	void GaussianEstimateDialog::setDisplayYUnits( const QString& unitStr ) {
		displayYUnits = unitStr;
		setDisplayYUnits();
	}

	QString GaussianEstimateDialog::getDisplayYUnits() const {
		return displayYUnits;
	}

	void GaussianEstimateDialog::setDisplayYUnits() {
		for ( int i = 0; i < plots.size(); i++ ) {
			plots[i]->setDisplayYUnits( displayYUnits );
		}
	}

//------------------------------------------------------------------------
//                Accessors
//------------------------------------------------------------------------

	SpecFitGaussian GaussianEstimateDialog::getEstimate( int index ) {
		return plots[index]->getEstimate();
	}

	QString GaussianEstimateDialog::getUnits() const {
		QString unitStr = ui.axisUnitsComboBox->currentText();
		return unitStr;
	}

//------------------------------------------------------------------------
//                         Spectral Line Search
//------------------------------------------------------------------------

	/*void GaussianEstimateDialog::searchMolecules(){
		if ( ! searchDialog.isVisible() ){
			searchDialog.updateReferenceFrame();
			searchDialog.show();
		}
	}*/

	/*void GaussianEstimateDialog::molecularLinesChanged(){
		if ( plots.size() > 1 ){
			selectEstimateDialog.setEstimateCount(plots.size());
			selectEstimateDialog.show();
		}
		else {
			updateMolecularLines();
		}
	}*/

	/*void GaussianEstimateDialog::updateMolecularLines( ){
		QList<int> estimates;
		if ( plots.size() > 1 ){
			estimates = selectEstimateDialog.getSelectedEstimates();
		}
		else {
			estimates << 0;
		}
		QList<int> lineIndices = searchDialog.getLineIndices();
		QString searchUnits = searchDialog.getUnit();
		int count = qMin( lineIndices.size(), plots.size() );
		Converter* converter = NULL;
		if ( searchUnits != unitStr ){
			converter = Converter::getConverter( searchUnits, unitStr );
		}
		for ( int i = 0; i < count; i++ ){
			Float center;
			Float peak;
			QString molecularName;
			QString chemicalName;
			QString resolvedQNs;
			QString frequencyUnits;
			searchDialog.getLine(lineIndices[i], peak, center, molecularName, chemicalName,
					resolvedQNs, frequencyUnits );

			//First we have to make the frequency value redshifted.
			double shiftedCenter = searchDialog.getRedShiftedValue( false, center );

			//Convert it to the same units we are using
			if ( converter != NULL ){
				shiftedCenter = converter->convert( shiftedCenter );
			}
			for( int j = 0; j < static_cast<int>(estimates.size()); j++ ){
				plots[estimates[j]]->molecularLineChanged( peak, shiftedCenter,
						molecularName, chemicalName, resolvedQNs, frequencyUnits );
			}
		}
		delete converter;
	}*/

//-------------------------------------------------------------------
//                     Slots
//-------------------------------------------------------------------

	void GaussianEstimateDialog::plotsCoordinatedChanged( int state ) {
		if ( state == Qt::Checked ) {
			plotsCoordinated = true;
		} else {
			plotsCoordinated = false;
		}
	}

	void GaussianEstimateDialog::coordinatedValuesChanged( float fwhmVal ) {
		if ( plotsCoordinated ) {
			for ( int i = 0; i < plots.size(); i++ ) {
				plots[i]->setSliderValueFWHM( fwhmVal );
			}
		}
	}


	void GaussianEstimateDialog::unitsChanged( int /*index*/ ) {
		QString newUnitStr = ui.axisUnitsComboBox->currentText();
		if ( unitStr != newUnitStr ) {
			Converter* converter = Converter::getConverter(unitStr, newUnitStr);
			//Reset the curve data
			this->xVals = this->translateDataUnits( xVals, converter );
			setCurveData();

			//Reset the gaussian estimates
			for ( int i = 0; i < plots.size(); i++ ) {
				plots[i] -> unitsChanged( unitStr, newUnitStr );
			}
			delete converter;
			//Store the units we are now using.
			unitStr = newUnitStr;
		}
	}


	/*void GaussianEstimateDialog::clearGraphs() {
		if ( plots.size() > 1 ){
			clearPlotDialog.setEstimateCount( plots.size() );
			clearPlotDialog.show();
		}
		else {
			clearSelectedPlots();
		}
	}*/

	/*void GaussianEstimateDialog::clearSelectedPlots(){
		QList<int> estimates;
		if ( plots.size() > 1 ){
				estimates = selectEstimateDialog.getSelectedEstimates();
		}
		else {
			estimates << 0;
		}
		for( int j = 0; j < static_cast<int>(estimates.size()); j++ ){
			plots[estimates[j]]->clearMolecularLines();
		}
	}*/

//--------------------------------------------------------------------------------------
//                     Utility Methods
//--------------------------------------------------------------------------------------

	Vector<float> GaussianEstimateDialog::translateDataUnits( const Vector<float>& xValues, Converter* converter ) {
		Vector<float> convertedXVals( xValues.size() );
		if ( converter != NULL ) {
			Vector<double> xVals( xValues.size());
			for ( int i = 0; i < static_cast<int>(xValues.size()); i++ ) {
				xVals[i] = xValues[i];
			}
			Vector<double> convertedXValues = converter-> convert( xVals );
			int count = convertedXValues.size();

			for ( int i = 0; i < count; i++ ) {
				convertedXVals[i] = convertedXValues[i];
			}
		}

		return convertedXVals;
	}

	void GaussianEstimateDialog::initializeLimits( const Vector<float>& values, Float* const min, Float* const max ) {
		int count = static_cast<int>(values.size());
		if ( count > 0 ) {
			*min = values[0];
			*max = values[0];
			for ( int i = 0; i < count; i++ ) {
				if ( *min > values[i] ) {
					*min = values[i];
				}
				if ( *max < values[i] ) {
					*max = values[i];
				}
			}
		}
	}


	GaussianEstimateDialog::~GaussianEstimateDialog() {

	}
}
