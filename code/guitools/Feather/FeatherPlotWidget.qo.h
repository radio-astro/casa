#ifndef FEATHERPLOTWIDGET_QO_H
#define FEATHERPLOTWIDGET_QO_H

#include <QtGui/QWidget>

#include <guitools/Feather/FeatherPlotWidget.ui.h>
#include <guitools/Feather/FeatherPlot.h>
#include <casa/Arrays/Vector.h>
#include <qwt_plot.h>
#include <qwt_legend.h>
#include <pair.h>
using namespace std;

namespace casa {
class FeatherPlotWidget : public QWidget
{
    Q_OBJECT

public:
    FeatherPlotWidget(const QString& title, bool originalData, QWidget *parent = 0);
    void setSingleDishWeight( const Vector<Float>& xValues, const Vector<Float>& yValues );
    void setInterferometerWeight( const Vector<Float>& xValues, const Vector<Float>& yValues );
    void setSingleDishData( const Vector<Float>& xValues, const Vector<Float>& yValues );
    void setInterferometerData( const Vector<Float>& xValues, const Vector<Float>& yValues );
    void resetPlot( FeatherPlot::PlotType plotType );
    void setPlotColors( QColor dataSDColor, QColor dataINTColor,
    		QColor weightSDColor, QColor weightINTColor, QColor scatterPlotColor );
    void setLineThickness( int thickness );
    void setLegendVisibility( bool v );
    ~FeatherPlotWidget();
protected:
    virtual void contextMenuEvent( QContextMenuEvent* event );

private slots:
	void changePlotType();
	void changeZoom90();
	void zoomNeutral();

private:
	void resetColors();
    void initializeActions();
    void initializeRangeLimitedData( double minValue, double maxValue,
    		QVector<double>& xValues, QVector<double>& yValues,
    		const QVector<double>& originalXValues, const QVector<double>& originalYValues,
    		Double* xMin, Double* xMax ) const;
    void initializeDomainLimitedData( double minValue, double maxValue,
        		QVector<double>& xValues, QVector<double>& yValues,
        		const QVector<double>& originalXValues, const QVector<double>& originalYValues) const;
    pair<double,double> getMaxMin( QVector<double> values ) const;
    bool getPracticalLegendVisibility() const;
    void addZoomNeutralCurves();
    void addZoomNeutralScatterPlot();
    FeatherPlot* plot;

    QColor singleDishWeightColor;
    QColor singleDishDataColor;
    QColor interferometerWeightColor;
    QColor interferometerDataColor;
    QColor scatterPlotColor;

    QVector<double> singleDishWeightXValues;
    QVector<double> singleDishWeightYValues;
    QVector<double> interferometerWeightXValues;
    QVector<double> interferometerWeightYValues;
    QVector<double> singleDishDataXValues;
    QVector<double> singleDishDataYValues;
    QVector<double> interferometerDataXValues;
    QVector<double> interferometerDataYValues;


    QAction plotTypeAction;
    QAction zoom90Action;
    QAction zoomNeutralAction;
    QMenu contextMenu;

    QString singleDishFunction;
    QString interferometerFunction;
    QString singleDishWeightFunction;
    QString interferometerWeightFunction;
    QString singleDishDataFunction;
    QString interferometerDataFunction;

    bool mouseMove;
    bool legendVisible;
    bool originalData;
    int lineThickness;
    QString plotTitle;
    Ui::FeatherPlotWidgetClass ui;
};

}
#endif // FEATHERPLOTWIDGET_H
