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
#include "SearchMoleculesWidget.qo.h"


#include <display/QtPlotter/Util.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/Display/Options.h>
#include <spectrallines/Splatalogue/SplatalogueTable.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDoppler.h>
#include <casa/Quanta/MVDoppler.h>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QTimer>
#include <QDebug>
#include <assert.h>
#include <iostream>
using namespace std;

namespace casa {

	const QString SearchMoleculesWidget::SPLATALOGUE_UNITS="MHz";
	const double SearchMoleculesWidget::SPLATALOGUE_DEFAULT_MIN = -1;
	const double SearchMoleculesWidget::SPLATALOGUE_DEFAULT_MAX = -1;

	QString SearchMoleculesWidget::initialReferenceStr = "LSRK";

	void SearchMoleculesWidget::setInitialReferenceFrame( QString frameStr ) {
		initialReferenceStr = frameStr;
	}


	SearchMoleculesWidget::SearchMoleculesWidget(QWidget *parent)
		: QWidget(parent), unitStr( SPLATALOGUE_UNITS ),
		  searchThread( NULL ), searcher( NULL ), canvas(NULL),
		  progressBar( this ), searchResultCount(0),
		  searchResultOffset(0), searchResultLimit(500),
		  resultDisplay( NULL ) {
		ui.setupUi(this);

		//Progress Bar
		//We need the user to be able to cancel the thread through a provided
		//cancel button, but not be able to close a parent window while the
		//search thread is working causing the windowing system to take over
		//and making the program terminate.
		progressBar.setWindowTitle( "Line Search");
		progressBar.setLabelText( "Searching for molecular lines ...");
		progressBar.setWindowModality( Qt::WindowModal );

		Qt::WindowFlags flags = Qt::Dialog;
		flags |= Qt::FramelessWindowHint;
		progressBar.setWindowFlags( flags);
		progressBar.setMinimum(0);
		progressBar.setMaximum(0);
		connect( &progressBar, SIGNAL(canceled()), this, SLOT(stopSearch()));

		//Range initialization
		QList<QString> frequencyUnitList;
		frequencyUnitList << "Hz" << "KHz" << "MHz" << "GHz" << "Angstrom" << "nm"
		                  << "um" << "mm" << "cm" << "m";
		for ( int i = 0; i < frequencyUnitList.size(); i++ ) {
			ui.rangeUnitComboBox->addItem( frequencyUnitList[i]);
		}
		ui.rangeUnitComboBox->setCurrentIndex(frequencyUnitList.indexOf(SPLATALOGUE_UNITS));
		this->unitStr = SPLATALOGUE_UNITS;
		connect( ui.rangeUnitComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT( searchUnitsChanged(const QString&)));

		//Astronomical Filters
		QList<QString> astronomicalFiltersList;
		astronomicalFiltersList<<"None"<<"Top 20 list"<<"Planetary Atmosphere"<<"Hot Cores"
		                       <<"Dark Clouds"<<"Diffuse Clouds"<<"Comets"<<"AGB/PPN/PN"<<"Extragalactic";
		for ( int i = 0; i < astronomicalFiltersList.size(); i++ ) {
			ui.astronomicalFilterComboBox->addItem( astronomicalFiltersList[i] );
		}

		//Validators for the LineEdits
		QDoubleValidator* validator = new QDoubleValidator( this );
		ui.rangeMinLineEdit->setValidator( validator );
		ui.rangeMaxLineEdit->setValidator( validator );
		ui.dopplerLineEdit->setValidator( validator );
		ui.dopplerLineEdit->setText( QString::number(0) );

		radialVelocityTypeMap.insert("LSRK", MRadialVelocity::LSRK);
		radialVelocityTypeMap.insert("LSRD", MRadialVelocity::LSRD);
		radialVelocityTypeMap.insert("BARY", MRadialVelocity::BARY);
		radialVelocityTypeMap.insert("TOPO", MRadialVelocity::TOPO);
		QList<QString> keys = radialVelocityTypeMap.keys();
		for ( int i = 0; i < keys.size(); i++ ) {
			ui.referenceFrameCombo->addItem( keys[i] );
		}
		velocityUnitsList<<"km/s"<<"m/s";
		for ( int i = 0; i < velocityUnitsList.size(); i++ ) {
			ui.dopplerUnitsComboBox->addItem( velocityUnitsList[i] );
		}
		dopplerVelocityUnitStr = velocityUnitsList[0];
		dopplerTypeMap.insert( "Radio", MDoppler::RADIO);
		dopplerTypeMap.insert( "Optical", MDoppler::OPTICAL);
		dopplerTypeMap.insert( "Relativistic", MDoppler::RELATIVISTIC );
		int radioIndex = 0;
		QList<QString> dopplerKeys = dopplerTypeMap.keys();
		for ( int i = 0; i < dopplerKeys.size(); i++ ) {
			ui.dopplerTypeCombo->addItem( dopplerKeys[i] );
			if ( dopplerTypeMap[dopplerKeys[i]]  == MDoppler::RADIO ) {
				radioIndex = i;
			}
		}
		ui.dopplerTypeCombo->setCurrentIndex( radioIndex );

		//Trigger the ui to update based on the default redshift
		ui.redshiftRadio->setChecked( true );
		dopplerInVelocity = true;
		this->dopplerShiftChanged();

		//Name versus chemical search
		QButtonGroup* chemGroup = new QButtonGroup( this );
		chemGroup->addButton( ui.nameRadioButton );
		chemGroup->addButton( ui.formulaRadioButton );

		//Signal/Slot
		connect( ui.searchButton, SIGNAL(clicked()), this, SLOT(search()));
		connect( ui.velocityRadio, SIGNAL(clicked()), this, SLOT(dopplerShiftChanged()));
		connect( ui.redshiftRadio, SIGNAL(clicked()), this, SLOT(dopplerShiftChanged()));
		connect( ui.dopplerUnitsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(dopplerVelocityUnitsChanged()));

	}

//-------------------------------------------------------------------------------------
//                        Initialization
//-------------------------------------------------------------------------------------


