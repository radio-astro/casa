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
#ifndef SETTINGSWIDGET_QO_H
#define SETTINGSWIDGET_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/SettingsWidget.ui.h>
#include <casa/Arrays/Vector.h>

namespace casa {


	class QtCanvas;
	class LogIO;
	class ProfileTaskMonitor;
	class ProfileTaskFacilitator;

	class SettingsWidget : public QWidget {
		Q_OBJECT

	public:
		void setCollapseVals(const Vector<Float> &spcVals);
		void setCanvas( QtCanvas* pCanvas );
		void setTaskMonitor( ProfileTaskMonitor* fitMonitor );
		void setCurveName( const QString& curveName );
		void addCurveName( const QString& curveName );
		void reset( bool taskChanged = false );
		void setLogger( LogIO* log );
		void setUnits( QString units );
		void setDisplayYUnits( const QString& units );
		void setImageYUnits( const QString& units );
		void setRange( double start, double end );
		static bool isOptical();
		static void setOptical( bool optical );
		void setTaskSpecLineFitting( bool specLineFitting );
		void pixelsChanged(int, int );
		SettingsWidget(QWidget *parent = 0);
		~SettingsWidget();



	private slots:
		void clear();
		void setFitRange(double start, double end );
		void gaussCountChanged( int estimateCount );

	private:
		QtCanvas* pixelCanvas;
		ProfileTaskFacilitator* taskHelper;
		ProfileTaskFacilitator* taskHelperOptical;
		ProfileTaskFacilitator* taskHelperRadio;
		LogIO* logger;
		ProfileTaskMonitor* taskMonitor;
		Ui::SettingsWidget ui;
		static bool optical;
		bool specLineFitting;
		bool newCollapseVals;

	};
}
#endif // SPECFITSETTINGSWIDGET_QO_H
