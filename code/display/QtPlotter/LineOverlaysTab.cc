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
#include "LineOverlaysTab.qo.h"
#include <QPushButton>
#include <display/QtPlotter/SearchMoleculesWidget.qo.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/Util.h>
#include <QFileDialog>
#include <QDebug>
namespace casa {
	LineOverlaysTab::LineOverlaysTab(QWidget *parent)
		: QWidget(parent), pixelCanvas( NULL ), searchResults( this ), searchRedshiftDialog( this ) {
		ui.setupUi(this);

		QHBoxLayout* layout = new QHBoxLayout();
		layout->setContentsMargins(2,2,2,2);
		searchWidget = new SearchMoleculesWidget( this);
		layout->addWidget( searchWidget );
		ui.searchWidgetHolder->setLayout( layout );
		searchWidget->setResultDisplay( &searchResults );

		connect( &searchResults, SIGNAL(graphSelectedLines()), this, SLOT(graphSelectedLines()));
		connect( &searchResults, SIGNAL(graphSelectedSpecies()), this, SLOT(graphSelectedSpecies()));
		connect( &searchResults, SIGNAL(showPreviousSearchResults()), searchWidget, SLOT(prevResults()));
		connect( &searchResults, SIGNAL(showNextSearchResults()), searchWidget, SLOT(nextResults()));
		connect( searchWidget, SIGNAL(searchCompleted()), this, SLOT(searchCompleted()));
		connect( searchWidget, SIGNAL(redshiftChanged()), this, SLOT(redshiftChanged()));
		connect( ui.clearLinesButton, SIGNAL(clicked()), this, SLOT(eraseLines()));
		connect( ui.saveLinesButton, SIGNAL(clicked()), this, SLOT(saveIdentifiedLines()));
	}

	void LineOverlaysTab::setCanvas( QtCanvas* canvas ) {
		pixelCanvas = canvas;
		connect( pixelCanvas, SIGNAL(findRedshiftAt(double,double)), this, SLOT(findRedshift(double,double)));
		searchWidget->setCanvas( pixelCanvas );
	}

	void LineOverlaysTab::redshiftChanged( ){
		QList<MolecularLine* >  lines = pixelCanvas->getMolecularLines();
		int lineCount = lines.size();
		if ( lineCount > 0 ) {
			int lineCount = lines.size();
			for ( int i = 0; i < lineCount; i++ ){
				float lineCenter = lines[i]->getOriginalFrequency();
				QString originalUnits = lines[i]->getOriginalUnits();
				QString graphUnits = pixelCanvas->getUnits();
				Converter* converter = NULL;
				if ( originalUnits != graphUnits ){
					converter = Converter::getConverter( originalUnits,graphUnits );
				}
				float shiftedCenter = getShiftedCenter( lineCenter, converter );
				lines[i]->setCenter( shiftedCenter );
			}
			pixelCanvas->refreshPixmap();
		}
	}

	void LineOverlaysTab::searchCompleted() {
		int lineCount = searchResults.getLineCount();
		if ( lineCount > 0 ) {
			searchResults.show();
			//Added because the search results were apparently
			//coming up behind other windows on the MAC
			searchResults.raise();
			searchResults.activateWindow();
		}
	}


	void LineOverlaysTab::setRange( double min, double max, String units  ) {
		searchWidget->setRange( min, max, units.c_str() );
	}



	void LineOverlaysTab::graphSelectedSpecies() {
		QList<int> lineIndices = searchResults.getLineIndices();
		QString searchUnits = searchWidget->getUnit();
		QString graphUnits = pixelCanvas->getUnits();
		Converter* converter = NULL;
		if ( searchUnits != graphUnits ) {
			converter = Converter::getConverter( searchUnits, graphUnits );
		}
		int count = lineIndices.size();
		if ( count == 0 ) {
			QString msg = "Please select one or more molecular species\n from the search results to graph";
			Util::showUserMessage( msg, this );
		} else {
			QList<QString> usedSpecies;
			for ( int i = 0; i < count; i++ ) {
				Float center;
				Float peak;
				QString molecularName;
				QString chemicalName;
				QString resolvedQNs;
				QString frequencyUnit;
				bool lineExists = searchResults.getLine(lineIndices[i], peak,
				                                        center, molecularName, chemicalName, resolvedQNs,
				                                        frequencyUnit);
				if ( lineExists ) {
					if ( !usedSpecies.contains( molecularName )) {
						QList<float> peaks;
						QList<float> centers;
						QList<QString> chemicalNames;
						QList<QString> resolvedQNs;
						QString frequencyUnit;
						searchResults.getLines( peaks, centers, molecularName, chemicalNames,
						                        resolvedQNs, frequencyUnit );
						for ( int j = 0; j < centers.size(); j++ ) {
							addLineToPixelCanvas(centers[j], peaks[j], molecularName,
							                     chemicalNames[j], resolvedQNs[j], frequencyUnit, converter);
						}
						usedSpecies.append( molecularName );
					}
				} else {
					qDebug() << "Could not retrieve line i="<< i<<" lineIndex="<<lineIndices[i];
				}
			}
		}
		delete converter;
	}

	float LineOverlaysTab::getShiftedCenter( float center, Converter* converter ){
		//First we have to make the frequency value redshifted.
		double shiftedCenter = searchWidget->getRedShiftedValue( false, center );

		//Now Convert it to the same units the pixel canvas is using.
		if ( converter != NULL ) {
			shiftedCenter = converter->convert( shiftedCenter, spectralCoordinate);
		}
		return shiftedCenter;
	}

