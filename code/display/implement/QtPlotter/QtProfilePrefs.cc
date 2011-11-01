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

#include <QDialog>

namespace casa { 

QtProfilePrefs::~QtProfilePrefs()
{
}

QtProfilePrefs::QtProfilePrefs(QWidget *parent)
:QDialog(parent)
{
	setupUi(this);
	connect(multiProf, SIGNAL(stateChanged(int)),
			this, SLOT(adjustBoxes(int)));
	connect(buttonBox, SIGNAL(accepted()),
			this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()),
			this, SLOT(close()));
}

QtProfilePrefs::QtProfilePrefs(QWidget *parent, int stateAutoX, int stateAutoY, int stateMProf, int stateRel)
:QDialog(parent)
{
	// paint the GUI
	setupUi(this);

	// set boxes according to input
	autoScaleX->setChecked(stateAutoX);
	autoScaleY->setChecked(stateAutoY);
	multiProf->setChecked(stateMProf);
	if (stateMProf){
		relative->setChecked(stateRel);
	}
	else {
		relative->setChecked(false);
		relative->setEnabled(false);
	}

	connect(multiProf, SIGNAL(stateChanged(int)),
			this, SLOT(adjustBoxes(int)));
	connect(buttonBox, SIGNAL(accepted()),
			this, SLOT(accepted()));
	connect(buttonBox, SIGNAL(rejected()),
			this, SLOT(close()));
}

void QtProfilePrefs::accepted(){
	emit currentPrefs((int)autoScaleX->checkState(), (int)autoScaleY->checkState(), (int)multiProf->checkState(), (int)relative->checkState()) ;
	close();
}

void QtProfilePrefs::adjustBoxes(int st){
	relative->setChecked(false);
	if (st)
		relative->setEnabled(true);
	else
		relative->setEnabled(false);
}
}