	void SearchMoleculesWidget::setRange( double min, double max, QString units ) {

		double convertedMinValue = setRangeValue( min, units );
		double convertedMaxValue = setRangeValue( max, units );
		if ( convertedMinValue < convertedMaxValue ){
			ui.rangeMinLineEdit->setText( QString::number(convertedMinValue, 'f', 6));
			ui.rangeMaxLineEdit->setText( QString::number(convertedMaxValue, 'f', 6));
		}
		else {
			ui.rangeMinLineEdit->setText( QString::number(convertedMaxValue, 'f', 6));
			ui.rangeMaxLineEdit->setText( QString::number(convertedMinValue, 'f', 6));
		}
	}

	double SearchMoleculesWidget::setRangeValue( double value, QString units) {
		double convertedValue = value;
		if ( unitStr != units ) {
			Converter* converter = Converter::getConverter( units, unitStr );
			convertedValue = converter->convert( value, coord);
			delete converter;
		}
		return convertedValue;
	}

	void SearchMoleculesWidget::setSpectralCoordinate(SpectralCoordinate coord ){
		this->coord = coord;
	}

	void SearchMoleculesWidget::setResultDisplay( SearchMoleculesResultDisplayer* resultDisplay ) {
		this->resultDisplay = resultDisplay;
	}

	void SearchMoleculesWidget::setCanvas( QtCanvas* drawCanvas ) {
		canvas = drawCanvas;
		//setSearchRangeDefault();
	}

	void SearchMoleculesWidget::updateReferenceFrame() {
		QList<QString> keys = radialVelocityTypeMap.keys();
		for ( int i = 0; i < keys.size(); i++ ) {
			if ( keys[i]  == initialReferenceStr ) {
				ui.referenceFrameCombo->setCurrentIndex(i);
				break;
			}
		}
	}
//--------------------------------------------------------------------------------------
//                        Signal/Slot
//--------------------------------------------------------------------------------------

