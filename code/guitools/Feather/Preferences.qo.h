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
#ifndef PREFERENCES_QO_H
#define PREFERENCES_QO_H

#include <QtGui/QDialog>
#include <guitools/Feather/Preferences.ui.h>

namespace casa {

class Preferences : public QDialog
{
    Q_OBJECT

public:
    const static QString ORGANIZATION;
    const static QString APPLICATION;
    Preferences(QWidget *parent = 0);
    void setPlaneCount( int count );
    bool isDisplayOutputFunctions() const;
    bool isDisplayLegend() const;
    bool isDisplayOutputScatterPlot() const;
    bool isDisplayX() const;
    bool isDisplayY() const;
    bool isLogAmplitude() const;
    bool isLogUV() const;
    bool isXAxisUV() const;
    bool isPlaneAveraged() const;
    int getPlaneIndex() const;
    int getLineThickness() const;
    int getDotSize() const;
    ~Preferences();


signals:
	void preferencesChanged();

private slots:
	void preferencesAccepted();
	void preferencesRejected();
	void xAxisChanged();
	void planeModeChanged();

private:
	void initializeCustomSettings();
	void persist();
	void reset();

	const static QString LINE_THICKNESS;
	const static QString DISPLAY_OUTPUT_FUNCTIONS;
	const static QString DISPLAY_LEGEND;
	const static QString DISPLAY_OUTPUT_SCATTERPLOT;
	const static QString DISPLAY_Y_PLOTS;
	const static QString DISPLAY_X_PLOTS;
	const static QString DOT_SIZE;
	const static QString LOG_AMPLITUDE;
	const static QString LOG_UV;
	const static QString DISPLAY_X_AXIS_UV;
	const static QString PLANE_AVERAGED;

    Ui::PreferencesClass ui;
    int lineThickness;
    int dotSize;
    bool displayOutputFunctions;
    bool displayOutputScatterPlot;
    bool displayYPlots;
    bool displayXPlots;
    bool displayLegend;
    bool logAmplitude;
    bool logUV;
    bool xAxisUV;
    bool planeAveraged;
    int planeIndex;
};

}

#endif // PREFERENCES_H
