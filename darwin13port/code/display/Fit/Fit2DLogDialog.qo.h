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
#ifndef FIT2DLOGDIALOG_QO_H
#define FIT2DLOGDIALOG_QO_H

#include <QtGui/QDialog>
#include <display/Fit/Fit2DLogDialog.ui.h>

/**
 * Displays the log from a fit.
 */

namespace casa {

	class Fit2DLogDialog : public QDialog {
		Q_OBJECT

	public:
		Fit2DLogDialog(QWidget *parent = 0);
		bool setLog( const QString& fullPath );
		~Fit2DLogDialog();

	private slots:
		void logViewFinished();

	private:
		Ui::Fit2DLogDialogClass ui;
	};

}
#endif // FIT2DLOGDIALOG_QO_H
