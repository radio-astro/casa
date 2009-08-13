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

#include <casaqt/QtUtilities/QtEditingWidget.qo.h>
#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Actions/PlotMSAction.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>

namespace casa {

/////////////////////////////////
// PLOTMSEXPORTTAB DEFINITIONS //
/////////////////////////////////

PlotMSExportTab::PlotMSExportTab(PlotMSPlotter* parent) :
        PlotMSPlotSubtab(parent) {
    setupUi(this);
    
    // Setup widgets.
    itsFileWidget_ = new QtFileWidget(false, true);
    QtUtilities::putInFrame(fileFrame, itsFileWidget_);
    
    format->addItem("[by file extension]");
    vector<String> formats = PlotExportFormat::supportedFormatStrings();
    for(unsigned int i = 0; i < formats.size(); i++)
        format->addItem(formats[i].c_str());
    
    // Connect widgets.
    const QMap<PlotMSAction::Type, QAction*>& actMap=  parent->plotActionMap();
    connect(exportButton, SIGNAL(clicked()),
            actMap[PlotMSAction::PLOT_EXPORT], SLOT(trigger()));
}

PlotMSExportTab::~PlotMSExportTab() { }


PlotExportFormat PlotMSExportTab::currentlySetExportFormat() const {
    String file = itsFileWidget_->getFile();
    PlotExportFormat::Type t = (format->currentIndex() == 0) ?
            PlotExportFormat::typeForExtension(file) :
            PlotExportFormat::exportFormat(
                format->currentText().toStdString());

    PlotExportFormat format(t, file);
    format.resolution = highRes->isChecked() ? PlotExportFormat::HIGH :
                                               PlotExportFormat::SCREEN;
    format.dpi = (dpi->isVisible() && dpi->isChecked()) ?
                 dpiSpinner->value() : -1;
    if(ExportTab::size->isVisible() && ExportTab::size->isChecked()) {
        format.width  = sizeSpinner1->value();
        format.height = sizeSpinner2->value();
    } else {
        format.width  = -1;
        format.height = -1;
    }
    
    return format;
}

}
