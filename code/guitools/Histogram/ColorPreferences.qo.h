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
#ifndef HISTOGRAM_COLOR_PREFERENCES_QO_H_2
#define HISTOGRAM_COLOR_PREFERENCES_QO_H_2

#include <QtGui/QDialog>

#include <guitools/Histogram/ColorPreferences.ui.h>

class QSettings;

namespace casa {

/**
 * Allows user specification of the colors used in drawing
 * the histogram and pluggable functionality.
 */

class ColorPreferences : public QDialog {
    Q_OBJECT

public:
    ColorPreferences(QWidget *parent = 0);
    void setFitColorsVisible( bool visible );
    QColor getHistogramColor() const;
    QColor getFitEstimateColor() const;
    QColor getFitCurveColor() const;
    ~ColorPreferences();

signals:
	void colorsChanged();

private slots:
	void colorsAccepted();
	void colorsRejected();
	void selectHistogramColor();
	void selectFitCurveColor();
	void selectFitEstimateColor();

private:
    void initializeUserColors();
    QString readCustomColor( QSettings& settings, const QString& identifier);
    void setButtonColor( QPushButton* button, QColor color );
    QColor getButtonColor( QPushButton* button ) const;
    void showColorDialog( QPushButton* source );
    void persistColors();
    void resetColors();

    QColor histogramColor;
    QColor fitEstimateColor;
    QColor fitCurveColor;
    Ui::ColorPreferencesClass ui;
    static const QString APPLICATION;
    static const QString ORGANIZATION;
    static const QString HISTOGRAM_COLOR;
    static const QString FIT_ESTIMATE_COLOR;
    static const QString FIT_CURVE_COLOR;
};
}
#endif // PREFERENCESCOLOR_QO_H
