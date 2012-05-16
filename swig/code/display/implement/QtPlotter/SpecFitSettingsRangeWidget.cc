#include "SpecFitSettingsRangeWidget.qo.h"
#include <QString>
#include <QDoubleValidator>
namespace casa {
SpecFitSettingsRangeWidget::SpecFitSettingsRangeWidget(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	doubleValidator = new QDoubleValidator( this );
}

SpecFitSettingsRangeWidget::~SpecFitSettingsRangeWidget()
{

}

void SpecFitSettingsRangeWidget::setCollapseRange( float xmin, float xmax ){
	if (xmax < xmin){
		ui.minLineEdit->clear();
		ui.maxLineEdit->clear();
	}
	else {
		QString startStr;
		QString endStr;
		startStr.setNum(xmin);
		endStr.setNum(xmax);
		ui.minLineEdit->setText(startStr);
		ui.maxLineEdit->setText(endStr);
	}
}
}
