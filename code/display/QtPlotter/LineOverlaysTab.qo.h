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
#ifndef LINEOVERLAYSTAB_QO_H
#define LINEOVERLAYSTAB_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/LineOverlaysTab.ui.h>
#include <display/QtPlotter/LineOverlaysSearchResultsDialog.qo.h>
#include <display/QtPlotter/SearchRedshiftDialog.qo.h>
#include <casa/BasicSL/String.h>


namespace casa {

	class QtCanvas;
	class SearchMoleculesWidget;
	class Converter;

	class LineOverlaysTab : public QWidget {
		Q_OBJECT

	public:
		LineOverlaysTab(QWidget *parent = 0);
		void setRange( double min, double max, String units );
		void setInitialReferenceFrame( QString referenceStr );
		void setCanvas( QtCanvas* canvas );
		~LineOverlaysTab();

	private slots:
		void graphSelectedLines();
		void graphSelectedSpecies();
		void searchCompleted();
		void eraseLines();
		void saveIdentifiedLines();
		void findRedshift( double center, double peak );

	private:
		void addLineToPixelCanvas( float center, float peak,
		                           QString molecularName, QString chemicalName,
		                           QString resolvedQNs, QString frequencyUnit, Converter* converter);

		Ui::LineOverlaysTabClass ui;
		SearchMoleculesWidget* searchWidget;
		QtCanvas* pixelCanvas;
		LineOverlaysSearchResultsDialog searchResults;
		SearchRedshiftDialog searchRedshiftDialog;
	};
}
#endif // LINEOVERLAYSTAB_QO_H
