/*
 * FeatherPlot.h
 *
 *  Created on: Oct 23, 2012
 *      Author: slovelan
 */

#ifndef FEATHERPLOT_H_
#define FEATHERPLOT_H_

#include <QMenu>
#include <QList>
#include <qwt_plot.h>
#include <qwt_legend.h>

class QwtPlotZoomer;
class QwtPlotCurve;

namespace casa {

class FeatherPlot : public QwtPlot {
public:
	FeatherPlot(QWidget* parent );
	enum PlotType { SLICE_CUT, SCATTER_PLOT, ORIGINAL };
	virtual ~FeatherPlot();
	void clearCurves();
	void addCurve( QVector<double> xValues, QVector<double> yValues,
			QColor curveColor, const QString& curveTitle, QwtPlot::Axis yAxis );
	void initializePlot( const QString& title, PlotType plotType );
	//void zoomNeutral();
	FeatherPlot::PlotType getPlotType() const;
	void setFunctionColor( const QString& curveID, const QColor& color );
	void setLineThickness( int thickness );
	void setLegendVisibility( bool visible );
private:
	int getCurveIndex( const QString& curveTitle ) const;
	void setCurveLineThickness( int curveIndex );
	QwtLegend legend;
	QwtPlotZoomer *zoomer;
	QwtPlotZoomer *zoomerRight;
	PlotType plotType;
	bool legendVisible;
	int lineThickness;

	const int DOT_FACTOR;
    QList<QwtPlotCurve*> curves;
};

} /* namespace casa */
#endif /* FEATHERPLOT_H_ */
