//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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
#include "Fit2DLogDialog.qo.h"
#include <QFile>
#include <QTextStream>
#include <QStringBuilder>
#include <QDebug>

namespace casa {

	Fit2DLogDialog::Fit2DLogDialog(QWidget *parent)
		: QDialog(parent) {
		ui.setupUi(this);
		this->setWindowTitle( "Fit 2D Results");
		connect(ui.closeButton, SIGNAL(clicked()), this, SLOT(logViewFinished()));
	}

	bool Fit2DLogDialog::setLog( const QString& fullPath ) {
		QFile logFile( fullPath);
		bool successfulRead = true;
		if ( logFile.open(QIODevice::ReadOnly) ) {
			QTextStream in( &logFile );
			while ( !in.atEnd() ) {
				QString line = in.readLine();
				ui.logTextEdit->append( line );
			}
			logFile.close();
		} else {
			successfulRead = false;
		}
		return successfulRead;
	}

	void Fit2DLogDialog::logViewFinished() {
		close();
	}

	Fit2DLogDialog::~Fit2DLogDialog() {

	}
}
