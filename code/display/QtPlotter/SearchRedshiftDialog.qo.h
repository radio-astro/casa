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
#ifndef SEARCHREDSHIFTDIALOG_QO_H
#define SEARCHREDSHIFTDIALOG_QO_H

#include <QtGui/QDialog>
#include <QProgressDialog>
#include <casa/BasicSL/String.h>
#include <display/QtPlotter/SearchRedshiftDialog.ui.h>
#include <measures/Measures/MRadialVelocity.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDoppler.h>

namespace casa {

	class SearchThread;

	class SearchRedshiftDialog : public QDialog {
		Q_OBJECT

	public:
		SearchRedshiftDialog(QWidget *parent = 0);
		void setCenter( double centerVal );
		void setUnits( QString unitStr );
		void setDatabasePath( String path );
		void setLocalSearch( bool local );
		void setFrequencyType( MRadialVelocity::Types mType );
		void setDopplerType( MDoppler::Types type );
		void setIdentifiedLines( const QList<QString>& lineNames );
		~SearchRedshiftDialog();

	public slots:
		void show();
		void findRedshift();
		void searchFinished();
		void stopSearch();

	private:
		void setResultsVisible( bool visible );
		double getTargetFrequency() const;
		Ui::SearchRedshiftDialogClass ui;
		String databasePath;
		bool localSearch;
		bool searchInterrupted;
		SearchThread* searchThread;
		QProgressDialog progressBar;
		MFrequency::Types frequencyType;
		MRadialVelocity::Types radialVelocityType;
		MDoppler::Types dopplerType;
	};
}
#endif // SEARCHREDSHIFTDIALOG_QO_H
