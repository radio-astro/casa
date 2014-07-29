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
#ifndef SLICECOLORPREFERENCES_QO_H
#define SLICECOLORPREFERENCES_QO_H

#include <QtGui/QDialog>
#include <QSettings>
#include <display/Slicer/SliceColorPreferences.ui.h>

namespace casa {

	class SliceColorPreferences : public QDialog {
		Q_OBJECT

	public:
		SliceColorPreferences(QWidget *parent = 0);
		bool isViewerColors() const;
		bool isPolylineUnit() const;
		QList<QColor> getAccumulatedSliceColors() const;
		~SliceColorPreferences();

	signals:
		void colorsChanged();

	private slots:
		void resetColors();
		void addColorAccumulated();
		void removeColorAccumulated();
		void colorsAccepted();
		void colorsRejected();
		void useViewerColorsChanged( bool viewerColors );

	private:
		void persistColors();
		void populateAccumulateColors( );
		void showColorDialog( QPushButton* source );
		void initializeUserColors();
		void readCustomColor( QSettings& settings, const QString& countKey,
		                      const QString& baseLookup, QList<QString>& colorList );
		void addColorListItem( QListWidget* list, const QColor& listColor );
		static const QString APPLICATION;
		static const QString ORGANIZATION;
		static const QString ACCUMULATED_COLOR;
		static const QString ACCUMULATED_COLOR_COUNT;
		static const QString VIEWER_COLORS;
		static const QString POLYLINE_UNIT;
		bool viewerColors;
		bool polylineUnit;
		const QString POLYGONAL_CHAIN;
		const QString LINE_SEGMENT;
		QStringList colorUnits;
		QList<QString> accumulateColorList;
		Ui::SliceColorPreferencesClass ui;
	};

}
#endif // SLICECOLORPREFERENCES_QO_H