	void SearchMoleculesWidget::convertRangeLineEdit( QLineEdit* lineEdit, Converter* converter ) {
		QString editText = lineEdit->text();
		if ( editText != NULL && !editText.isEmpty()) {
			if ( converter != NULL ) {
				double val = editText.toDouble();
				val = converter->convert( val, coord);
				lineEdit->setText( QString::number( val ));
			}
		}
	}


	void SearchMoleculesWidget::searchUnitsChanged( const QString& newUnits ) {
		//If the units have changed
		if ( unitStr != newUnits ) {
			//Readjust the text fields
			Converter* converter = Converter::getConverter( unitStr, newUnits );
			convertRangeLineEdit(ui.rangeMinLineEdit, converter);
			convertRangeLineEdit(ui.rangeMaxLineEdit, converter);
			delete converter;
		}
		unitStr = newUnits;
	}

	void SearchMoleculesWidget::dopplerShiftChanged() {
		//Redshift is unitless so we hide the unit combo if the doppler shift
		//is specified with a redshift.  We also hide the reference frame.
		bool unitsVelocity = ui.velocityRadio->isChecked();
		ui.dopplerUnitsComboBox->setVisible( unitsVelocity );
		ui.referenceFrameLabel->setVisible( unitsVelocity );
		ui.referenceFrameCombo->setVisible( unitsVelocity );

		if ( unitsVelocity != dopplerInVelocity ) {
			//We need to convert a value in doppler text field to the new units.
			//Make sure there is a value to convert:
			QString valStr = ui.dopplerLineEdit->text();

			if ( valStr.length() > 0 ) {
				QString unitStr = ui.dopplerUnitsComboBox->currentText();
				String unitString = unitStr.toStdString();
				double val = valStr.toDouble();
				QString referenceStr = ui.referenceFrameCombo->currentText();
				MRadialVelocity::Types referenceType = radialVelocityTypeMap.value( referenceStr );
				QString dopplerTypeStr = ui.dopplerTypeCombo->currentText();
				MDoppler::Types dopplerType = dopplerTypeMap.value( dopplerTypeStr );
				Bool valid = true;
				if ( unitsVelocity ) {
					//From doppler to velocity
					MVDoppler mvDoppler( val );
					MDoppler doppler( mvDoppler, dopplerType );
					MRadialVelocity mVelocity = MRadialVelocity::fromDoppler(doppler, referenceType);
					Quantity velQuantity = mVelocity.get( unitString );
					val = velQuantity.getValue();
				} else {
					//From velocity to doppler
					MDoppler dop = MRadialVelocity( Quantity(val, unitString), referenceType).toDoppler();
					MDoppler doppler = MDoppler::Convert ( dop, dopplerType)();
					val = doppler.getValue ();
					if ( isnan( val )) {
						valid = false;
					}
				}
				if ( valid ) {
					ui.dopplerLineEdit->setText( QString::number( val ));
				} else {
					ui.dopplerLineEdit->setText( "" );
				}
			}
			dopplerInVelocity = unitsVelocity;
		}
	}

	void SearchMoleculesWidget::dopplerVelocityUnitsChanged() {
		QString currentUnits = ui.dopplerUnitsComboBox->currentText();
		if ( dopplerVelocityUnitStr != currentUnits ) {
			QString dopplerStr = ui.dopplerLineEdit->text();
			if (dopplerStr.length() > 0 ) {
				double dopplerVal = dopplerStr.toDouble();

				Converter* converter = Converter::getConverter( dopplerVelocityUnitStr, currentUnits );
				dopplerVal = converter->convert( dopplerVal, coord );
				ui.dopplerLineEdit->setText( QString::number( dopplerVal ));
				delete converter;
			}
			dopplerVelocityUnitStr = currentUnits;
		}
	}

//------------------------------------------------------------------------------------
//                 Performing the search and displaying the results
//-----------------------------------------------------------------------------------



