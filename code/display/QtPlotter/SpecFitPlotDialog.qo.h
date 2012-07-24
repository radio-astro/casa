#ifndef SPECFITPLOTDIALOG_QO_H
#define SPECFITPLOTDIALOG_QO_H

#include <QtGui/QWidget>
#include <QList>
#include <casa/Arrays/Vector.h>
#include <display/QtPlotter/SpecFitPlotDialog.ui.h>

class QwtPlot;
namespace casa {

class SpecFitPlotDialog : public QDialog
{
    Q_OBJECT

public:
    SpecFitPlotDialog(QWidget *parent = 0);
    void setData( Vector<Float> xValues, Vector<Float> yValues );
    void addPlot( const Vector<Float>& xValues, const Vector<Float>& yValues, QColor color );
    ~SpecFitPlotDialog();

private:
    static void toDoubleVectors( const Vector<Float>& sourceXValues, const Vector<Float>& sourceYValues,
    		QVector<double>& destXValues, QVector<double>& destYValues );
    Ui::SpecFitPlotDialogClass ui;
    QVector<double> xVals;
    QVector<double> yVals;
    QList<QwtPlot*> plots;
};

}
#endif // SPECFITPLOTDIALOG_QO_H
