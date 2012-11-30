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
#include "FitWidget.qo.h"
#include <guitools/Histogram/Fitter.h>
#include <guitools/Histogram/FitterGaussian.h>
#include <guitools/Histogram/FitterPoisson.h>
#include <QMessageBox>
namespace casa {

FitWidget::FitWidget(QWidget *parent)
    : QWidget(parent){
	ui.setupUi(this);

	QDoubleValidator* posValidator = new QDoubleValidator( 0, std::numeric_limits<double>::max(), 10, this);
	QDoubleValidator* validator = new QDoubleValidator(std::numeric_limits<double>::min(),
			std::numeric_limits<double>::max(), 10, this);
	ui.gaussCenterLineEdit->setValidator( validator );
	ui.gaussPeakLineEdit->setValidator( validator );
	ui.gaussFWHMLineEdit->setValidator( validator );
	ui.poissonLambdaLineEdit->setValidator( posValidator );
	connect( ui.gaussCenterLineEdit, SIGNAL(textEdited( const QString&)), this, SLOT( centerEdited( const QString& )));
	connect( ui.gaussPeakLineEdit, SIGNAL(textEdited( const QString&)), this, SLOT( peakEdited( const QString& )));
	connect( ui.gaussFWHMLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT( fwhmEdited( const QString& )));
	connect( ui.poissonLambdaLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT( lambdaEdited( const QString&)));

	QStringList fitList =(QStringList()<< "Gaussian" << "Poisson");
	for ( int i = 0; i < fitList.size(); i++ ){
		ui.fitDistributionComboBox->addItem( fitList[i] );
	}
	connect( ui.fitDistributionComboBox, SIGNAL( currentIndexChanged( int )),
			this, SLOT(fitSelected(int)));

	fitter = NULL;
	fitterGaussian = new FitterGaussian();
	fitterPoisson = new FitterPoisson();
	fitter = fitterGaussian;
	connect( ui.fitButton, SIGNAL(clicked()), this, SLOT(doFit()));

}

bool FitWidget::isGaussian() const {
	int index = ui.fitDistributionComboBox->currentIndex();
	bool gaussianMode = false;
	if ( index == GAUSSIAN_MODE ){
		gaussianMode = true;
	}
	return gaussianMode;
}

double FitWidget::getCenter() const {
	double centerVal = ui.gaussCenterLineEdit->text().toDouble();
	return centerVal;
}

double FitWidget::getPeak() const {
	double peakVal = ui.gaussPeakLineEdit->text().toDouble();
	return peakVal;
}

double FitWidget::getFWHM() const {
	double fwhmVal = ui.gaussFWHMLineEdit->text().toDouble();
	return fwhmVal;
}

void FitWidget::setCenterPeak( double center, double peak ){
	resetFWHM( center );
	ui.gaussCenterLineEdit->setText( QString::number(center));
	ui.gaussPeakLineEdit->setText( QString::number(peak));
	fitterGaussian->setCenter( center );
	fitterGaussian->setPeak( peak );
}

void FitWidget::setFWHM( double fwhm ){
	QString centerStr = ui.gaussCenterLineEdit->text();
	double centerValue = centerStr.toDouble();
	double fwhmDistance = qAbs( centerValue - fwhm ) * 2;
	ui.gaussFWHMLineEdit->setText( QString::number(fwhmDistance));
	fitterGaussian->setFWHM( fwhmDistance );
}

void FitWidget::resetFWHM( double newCenter ){
	QString fwhmStr = ui.gaussFWHMLineEdit->text();
	double oldFwhmDistance = fwhmStr.toDouble();
	if ( oldFwhmDistance > 0 ){
		QString oldCenterStr = ui.gaussCenterLineEdit->text();
		double oldCenter = oldCenterStr.toDouble();
		double fwhmPt = oldCenter + oldFwhmDistance / 2;
		double newFwhmDistance = qAbs( fwhmPt - newCenter ) * 2;
		ui.gaussFWHMLineEdit->setText( QString::number( newFwhmDistance ));
	}
}

void FitWidget::centerEdited( const QString& centerText ){
	bool validValue = false;
	double center = centerText.toDouble(&validValue);
	if ( validValue ){
		fitterGaussian->setCenter( center );
		emit gaussianFitChanged();
	}
}

void FitWidget::peakEdited( const QString& peakText ){
	bool validValue = false;
	double peak = peakText.toDouble(&validValue);
	if ( validValue ){
		fitterGaussian->setPeak( peak );
		emit gaussianFitChanged();
	}
}

void FitWidget::fwhmEdited( const QString& fwhmText ){
	bool validValue = false;
	double fwhm = fwhmText.toDouble(&validValue);
	if ( validValue ){
		fitterGaussian->setFWHM( fwhm );
		emit gaussianFitChanged();
	}
}

void FitWidget::lambdaEdited( const QString& lambdaText ){
	bool validValue = false;
	double lambda = lambdaText.toDouble(&validValue );
	if ( validValue ){
		fitterPoisson->setLambda( lambda );
		emit poissonFitChanged();
	}
}

void FitWidget::setXValues( Vector<Float> dataValues ){
	xValues = dataValues;
}

void FitWidget::fitSelected( int index ){
	ui.fitStackedWidget->setCurrentIndex( index );
	if ( index == GAUSSIAN_MODE ){
		fitter = fitterGaussian;
	}
	else if ( index == POISSON_MODE ){
		fitter = fitterPoisson;
	}
	emit fitModeChanged();
}

//-----------------------------------------------------------
//                    Fitting
//-----------------------------------------------------------

Vector<Float> FitWidget::getFitValues(){
	Vector<Float> fitValues;
	if ( fitter != NULL ){
		fitValues = fitter->getFitValues();
	}
	return fitValues;
}

void FitWidget::doFit() {
	fitter->setData( xValues );
	const QString errorMsg;
	bool successfulFit = fitter->doFit();
	if ( successfulFit ){
		emit dataFitted();
	}
	else {
		QMessageBox::warning( this, "Problem Fitting Data", errorMsg );
	}
}

FitWidget::~FitWidget(){
	delete fitterGaussian;
	delete fitterPoisson;
}
}
