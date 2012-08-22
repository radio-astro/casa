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
#include <QDebug>
namespace casa {
LineOverlaysTab::LineOverlaysTab(QWidget *parent)
    : QWidget(parent), pixelCanvas( NULL ), searchResults( this ), searchRedshiftDialog( this )
{
	ui.setupUi(this);

	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(2,2,2,2);
	searchWidget = new SearchMoleculesWidget( this);
	layout->addWidget( searchWidget );
	ui.searchWidgetHolder->setLayout( layout );
	searchWidget->setResultDisplay( &searchResults );

	connect( &searchResults, SIGNAL(graphSelectedLines()), this, SLOT(graphSelectedLines()));
	connect( &searchResults, SIGNAL(graphSelectedSpecies()), this, SLOT(graphSelectedSpecies()));
	connect( searchWidget, SIGNAL(searchCompleted()), this, SLOT(searchCompleted()));
	connect( ui.clearLinesButton, SIGNAL(clicked()), this, SLOT(eraseLines()));
}

void LineOverlaysTab::setCanvas( QtCanvas* canvas ){
	pixelCanvas = canvas;
	connect( pixelCanvas, SIGNAL(findRedshiftAt(double,double)), this, SLOT(findRedshift(double,double)));
}

void LineOverlaysTab::searchCompleted(){
	int lineCount = searchResults.getLineCount();
	if ( lineCount > 0 ) {
		searchResults.show();
	}
}


void LineOverlaysTab::setRange( float min, float max, String units  ){
	searchWidget->setRange( min, max, units.c_str() );
}



void LineOverlaysTab::graphSelectedSpecies(){
	QList<int> lineIndices = searchResults.getLineIndices();
	QString searchUnits = searchWidget->getUnit();
	QString graphUnits = pixelCanvas->getUnits();
	Converter* converter = NULL;
	if ( searchUnits != graphUnits ){
		converter = Converter::getConverter( searchUnits, graphUnits );
	}
	int count = lineIndices.size();
	if ( count == 0 ){
		QString msg = "Please select one or more molecular species\n from the search results to graph";
		Util::showUserMessage( msg, this );
	}
	else {
		QList<QString> usedSpecies;
		for ( int i = 0; i < count; i++ ){
			Float center;
			Float peak;
			QString molecularName;
			bool lineExists = searchResults.getLine(lineIndices[i], peak, center, molecularName );
			if ( lineExists ){
				if ( !usedSpecies.contains( molecularName )){
					QList<float> peaks;
					QList<float> centers;
					searchResults.getLines( peaks, centers, molecularName );
					for ( int j = 0; j < centers.size(); j++ ){
						if ( converter != NULL ){
							centers[j] = converter->convert( centers[j] );
						}
						if ( pixelCanvas != NULL ){
							MolecularLine* line = new MolecularLine( centers[j], peaks[j], molecularName );
							pixelCanvas->addMolecularLine( line );
						}
					}
					usedSpecies.append( molecularName );
				}
			}
			else {
				qDebug() << "Could not retrieve line i="<< i<<" lineIndex="<<lineIndices[i];
			}
		}
	}
	delete converter;
}

void LineOverlaysTab::graphSelectedLines(){
	QList<int> lineIndices = searchResults.getLineIndices();
	QString searchUnits = searchWidget->getUnit();
	QString graphUnits = pixelCanvas->getUnits();
	Converter* converter = NULL;
	if ( searchUnits != graphUnits ){
		converter = Converter::getConverter( searchUnits, graphUnits );
	}
	int count = lineIndices.size();
	if ( count == 0 ){
		QString msg = "Please select one or more molecules\n from the search results to graph";
		Util::showUserMessage( msg, this );
	}
	else {
		for ( int i = 0; i < count; i++ ){
			Float center;
			Float peak;
			QString molecularName;
			bool lineExists = searchResults.getLine(lineIndices[i], peak, center, molecularName );
			if ( lineExists ){
				if ( converter != NULL ){
					center = converter->convert( center );
				}
				if ( pixelCanvas != NULL ){
					MolecularLine* line = new MolecularLine( center, peak, molecularName );
					pixelCanvas->addMolecularLine( line );
				}
			}
			else {
				qDebug() << "Could not retrieve line i="<< i<<" lineIndex="<<lineIndices[i];
			}
		}
	}
	delete converter;
}

void LineOverlaysTab::setInitialReferenceFrame( QString referenceStr ){
	SearchMoleculesWidget::setInitialReferenceFrame( referenceStr );
	searchWidget->updateReferenceFrame();
}

void LineOverlaysTab::eraseLines(){
	pixelCanvas->clearMolecularLines( true );
}



void LineOverlaysTab::findRedshift( double center, double peak ){
	QString unitStr = pixelCanvas->getUnits();
	searchRedshiftDialog.setUnits( unitStr );
	searchRedshiftDialog.setCenter( center );
	searchRedshiftDialog.setLocalSearch( searchWidget->isLocal());
	searchRedshiftDialog.setDatabasePath( searchWidget->getDatabasePath());
	searchRedshiftDialog.setDopplerType( searchWidget->getDopplerType());
	searchRedshiftDialog.setFrequencyType( searchWidget->getReferenceFrame());

	searchRedshiftDialog.show();
}


LineOverlaysTab::~LineOverlaysTab()
{

}
}
