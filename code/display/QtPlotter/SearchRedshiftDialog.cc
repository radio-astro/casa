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
#include "SearchRedshiftDialog.qo.h"
#include <display/QtPlotter/SearchMoleculesWidget.qo.h>
#include <display/QtPlotter/Util.h>
#include <display/Display/Options.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDoppler.h>
#include <QDebug>
#include <assert.h>
#include <limits>
#include <cmath>
using namespace std;

namespace casa {

	SearchRedshiftDialog::SearchRedshiftDialog(QWidget *parent)
		: QDialog(parent), searchThread( NULL ), progressBar( this ) {
		ui.setupUi(this);

		setWindowTitle( "Find Redshift");

		//Progress Bar
		progressBar.setWindowTitle( "Redshift Search");
		progressBar.setLabelText( "Searching for redshift ...");
		progressBar.setWindowModality( Qt::WindowModal );
		Qt::WindowFlags flags = Qt::Dialog;
		flags |= Qt::FramelessWindowHint;
		progressBar.setWindowFlags( flags );
		progressBar.setMinimum(0);
		progressBar.setMaximum(0);
		connect( &progressBar, SIGNAL(canceled()), this, SLOT(stopSearch()));

		QDoubleValidator* validator = new QDoubleValidator( this );
		ui.centerLineEdit->setValidator( validator );

		ui.velocityLineEdit->setEnabled( false );
		ui.redshiftLineEdit->setEnabled( false );

		connect( ui.findRedshiftButton, SIGNAL(clicked()), this, SLOT(findRedshift()));
		connect( ui.closeButton, SIGNAL(clicked()), this, SLOT(close()));
	}

	void SearchRedshiftDialog::setResultsVisible( bool visible ) {
		ui.redshiftLabel->setVisible( visible );
		ui.redshiftLineEdit->setVisible( visible );
		ui.velocityLineEdit->setVisible( visible );
		ui.velocityLabel->setVisible( visible );
		ui.velocityUnitLabel->setVisible( visible );
		ui.line->setVisible( visible );
	}

	void SearchRedshiftDialog::setCenter( double centerVal ) {
		ui.centerLineEdit->setText( QString::number( centerVal ));
	}

	void SearchRedshiftDialog::setUnits( QString unitStr ) {
		ui.unitLabel->setText( Util::toHTML( unitStr ));
	}

	void SearchRedshiftDialog::setDatabasePath( String path ) {
		databasePath = path;
	}

	void SearchRedshiftDialog::setLocalSearch( bool local ) {
		localSearch = local;
	}

	void SearchRedshiftDialog::setIdentifiedLines( const QList<QString>& lineNames ) {
		ui.speciesCombo->clear();
		for ( int i = 0; i < lineNames.size(); i++ ) {
			ui.speciesCombo->addItem( lineNames[i] );
		}
	}

	void SearchRedshiftDialog::setSpectralCoordinate( SpectralCoordinate coord ){
		spectralCoordinate = coord;
	}

	void SearchRedshiftDialog::show() {
		setResultsVisible( false );
		QDialog::show();
	}

	void SearchRedshiftDialog::setDopplerType( MDoppler::Types type ) {
		dopplerType = type;
	}

	void SearchRedshiftDialog::setFrequencyType( MRadialVelocity::Types type ) {
		radialVelocityType = type;
		if ( type == MRadialVelocity::LSRK ) {
			frequencyType = MFrequency::LSRK;
		} else if ( type == MRadialVelocity::LSRD ) {
			frequencyType = MFrequency::LSRD;
		} else if ( type == MRadialVelocity::BARY ) {
			frequencyType = MFrequency::BARY;
		} else if ( type == MRadialVelocity::TOPO ) {
			frequencyType = MFrequency::TOPO;
		} else {
			qDebug() << "Unrecognized radial velocity type: "<<type;
			assert( false );
		}
	}

//---------------------------------------------------------------------------
//                 Performing the Search
//---------------------------------------------------------------------------

