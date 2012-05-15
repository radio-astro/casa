#include "SpecFitSettingsGaussWidget.qo.h"
#include <QtGui>
namespace casa {
SpecFitSettingsGaussWidget::SpecFitSettingsGaussWidget(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	connect( ui.gaussCountSpinBox, SIGNAL(valueChanged(int)), this, SLOT(adjustTableRowCount(int)));

	ui.estimateTable->setColumnCount( END_COLUMN );
	QStringList tableHeaders =(QStringList()<< "Peak" << "Center" << "FWHM" << "Fixed");
	ui.estimateTable->setHorizontalHeaderLabels( tableHeaders );

}

SpecFitSettingsGaussWidget::~SpecFitSettingsGaussWidget()
{

}

void SpecFitSettingsGaussWidget::adjustTableRowCount( int count ){
	//qDebug() << "New table row count is "  << count;
	ui.estimateTable -> setRowCount( count );
}

}
