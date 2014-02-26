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
#include <limits>

namespace casa {

RangeControlsWidget::RangeControlsWidget(QWidget *parent)
    : QWidget(parent), percentCalculator(NULL) {
	ui.setupUi(this);
	ignoreRange = false;

	//Min & max bounds
	float maxFloat = std::numeric_limits<double>::max();
	minMaxValidator = new QDoubleValidator( -1 * maxFloat, maxFloat, 10, this );
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
	rangeModeChanged( ui.percentileCheckBox->isChecked());

	connect( ui.clearRangeButton, SIGNAL(clicked()), this, SLOT(clearRange()));
}

void RangeControlsWidget::hideMaximum(){
	ui.rangeLayout->removeItem( ui.maxLayout );
	ui.maxLayout->setParent( NULL );
	ui.maxLineEdit->setParent( NULL );
	ui.maxLabel->setParent( NULL );
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

void RangeControlsWidget::setImage(const std::tr1::shared_ptr<const ImageInterface<Float > > image ){
	this->image = image;
	Unit unit = this->image->units();
	QString unitStr( unit.getName().c_str());
	ui.unitsLabel->setText( unitStr );
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
			percentCalculator->work();
			percentilesDone();
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

void RangeControlsWidget::clearRange(){
	ui.minLineEdit->setText("");
	ui.maxLineEdit->setText("");
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
	//If we are zooming, we don't want to change the actual data range.
	if ( ignoreRange ){
		return;
	}
	rangeMin = min;
	rangeMax = max;

	//If we are calculating a percentage based range, we need to update
	//the percentage based on the new data limits.
	if ( ui.percentileCheckBox->isChecked()){
		percentageChanged( percentage );
	}
}

RangeControlsWidget::~RangeControlsWidget(){
	if ( ui.maxLayout->parent() == NULL ){
		delete ui.maxLineEdit;
		delete ui.maxLabel;
		delete ui.maxLayout;
	}
}

//*************************************************************************
//                       Percentage Calculator
//*************************************************************************

PercentageCalculator::PercentageCalculator( float minValue, float maxValue, const std::tr1::shared_ptr<const ImageInterface<Float> > image ){
	this->minValue = minValue;
	this->maxValue = maxValue;
	this->image = image;
}



void PercentageCalculator::work(){
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
