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
#ifndef LEGEND_PREFERENCES_QO_H
#define LEGEND_PREFERENCES_QO_H

#include <QDialog>

#include <display/QtPlotter/LegendPreferences.ui.h>

namespace casa {

	class CanvasHolder;

//Allows the user to customize properties of the spectral profile legend.
	class LegendPreferences : public QDialog {
		Q_OBJECT

	public:
		LegendPreferences(CanvasHolder* canvas, QWidget *parent = 0);
		void show();
		~LegendPreferences();

	private slots:
		void legendVisibilityChanged();
		void legendLocationChanged( int index );
		void accept();
		void reject();

	private:
		void initializeUserPreferences();
		void initializeCurveLabels();
		void curveLabelChange();
		void persist();
		void canvasLegendChange();
		void colorBarVisibilityChange();

		bool showLegendDefault;
		bool showColorBarDefault;
		int legendPositionDefault;

		const static QString LEGEND_VISIBLE;
		const static QString LEGEND_LOCATION;
		const static QString LEGEND_COLOR_BAR;
		Ui::LegendPreferences ui;
		CanvasHolder* canvasHolder;

	};
}

#endif // LEGENDPREFERENCES_H
