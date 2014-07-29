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
#include "BinCountWidget.qo.h"
#include <QDebug>

namespace casa {

int BinCountWidget::DEFAULT_COUNT = 25;

BinCountWidget::BinCountWidget(QWidget *parent)
    : QWidget(parent){
	ui.setupUi(this);
	setDefaultBackground();
	QIntValidator* intValidator = new QIntValidator(1, 1000, this );
	ui.binCountLineEdit->setValidator( intValidator );
	connect( ui.binCountLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(binCountTextChanged(const QString&)));
	connect( ui.binCountSlider, SIGNAL(valueChanged(int)), this, SLOT(binValueChanged(int)));
}

void BinCountWidget::setDefaultBackground(){
	setAutoFillBackground( true );
	QPalette pal = palette();
	QColor bColor = pal.color(QPalette::Button );
	pal.setColor( QPalette::Background, bColor );
	setPalette( pal );
}

int BinCountWidget::getBinCount() const {
	return ui.binCountSlider->value();
}

void BinCountWidget::setBinCount( int binCount ){
	int oldValue = ui.binCountSlider->value();
	if ( oldValue != binCount ){
		ui.binCountSlider->setValue( binCount );
	}
}

void BinCountWidget::binCountTextChanged( const QString& binCount ){
	bool validValue = false;
	int binCountInt = binCount.toInt(&validValue );
	if ( validValue ){
		ui.binCountSlider->setValue( binCountInt );
	}
}

void BinCountWidget::binValueChanged( int value ){
	ui.binCountLineEdit->blockSignals( true );
	ui.binCountLineEdit->setText( QString::number( value ));
	ui.binCountLineEdit->blockSignals( false );
	emit binCountChanged( value );
}

BinCountWidget::~BinCountWidget(){

}
}
