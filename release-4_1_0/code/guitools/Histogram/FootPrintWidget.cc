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
#include "FootPrintWidget.qo.h"

namespace casa {

FootPrintWidget::FootPrintWidget(QWidget *parent)
    : QWidget(parent){

	ui.setupUi(this);

	QButtonGroup* group = new QButtonGroup( this );
	group->addButton( ui.imageRadioButton );
	group->addButton( ui.selectedRegionRadioButton );
	group->addButton( ui.allRegionsRadioButton );
	ui.selectedRegionRadioButton->setChecked( true );

	connect( ui.imageRadioButton, SIGNAL(toggled(bool)),
			this, SLOT(imageModeSelected(bool)));
	connect( ui.selectedRegionRadioButton, SIGNAL(toggled(bool)),
			this, SLOT(regionModeSelected(bool)));
	connect( ui.allRegionsRadioButton, SIGNAL(toggled(bool)),
			this, SLOT(regionAllModeSelected(bool)));
}

void FootPrintWidget::setPlotMode( int mode ){
	blockSignals(true );
	if ( mode == IMAGE_MODE){
		ui.imageRadioButton->setChecked( true );
	}
	else if ( mode == REGION_MODE ){
		ui.selectedRegionRadioButton->setChecked( true );
	}
	else {
		ui.allRegionsRadioButton->setChecked( true );
	}
	blockSignals( false );
}

void FootPrintWidget::imageModeSelected( bool selected ){
	if ( selected ){
		emit plotModeChanged( IMAGE_MODE );
	}
}

void FootPrintWidget::regionModeSelected( bool selected ){
	if ( selected ){
		emit plotModeChanged( REGION_MODE );
	}
}

void FootPrintWidget::regionAllModeSelected( bool selected ){
	if ( selected ){
		emit plotModeChanged( REGION_ALL_MODE );
	}
}

FootPrintWidget::~FootPrintWidget(){

}
}
