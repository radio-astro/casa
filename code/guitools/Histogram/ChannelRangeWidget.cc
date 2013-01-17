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
#include "ChannelRangeWidget.qo.h"

namespace casa {

ChannelRangeWidget::ChannelRangeWidget(QWidget *parent)
    : QWidget(parent){
	ui.setupUi(this);

	ui.minSpinBox->setMinimum( 0 );
	ui.minSpinBox->setMaximum( 0 );
	ui.maxSpinBox->setMinimum( 0 );
	ui.maxSpinBox->setMaximum( 0 );
	connect(ui.minSpinBox,SIGNAL(valueChanged(int)), this, SLOT(valueChangedMin(int)));
	connect(ui.maxSpinBox,SIGNAL(valueChanged(int)), this, SLOT(valueChangedMax(int)));
	connect(ui.automaticChannelCheckBox, SIGNAL(toggled(bool)), this, SLOT(automaticChannelsChanged(bool)));
}

void ChannelRangeWidget::automaticChannelsChanged( bool enabled ){
	ui.minSpinBox->setEnabled( !enabled );
	ui.maxSpinBox->setEnabled( !enabled );
}

bool ChannelRangeWidget::isAllChannels() const {
	int minValue = ui.minSpinBox->value();
	int maxValue = ui.maxSpinBox->value();
	bool allChannels = false;
	if ( minValue == ui.minSpinBox->minimum()){
		if ( maxValue == ui.maxSpinBox->maximum()){
			allChannels = true;
		}
	}
	return allChannels;
}

bool ChannelRangeWidget::isAutomatic() const {
	return ui.automaticChannelCheckBox->isChecked();
}

void ChannelRangeWidget::valueChangedMin( int value ){
	ui.maxSpinBox->setMinimum( value );
	int maxValue = ui.maxSpinBox->value();
	emit rangeChanged( value, maxValue, isAllChannels(), isAutomatic() );

}

void ChannelRangeWidget::valueChangedMax( int value ){
	ui.minSpinBox->setMaximum( value );
	int minValue = ui.minSpinBox->value();
	emit rangeChanged( minValue, value, isAllChannels(), isAutomatic() );
}

void ChannelRangeWidget::setAutomatic( bool autoChannels ){
	ui.automaticChannelCheckBox->setChecked( autoChannels );
}

void ChannelRangeWidget::setRange( int minRange, int maxRange ){
	int minValue = ui.minSpinBox->value();
	if ( minValue != minRange ){
		blockSignals( true );
		ui.minSpinBox->setValue( minRange );
		blockSignals( false );
	}
	int maxValue = ui.maxSpinBox->value();
	if ( maxValue != maxRange ){
		blockSignals( true );
		ui.maxSpinBox->setValue( maxRange );
		blockSignals( false );
	}
}

void ChannelRangeWidget::setChannelCount( int count ){
	int actualMax = count - 1;
	ui.maxSpinBox->setMaximum( actualMax );
	ui.maxSpinBox->setValue( actualMax );
}

void ChannelRangeWidget::setChannelValue( int value ){
	if ( ui.automaticChannelCheckBox->isChecked()){
		blockSignals( true );
		ui.minSpinBox->setValue( value );
		ui.maxSpinBox->setValue( value );
		blockSignals( false );
		emit rangeChanged( value, value, false, true );
	}
}

ChannelRangeWidget::~ChannelRangeWidget(){

}
}
