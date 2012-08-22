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

#include "LineOverlaysSearchResultsDialog.qo.h"
#include <display/QtPlotter/Util.h>
#include <display/QtPlotter/SearchMoleculesResultsWidget.qo.h>

namespace casa {

const QString LineOverlaysSearchResultsDialog::NO_LINES_SELECTED = "Please select one or more lines from the search results to graph";

LineOverlaysSearchResultsDialog::LineOverlaysSearchResultsDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	QHBoxLayout* layout = new QHBoxLayout();
	searchResultsWidget = new SearchMoleculesResultsWidget( this );
	layout->addWidget( searchResultsWidget );
	ui.searchResultsHolder->setLayout( layout );


	connect( ui.graphSelectedButton, SIGNAL(clicked()), this, SLOT(drawSelectedLines()));
	connect( ui.graphSelectedSpeciesButton, SIGNAL(clicked()), this, SLOT(drawSelectedSpecies()));
	connect( ui.closeButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void LineOverlaysSearchResultsDialog::drawSelectedSpecies(){
	int count = searchResultsWidget->getLineCount();
	if ( count > 0 ){
		emit graphSelectedSpecies();
	}
	else {
		QString msg( NO_LINES_SELECTED );
		Util::showUserMessage( msg, this );
	}
}

void LineOverlaysSearchResultsDialog::drawSelectedLines(){
	int count = searchResultsWidget->getLineCount();
	if ( count > 0 ){
		emit graphSelectedLines();
	}
	else {
		QString msg( NO_LINES_SELECTED );
		Util::showUserMessage( msg, this );
	}
}

int LineOverlaysSearchResultsDialog::getLineCount() const {
	return searchResultsWidget->getLineCount();
}

QList<int> LineOverlaysSearchResultsDialog::getLineIndices() const{
	return searchResultsWidget->getLineIndices();
}

bool LineOverlaysSearchResultsDialog::getLine(int lineIndex, Float& peak, Float& center,
           		QString& molecularName ) const{
	return searchResultsWidget->getLine( lineIndex, peak, center, molecularName );
}

void LineOverlaysSearchResultsDialog::getLines( QList<float>& peaks, QList<float>& centers, QString molecularName ) const{
	return searchResultsWidget->getLines( peaks, centers, molecularName );
}

void LineOverlaysSearchResultsDialog::displaySearchResults( const Record& record ){
	searchResultsWidget->displaySearchResults( record );
}

LineOverlaysSearchResultsDialog::~LineOverlaysSearchResultsDialog()
{

}
}
