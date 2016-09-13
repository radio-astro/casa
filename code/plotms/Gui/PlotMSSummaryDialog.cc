//# Copyright (C) 2009
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
//# $Id: $
#include "PlotMSSummaryDialog.qo.h"
#include <QDebug>


namespace casa {
PlotMSSummaryDialog::PlotMSSummaryDialog(QDialog *parent)
    : QDialog(parent){
	ui.setupUi(this);

	setWindowTitle( "Summarize" );

	//Buttons
	connect( ui.closeButton, SIGNAL(clicked()), this, SLOT(closeDialog()));
	connect( ui.summaryButton, SIGNAL(clicked()), this, SLOT(summarize()));

	// Set up summary choices
	const vector<String>& types = PMS::summaryTypeStrings();
	for(unsigned int i = 0; i < types.size(); i++){
		ui.summaryType->addItem(types[i].c_str());
	}
}

void PlotMSSummaryDialog::closeDialog(){
	done( -1);
}

void PlotMSSummaryDialog::summarize(){
	done( 1);
}

bool PlotMSSummaryDialog::isVerbose() const {
	return ui.summaryVerbose->isChecked();
}

void PlotMSSummaryDialog::filesChanged(const vector<String>& fileNames){
	QString oldSelection = ui.fileCombo->currentText();
	ui.fileCombo->clear();
	int fileNameCount = fileNames.size();
	for ( int i = 0; i < fileNameCount; i++ ){
		ui.fileCombo->addItem( fileNames[i].c_str());
	}
	int oldIndex = ui.fileCombo->findText( oldSelection );
	if ( oldIndex >= 0 ){
		ui.fileCombo->setCurrentIndex( oldIndex );
	}
}

String PlotMSSummaryDialog::getFileName() const {
	QString fileNameStr = ui.fileCombo->currentText();
	return fileNameStr.toStdString();
}

PMS::SummaryType PlotMSSummaryDialog::getSummaryType() const {
	QString summaryStr = ui.summaryType->currentText();
	bool valid = false;
	PMS::SummaryType type = PMS::summaryType( summaryStr.toStdString(), &valid );
	if ( !valid ){
		type = PMS::S_ALL;
	}
	return type;
}


PlotMSSummaryDialog::~PlotMSSummaryDialog(){
}
}
