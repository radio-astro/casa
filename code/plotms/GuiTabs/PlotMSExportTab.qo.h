//# PlotMSExportTab.qo.h: Plot tab for managing export.
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
#ifndef PLOTMSEXPORTTAB_QO_H_
#define PLOTMSEXPORTTAB_QO_H_

#include <plotms/GuiTabs/PlotMSExportTab.ui.h>
#include <plotms/PlotMS/PlotMSExportParam.h>
#include <graphics/GenericPlotter/PlotOptions.h>

namespace casa {

//# Forward declarations
class QtFileWidget;


// Subclass of PlotMSPlotSubtab to manage exporting plots.
class PlotMSExportTab : public QDialog {
    Q_OBJECT
    
public:
    // Constructor which takes the parent tab and plotter.
    PlotMSExportTab(QWidget* parent = NULL);
    
    // Destructor.
    virtual ~PlotMSExportTab();
    
    // See PlotMSTab::currentlySetExportFormat().
    PlotExportFormat currentlySetExportFormat() const;
    void setExportFormat(PlotExportFormat format);
    
    //Returns the export range (All, Current, etc).
    PlotMSExportParam getExportParams() const;

signals:
	void exportRangeChanged();

private slots:
	void closeDialog();
	void doExport();

private:
    // Widget for file selection.
    QtFileWidget* itsFileWidget_;
    Ui::ExportTab ui;
};

}

#endif /* PLOTMSEXPORTTAB_QO_H_ */
