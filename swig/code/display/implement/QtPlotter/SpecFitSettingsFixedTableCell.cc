#include "SpecFitSettingsFixedTableCell.qo.h"
#include <QDebug>

SpecFitSettingsFixedTableCell::SpecFitSettingsFixedTableCell(QWidget *parent)
    : QWidget(parent),
      GAUSSIAN_ESTIMATE_FIXED_CENTER("c"),
      GAUSSIAN_ESTIMATE_FIXED_PEAK( "p"),
      GAUSSIAN_ESTIMATE_FIXED_FWHM( "f")
{
	ui.setupUi(this);
}

QSize SpecFitSettingsFixedTableCell::sizeHint() const {
	QSize suggestedSize( 150, 35 );
	return suggestedSize;
}

QString SpecFitSettingsFixedTableCell::getFixedStr() const {
	QString fixedEstimateStr;
	if ( ui.centerCheckBox->isChecked() ){
		fixedEstimateStr.append( GAUSSIAN_ESTIMATE_FIXED_CENTER );
	}
	if ( ui.peakCheckBox->isChecked() ){
		fixedEstimateStr.append( GAUSSIAN_ESTIMATE_FIXED_PEAK );
	}
	if ( ui.fwhmCheckBox->isChecked() ){
		fixedEstimateStr.append( GAUSSIAN_ESTIMATE_FIXED_FWHM );
	}
	return fixedEstimateStr;
}


SpecFitSettingsFixedTableCell::~SpecFitSettingsFixedTableCell()
{

}
