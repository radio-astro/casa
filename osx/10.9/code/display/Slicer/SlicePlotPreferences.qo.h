//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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
#ifndef SLICEPLOTPREFERENCES_QO_H
#define SLICEPLOTPREFERENCES_QO_H

#include <QtGui/QDialog>
#include <display/Slicer/SlicePlotPreferences.ui.h>

namespace casa {

	class SlicePlotPreferences : public QDialog {
		Q_OBJECT

	public:
		SlicePlotPreferences(QWidget *parent = 0);
		int getLineWidth() const;
		int getMarkerSize() const;
		~SlicePlotPreferences();

	signals:
		void plotPreferencesChanged();

	private slots:
		void preferencesAccepted();
		void preferencesRejected();

	private:
		void initializeCustomSettings();
		void reset();
		void persist();

		int lineThickness;
		int markerSize;

		Ui::SlicePlotPreferencesClass ui;
		const static QString ORGANIZATION;
		const static QString APPLICATION;
		const static QString LINE_THICKNESS;
		const static QString MARKER_SIZE;
	};

}
#endif // PLOTPREFERENCES_QO_H
