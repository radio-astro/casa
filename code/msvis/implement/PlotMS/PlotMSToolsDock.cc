//# PlotMSToolsDock.cc: Widget for tools dock.
//# Copyright (C) 2008
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
//#
//# $Id:  $
#include <msvis/PlotMS/PlotMSToolsDock.qo.h>

#include <QFileDialog>

namespace casa {

/////////////////////////////////
// PLOTMSTOOLSDOCK DEFINITIONS //
/////////////////////////////////

// TODO PlotMSToolsDock: export pagination

PlotMSToolsDock::PlotMSToolsDock() {
    setupUi(this);
    itsDock_ = NULL;
    
    // formats
    ETformatChooser->addItem("[by file extension]");
    vector<String> formats = PlotExportFormat::supportedFormatStrings();
    for(unsigned int i = 0; i < formats.size(); i++)
        ETformatChooser->addItem(formats[i].c_str());
    
    // log levels
    vector<String> levels = PlotMSLogger::levelStrings();
    for(unsigned int i = 0; i < levels.size(); i++)
        LTlevel->addItem(levels[i].c_str());
    
    connect(MTradSelect, SIGNAL(toggled(bool)), SLOT(mouseTool(bool)));
    connect(MTradZoom, SIGNAL(toggled(bool)), SLOT(mouseTool(bool)));
    connect(MTradPan, SIGNAL(toggled(bool)), SLOT(mouseTool(bool)));
    connect(MTradNone, SIGNAL(toggled(bool)), SLOT(mouseTool(bool)));
    connect(MTtrackerHover, SIGNAL(toggled(bool)), SLOT(tracker()));
    connect(MTtrackerDisplay, SIGNAL(toggled(bool)), SLOT(tracker()));
    connect(ETfileBrowse, SIGNAL(clicked()), SLOT(exportBrowse()));
    connect(ETbutton, SIGNAL(clicked()), SLOT(exportClicked()));
    connect(LTlevel, SIGNAL(currentIndexChanged(const QString&)),
            SLOT(logLevel(const QString&)));
}

PlotMSToolsDock::~PlotMSToolsDock() { }


QDockWidget* PlotMSToolsDock::getDockWidget() {
    if(itsDock_ == NULL) {
        itsDock_ = new QDockWidget();
        itsDock_->setWindowTitle("PlotMS Tools");
        itsDock_->setWidget(this);
        itsDock_->resize(size());
    }
    return itsDock_;
}

void PlotMSToolsDock::setOptions(const PlotMSOptions& opts) {
    unsigned int rows = opts.nrows(), cols = opts.ncols();
    ETcanvasFrame->setVisible(rows > 1 || cols > 1);
    ETcanvasRow->setMaximum(rows - 1);
    ETcanvasCol->setMaximum(cols - 1);
}

void PlotMSToolsDock::setTrackerText(const String& text) {
    MTtrackerEdit->setPlainText(text.c_str());
}

void PlotMSToolsDock::setLogLevel(PlotMSLogger::Level level) {
    QString str(PlotMSLogger::level(level).c_str());
    for(int i = 0; i < LTlevel->count(); i++) {
        if(LTlevel->itemText(i) == str) {
            LTlevel->setCurrentIndex(i);
            break;
        }
    }
}


void PlotMSToolsDock::notifyTrackerChanged(PlotTrackerTool& tool) {
    if(MTtrackerDisplay->isChecked())
        MTtrackerEdit->setPlainText(tool.getAnnotation()->text().c_str());
}


void PlotMSToolsDock::mouseTool(bool on) {
    if(!on) return; // avoid duplicates
    PlotStandardMouseToolGroup::Tool t = PlotStandardMouseToolGroup::NONE;
    if(MTradSelect->isChecked())     t = PlotStandardMouseToolGroup::SELECT;
    else if(MTradZoom->isChecked())  t = PlotStandardMouseToolGroup::ZOOM;
    else if(MTradPan->isChecked())   t = PlotStandardMouseToolGroup::PAN;
    emit mouseToolChanged(t);
}

void PlotMSToolsDock::tracker() {
    emit trackerTurned(MTtrackerHover->isChecked(),
                       MTtrackerDisplay->isChecked());
}

void PlotMSToolsDock::exportBrowse() {
    QString file = QFileDialog::getSaveFileName(this, "Export File...");
    if(!file.isEmpty()) ETfileEdit->setText(file);
}

void PlotMSToolsDock::exportClicked() {
    QString qfile = ETfileEdit->text();
    String file =   qfile.toStdString();
    bool ok;
    PlotExportFormat::Type t=PlotExportFormat::exportFormat(
                             ETformatChooser->currentText().toStdString(),&ok);
    if(ETformatChooser->currentIndex() != 0 && !ok) {
        emit showError("Could not parse export format!  (Shouldn't happen.)",
                       "Export Error", false);
        return;
    }
    if(ETformatChooser->currentIndex() == 0) {
        t = PlotExportFormat::typeForExtension(file, &ok);
        if(!ok) {
            emit showError("Invalid format extension for filename!",
                            "Export Error", false);
            return;
        }
    }
    PlotExportFormat format(t, file);
    format.resolution = EThighResBox->isChecked()? PlotExportFormat::HIGH :
                                                   PlotExportFormat::SCREEN;
    format.dpi = ETdpiBox->isVisible() && ETdpiBox->isChecked() ?
                 ETdpiSpinner->value() : -1;
    if(ETsizeBox->isVisible() && ETsizeBox->isChecked()) {
        format.width  = ETsizeSpinner1->value();
        format.height = ETsizeSpinner2->value();
    } else {
        format.width  = -1;
        format.height = -1;
    }
    
    int row = -1, col = -1;
    if(ETcanvasFrame->isVisible() && ETcanvasSingle->isChecked()) {
        row = ETcanvasRow->value();
        col = ETcanvasCol->value();
    }
    
    emit exportRequested(format, row, col);
}

void PlotMSToolsDock::logLevel(const QString& text) {
    bool ok;
    PlotMSLogger::Level level = PlotMSLogger::level(text.toStdString(), &ok);
    if(ok) emit logLevelChanged(level);
}

}
