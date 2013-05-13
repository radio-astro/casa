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

#ifndef QtProfilePrefs_H
#define QtProfilePrefs_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Inputs/Input.h>
#include <casa/Arrays/IPosition.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/DisplayEvents/MWCCrosshairTool.h>
#include <display/QtPlotter/QtMWCTools.qo.h>
#include <display/Display/PanelDisplay.h>
#include <display/Utilities/Lowlevel.h>

#include <images/Regions/ImageRegion.h>
#include <images/Images/ImageInterface.h>
#include <measures/Measures/Stokes.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <imageanalysis/ImageAnalysis/SpectralCollapser.h>
#include <synthesis/MSVis/StokesVector.h>


#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QMainWindow>
#include <QMouseEvent>
#include <QToolButton>
#include <QDialog>
#include <QCheckBox>
#include <QPixmap>
#include <QLineEdit>
#include <QComboBox>
#include <map>
#include <vector>
#include <QHash>
#include <QHashIterator>
#include <graphics/X11/X_exit.h>

#include <display/QtPlotter/QtProfilePrefsGUI.ui.h>

namespace casa {

	class QtProfilePrefs : public QDialog, Ui::QtProfilePrefsGUI {
		Q_OBJECT

	public:
		QtProfilePrefs(QWidget *parent = 0);
		QtProfilePrefs(QWidget *parent, bool stateAutoX, bool stateAutoY, int showGrid,
		               int stateMProf, int stateRel, bool showToolTips, bool showTopAxis,
		               bool displayStepFunction, bool opticalFitter, bool channelLine);
		~QtProfilePrefs();
		void syncUserPreferences();

	signals:
		void currentPrefs(bool stateAutoX, bool stateAutoY, int showGrid, int stateMProf,
		                  int stateRel, bool showToolTips, bool showTopAxis,
		                  bool displayStepFunction, bool opticalFitter, bool channelLine);

	private slots:
		void accepted();
		void rejected();
		void adjustBoxes(int st);

	private:
		void initializeConnections();
		void persist();
		void reset();


		static const QString X_AUTO_SCALE;
		static const QString Y_AUTO_SCALE;
		static const QString SHOW_GRID;
		static const QString OVERLAY;
		static const QString RELATIVE;
		static const QString TOOLTIPS;
		static const QString TOP_AXIS;
		static const QString STEP_FUNCTION;
		static const QString OPTICAL;
		static const QString CHANNEL_LINE;

		bool xAutoScaleDefault;
		bool yAutoScaleDefault;
		bool showGridDefault;
		bool overlayDefault;
		bool relativeDefault;
		bool toolTipsDefault;
		bool topAxisDefault;
		bool stepFunctionDefault;
		bool opticalDefault;
		bool channelLineDefault;
	};

}
#endif

