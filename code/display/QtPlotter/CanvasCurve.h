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
#ifndef CANVASCURVE_H_
#define CANVASCURVE_H_

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <vector>
#include <QColor>
#include <QString>

namespace casa {

typedef std::vector<double> CurveData;
typedef std::vector<double> ErrorData;

class CanvasCurve {
public:
	CanvasCurve();
	CanvasCurve( CurveData curveData, ErrorData errorData,
			QString legend, QColor curveColor, int curveType );
	QColor getColor() const;
	void setColor( QColor color );
	QString getLegend() const;
	void setLegend( const QString& legend );
	double convertValue( double value, const QString& oldUnits, const QString& newUnits ) const;
	void scaleYValues( const QString& oldDisplayUnits, const QString& yUnitDisplay );
	int getCurveType() const;
	CurveData getCurveData();
	Vector<float> getXValues() const;
	Vector<float> getYValues() const;
	CurveData getErrorData();
	void storeData( const QString& oldUnits );
	QString getToolTip( double x, double y , const double X_ERROR,
			const double Y_ERROR, const QString& xUnit, const QString& yUnit ) const;
	void getMinMax(Double& xmin, Double& xmax, Double& ymin,
			Double& ymax, bool plotError ) const;
	virtual ~CanvasCurve();

private:
	double getMax() const;
	void scaleYValuesCurve( const QString& oldUnits, const QString& newUnits );
	void scaleYValuesError( const QString& oldUnits, const QString& newUnits );
	double percentToValue( double yValue ) const;
	double valueToPercent( double yValue ) const;
	double convert( double yValue, const QString oldUnits, const QString newUnits ) const;
	String adjustForKelvin( const QString& units, const QString& otherUnits ) const;
	static const QString FRACTION_OF_PEAK;
	static const QString JY_BEAM;
	static const QString JY_BEAM_SHORT;
	static const QString KELVIN;
	static const QString KELVIN_SHORT;
	QColor curveColor;
	QString legend;
	CurveData curveData;
	ErrorData errorData;
	QString storedUnits;
	double storedMax;
	int curveType;
};

} /* namespace casa */
#endif /* CANVASCURVE_H_ */
