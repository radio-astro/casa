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
