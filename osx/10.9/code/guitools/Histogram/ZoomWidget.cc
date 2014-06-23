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
#include "ZoomWidget.qo.h"
#include <images/Images/ImageInterface.h>
#include <images/Regions/ImageRegion.h>
#include <lattices/Lattices/LatticeFractile.h>
#include <QDebug>
#include <QMessageBox>

namespace casa {

ZoomWidget::ZoomWidget(bool rangeControls, QWidget *parent)
    : QWidget(parent){
	ui.setupUi(this);
	setDefaultBackground();

	//Combo box for setting the percent zoom
	zoomList =(QStringList()<< "95" << "98" << "99.5"<<"99.9");
	for ( int i = 0; i < zoomList.size(); i++ ){
		ui.percentageComboBox->addItem( zoomList[i] );
	}
	QDoubleValidator* validator = new QDoubleValidator( 0.000001, 99.999999, 7, this );
	ui.percentageComboBox->setValidator( validator );
	ui.percentageComboBox->setEnabled( false );

	//Radio buttons
	QButtonGroup* zoomGroup = new QButtonGroup( this );
	zoomGroup->addButton( ui.percentageRadioButton );
	zoomGroup->addButton( ui.neutralRadioButton );
	if ( !rangeControls ){
		ui.rangeRadioButton->setVisible( false );
	}
	else {
		zoomGroup->addButton( ui.rangeRadioButton );
	}
	ui.neutralRadioButton->setChecked( true );
	connect( ui.percentageRadioButton, SIGNAL( toggled(bool)), this, SLOT(percentageToggled(bool)));


	connect( ui.zoomButton, SIGNAL(clicked()), this, SLOT(zoom()));

}

void ZoomWidget::setDefaultBackground(){
	setAutoFillBackground( true );
	QPalette pal = palette();
	QColor bColor = pal.color(QPalette::Button );
	pal.setColor( QPalette::Background, bColor );
	setPalette( pal );
}

void ZoomWidget::copyState( ZoomWidget* other ){
	if ( other != NULL ){
		blockSignals( true );
		ui.percentageRadioButton->setChecked( other->ui.percentageRadioButton->isChecked());
		ui.neutralRadioButton->setChecked( other->ui.neutralRadioButton->isChecked( ));
		ui.rangeRadioButton->setChecked( other->ui.rangeRadioButton->isChecked());
		const QString& currentPercent = other->ui.percentageComboBox->currentText();
		int index = zoomList.indexOf( currentPercent );
		if ( index >= 0 ){
			ui.percentageComboBox->setCurrentIndex( index );
		}
		else {
			ui.percentageComboBox->setEditText( currentPercent );
		}
		blockSignals( false );
	}
}

void ZoomWidget::percentageToggled( bool selected ){
	ui.percentageComboBox->setEnabled( selected );
}

void ZoomWidget::zoom(){
	if ( ui.neutralRadioButton->isChecked()){
		emit zoomNeutral();
	}
	else if ( ui.rangeRadioButton->isChecked()){
		emit zoomGraphicalRange();
	}
	else {
		calculateRange();
	}
	emit finished( );
}

void ZoomWidget::calculateRange( ){
	const int TOTAL_PERCENT = 100;
	bool valid = true;
	QString percentStr = ui.percentageComboBox->currentText();
	float percent = percentStr.toFloat(&valid);
	if ( valid && 0 < percent && percent < 100 ){
		float leftPercent = TOTAL_PERCENT - percent;
		//Generate a symmetric interval.
		float minValue = leftPercent / 2;
		float maxValue = TOTAL_PERCENT - leftPercent / 2;
		minValue = minValue / TOTAL_PERCENT;
		maxValue = maxValue / TOTAL_PERCENT;
		if ( image ){
			Vector<Float> intensities = LatticeFractile<Float>::maskedFractiles (*image, minValue, maxValue);
			int intensityCount = intensities.nelements();
			if ( intensityCount == 2 ){
				emit zoomRange( intensities[0], intensities[1]);
			}
		}
		else {
			QMessageBox::warning( this, "Missing Image", "An image must be loaded in order to zoom.");
		}
	}
}

void ZoomWidget::setImage( const shared_ptr<const ImageInterface<Float> > image ){
	this->image = image;
}

void ZoomWidget::setRegion( ImageRegion* region ){
	this->region = region;
}


ZoomWidget::~ZoomWidget()
{

}
}
