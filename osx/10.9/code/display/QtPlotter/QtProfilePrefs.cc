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
#include <display/QtPlotter/QtProfilePrefs.qo.h>
#include <display/QtPlotter/Util.h>

#include <QDialog>
#include <QSettings>

namespace casa {


	const QString QtProfilePrefs::X_AUTO_SCALE = "x Auto Scale";
	const QString QtProfilePrefs::Y_AUTO_SCALE = "y Auto Scale";
	const QString QtProfilePrefs::SHOW_GRID = "Show Grid";
	const QString QtProfilePrefs::OVERLAY = "Overlay";
	const QString QtProfilePrefs::RELATIVE = "Relative";
	const QString QtProfilePrefs::TOOLTIPS = "Tooltips";
	const QString QtProfilePrefs::TOP_AXIS = "Top Axis";
	const QString QtProfilePrefs::STEP_FUNCTION = "Step Function";
	const QString QtProfilePrefs::OPTICAL = "Optical";
	const QString QtProfilePrefs::CHANNEL_LINE = "Channel Line";
	const QString QtProfilePrefs::SINGLE_CHANNEL_IMAGE = "Single Channel Image";

	QtProfilePrefs::~QtProfilePrefs() {
	}

	QtProfilePrefs::QtProfilePrefs(QWidget *parent)
		:QDialog(parent) {
		setupUi(this);
		initializeConnections();
	}

	QtProfilePrefs::QtProfilePrefs(QWidget *parent, bool stateAutoX, bool stateAutoY,
	                               int stateGrid, int stateMProf, int stateRel, bool showToolTips,
	                               bool showTopAxis, bool displayAsStepFunction,
	                               bool opticalFit, bool channelLine, bool singleChannelImage )
		:QDialog(parent) {
		// paint the GUI
		setupUi(this);

		//Only use the default values passed in if the user has not indicated
		//any preferences.
		QSettings settings( Util::ORGANIZATION, Util::APPLICATION );
		xAutoScaleDefault = settings.value( X_AUTO_SCALE, stateAutoX).toBool();
		autoScaleX->setChecked( xAutoScaleDefault );

		yAutoScaleDefault = settings.value( Y_AUTO_SCALE, stateAutoY).toBool();
		autoScaleY->setChecked( yAutoScaleDefault );

		showGridDefault = settings.value( SHOW_GRID, stateGrid ).toBool();
		showGrid->setChecked( showGridDefault );

		overlayDefault = settings.value( OVERLAY, stateMProf ).toBool();
		multiProf->setChecked( overlayDefault );

		toolTipsDefault = settings.value( TOOLTIPS, showToolTips ).toBool();
		toolTipsCheckBox -> setChecked( toolTipsDefault );

		opticalDefault = settings.value( OPTICAL, opticalFit ).toBool();
		opticalSpecFitCheckBox -> setChecked( opticalDefault );

		topAxisDefault = settings.value( TOP_AXIS, showTopAxis ).toBool();
		if ( opticalDefault ){
			topAxisDefault = false;
			topAxisCheckBox->setEnabled( false );
		}
		topAxisCheckBox -> setChecked( topAxisDefault );

		stepFunctionDefault = settings.value( STEP_FUNCTION, displayAsStepFunction ).toBool();
		stepFunctionCheckBox->setChecked( stepFunctionDefault );

		relativeDefault = settings.value( RELATIVE, stateRel ).toBool();
		relative->setChecked( relativeDefault );

		relative->setEnabled(overlayDefault);

		channelLineDefault = settings.value( CHANNEL_LINE, channelLine ).toBool();
		channelLineCheckBox->setChecked( channelLineDefault );

		singleChannelImageDefault = settings.value( SINGLE_CHANNEL_IMAGE, singleChannelImage ).toBool();
		singlePlaneCheckBox->setChecked( singleChannelImageDefault );

		initializeConnections();
	}

