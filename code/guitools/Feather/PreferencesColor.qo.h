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
#ifndef PREFERENCESCOLOR_QO_H
#define PREFERENCESCOLOR_QO_H

#include <QtGui/QDialog>
#include <QMap>

#include <guitools/Feather/PreferencesColor.ui.h>
#include <guitools/Feather/CurveDisplay.h>
#include <guitools/Feather/FeatherCurveType.h>

namespace casa {

class PreferencesFunction;

/**
 * Manages the display properties of all the curves that can be shown on
 * the plots.
 */

class PreferencesColor : public QDialog
{
    Q_OBJECT

public:
    PreferencesColor(QWidget *parent = 0);
    typedef FeatherCurveType::CurveType CurveType;

    QMap<CurveType,CurveDisplay> getFunctionColors() const;
    CurveType getScatterXCurve() const;
    QList<CurveType> getScatterYCurve() const;

    void setDirtyEnabled( bool enabled );

    ~PreferencesColor();

signals:
	void colorsChanged();

private slots:
	void colorsAccepted();
	void colorsRejected();
	void populateScatterAxes();
	void populateScatterAxisY();

private:
	void initializeUser();
	void initializeCurvePreferences();
	void saveScatterSettings();
	void resetScatterSettings();
	void populateScatterAxisX();
	QList<QString> getCurveNames();
	CurveType getType( const QString& title ) const;
	CurveType findCurve( const QString& title ) const;
	void setCurveDefaults();
	void addCurvePreferences();
    void reset();
    void persist();

    Ui::PreferencesColorClass ui;

    void addCurvePreference( QWidget* holder, CurveType index );
    QMap<CurveType,PreferencesFunction*> curvePreferences;
    CurveType scatterXIndex;
    QList<CurveType> scatterYIndices;
    static const QString SCATTER_X_CURVE;
    static const QString SCATTER_Y_CURVES;

};
}

#endif // PREFERENCESCOLOR_QO_H
