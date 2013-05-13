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
#include <QSettings>
#include <guitools/Feather/PreferencesColor.ui.h>

namespace casa {

class PreferencesFunction;

class PreferencesColor : public QDialog
{
    Q_OBJECT

public:
    PreferencesColor(QWidget *parent = 0);
    enum FunctionColor {SD_WEIGHT_COLOR,SD_SLICE_COLOR,
    	INT_WEIGHT_COLOR,INT_SLICE_COLOR, END_COLOR };
    QMap<FunctionColor,QColor> getFunctionColors() const;
    QColor getScatterPlotColor() const;
    QColor getDishDiameterLineColor() const;
    QColor getZoomRectColor() const;
    QColor getSumColor() const;
    ~PreferencesColor();

signals:
	void colorsChanged();

private slots:
	//void selectSDWeightColor();
	//void selectSDSliceColor();
	//void selectINTWeightColor();
	//void selectINTSliceColor();
	//void selectScatterPlotColor();
	void selectDishDiameterLineColor();
	void selectZoomRectColor();
	//void selectSumColor();
	void colorsAccepted();
	void colorsRejected();

private:
	void initializeUserColors();
	void initializeCurvePreferences();
	//void storeCustomColor( QSettings& settings, FunctionColor index );
	//void storeMapColor( QPushButton* button, FunctionColor colorType );
	QString readCustomColor( QSettings& settings, int index);
    QColor getButtonColor( QPushButton* button ) const;
    void setButtonColor( QPushButton* button, QColor color );
    void showColorDialog( QPushButton* source );
    void resetColors();
    void persistColors();

    Ui::PreferencesColorClass ui;
    //QMap<FunctionColor,QColor> colorMap;
    QColor scatterPlotColor;
    QColor sumColor;
    QColor dishDiameterLineColor;
    QColor zoomRectColor;
    const int SCATTER_INDEX;
    const int SUM_INDEX;
    const int DISH_DIAMETER_INDEX;
    const int ZOOM_INDEX;

    typedef enum CurveTypes {SLICE_LOW,SLICE_HIGH,SLICE_SUM,SLICE_DIRTY,
    	WEIGHT_LOW,WEIGHT_HIGH,SCATTER_HIGH,SCATTER_DIRTY, CURVES_END };
    QMap<CurveTypes,PreferencesFunction*> curvePreferences;

    static const QString FUNCTION_COLOR;
};
}

#endif // PREFERENCESCOLOR_QO_H
