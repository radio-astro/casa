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
#include "OverWriteFileDialog.qo.h"
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

namespace casa {

OverWriteFileDialog::OverWriteFileDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle( "Overwrite Output File?" );

	connect( ui.browseButton, SIGNAL(clicked()), this, SLOT(browse()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(overWriteCancelled()));
	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(overWriteAccepted()));
}

void OverWriteFileDialog::setFile( const QString& fileName ){
	ui.directoryLineEdit->setText( fileName );
}

QString OverWriteFileDialog::getFile() const {
	return ui.fileLineEdit->text();
}

void OverWriteFileDialog::setDirectory( const QString& directoryName ){
	ui.fileLineEdit->setText( directoryName );
}

QString OverWriteFileDialog::getDirectory() const {
	return ui.directoryLineEdit->text();
}


void OverWriteFileDialog::overWriteCancelled(){
	this->close();
}

void OverWriteFileDialog::overWriteAccepted(){
	bool valid = true;
	QFile directoryFile( ui.directoryLineEdit->text());
	if ( !directoryFile.exists() ){
		valid = false;
		QMessageBox::warning( this, "Directory Incorrect", "Please check that the output directory path is valid.");
	}
	else if ( ui.fileLineEdit->text().trimmed().length() == 0 ){
		QMessageBox::warning( this, "File Not Specified", "Please specify an output file.");
		valid = false;
	}
	if ( valid ){
		emit overWriteOK();
		close();
	}
}

void OverWriteFileDialog::browse(){
	QString existingDirectory = ui.directoryLineEdit->text();
	if ( existingDirectory.length() == 0 ){
		QDir currentDirectory;
		existingDirectory = currentDirectory.absolutePath();
	}
	QString newDirectory = QFileDialog::getExistingDirectory( this, "Output Directory", existingDirectory );
	ui.directoryLineEdit->setText( newDirectory );
}



OverWriteFileDialog::~OverWriteFileDialog()
{

}
}
