#include "SpecFitSettingsPolynomialWidget.qo.h"
namespace casa {
SpecFitSettingsPolynomialWidget::SpecFitSettingsPolynomialWidget(QWidget *parent)
    : QWidget(parent)
{

	ui.setupUi(this);

	connect( ui.polyFitCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableWidgets(int)) );
	int state = ui.polyFitCheckBox->checkState();
	enableWidgets( state );
}

SpecFitSettingsPolynomialWidget::~SpecFitSettingsPolynomialWidget()
{

}

void SpecFitSettingsPolynomialWidget::enableWidgets( int state ){
	if ( state == Qt::Checked ){
		ui.polyOrderSpinBox -> setEnabled( true );
	}
	else {
		ui.polyOrderSpinBox -> setEnabled( false );
	}
}
}
