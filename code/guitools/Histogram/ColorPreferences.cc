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
#include "ColorPreferences.qo.h"
namespace casa {

const QString ColorPreferences::APPLICATION = "Histogram";
const QString ColorPreferences::ORGANIZATION = "NRAO/CASA";
const QString ColorPreferences::HISTOGRAM_COLOR = "Histogram Color";
const QString ColorPreferences::FIT_ESTIMATE_COLOR = "Histogram Fit Estimate Color";
const QString ColorPreferences::FIT_CURVE_COLOR = "Histogram Fit Curve Color";

ColorPreferences::ColorPreferences(QWidget *parent)
    : QDialog(parent),
      histogramColor(Qt::blue), fitEstimateColor(Qt::green), fitCurveColor(Qt::cyan){
	ui.setupUi(this);
	setWindowTitle( "Histogram Color Preferences");

	initializeUserColors();
}

void ColorPreferences::initializeUserColors(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( ORGANIZATION, APPLICATION );

	QString histogramColorName = readCustomColor( settings, HISTOGRAM_COLOR );
	if ( histogramColorName.length() > 0 ){
		histogramColor = QColor( histogramColorName );
	}

	QString fitEstimateColorName = readCustomColor( settings, FIT_ESTIMATE_COLOR );
	if ( fitEstimateColorName.length() > 0 ){
		fitEstimateColor = QColor( fitEstimateColorName );
	}

	QString fitCurveColorName = readCustomColor( settings, FIT_CURVE_COLOR );
	if ( fitCurveColorName.length() > 0 ){
		fitCurveColor = QColor( fitCurveColorName );
	}
}

QString ColorPreferences::readCustomColor( QSettings& settings, const QString& identifier){
	QString colorName = settings.value( identifier, "" ).toString();
	return colorName;
}

ColorPreferences::~ColorPreferences(){

}
}
