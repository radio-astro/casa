/*
 * CanvasCurve.h
 *
 *  Created on: Jun 7, 2012
 *      Author: slovelan
 */

#ifndef CANVASCURVE_H_
#define CANVASCURVE_H_

#include <casa/aips.h>
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
			QString legend, QColor curveColor);
	QColor getColor() const;
	QString getLegend() const;
	CurveData& getCurveData();
	CurveData& getErrorData();
	QString getToolTip( double x, double y , const double X_ERROR,
			const double Y_ERROR, const QString& xUnit, const QString& yUnit ) const;
	void getMinMax(Double& xmin, Double& xmax, Double& ymin,
			Double& ymax, bool plotError ) const;
	virtual ~CanvasCurve();

private:
	QColor curveColor;
	QString legend;
	CurveData curveData;
	ErrorData errorData;
};

} /* namespace casa */
#endif /* CANVASCURVE_H_ */