	void SearchMoleculesWidget::initializeSearchRange( QLineEdit* lineEdit, Double& value/*, MDoppler redShift*/ ) {
		QString valueStr = lineEdit->text();
		if ( !valueStr.isEmpty() ) {
			value = valueStr.toDouble();
			//Convert the range value to Splatalogue Units
			if ( unitStr != SPLATALOGUE_UNITS ) {
				Converter* converter = Converter::getConverter(unitStr, SPLATALOGUE_UNITS);
				value = converter->convert( value, coord );
				delete converter;
			}

			//Factor in the specified redshift.
			value = getRedShiftedValue( true, value );
		}
	}

	QList<QString> SearchMoleculesWidget::getSearchChemicals(){
		//Get the names/chemical formulas to search for.
		QList<QString> moleculeList;
		QString searchList = ui.searchLineEdit->text();
		if ( ! searchList.isEmpty() ) {
			moleculeList = searchList.split(",");
		}
		return moleculeList;
	}

	vector<string> SearchMoleculesWidget::convertStringFormats( const QList<QString>& names ){
		//The search engine needs casa independent units
		int masterListCount = names.size();
		vector<string> chemNames( masterListCount );
		for ( int i = 0; i < masterListCount; i++ ) {
			chemNames[i] = names[i].trimmed().toStdString();
		}
		return chemNames;
	}

	vector<string> SearchMoleculesWidget::initializeChemicalNames() {
		//Get the names of chemicals to search for.
		QList<QString> moleculeList;
		if ( ui.chemicalGroupBox->isChecked() && ui.nameRadioButton->isChecked()){
			moleculeList = getSearchChemicals();
		}

		//We need to have upper and lower case variations of all of the
		//molecules.
		QList<QString> moleculeMasterList;
		for ( int i = 0; i < moleculeList.size(); i++ ) {
			if ( moleculeList[i].trimmed().size() > 0 ) {
				moleculeMasterList.append( Util::getTitleCaseVariations( moleculeList[i]));
			}
		}

		//The search engine needs casa independent units
		vector<string> chemNames = convertStringFormats( moleculeMasterList );
		return chemNames;
	}

	vector<string> SearchMoleculesWidget::initializeChemicalFormulas(){
		//Get the names of chemical formulas to search for.
		QList<QString> moleculeList;
		if ( ui.chemicalGroupBox->isChecked() && ui.formulaRadioButton->isChecked()){
			moleculeList = getSearchChemicals();
		}

		vector<string> chemFormulas = convertStringFormats( moleculeList );
		return chemFormulas;
	}

	double SearchMoleculesWidget::getRedShiftedValue( bool reverseRedshift, double value ) const {
		Vector<Double> inputValues(1);
		inputValues[0] = value;
		Unit splatalogueUnit( SPLATALOGUE_UNITS.toStdString());
		Quantum< Vector<Double> > quantum( inputValues, splatalogueUnit );
		MDoppler doppler = getRedShiftAdjustment( reverseRedshift );
		Quantum< Vector<Double> > outputQuantum = doppler.shiftFrequency(inputValues);
		Vector<Double> outputValues = outputQuantum.getValue();
		double result = outputValues[0];
		return result;
	}

	void SearchMoleculesWidget::setAstronomicalFilters( Searcher* searcher ) {
		int filterIndex = ui.astronomicalFilterComboBox->currentIndex();
		if ( filterIndex > 0 ) {
			switch( filterIndex ) {
			case TOP_20:
				searcher->setFilterTop20();
				break;
			case PLANETARY_ATMOSPHERE:
				searcher->setFilterPlanetaryAtmosphere();
				break;
			case HOT_CORES:
				searcher->setFilterHotCores();
				break;
			case DARK_CLOUDS:
				searcher->setFilterDarkClouds();
				break;
			case DIFFUSE_CLOUDS:
				searcher->setFilterDiffuseClouds();
				break;
			case COMETS:
				searcher->setFilterComets();
				break;
			case AGB_PPN_PN:
				searcher->setFilterAgbPpnPn();
				break;
			case EXTRAGALACTIC:
				searcher->setFilterExtragalactic();
				break;
			default:
				assert( false );
			}
		}
	}