	void SearchRedshiftDialog::findRedshift() {

		//Make sure our target frequency is not empty before we proceed.
		QString centerValStr = ui.centerLineEdit->text();
		if ( centerValStr.length() == 0 ) {
			QString msg( "Please specify the center of the molecular line.");
			Util::showUserMessage( msg, this );
			setResultsVisible( false );
			return;
		}

		//Acquire the searcher that will do the search for us.
		Searcher* searcher = SearcherFactory::getSearcher( localSearch);
		if ( searcher == NULL) {
			QString msg( "The local database was not found so search functionality is unavailable.");
			Util::showUserMessage( msg, this );
			return;
		}
		searcher->setSearchResultLimit( -1 );

		vector<string> chemNames;
		searcher->setChemicalNames( chemNames );

		//Get the search parameters
		QString speciesName = ui.speciesCombo->currentText();
		if ( speciesName.length() > 0 ) {
			vector<string> speciesList( 1 );
			speciesList[0] = speciesName.toStdString();
			searcher->setSpeciesNames( speciesList );

			//Range for the search.
			Double minValue = getTargetFrequency();
			Double maxValue = minValue + 10000;
			searcher->setFrequencyRange( minValue, maxValue );

			//Start the background thread that will do the search
			delete searchThread;
			searchThread = new SearchThread( searcher, 0 );
			searchThread->setCountNeeded( false );
			searchInterrupted = false;
			connect( searchThread, SIGNAL( finished() ), this, SLOT(searchFinished()));
			searchThread->start();
			progressBar.show();
		} else {
			QString msg( "The species name cannot be left blank.");
			Util::showUserMessage( msg, this );
			setResultsVisible( false );
		}
	}

	void SearchRedshiftDialog::stopSearch() {
		if ( searchThread != NULL && searchThread->isRunning()) {
			searchInterrupted = true;
			searchThread->stopSearch();
		}
	}

	double SearchRedshiftDialog::getTargetFrequency() const {
		QString centerValStr = ui.centerLineEdit->text();
		double centerVal = centerValStr.toDouble();

		QString unitStr = Util::stripFont(ui.unitLabel->text());
		if ( unitStr != SearchMoleculesWidget::SPLATALOGUE_UNITS ) {
			Converter* converter = Converter::getConverter( unitStr, SearchMoleculesWidget::SPLATALOGUE_UNITS );
			centerVal = converter->convert( centerVal, spectralCoordinate );
			delete converter;
		}
		return centerVal;
	}

	void SearchRedshiftDialog::searchFinished() {
		progressBar.hide();
		if ( !searchInterrupted ) {
			vector<SplatResult> results = searchThread->getResults();
			int resultCount = results.size();
			if ( resultCount == 0 ) {
				progressBar.hide();
				setResultsVisible( false );
				QString msg( "There were no matching species in the database.");
				Util::showUserMessage( msg, this );
				return;
			}

			//Find the frequency nearest to but less than that indicated by the user.
			double targetFrequency = getTargetFrequency();
			double restFrequency = -1;
			for ( int i = 0; i < resultCount; i++ ) {
				//Record line = results.asRecord("*" + String::toString(i) );

				//Frequency
				double freqValue = results[i].getFrequency().first;


				//In GHz, the redshifted value (targetFrequency) will be smaller
				//than the rest frequency.  We are looking for the rest frequency
				//that is just slightly bigger than the target.
				if ( freqValue >= targetFrequency ) {
					//First one we found;
					if ( restFrequency == -1 ) {
						restFrequency = freqValue;
					}
					//See if this one is closer that our current best
					else if ( freqValue < restFrequency ) {
						restFrequency = freqValue;
					}
				}

				//Keep the GUI responsive while we are setting up the table
				QCoreApplication::processEvents();
			}


			//Calculate the redshift and velocity
			if ( restFrequency != -1 ) {
				String splatalogueUnitStr( SearchMoleculesWidget::SPLATALOGUE_UNITS.toStdString());
				Quantity targetQuantity( targetFrequency, splatalogueUnitStr );
				MVFrequency mvFreq( targetQuantity );
				MFrequency freq( mvFreq, frequencyType );
				Quantity restQuantity( restFrequency, splatalogueUnitStr );
				MVFrequency restFreq( restQuantity );
				MDoppler velDoppler = freq.toDoppler( restFreq );
				MDoppler doppler = MDoppler::Convert( velDoppler, dopplerType)();

				//Redshift
				double redshiftVal = doppler.getValue();
				ui.redshiftLineEdit->setText( QString::number( redshiftVal ));

				//Velocity
				QString unitStr = Util::stripFont( ui.velocityUnitLabel->text());
				MVDoppler mvDoppler( redshiftVal );
				MDoppler veldoppler2( mvDoppler, dopplerType );
				MRadialVelocity mVelocity = MRadialVelocity::fromDoppler(veldoppler2, radialVelocityType);
				String unitString = unitStr.toStdString();
				Quantity velQuantity = mVelocity.get( unitString );
				double velocity = velQuantity.getValue();
				if ( !std::isnan(velocity) ) {
					ui.velocityLineEdit->setText( QString::number( velocity ));
				} else {
					ui.velocityLineEdit->setText("");
					QString msg( "Could not accurately determine velocity.");
					Util::showUserMessage( msg, this );
				}
				setResultsVisible( true );
			} else {
				QString msg( "Could not find a rest frequency matching the search criteria.");
				Util::showUserMessage( msg, this );
				setResultsVisible( false );
			}
		}

	}




	SearchRedshiftDialog::~SearchRedshiftDialog() {
		delete searchThread;
	}
}
