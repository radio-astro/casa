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
#include "SelectEstimateDialog.qo.h"
#include <QDebug>

namespace casa {
SelectEstimateDialog::SelectEstimateDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	connect( ui.clearButton, SIGNAL(clicked()), this, SLOT(deselectAll()));
	connect( ui.selectAllButton, SIGNAL(clicked()), this, SLOT(selectAll()));
}

void SelectEstimateDialog::initList(){
	ui.listWidget->clear();
	for ( int i = 0; i < estimateCount; i++ ){
		QListWidgetItem* item = new QListWidgetItem( ui.listWidget );
		item->setText( "Estimate "+ QString::number(i+1));
		item->setCheckState( Qt::Checked );
	}
}

void SelectEstimateDialog::deselectAll(){
	for ( int i = 0; i < estimateCount; i++ ){
		QListWidgetItem* item = ui.listWidget->item( i );
		item->setCheckState( Qt::Unchecked );
	}
}

void SelectEstimateDialog::selectAll(){
	for ( int i = 0; i < estimateCount; i++ ){
		QListWidgetItem* item = ui.listWidget->item( i );
		item->setCheckState( Qt::Checked );
	}
}

void SelectEstimateDialog::setEstimateCount( int count ){
	estimateCount = count;
	initList();
}

QList<int> SelectEstimateDialog::getSelectedEstimates(){
	QList<int> selectedEstimates;
	qDebug() << "Number of estimates is " << estimateCount;
	for ( int i = 0; i < estimateCount; i++ ){
		QListWidgetItem* item = ui.listWidget->item( i );
		qDebug() << "Item i="<<i<<" selected is " << item->isSelected();
		if ( item->checkState() == Qt::Checked ){
			qDebug()<< "Added " << i;
			selectedEstimates.append( i );
		}
	}
	qDebug() << "Returning " << selectedEstimates.size() << " estimates.";
	return selectedEstimates;
}

SelectEstimateDialog::~SelectEstimateDialog()
{

}
}
