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
#include "SearchMoleculesDialog.qo.h"
#include <display/QtPlotter/SearchMoleculesWidget.qo.h>
#include <display/QtPlotter/SearchMoleculesResultsWidget.qo.h>
#include <casa/Exceptions/Error.h>
#include <QSpacerItem>
#include <QDebug>

namespace casa {



	SearchMoleculesDialog::SearchMoleculesDialog(QWidget *parent)
		: QDialog(parent) {
		ui.setupUi(this);

		QHBoxLayout* layout = new QHBoxLayout();
		searchWidget = new SearchMoleculesWidget( this );
		layout->addWidget( searchWidget );
		ui.searchWidgetHolder->setLayout( layout );

		QVBoxLayout* layout2 = new QVBoxLayout();
		searchResultsWidget = new SearchMoleculesResultsWidget( this );
		searchResultsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		searchWidget->setResultDisplay( searchResultsWidget );
		//layout2->insertStretch( 0, 1);
		layout2->addWidget( searchResultsWidget );
		//layout2->insertStretch( 2, 1);
		ui.searchResultsHolder->setLayout( layout2 );

		connect( ui.applyButton, SIGNAL(clicked()), this, SIGNAL(moleculesSelected()));
		connect( ui.closeButton, SIGNAL(clicked()), this, SLOT(reject()));
	}


	void SearchMoleculesDialog::setRange( float min, float max, QString units ) {
		searchWidget->setRange( min, max, units );
	}

	void SearchMoleculesDialog::updateReferenceFrame() {
		searchWidget->updateReferenceFrame();
	}


//-----------------------------------------------------------------------------
//                        Accessors
//-----------------------------------------------------------------------------

	QString SearchMoleculesDialog::getUnit() const {
		return searchWidget->getUnit();
	}

	QList<int> SearchMoleculesDialog::getLineIndices() const {
		return searchResultsWidget->getLineIndices();
	}

	bool SearchMoleculesDialog::getLine(int lineIndex, Float& peak, Float& center,
	                                    QString& molecularName, QString& chemicalName, QString& resolvedQNs,
	                                    QString& frequencyUnit ) const {
		return searchResultsWidget->getLine( lineIndex, peak, center, molecularName,
		                                     chemicalName, resolvedQNs, frequencyUnit );
	}

	double SearchMoleculesDialog::getRedShiftedValue( bool reverseRedshift, float value ) const {
		return searchWidget->getRedShiftedValue( reverseRedshift, value );
	}




	SearchMoleculesDialog::~SearchMoleculesDialog() {

	}
}