	void LineOverlaysTab::addLineToPixelCanvas( float center, float peak, QString molecularName,
	        QString chemicalName, QString resolvedQNs, QString frequencyUnit, Converter* converter) {
		double shiftedCenter = getShiftedCenter( center, converter );


		//Add the line to the pixel canvas.
		if ( pixelCanvas != NULL ) {
			MolecularLine* line = new MolecularLine( shiftedCenter, peak,
			        molecularName, chemicalName, resolvedQNs, frequencyUnit, center );
			pixelCanvas->addMolecularLine( line );
		}
	}


	void LineOverlaysTab::graphSelectedLines() {
		QList<int> lineIndices = searchResults.getLineIndices();
		QString searchUnits = searchWidget->getUnit();
		QString graphUnits = pixelCanvas->getUnits();
		Converter* converter = NULL;
		if ( searchUnits != graphUnits ) {
			converter = Converter::getConverter( searchUnits, graphUnits );
		}
		int count = lineIndices.size();
		if ( count == 0 ) {
			QString msg = "Please select one or more molecules\n from the search results to graph";
			Util::showUserMessage( msg, this );
		} else {
			for ( int i = 0; i < count; i++ ) {
				Float center;
				Float peak;
				QString molecularName;
				QString chemicalName;
				QString frequencyUnit;
				QString resolvedQN;
				bool lineExists = searchResults.getLine(lineIndices[i], peak,
				                                        center, molecularName, chemicalName, resolvedQN, frequencyUnit );
				if ( lineExists ) {
					addLineToPixelCanvas(center, peak, molecularName,
					                     chemicalName, resolvedQN, frequencyUnit, converter);
				} else {
					qDebug() << "Could not retrieve line i="<< i<<" lineIndex="<<lineIndices[i];
				}
			}
		}
		delete converter;
	}

	void LineOverlaysTab::setInitialReferenceFrame( QString referenceStr ) {
		SearchMoleculesWidget::setInitialReferenceFrame( referenceStr );
		searchWidget->updateReferenceFrame();
	}

	void LineOverlaysTab::eraseLines() {
		pixelCanvas->clearMolecularLines( true );
	}

	void LineOverlaysTab::unitsChanged(QString graphUnits ){

		QList<MolecularLine* >  lines = pixelCanvas->getMolecularLines();
		int lineCount = lines.size();
		for ( int i = 0; i < lineCount; i++ ){
			float lineCenter = lines[i]->getOriginalFrequency();
			QString originalUnits = lines[i]->getOriginalUnits();
			Converter* converter = NULL;
			if ( originalUnits != graphUnits ){
				converter = Converter::getConverter( originalUnits,graphUnits );
				float shiftedCenter = getShiftedCenter( lineCenter, converter );
				lines[i]->setCenter( shiftedCenter );
			}
			delete converter;
		}
	}

	void LineOverlaysTab::setSpectralCoordinate( SpectralCoordinate coord ){
		searchWidget->setSpectralCoordinate( coord );
		searchRedshiftDialog.setSpectralCoordinate( coord );
		spectralCoordinate = coord;
	}

	void LineOverlaysTab::findRedshift( double center, double /*peak*/ ) {
		QString unitStr = pixelCanvas->getUnits();
		searchRedshiftDialog.setUnits( unitStr );
		searchRedshiftDialog.setCenter( center );
		searchRedshiftDialog.setLocalSearch( searchWidget->isLocal());
		//searchRedshiftDialog.setDatabasePath( searchWidget->getDatabasePath());
		searchRedshiftDialog.setDopplerType( searchWidget->getDopplerType());
		searchRedshiftDialog.setFrequencyType( searchWidget->getReferenceFrame());
		searchRedshiftDialog.setIdentifiedLines( pixelCanvas->getMolecularLineNames());
		searchRedshiftDialog.show();
	}

	void LineOverlaysTab::saveIdentifiedLines() {
		std::string homedir = getenv("HOME");
		QFileDialog fd( this, tr("Specify a file for storing the graphed molecular lines."),
		                QString(homedir.c_str()), "");
		fd.setFileMode( QFileDialog::AnyFile );
		if ( fd.exec() ) {
			QStringList fileNames = fd.selectedFiles();
			if ( fileNames.size() > 0 ) {
				QString outputFileName = fileNames[0];
				QFile file( outputFileName );
				if (file.open(QFile::WriteOnly | QFile::Truncate)) {
					QTextStream out( &file );
					QList<MolecularLine* >  lines = pixelCanvas->getMolecularLines();
					for ( int i = 0; i < static_cast<int>(lines.size()); i++ ) {
						out << "\n";
						out << "Line "<<(i+1)<<"\n";
						lines[i]->toStream( &out );
						out.flush();
					}
					file.close();
					QString msg( "Graphed molecular line information was saved \n to "+outputFileName);
					Util::showUserMessage( msg, this );
				} else {
					QString msg( "Could not open "+outputFileName+" for writing.");
					Util::showUserMessage( msg, this);
				}
			} else {
				QString msg( "There were no graphed molecular lines to save.");
				Util::showUserMessage( msg, this );
			}

		}

	}


	LineOverlaysTab::~LineOverlaysTab() {

	}
}
