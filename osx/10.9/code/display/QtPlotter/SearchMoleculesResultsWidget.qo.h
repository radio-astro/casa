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
#ifndef SEARCHMOLECULESRESULTSWIDGET_QO_H
#define SEARCHMOLECULESRESULTSWIDGET_QO_H

#include <QtGui/QWidget>
#include <casa/Containers/Record.h>
#include <display/QtPlotter/SearchMoleculesResultsWidget.ui.h>
#include <display/QtPlotter/SearchMoleculesResultDisplayer.h>
namespace casa {

	class SearchMoleculesResultsWidget : public QWidget, public SearchMoleculesResultDisplayer {
		Q_OBJECT

	public:
		SearchMoleculesResultsWidget(QWidget *parent = 0);
		~SearchMoleculesResultsWidget();
		QList<int> getLineIndices() const;
		bool getLine(int lineIndex, Float& peak, Float& center,
		             QString& molecularName, QString& chemicalName,
		             QString& resolvedQNs,QString& frequencyUnits ) const;
		void getLines( QList<float>& peaks, QList<float>& centers,
		               QString molecularName, QList<QString>& chemicalNames,
		               QList<QString>& resolvedQNSs, QString frequencyUnit) const;
		int getLineCount() const;
		int getSelectedLineCount() const;
		void displaySearchResults( const vector<SplatResult>& results, int offset, int totalCount );

	private:

		void initializeTable();
		void setTableValue( int row, int col, const QString& val );
		void setTableValueHTML( int row, int col, const QString& val );
		void setTableValue( int row, int col, double val );
		Ui::SearchMoleculesResultsWidgetClass ui;
		enum ResultColumns { COL_SPECIES_ID, COL_SPECIES, COL_CHEMICAL, COL_FREQUENCY,
		                     COL_QN, COL_INTENSITY, COLUMN_COUNT
		                   };

	};

}

#endif // SEARCHMOLECULESRESULTSWIDGET_H
