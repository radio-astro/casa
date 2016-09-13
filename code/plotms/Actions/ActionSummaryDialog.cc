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

#include "ActionSummaryDialog.h"
#include <plotms/Actions/ActionSummary.h>
#include <plotms/Actions/ActionFactory.h>
#include <plotms/Client/Client.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Gui/PlotMSSummaryDialog.qo.h>
#include <QMessageBox>
#include <QDebug>

namespace casa {

ActionSummaryDialog::ActionSummaryDialog(Client* client)
	: PlotMSAction( client ){
	itsType_=SUMMARY_DIALOG;
	setUseThreading( false );
	summarizeDialog = NULL;

}

bool ActionSummaryDialog::doActionSpecific( PlotMSApp* plotms){
	vector<String> files  = client->getFiles();
	bool success = true;
	if ( files.size() == 0 ){
		success = false;
		QMessageBox::warning( NULL, "Missing Files", "Please load at least one file in the 'Plot' tab.");
	}
	else {
		if ( summarizeDialog == NULL ){
			summarizeDialog = new PlotMSSummaryDialog();
		}

		summarizeDialog->filesChanged( files );
		int result = summarizeDialog->exec();

		//Return a summary.
		if ( result == 1 ){
			ActionSummary summary( client );
			summary.setFile( summarizeDialog->getFileName());
			summary.setSummaryType( summarizeDialog->getSummaryType());
			summary.setVerbose( summarizeDialog->isVerbose());
			success = summary.doAction( plotms );
			if ( !success ){
				itsDoActionResult_ = summary.doActionResult();
			}
		}
	}
	return success;
}

ActionSummaryDialog::~ActionSummaryDialog() {
	delete summarizeDialog;
}

} /* namespace casa */
