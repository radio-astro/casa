#include "SpecFitEstimateDialog.qo.h"

namespace casa {

SpecFitEstimateDialog::SpecFitEstimateDialog(const GaussFitEstimate* const estimate, QWidget* parent )
    : QDialog(parent)
{
	ui.setupUi(this);
	setModal( true );
	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	//populate the fields from the estimate
}

void SpecFitEstimateDialog::updateEstimate( GaussFitEstimate* const estimate ) const {
	estimate -> setCenter( ui.centerPixelLineEdit->text().toFloat());
	estimate -> setPeak( ui.peakIntensityLineEdit->text().toFloat());
	estimate -> setBeamWidth( ui.fwhmLineEdit->text().toFloat());
	estimate -> setCenterFixed( ui.centerCheckBox->isChecked());
	estimate -> setPeakFixed( ui.peakCheckBox -> isChecked());
	estimate -> setBeamWidthFixed( ui.fwhmCheckBox -> isChecked());
}


SpecFitEstimateDialog::~SpecFitEstimateDialog()
{

}

} // end namespace casa