	void QtProfilePrefs::syncUserPreferences() {
		//Update the profiler with user preferences.
		emit currentPrefs(autoScaleX->checkState(), autoScaleY->checkState(), (int)showGrid->checkState(),
		                  (int)multiProf->checkState(), (int)relative->checkState(), toolTipsCheckBox->checkState(),
		                  topAxisCheckBox->checkState(), stepFunctionCheckBox->checkState(),
		                  opticalSpecFitCheckBox->checkState(),
		                  channelLineCheckBox->checkState(),
		                  singlePlaneCheckBox->checkState()) ;
	}

	void QtProfilePrefs::initializeConnections() {
		connect(multiProf, SIGNAL(stateChanged(int)), this, SLOT(adjustBoxes(int)));
		connect(opticalSpecFitCheckBox, SIGNAL(stateChanged(int)), this, SLOT(opticalStateChanged()));
		connect(buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
		connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejected()));
	}

	void QtProfilePrefs::opticalStateChanged(){
		bool opticalEnabled = opticalSpecFitCheckBox->isChecked();
		topAxisCheckBox->setEnabled(!opticalEnabled  );
		if ( opticalEnabled ){
			topAxisCheckBox->setChecked( false );
		}
	}

	void QtProfilePrefs::reset() {
		autoScaleX->setChecked(xAutoScaleDefault);
		autoScaleY->setChecked( yAutoScaleDefault );
		showGrid->setChecked( showGridDefault );
		multiProf->setChecked( overlayDefault );
		toolTipsCheckBox->setChecked( toolTipsDefault );
		topAxisCheckBox->setChecked( topAxisDefault );
		opticalSpecFitCheckBox->setChecked( opticalDefault );
		stepFunctionCheckBox->setChecked( stepFunctionDefault );
		relative->setChecked( relativeDefault );
		relative->setEnabled( overlayDefault );
		channelLineCheckBox->setChecked( channelLineDefault );
		singlePlaneCheckBox->setChecked( singleChannelImageDefault );
	}

	void QtProfilePrefs::persist() {

		QSettings settings( Util::ORGANIZATION, Util::APPLICATION );

		xAutoScaleDefault = autoScaleX->isChecked();
		settings.setValue( X_AUTO_SCALE, xAutoScaleDefault );

		yAutoScaleDefault = autoScaleY->isChecked();
		settings.setValue( Y_AUTO_SCALE, yAutoScaleDefault );

		showGridDefault = showGrid->isChecked();
		settings.setValue( SHOW_GRID, showGridDefault );

		overlayDefault = multiProf->isChecked();
		settings.setValue( OVERLAY, overlayDefault );

		toolTipsDefault = toolTipsCheckBox->isChecked();
		settings.setValue( TOOLTIPS, toolTipsDefault );

		topAxisDefault = topAxisCheckBox->isChecked();
		settings.setValue( TOP_AXIS, topAxisDefault );

		opticalDefault = opticalSpecFitCheckBox->isChecked();
		settings.setValue( OPTICAL, opticalDefault );

		stepFunctionDefault = stepFunctionCheckBox->isChecked();
		settings.setValue( STEP_FUNCTION, stepFunctionDefault );

		relativeDefault = relative->isChecked();
		settings.setValue( RELATIVE, relativeDefault );

		channelLineDefault = channelLineCheckBox->isChecked();
		settings.setValue( CHANNEL_LINE, channelLineDefault );

		singleChannelImageDefault = singlePlaneCheckBox->isChecked();
		settings.setValue( SINGLE_CHANNEL_IMAGE, singleChannelImageDefault );
	}


	void QtProfilePrefs::accepted() {
		persist();
		emit currentPrefs(autoScaleX->checkState(), autoScaleY->checkState(), (int)showGrid->checkState(),
		                  (int)multiProf->checkState(), (int)relative->checkState(), toolTipsCheckBox->checkState(),
		                  topAxisCheckBox->checkState(), stepFunctionCheckBox->checkState(),
		                  opticalSpecFitCheckBox->checkState(),
		                  channelLineCheckBox->checkState(),
		                  singlePlaneCheckBox->checkState() ) ;
		close();
	}

	void QtProfilePrefs::rejected() {
		reset();
		close();
	}

	void QtProfilePrefs::adjustBoxes(int st) {
		relative->setChecked(false);
		if (st)
			relative->setEnabled(true);
		else
			relative->setEnabled(false);
	}
}
