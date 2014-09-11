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

#include "ActionExportDialog.h"
#include <plotms/Actions/ActionExport.h>
#include <plotms/Actions/ActionFactory.h>
#include <plotms/Client/Client.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/GuiTabs/PlotMSExportTab.qo.h>
#include <QDebug>

namespace casa {

ActionExportDialog::ActionExportDialog(Client* client)
	: PlotMSAction( client ){
	itsType_=EXPORT_DIALOG;
	setUseThreading( false );
	exportDialog = NULL;

}

bool ActionExportDialog::doActionSpecific( PlotMSApp* plotms){
	if ( exportDialog == NULL ){
		exportDialog = new PlotMSExportTab();
                PlotExportFormat oldFormat = plotms->getExportFormat();
                exportDialog->setExportFormat(oldFormat);
	}

	int result = exportDialog->exec();
	bool success = true;
	//Perform an export.
	if ( result == 1 ){
		ActionExport exportAction( client );
		PlotExportFormat format = exportDialog->currentlySetExportFormat();
		exportAction.setExportFormat( format );
		PlotMSExportParam exportParams = exportDialog->getExportParams();
		plotms->setExportParameters( exportParams );
		plotms->setExportFormat( format );
		success = exportAction.doAction( plotms );
		if ( !success ){
			itsDoActionResult_ = exportAction.doActionResult();
		}
	}
	return success;
}

ActionExportDialog::~ActionExportDialog() {
	delete exportDialog;
}

} /* namespace casa */
