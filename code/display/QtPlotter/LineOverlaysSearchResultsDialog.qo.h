
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

#ifndef LINEOVERLAYSSEARCHRESULTSDIALOG_QO_H
#define LINEOVERLAYSSEARCHRESULTSDIALOG_QO_H
#include <QtGui/QDialog>
#include <display/QtPlotter/LineOverlaysSearchResultsDialog.ui.h>
#include <display/QtPlotter/SearchMoleculesResultDisplayer.h>
#include <casa/Containers/Record.h>
#include <casa/aips.h>
namespace casa {

	class SearchMoleculesResultsWidget;

	class LineOverlaysSearchResultsDialog : public QDialog, public SearchMoleculesResultDisplayer {
		Q_OBJECT

	public:
		LineOverlaysSearchResultsDialog(QWidget *parent = 0);
		QList<int> getLineIndices() const;
		bool getLine(int lineIndex, Float& peak, Float& center,
		             QString& molecularName, QString& chemicalName,
		             QString& resolvedQNs, QString& frequencyUnit ) const;
		void getLines( QList<float>& peaks, QList<float>& centers,
		               QString& molecularName, QList<QString>& chemicalNames,
		               QList<QString>& resolvedQNs, QString& frequencyUnit ) const;
		void displaySearchResults( const vector<SplatResult>& results, int offset,
		                           int totalCount);
		int getLineCount() const;
		~LineOverlaysSearchResultsDialog();

	signals:
		void graphSelectedLines();
		void graphSelectedSpecies();
		void showPreviousSearchResults();
		void showNextSearchResults();

	private slots:
		void drawSelectedLines();
		void drawSelectedSpecies();


	private:
		void setSearchScrollingVisible( bool visible );
		Ui::LineOverlaysSearchResultsDialogClass ui;
		SearchMoleculesResultsWidget* searchResultsWidget;
		const static QString NO_LINES_SELECTED;
	};

}
#endif // LINEOVERLAYSSEARCHRESULTSDIALOG_QO_H
