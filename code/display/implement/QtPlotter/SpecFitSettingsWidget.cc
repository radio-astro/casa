#include <display/QtPlotter/SpecFitSettingsWidget.qo.h>
#include <display/QtPlotter/SpecFitSettingsPolynomialWidget.qo.h>
#include <display/QtPlotter/SpecFitSettingsGaussWidget.qo.h>
#include <display/QtPlotter/SpecFitSettingsRangeWidget.qo.h>
#include <display/QtPlotter/SpecFitEstimateDialog.qo.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <QtGui>
#include <QTableWidgetItem>
#include <QHBoxLayout>
#include <assert.h>

namespace casa {


SpecFitSettingsWidget::SpecFitSettingsWidget(QWidget *parent)
    : QWidget(parent), NEW_ESTIMATE(-1), pixelCanvas( NULL )
{
	ui.setupUi(this);

	QHBoxLayout* hbox = new QHBoxLayout();
	rangeWidget = new SpecFitSettingsRangeWidget( this );
	polyWidget = new SpecFitSettingsPolynomialWidget( this );
	gaussWidget = new SpecFitSettingsGaussWidget( this );
	hbox -> addWidget( rangeWidget );
	hbox -> addWidget( polyWidget );
	hbox -> addWidget( gaussWidget );
	ui.settingsArea -> setLayout( hbox );


	connect( ui.fitButton, SIGNAL(clicked()), this, SLOT(specLineFit()));
	connect( ui.cleanButton, SIGNAL(clicked()), this, SLOT(showMainCurve()));



}

void SpecFitSettingsWidget::setCanvas( QtCanvas* pCanvas ){
	pixelCanvas = pCanvas;
	connect(pixelCanvas, SIGNAL(xRangeChanged(float, float)), this, SLOT(setCollapseRange(float,float)));

}





void SpecFitSettingsWidget::setCollapseRange(float start, float end ){

	rangeWidget -> setCollapseRange( start, end );
}

void SpecFitSettingsWidget::showGaussEstimateDialog( int estimateId ){
	GaussFitEstimate* estimate = NULL;
	/*if ( estimateId == NEW_ESTIMATE ){
		estimate = new GaussFitEstimate();
	}
	else {
		assert( 0 <= estimateId && estimateId <= estimates.size());
		estimate = estimates[estimateId];
	}
	SpecFitEstimateDialog estimateDialog( estimate, this );
	int result = estimateDialog.exec();
	qDebug()<< "Showed dialog result=" << result << " accepted is "<< QDialog::Accepted;
	if ( result == QDialog::Accepted ){
		estimateDialog.updateEstimate( estimate );
		if ( estimateId == NEW_ESTIMATE ){
			estimates.push_back( estimate );
			updateTable();
		}
	}
	else {
		delete estimate;
	}*/
}



void SpecFitSettingsWidget::showMainCurve(){

}


void SpecFitSettingsWidget::specLineFit(){

	/**itsLog << LogOrigin("QtProfile", "SpectralLineFit");

	// get the values
	float startVal = chanMinLineEdit -> text().toFloat();
	float endVal = chanMaxLineEdit -> text().toFloat();

	bool doFitGauss = gaussRadioButton->isChecked();
	bool doFitPoly = !doFitGauss;
	int polyN = -1;
	if ( !doFitPoly ){
		polyN = polyOrderSpinBox->value();
	}

	// do the fit
	String  msg;
	if (!fitter->fit(z_xval, z_yval, z_eval, startVal, endVal, doFitGauss, doFitPoly, polyN, msg)){
		//msg = String("Data could not be fitted!");
		profileStatus->showMessage(QString(msg.c_str()));
	}
	else{
		if (fitter->getStatus() == SpectralFitter::SUCCESS){
			// get the fit values
			Vector<Float> z_xfit, z_yfit;
			fitter->getFit(z_xval, z_xfit, z_yfit);
			// report problems
			if (z_yfit.size()<1){
				msg = String("There exist no fit values!");
				*itsLog << LogIO::WARN << msg << LogIO::POST;
				profileStatus->showMessage(QString(msg.c_str()));
				return;
			}

			// overplot the fit values
			QString fitName = fileName + "FIT" + QString::number(startVal)
				+ "-" + QString::number(endVal) + QString(xaxisUnit.c_str());
			qDebug() << "Spec Line fit values are:";
			for ( int i = 0; i < z_xfit.size(); i++ ){
				qDebug() << "x="<<z_xfit[i] << " y=" <<z_yfit[i];
			}
			pixelCanvas->addPolyLine(z_xfit, z_yfit, fitName);
		}
		profileStatus->showMessage(QString((fitter->report(*itsLog, xaxisUnit, String(yUnit.toLatin1().data()), String(yUnitPrefix.toLatin1().data()))).c_str()));
	}*/
}


SpecFitSettingsWidget::~SpecFitSettingsWidget()
{
	/*while( !estimates.isEmpty() ){
		delete estimates.takeFirst();
	}*/
}
} // end namespace
