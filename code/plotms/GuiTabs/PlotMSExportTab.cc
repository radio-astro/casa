//# PlotMSExportTab.cc: Plot tab for managing export.
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
#include <plotms/GuiTabs/PlotMSExportTab.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <casaqt/QtUtilities/QtEditingWidget.qo.h>
#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Actions/PlotMSAction.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <QDebug>
namespace casa {

/////////////////////////////////
// PLOTMSEXPORTTAB DEFINITIONS //
/////////////////////////////////

PlotMSExportTab::PlotMSExportTab(QWidget* parent):
        QDialog(parent) {
    ui.setupUi(this);
    setWindowTitle( "Export Plots");
    
    // Setup widgets.
    itsFileWidget_ = new QtFileWidget(false, true);
    QtUtilities::putInFrame(ui.fileFrame, itsFileWidget_);

    ui.format->addItem("[by file extension]");
    vector<String> formats = PlotExportFormat::supportedFormatStrings();
    for(unsigned int i = 0; i < formats.size(); i++)
        ui.format->addItem(formats[i].c_str());
    
    const vector<String>& exportRanges = PMS::exportRangeStrings();
    for(unsigned int i = 0; i < exportRanges.size(); i++){
    	ui.exportRangeCombo->addItem(exportRanges[i].c_str());
    }
    ui.exportRangeCombo->setCurrentIndex( PMS::DEFAULT_EXPORT_RANGE );

    //connect(ui.exportRangeCombo, SIGNAL(currentIndexChanged(int)), SIGNAL(exportRangeChanged()));
    connect( ui.msNamesButton, SIGNAL(clicked()), this, SLOT(insertMSNames()) );
    connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(closeDialog()));
    connect( ui.exportButton, SIGNAL(clicked()), this, SLOT(doExport()));
}

PlotMSExportTab::~PlotMSExportTab() { }

PlotMSExportParam PlotMSExportTab::getExportParams() const {
	PlotMSExportParam params;
	QString rangeStr =  ui.exportRangeCombo->currentText();
	params.setExportRange( rangeStr.toStdString());
	return params;
}

void PlotMSExportTab::insertMSNames()
{
	String exportName = "";
	for (uInt i=0; i < MSNames_.size(); i++) {
		// Extract MS name from full path
		String msName = getMsNameFromPath(MSNames_[i]);
		if (exportName.length() > 0) exportName += '_';
		exportName += msName;
	}
    	itsFileWidget_->setFile(exportName);
}

String PlotMSExportTab::getMsNameFromPath(String msfilepath)
{
 	Int endOfPath = msfilepath.find_last_of('/');
	Int startOfExt = msfilepath.rfind(".ms");
	return msfilepath.at(endOfPath+1, startOfExt-endOfPath-1);
}

void PlotMSExportTab::setExportFormat(PlotExportFormat format)
{
    itsFileWidget_->setFile(format.location);
}

PlotExportFormat PlotMSExportTab::currentlySetExportFormat() const {
    String file = itsFileWidget_->getFile();
    PlotExportFormat::Type t = (ui.format->currentIndex() == 0) ?
            PlotExportFormat::typeForExtension(file) :
            PlotExportFormat::exportFormat(ui.format->currentText().toStdString());
    PlotExportFormat format(t, file);
    format.resolution = ui.highRes->isChecked() ? PlotExportFormat::HIGH :
                                               PlotExportFormat::SCREEN;
    format.dpi = (ui.dpi->isVisible() && ui.dpi->isChecked()) ?
                 ui.dpiSpinner->value() : -1;
    if(ui.ExportTab::size->isVisible() && ui.ExportTab::size->isChecked()) {
        format.width  = ui.sizeSpinner1->value();
        format.height = ui.sizeSpinner2->value();
    } else {
        format.width  = -1;
        format.height = -1;
    }
    return format;
}

void PlotMSExportTab::closeDialog(){
	done( -1);
}

void PlotMSExportTab::doExport(){
	done( 1);
}

}
