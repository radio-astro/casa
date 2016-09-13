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
#include <QDebug>
#include <limits>

namespace casa {

FitWidget::FitWidget(QWidget *parent)
    : QWidget(parent){
	ui.setupUi(this);
	setSolutionVisible( false );

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

	fitId = -1;
	fitter = NULL;
	fitterGaussian = new FitterGaussian();
	fitterPoisson = new FitterPoisson();
	fitter = fitterGaussian;
	ui.fitStackedWidget->setCurrentIndex(0);
	connect( ui.fitButton, SIGNAL(clicked()), this, SLOT(doFit()));
	connect( ui.clearFitButton, SIGNAL(clicked()), this, SLOT(clearFit()));
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

double FitWidget::getLambda() const {
	double lambdaVal = ui.poissonLambdaLineEdit->text().toDouble();
	return lambdaVal;
}

void FitWidget::setUnits( const QString& units ){
	fitter->setUnits( units );
}

void FitWidget::setLambda( double lambda ){
	ui.poissonLambdaLineEdit->setText( QString::number(lambda));
	fitterPoisson->setLambda( lambda );
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

void FitWidget::setValues( int id, Vector<Float> dataValuesX, Vector<Float> dataValuesY ){
	fitId = id;
	fitterPoisson->setData( dataValuesX, dataValuesY );
	fitterGaussian->setData( dataValuesX, dataValuesY );
}

void FitWidget::fitSelected( int index ){
	int oldIndex = ui.fitStackedWidget->currentIndex();
	if ( oldIndex != index ){
		clearFit();
		ui.fitStackedWidget->setCurrentIndex( index );
		if ( index == GAUSSIAN_MODE ){
			fitter = fitterGaussian;
		}
		else if ( index == POISSON_MODE ){
			fitter = fitterPoisson;
		}
		emit fitModeChanged();
	}
}

void FitWidget::restrictDomain( double xMin, double xMax ){
	fitterGaussian->restrictDomain( xMin, xMax );
	fitterPoisson->restrictDomain( xMin, xMax );
}

void FitWidget::clearDomainLimits(){
	fitterGaussian->clearDomainLimits();
	fitterPoisson->clearDomainLimits();
}

//-----------------------------------------------------------
//                    Fitting
//-----------------------------------------------------------

Vector<Float> FitWidget::getFitValues() const {
	Vector<Float> fitValues;
	if ( fitter != NULL ){
		fitValues = fitter->getFitValues();
	}
	return fitValues;
}

Vector<Float> FitWidget::getFitValuesX() const {
	Vector<Float> fitValuesX;
	if ( fitter != NULL ){
		fitValuesX = fitter->getFitValuesX();
	}
	return fitValuesX;
}

void FitWidget::setSolutionVisible( bool visible ){
	ui.fitResultsTextEdit->setVisible( visible );
	ui.line->setVisible( visible );
}

void FitWidget::doFit() {
	/*QString rmsStr = ui.rmsLineEdit->text();
	double rmsVal = rmsStr.toDouble();
	if ( rmsVal <= 0 ){
		QMessageBox::warning( this, "RMS Invalid", "Please specify a positive value for fit rms.");
		return;
	}
	fitter->setRMS( rmsVal );*/
	//fitter->setRMS( std::numeric_limits<double>::max());
	bool successfulFit = fitter->doFit();
	setSolutionVisible( successfulFit );
	if ( successfulFit ){
		//We keep the initial estimates as they are, but update
		//the solution statistics.
		QString solutionStats = fitter->getSolutionStatistics();
		ui.fitResultsTextEdit->setText( solutionStats );
		QString statusMessage = fitter->getStatusMessage();
		emit dataFitted(statusMessage);
	}
	else {
		QString errorMsg = fitter->getErrorMessage();
		QString postMessage = "Error fitting the data.";
		if ( errorMsg.trimmed().length() > 0 ){
			postMessage = errorMsg;
		}
		QMessageBox::warning( this, "Fit Error", postMessage );
	}
}

void FitWidget::clearFit(){
	fitterPoisson->clearFit();
	fitterGaussian->clearFit();
	fitId = -1;
	ui.gaussCenterLineEdit->setText("");
	ui.gaussPeakLineEdit->setText("");
	ui.gaussFWHMLineEdit->setText("");
	ui.poissonLambdaLineEdit->setText("");
	setSolutionVisible( false );
	emit fitCleared();
}

FitWidget::~FitWidget(){
	delete fitterGaussian;
	delete fitterPoisson;
}

void FitWidget::toAscii( QTextStream& out) const {
	if ( fitter->isFit() ){
		QString fitTitle("#Fit Information");
		if ( fitId != -1 ){
			fitTitle.append( " for region "+QString::number( fitId) );
		}
		out << fitTitle << "\n";
		fitter->toAscii(out);
	}
}
}
