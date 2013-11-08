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

#ifndef COLOR_SUMMARYWIDGET_QO_H
#define COLOR_SUMMARYWIDGET_QO_H

#include <QtGui/QWidget>
#include <QSettings>
#include <display/QtPlotter/ColorSummaryWidget.ui.h>

namespace casa {

	class QtCanvas;

	class ColorSummaryWidget : public QDialog {
		Q_OBJECT

	public:
		ColorSummaryWidget(QWidget *parent = 0);
		~ColorSummaryWidget();
		void setColorCanvas( QtCanvas* colorCanvas );

	private slots:

		//Methods to add colors in various categories
		void addColorProfile();
		void addColorFit();
		void addColorFitSummary();
		void channelLineColorChanged();
		void molecularLineColorChanged();
		void zoomRectColorChanged();
		void initialGaussianEstimateColorChanged();

		//Methods to remove colors in various categories
		void removeColorProfile();
		void removeColorFit();
		void removeColorFitSummary();

		//User changed to a different color scheme (traditional,
		//alternative, custom, etc)
		void colorSchemeChanged();

		//Stores any changes to the colors used by the application
		//before closing the dialog.
		void reject();
		void accept();

	private:
		void addColor( QListWidget* list );
		void removeColor( QListWidget* list );
		void initializeColors();
		void addColorListItem( QListWidget* list, const QColor& listColor );
		void readCustomColor( QSettings& settings, const QString& countKey,
		                      const QString& baseLookup, QList<QString>& list );
		void populateColorList( const QList<QString>& colors, QListWidget* list );
		void copyViewList(QListWidget* listWidget, QList<QString>& canvasList);
		void copyViewLists();
		void initializePresetColors();
		void initializeUserColors();
		void populateColorLists();
		void clearColorLists();
		void registerColorChange();
		void clearColorChange();
		void pixelCanvasColorChange();
		void persistColorList( QSettings& settings, QListWidget* list,
		                       const QString& baseStr, const QString& countStr );
		void persist();
		void setLabelColor( QLabel* label, QString colorName  );

		Ui::ColorSummaryWidget ui;
		QtCanvas* pixelCanvas;
		QList<QString> mainCurveColorList;
		QList<QString> fitCurveColorList;
		QList<QString> fitSummaryCurveColorList;
		QList<QString>  traditionalCurveColorList;
		QList<QString> customMainList;
		QList<QString> customFitList;
		QList<QString> customFitSummaryList;
		QColor channelLineColor;
		QColor zoomRectColor;
		QColor molecularLineColor;
		QColor initialGaussianEstimateColor;
		enum ColorCategory {MAIN_COLOR, FIT_COLOR, SUMMARY_FIT_COLOR, END_COLOR_CATEGORY };
		enum SchemeCategory {TRADITIONAL,ALTERNATIVE, CUSTOM };

		static const QString CUSTOM_PROFILE_COLOR;
		static const QString CUSTOM_FIT_COLOR;
		static const QString CUSTOM_SUMMARY_COLOR;
		static const QString CUSTOM_PROFILE_COLOR_COUNT;
		static const QString CUSTOM_FIT_COLOR_COUNT;
		static const QString CUSTOM_SUMMARY_COLOR_COUNT;
		static const QString COLOR_SCHEME_PREFERENCE;
		static const QString CHANNEL_LINE_COLOR;
		static const QString INITIAL_GAUSSIAN_ESTIMATE_COLOR;
		static const QString MOLECULAR_LINE_COLOR;
		static const QString ZOOM_RECT_COLOR;

		bool traditionalChange;
		bool alternativeChange;
	};
}

#endif // COLORSUMMARYWIDGET_Q0_H
