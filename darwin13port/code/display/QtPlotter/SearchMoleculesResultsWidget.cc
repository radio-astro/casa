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
#include "SearchMoleculesResultsWidget.qo.h"
#include <display/QtPlotter/SearchMoleculesWidget.qo.h>
#include <display/QtPlotter/SearchMoleculesResultsWidget.qo.h>
#include <display/QtPlotter/Util.h>
#include <QDebug>
#include <QtCore/qmath.h>

namespace casa {


	SearchMoleculesResultsWidget::SearchMoleculesResultsWidget(QWidget *parent)
		: QWidget(parent) {
		ui.setupUi(this);
		initializeTable();
	}

	void SearchMoleculesResultsWidget::initializeTable() {
		QStringList tableHeaders(QStringList() << "ID"<<"Species" << "Chemical Name" <<
		                         "Frequency(MHz)" << "Resolved QNs" << "Intensity" );
		ui.searchResultsTable->setColumnCount( COLUMN_COUNT );
		ui.searchResultsTable->setHorizontalHeaderLabels( tableHeaders );
		ui.searchResultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui.searchResultsTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn );
		ui.searchResultsTable->setSortingEnabled( true );
		ui.searchResultsTable->verticalHeader()->setVisible( false );
	}


	void SearchMoleculesResultsWidget::displaySearchResults( const vector<SplatResult>& results,
	        int /*offset*/, int /*countTotal*/ ) {
		int resultCount = results.size();
		if ( resultCount == 0 ) {
			QString msg( "There were no search results matching the given criteria.");
			Util::showUserMessage( msg, this );
		}
		ui.searchResultsTable->setRowCount( resultCount );
		ui.searchResultsTable->clearSelection();
		for ( int i = 0; i < resultCount; i++ ) {

			//Record line = results.asRecord("*" + String::toString(i) );
			//Species Id
			long speciesId = results[i].getSpeciesId();
			setTableValue( i, COL_SPECIES_ID, speciesId );

			//Species
			string species = results[i].getSpecies();
			setTableValueHTML( i, COL_SPECIES, species.c_str() );

			//Chemical Name
			string chemName = results[i].getChemicalName();
			setTableValue( i, COL_CHEMICAL, chemName.c_str() );

			//Frequency
			pair<double,string> freqResult = results[i].getFrequency();
			//Default frequency units seem to be GHz
			setTableValue( i, COL_FREQUENCY, freqResult.first );

			//QNS
			string qns = results[i].getQuantumNumbers();
			setTableValueHTML( i, COL_QN, qns.c_str());

			//Intensity
			float intensity = results[i].getIntensity();
			setTableValue( i, COL_INTENSITY, intensity );

			//Keep the GUI responsive while we are setting up the table
			QCoreApplication::processEvents();
		}
		ui.searchResultsTable->resizeColumnsToContents();
	}


	int SearchMoleculesResultsWidget::getSelectedLineCount() const {
		QList<QTableWidgetSelectionRange> ranges = ui.searchResultsTable->selectedRanges();
		int lineCount = 0;
		for ( int i = 0; i < ranges.size(); i++ ) {
			int rangeRowCount = ranges[i].rowCount();
			lineCount = lineCount + rangeRowCount;
		}
		return lineCount;
	}

	int SearchMoleculesResultsWidget::getLineCount() const {
		int count = ui.searchResultsTable->rowCount();
		return count;
	}

	QList<int> SearchMoleculesResultsWidget::getLineIndices() const {
		QList<QTableWidgetSelectionRange> ranges = ui.searchResultsTable->selectedRanges();
		QList<int> lineIndices;
		for ( int i = 0; i < ranges.size(); i++ ) {
			int rangeRowCount = ranges[i].rowCount();
			int index1 = ranges[i].bottomRow();

			int index2 = ranges[i].topRow();
			int minIndex = qMin( index1, index2 );
			for ( int j = 0; j < rangeRowCount; j++ ) {
				int index = minIndex + j;
				if ( !lineIndices.contains( index )) {
					lineIndices.append( index );
				}
			}
		}
		return lineIndices;
	}

	void SearchMoleculesResultsWidget::getLines( QList<float>& peaks,
	        QList<float>& centers, QString molecularName, QList<QString>& chemNames,
	        QList<QString>& resolvedQNSs, QString frequencyUnit ) const {
		int rowCount = ui.searchResultsTable->rowCount();
		for ( int i = 0; i < rowCount; i++ ) {
			QLabel* speciesItem = dynamic_cast<QLabel*>(ui.searchResultsTable->cellWidget( i, COL_SPECIES ));
			if ( speciesItem != NULL ) {
				QString speciesName = speciesItem->text();
				if ( speciesName == molecularName ) {
					Float peak;
					Float center;
					QString chemicalName;
					QString resolvedQNs;
					bool success = getLine( i, peak, center, speciesName,
					                        chemicalName, resolvedQNs, frequencyUnit );
					if ( success ) {
						peaks.append( peak );
						centers.append( center );
						chemNames.append( chemicalName );
						resolvedQNSs.append( resolvedQNs );
					}

				}
			}
		}
	}

	bool SearchMoleculesResultsWidget::getLine(int lineIndex, Float& peak, Float& center,
	        QString& molecularName, QString& chemicalName, QString& resolvedQNs,
	        QString& frequencyUnits) const {
		bool lineExists = true;
		//Name
		QLabel* speciesItem = dynamic_cast<QLabel*>(ui.searchResultsTable->cellWidget( lineIndex, COL_SPECIES ));
		if ( speciesItem != NULL ) {
			molecularName = speciesItem->text();

			//Frequency
			QTableWidgetItem* frequencyItem = ui.searchResultsTable->item(lineIndex, COL_FREQUENCY );
			if ( frequencyItem != NULL ) {
				QString freqStr = frequencyItem->text();
				center = freqStr.toFloat();
			}
			frequencyUnits = SearchMoleculesWidget::SPLATALOGUE_UNITS;

			//Intensity
			QTableWidgetItem* peakItem = ui.searchResultsTable->item(lineIndex, COL_INTENSITY );
			if ( peakItem != NULL ) {
				QString peakStr = peakItem->text();
				peak = peakStr.toFloat();
				peak = peak * -1;
			}

			//Chemical Name
			QTableWidgetItem* chemNameItem = ui.searchResultsTable->item(lineIndex, COL_CHEMICAL );
			if ( chemNameItem != NULL ) {
				chemicalName = chemNameItem->text();
			}

			//Resolved QNs
			QLabel* label =	dynamic_cast<QLabel*>(ui.searchResultsTable->cellWidget( lineIndex, COL_QN ));
			QString qnsHtml = label->text();
			resolvedQNs = Util::stripFont( qnsHtml );
		} else {
			lineExists = false;
		}

		return lineExists;
	}

//--------------------------------------------------------------------------------------
//                   Utility - Search Results Table
//--------------------------------------------------------------------------------------

	void SearchMoleculesResultsWidget::setTableValue(int row, int col, double val ) {
		QString numberStr = QString::number( val );
		setTableValue( row, col, numberStr );
	}

	void SearchMoleculesResultsWidget::setTableValue( int row, int col, const QString& val ) {
		QTableWidgetItem* tableItem = new QTableWidgetItem();
		tableItem -> setText( val );
		ui.searchResultsTable->setItem( row, col, tableItem );
	}

	void SearchMoleculesResultsWidget::setTableValueHTML( int row, int col, const QString& val ) {
		QLabel* label = new QLabel( this );
		label->setTextFormat(Qt::RichText);
		QString htmlStr = "<html>"+val+"</html>";
		label->setText( val );
		ui.searchResultsTable->setCellWidget( row, col, label );
	}



	SearchMoleculesResultsWidget::~SearchMoleculesResultsWidget() {

	}

}