	void SearchMoleculesWidget::setSearchRangeDefault() {
		bool minEmpty = false;
		if ( ui.rangeMinLineEdit->text().trimmed().length() == 0) {
			minEmpty = true;
		}
		bool maxEmpty = false;
		if ( ui.rangeMaxLineEdit->text().trimmed().length() == 0) {
			maxEmpty = true;
		}

		if ( (minEmpty || maxEmpty) && canvas != NULL ) {
			//One or more of them has not been set so we ask the pixel
			//canvas what the full spectrum range is for the current zoom.
			QString searchUnits;
			double minValue;
			double maxValue;
			canvas->getCanvasDomain( &minValue, &maxValue, searchUnits );

			//Only replace the empty ones, not the ones the user has specified.
			double convertedMinVal = setRangeValue( minValue, searchUnits);
			double convertedMaxVal = setRangeValue( maxValue, searchUnits);
			double actualMin = convertedMinVal;
			double actualMax = convertedMaxVal;
			if ( actualMin > actualMax ){
				actualMin = convertedMaxVal;
				actualMax = convertedMinVal;
			}
			if (  minEmpty ) {
				if ( !isnan( actualMin ) ){
					ui.rangeMinLineEdit->setText( QString::number( actualMin, 'f', 6));
				}
			}
			if ( maxEmpty ) {
				if ( !isnan( actualMax ) ){
					ui.rangeMaxLineEdit->setText( QString::number( actualMax, 'f', 6));
				}
			}
		}
	}

	void SearchMoleculesWidget::search() {

		bool localSearcher = isLocal();

		//Initialize the scrolling parameters
		searchResultCount=0;
		searchResultOffset=0;

		//Acquire the searcher that will do the search for us.
		if ( searcher == NULL ) {
			searcher = SearcherFactory::getSearcher( localSearcher );
			if ( searcher != NULL ) {
				searcher->setSearchResultLimit( searchResultLimit );
			} else {
				QString msg( "Searches are not supported because the database was not found.");
				Util::showUserMessage( msg, this );
				return;
			}
		} else {
			searcher->reset();
		}

		//Get the chemical names
		vector<string> chemNames = initializeChemicalNames();
		searcher->setChemicalNames( chemNames );

		vector<string> chemFormulas = initializeChemicalFormulas();
		searcher->setSpeciesNames( chemFormulas );

		//Set the range for the search
		Double minValue = SPLATALOGUE_DEFAULT_MIN;
		Double maxValue = SPLATALOGUE_DEFAULT_MAX;
		//Make sure the range has been specified otherwise, we need
		//to use the min/max range of the canvas.  Apparently searches
		//using the splatalogue default min and max are not useful.
		setSearchRangeDefault();
		initializeSearchRange( ui.rangeMinLineEdit, minValue );
		initializeSearchRange( ui.rangeMaxLineEdit, maxValue );
		if ( minValue > maxValue ) {
			double tmp = minValue;
			minValue = maxValue;
			maxValue = tmp;
		}
		searcher->setFrequencyRange( minValue, maxValue );

		//Set any astronomical filters for the search
		setAstronomicalFilters( searcher );

		//Start the background thread that will do the search
		if ( searchThread != NULL ) {
			if ( searchThread->isRunning()) {
				stopSearch();
			}
			delete searchThread;
			searchThread = NULL;
		}
		startSearchThread();
		//searchFinished();
	}

	void SearchMoleculesWidget::startSearchThread() {
		searchInterrupted = false;
		searchThread = new SearchThread( searcher, searchResultOffset );
		connect( searchThread, SIGNAL( finished() ), this, SLOT(searchFinished()));
		searchThread->start();
		progressBar.show();
	}





	void SearchMoleculesWidget::nextResults() {
		int nextStart = searchResultOffset + searchResultLimit;
		if ( nextStart < searchResultCount ) {
			searchResultOffset = nextStart;
			startSearchThread();
		} else {
			qDebug() << "There are no NEXT search results";
		}
	}



