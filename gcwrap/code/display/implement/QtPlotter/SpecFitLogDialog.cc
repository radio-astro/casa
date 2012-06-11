#include "SpecFitLogDialog.qo.h"

namespace casa {
SpecFitLogDialog::SpecFitLogDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	connect( ui.closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

void SpecFitLogDialog::setLogResults( const QString& results ){
	ui.logTextEdit->setText( results );
}

SpecFitLogDialog::~SpecFitLogDialog()
{

}
}
