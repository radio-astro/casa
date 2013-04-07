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

#include "RangeControlsWidget.qo.h"
#include <images/Images/ImageInterface.h>
#include <lattices/Lattices/LatticeFractile.h>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>

namespace casa {

RangeControlsWidget::RangeControlsWidget(QWidget *parent)
    : QWidget(parent), percentCalculator(NULL) {
	ui.setupUi(this);
	ignoreRange = false;

	//Min & max bounds
	minMaxValidator = new QDoubleValidator( std::numeric_limits<double>::min(),
		std::numeric_limits<double>::max(), 10, this );
	ui.minLineEdit->setValidator( minMaxValidator );
	ui.maxLineEdit->setValidator( minMaxValidator );
	connect( ui.minLineEdit, SIGNAL(returnPressed()), this, SIGNAL(minMaxChanged()));
	connect( ui.maxLineEdit, SIGNAL(returnPressed()), this, SIGNAL(minMaxChanged()));

	//Percentile
	QStringList percentages = QStringList()<<"90"<<"95"<<"98"<<"99.5"<<"99.9";
	percentage = percentages[0];
	ui.percentileComboBox->addItems( percentages );
	ui.percentileComboBox->setEditable( true );
	QDoubleValidator* percentValidator = new QDoubleValidator( 0, 100, 2, this );
	ui.percentileComboBox->setValidator(percentValidator);
	connect( ui.percentileCheckBox, SIGNAL(toggled(bool)), this, SLOT(rangeModeChanged(bool)));
	connect( ui.percentileComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(percentageChanged(const QString&)));
	//connect( ui.percentileComboBox, SIGNAL( editTextChanged( const QString&)), this, SLOT(percentageChanged( const QString&)));
	rangeModeChanged( ui.percentileCheckBox->isChecked());

	connect( ui.clearRangeButton, SIGNAL(clicked()), this, SLOT(clearRange()));
}

void RangeControlsWidget::setRangeMaxEnabled( bool enabled ){
	ui.maxLineEdit->setEnabled( enabled );
}

void RangeControlsWidget::setRange( double min, double max, bool signalChange ){
	//Note:  because range changes can result in time intensive calculations,
	//we only want to send at most one range change signal.
	if ( !ignoreRange ){
		ui.minLineEdit->setText( QString::number( min ));
		ui.maxLineEdit->setText( QString::number( max ));
		if ( signalChange ){
			emit minMaxChanged();
		}
	}
}

void RangeControlsWidget::setImage( ImageInterface<float>* image ){
	this->image = image;
}

void RangeControlsWidget::percentageChanged( const QString& newPercentage ){
	bool validPercentage = false;
	float percent = newPercentage.toFloat(&validPercentage );
	int TOTAL_PERCENT = 100;
	if ( validPercentage && 0 < percent && percent < TOTAL_PERCENT ){
		percentage = newPercentage;
		float leftPercent = TOTAL_PERCENT - percent;
		//Generate a symmetric interval.
		float minValue = leftPercent / 2;
		float maxValue = TOTAL_PERCENT - leftPercent / 2;
		minValue = minValue / TOTAL_PERCENT;
		maxValue = maxValue / TOTAL_PERCENT;
		if ( image != NULL ){
			if ( percentCalculator != NULL ){
				delete percentCalculator;
				percentCalculator = NULL;
			}
			percentCalculator = new PercentageCalculator( minValue, maxValue, image );
			connect(percentCalculator, SIGNAL(finished()), this, SLOT(percentilesDone()));
			percentCalculator->start();
		}
		else {
			QMessageBox::warning( this, "Missing Image", "An image must be loaded in order to calculate a range based on a percentage.");
		}
	}
}

void RangeControlsWidget::percentilesDone(){
	if ( percentCalculator != NULL ){
		float minRange = percentCalculator->getRangeMin();
		float maxRange = percentCalculator->getRangeMax();
		setRange( minRange, maxRange );
	}
}

void RangeControlsWidget::setRangeLimits( double min, double max ){
	minMaxValidator->setBottom( min );
	minMaxValidator->setTop( max );
}

void RangeControlsWidget::clearRange(){
	setRange( rangeMin, rangeMax, true );
	emit rangeCleared();
}

pair<double,double> RangeControlsWidget::getMinMaxValues() const {
	QString minValueStr = ui.minLineEdit->text();
	QString maxValueStr = ui.maxLineEdit->text();
	double minValue = minValueStr.toDouble();
	double maxValue = maxValueStr.toDouble();
	if ( minValue > maxValue ){
		double tmp = minValue;
		minValue = maxValue;
		maxValue = tmp;
	}
	pair<double,double> maxMinValues(minValue,maxValue);
	return maxMinValues;
}

void RangeControlsWidget::setIgnoreRange( bool ignore ){
	ignoreRange = ignore;
}

void RangeControlsWidget::rangeModeChanged( bool percentile ){
	ui.percentileComboBox->setEnabled( percentile );
	ui.minLineEdit->setEnabled( !percentile );
	ui.maxLineEdit->setEnabled( !percentile );
	if ( percentile ){
		QString newPercentage = ui.percentileComboBox->currentText();
		percentageChanged( newPercentage );
	}
}

void RangeControlsWidget::keyPressEvent( QKeyEvent* event ){
	if ( event->key() == Qt::Key_Enter ){
		QString newPercentage = ui.percentileComboBox->currentText();
		if ( percentage != newPercentage ){
			percentageChanged( newPercentage );
		}
	}
	QWidget::keyPressEvent( event );
}


void RangeControlsWidget::setDataLimits( double min, double max ){
	if ( ignoreRange ){
		return;
	}
	rangeMin = min;
	rangeMax = max;
	/*if ( ui.minLineEdit->text().length() == 0 &&
			ui.maxLineEdit->text().length() == 0 ){


		setRange( rangeMin, rangeMax, false );
	}
	else {*/
	if ( ui.minLineEdit->text().length() != 0 ){
		//Reset the range limits if they don't make sense for the
		//new data.
		pair<double, double> minMaxRange = this->getMinMaxValues();
		double rangeLimitMin = minMaxRange.first;
		double rangeLimitMax = minMaxRange.second;
		if ( rangeLimitMin < min || rangeLimitMin > max ){
			rangeLimitMin = min;
		}
		if ( rangeLimitMax < min || rangeLimitMax > max ){
			rangeLimitMax = max;
		}

		//If we have a range, we should reset it.
		pair<double,double> oldRange = getMinMaxValues();
		if ( oldRange.first != 0 && oldRange.second != 0  ){
			double newLow = qMax( oldRange.first, rangeLimitMin );
			double newHigh = qMin( oldRange.second, rangeLimitMax );
			this->setRange( newLow, newHigh );
		}
	}
}

RangeControlsWidget::~RangeControlsWidget(){

}

//*************************************************************************
//                       Percentage Calculator
//*************************************************************************

PercentageCalculator::PercentageCalculator( float minValue, float maxValue, ImageInterface<float>* image ){
	this->minValue = minValue;
	this->maxValue = maxValue;
	this->image = image;
}

void PercentageCalculator::run(){
	Vector<Float> intensities = LatticeFractile<Float>::maskedFractiles (*image, minValue, maxValue);
	int intensityCount = intensities.nelements();
	if ( intensityCount == 2 ){
		rangeMin = intensities[0];
		rangeMax = intensities[1];
	}
}

float PercentageCalculator::getRangeMin() const {
	return rangeMin;
}

float PercentageCalculator::getRangeMax() const {
	return rangeMax;
}

PercentageCalculator::~PercentageCalculator() {

}
}