	void SearchMoleculesWidget::prevResults() {
		int nextStart = searchResultOffset - searchResultLimit;
		if ( nextStart >= 0 ) {
			searchResultOffset = nextStart;
			startSearchThread();
		} else {
			qDebug() << "There are no PREVIOUS search results";
		}
	}
	void SearchMoleculesWidget::searchFinished() {
		if ( !searchInterrupted ) {
			String errorMsg = searchThread->getErrorMessage();
			if ( errorMsg.length() == 0 ) {
				if (searchResultOffset == 0 ) {
					searchResultCount = searchThread->getResultsCount();
				}
				searchResults = searchThread->getResults();
				bool noResults = searchResults.empty();
				if ( noResults ) {
					progressBar.hide();
				}
				resultDisplay ->displaySearchResults( searchResults,
				                                      searchResultOffset, searchResultCount);
				emit searchCompleted();
			} else {
				progressBar.hide();
				QString errorMessage = errorMsg.c_str();
				Util::showUserMessage( errorMessage, this );
			}
		}
		progressBar.hide();
	}


	MDoppler SearchMoleculesWidget::getRedShiftAdjustment( bool reverseDirection ) const {
		QString redShiftStr = ui.dopplerLineEdit->text();
		MDoppler::Types dopplerType = getDopplerType();
		MVDoppler defaultValue( 0 );
		MDoppler doppler( defaultValue, dopplerType );
		if ( ! redShiftStr.isEmpty() ) {
			double redShift = redShiftStr.toDouble();
			if ( ui.redshiftRadio->isChecked() ) {
				//Set the value of the doppler
				if ( reverseDirection ) {
					redShift = redShift * -1;
				}
				MVDoppler mvDoppler( redShift);
				doppler.set(mvDoppler);
			} else {
				//Velocity units:  convert to a doppler
				QString unitString = ui.dopplerUnitsComboBox->currentText();
				MRadialVelocity::Types referenceType = getReferenceFrame();
				String unitStr( unitString.toStdString());
				MDoppler baseDoppler = MDoppler::Convert ( MRadialVelocity( Quantity(redShift, unitStr ),
				                       referenceType).toDoppler(),dopplerType)();
				//Reverse the sign
				MVDoppler mvValue = baseDoppler.getValue();
				double redshiftVal = mvValue.getValue();
				if ( reverseDirection ) {
					redshiftVal = redshiftVal * -1;
				}
				MVDoppler reverseMvValue( redshiftVal );
				doppler.set( reverseMvValue );
			}
		}
		return doppler;
	}
//-----------------------------------------------------------------------------
//                        Accessors
//-----------------------------------------------------------------------------

	QString SearchMoleculesWidget::getUnit() const {
		return SPLATALOGUE_UNITS;
	}

	vector<SplatResult> SearchMoleculesWidget::getSearchResults() const {
		return searchResults;
	}

	bool SearchMoleculesWidget::isLocal() const {
		//Note:  this will need to be reimplemented if we ever have network
		//access to Splatalogue
		return true;
	}



	MDoppler::Types SearchMoleculesWidget::getDopplerType() const {
		QString dopplerStr = ui.dopplerTypeCombo->currentText();
		MDoppler::Types dopplerType = dopplerTypeMap[dopplerStr];
		return dopplerType;
	}
	MRadialVelocity::Types SearchMoleculesWidget::getReferenceFrame() const {
		QString referenceStr = ui.referenceFrameCombo->currentText();
		MRadialVelocity::Types velocityType = radialVelocityTypeMap[referenceStr ];
		return velocityType;
	}

	void SearchMoleculesWidget::stopSearch() {
		if ( searchThread != NULL && searchThread->isRunning()) {
			searchInterrupted = true;
			searchThread->stopSearch();
		}
	}

	SearchMoleculesWidget::~SearchMoleculesWidget() {
		delete searcher;
		delete searchThread;
	}
}
