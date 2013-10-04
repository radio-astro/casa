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
#ifndef FITWIDGET_QO_H
#define FITWIDGET_QO_H

#include <QtGui/QWidget>
#include <QTextStream>
#include <guitools/Histogram/FitWidget.ui.h>
#include <casa/Arrays/Vector.h>

namespace casa {

class Fitter;
class FitterGaussian;
class FitterPoisson;

/**
 * Main class for handling the pluggable fit functionality for
 * the histogram.
 */

class FitWidget : public QWidget {
    Q_OBJECT

public:
    FitWidget(QWidget *parent = 0);
    void setCenterPeak( double center, double peak );
    void setFWHM( double fwhm );
    void setLambda( double lambda );
    void setUnits( const QString& units );
    void setValues( int id, Vector<Float> xValues, Vector<Float> yValues );
    double getCenter() const;
    double getPeak() const;
    double getFWHM() const;
    double getLambda() const;
    Vector<Float> getFitValues() const;
    Vector<Float> getFitValuesX() const;
    bool isGaussian() const;
    void toAscii( QTextStream& out) const;
    void restrictDomain( double xMin, double xMax );
    void clearDomainLimits();
    ~FitWidget();

signals:
	void fitModeChanged();
	void dataFitted(const QString& msg);
	void gaussianFitChanged();
	void poissonFitChanged();
	void fitCleared();

public slots:
	void clearFit();

private slots:
	void centerEdited( const QString& centerText );
	void peakEdited( const QString& peakText );
	void fwhmEdited( const QString& fwhmText );
	void lambdaEdited( const QString& lambdaText );
	void fitSelected( int index );
	void doFit();

private:
	FitWidget( const FitWidget& );
	FitWidget& operator=( const FitWidget& );
	void setSolutionVisible( bool visible );
	void resetFWHM( double newCenter );
	enum FitMode {GAUSSIAN_MODE, POISSON_MODE, NO_MODE };
	Fitter* fitter;
	FitterGaussian* fitterGaussian;
	FitterPoisson* fitterPoisson;
    Ui::FitWidgetClass ui;
    int fitId;
};
}
#endif // FITWIDGET_QO_H
