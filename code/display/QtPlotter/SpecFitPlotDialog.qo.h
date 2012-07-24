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
    void setFitCount( int count );
    void addPlots();
    ~SpecFitPlotDialog();

private:
    static void toDoubleVectors( const Vector<Float>& sourceXValues, const Vector<Float>& sourceYValues,
    		QVector<double>& destXValues, QVector<double>& destYValues );
    Ui::SpecFitPlotDialogClass ui;
    int fitCount;
    QVector<double> xVals;
    QVector<double> yVals;
    QList<QwtPlot*> plots;
    QwtPlot* summaryPlot;
};

}
#endif // SPECFITPLOTDIALOG_QO_H
