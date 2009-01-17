//# PlotMSOptionsDock.cc: Widget for options dock.
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
#include <msvis/PlotMS/PlotMSOptionsDock.qo.h>

#include <msvis/PlotMS/PlotMS.h>

namespace casa {

PlotMSOptionsDock::PlotMSOptionsDock() {
    setupUi(this);
    itsParent_ = NULL;
    
    // MS selection
    // first item is whether to apply it or not
    QTreeWidgetItem* twi = new QTreeWidgetItem(PPMSSelWidget);
    twi->setText(0, "Apply");
    PPMSSelWidget->setItemWidget(twi, 1, itsApplySelection_ = new QCheckBox());
    // followed by fields
    vector<PlotMSSelection::Field> fields = PlotMSSelection::fields();
    QTreeWidgetItem* wi; QWidget* vw;
    for(unsigned int i = 0; i < fields.size(); i++) {
        wi = new QTreeWidgetItem(twi);
        wi->setText(0, PlotMSSelection::field(fields[i]).c_str());
        
        // TODO
        switch(PlotMSSelection::fieldType(fields[i])) {
        case PlotMSSelection::TInt:
            vw = new QSpinBox(); break;
        
        default:
            vw = new QLineEdit(); break;
        }
        
        PPMSSelWidget->setItemWidget(wi, 1, vw);
        itsSelectionValues_.insert(fields[i], vw);
    }
    
    // axes
    const vector<String>& axes = PMS::axesStrings();
    String xs = PMS::axis(PlotMSParameters::DEFAULT_XAXIS),
           ys = PMS::axis(PlotMSParameters::DEFAULT_YAXIS);
    unsigned int xdef = axes.size(), ydef = axes.size();
    for(unsigned int i = 0; i < axes.size(); i++) {
        PPxAxisChooser->addItem(axes[i].c_str());
        PPyAxisChooser->addItem(axes[i].c_str());
        if(axes[i] == xs) xdef = i;
        if(axes[i] == ys) ydef = i;
    }
    if(xdef < axes.size()) PPxAxisChooser->setCurrentIndex(xdef);
    if(ydef < axes.size()) PPyAxisChooser->setCurrentIndex(ydef);
    
    // data
    const vector<String>& data = PMS::dataColumnStrings();
    xs = PMS::dataColumn(PlotMSParameters::DEFAULT_DATACOLUMN);
    xdef = data.size();
    for(unsigned int i = 0; i < data.size(); i++) {
        PPxDataChooser->addItem(data[i].c_str());
        PPyDataChooser->addItem(data[i].c_str());
        if(data[i] == xs) xdef = i;
    }
    if(xdef < data.size()) {
        PPxDataChooser->setCurrentIndex(xdef);
        PPyDataChooser->setCurrentIndex(ydef);
    }
    
    // legend position
    vector<String> leg = PlotCanvas::allLegendPositionStrings();
    for(unsigned int i = 0; i < leg.size(); i++)
        COlegendChooser->addItem(leg[i].c_str());
    
    QWidget* w[] = {WOrows, WOcols, WOspacing, WOspacing, PPMSEdit,
                    PPMSSelWidget, PPMSBrowseButton, PPxAxisChooser,
                    PPxDataChooser, PPyAxisChooser, PPyDataChooser, COxAxis,
                    COxGridMajor, COxGridMinor, COyAxis, COyGridMajor,
                    COyGridMinor, COlegendBox, COlegendChooser, COtitleEdit,
                    itsApplyButton};
    itsEditingWidgets_.resize(sizeof(w)/sizeof(w[0]));
    for(unsigned int i = 0; i < itsEditingWidgets_.size(); i++)
        itsEditingWidgets_[i] = w[i];
    
    itsChangeFlag_ = false;
    connect(WOrows, SIGNAL(valueChanged(int)), SLOT(optionsChanged()));
    connect(WOcols, SIGNAL(valueChanged(int)), SLOT(optionsChanged()));
    connect(WOspacing, SIGNAL(valueChanged(int)), SLOT(optionsChanged()));
    connect(PPMSEdit, SIGNAL(textChanged(const QString&)),
            SLOT(optionsChanged()));
    connect(PPxAxisChooser, SIGNAL(currentIndexChanged(int)),
            SLOT(optionsChanged()));
    connect(PPxDataChooser, SIGNAL(currentIndexChanged(int)),
            SLOT(optionsChanged()));
    connect(PPyAxisChooser, SIGNAL(currentIndexChanged(int)),
            SLOT(optionsChanged()));
    connect(PPyDataChooser, SIGNAL(currentIndexChanged(int)),
            SLOT(optionsChanged()));
    connect(COxAxis, SIGNAL(toggled(bool)), SLOT(optionsChanged()));
    connect(COxGridMajor, SIGNAL(toggled(bool)), SLOT(optionsChanged()));
    connect(COxGridMinor, SIGNAL(toggled(bool)), SLOT(optionsChanged()));
    connect(COyAxis, SIGNAL(toggled(bool)), SLOT(optionsChanged()));
    connect(COyGridMajor, SIGNAL(toggled(bool)), SLOT(optionsChanged()));
    connect(COyGridMinor, SIGNAL(toggled(bool)), SLOT(optionsChanged()));
    connect(COlegendBox, SIGNAL(toggled(bool)), SLOT(optionsChanged()));
    connect(COlegendChooser, SIGNAL(currentIndexChanged(int)),
            SLOT(optionsChanged()));
    connect(COtitleEdit, SIGNAL(textChanged(const QString&)),
            SLOT(optionsChanged()));
    
    connect(PProw, SIGNAL(valueChanged(int)), SLOT(rowColChanged()));
    connect(PPcol, SIGNAL(valueChanged(int)), SLOT(rowColChanged()));
    connect(PPMSBrowseButton, SIGNAL(clicked()), SLOT(msBrowse()));
    connect(itsApplyButton, SIGNAL(clicked()), SLOT(apply()));
    itsChangeFlag_ = true;
}

PlotMSOptionsDock::~PlotMSOptionsDock() { }

QDockWidget* PlotMSOptionsDock::getDockWidget() {
    if(itsParent_ == NULL) {
        itsParent_ = new QDockWidget();
        itsParent_->setWindowTitle("PlotMS Options");
        itsParent_->setWidget(this);
        itsParent_->resize(size());
    }
    return itsParent_;
}

void PlotMSOptionsDock::setOptions(const PlotMSOptions& opts) {
    if(opts.nrows() == 0 || opts.ncols() == 0) return;
    
    itsOptions_ = opts;
    bool oldChange = itsChangeFlag_;
    itsChangeFlag_ = false;
    
    unsigned int rows = opts.nrows(), cols = opts.ncols();
    WOrows->setValue(rows);
    WOcols->setValue(cols);
    WOspacing->setValue(opts.spacing());
    
    PPframe->setVisible(rows > 1 || cols > 1);
    COframe->setVisible(rows > 1 || cols > 1);
    PProw->setMaximum(rows - 1); COrow->setMaximum(rows - 1);
    PPcol->setMaximum(cols - 1); COcol->setMaximum(cols - 1);
    rowColChanged();
    
    itsChangeFlag_ = oldChange;
    optionsChanged();
}

void PlotMSOptionsDock::setEditable(bool editable) {
    for(unsigned int i = 0; i < itsEditingWidgets_.size(); i++)
        itsEditingWidgets_[i]->setEnabled(editable);
}


PlotMSOptions PlotMSOptionsDock::currentOptions() const {
    PlotMSOptions opts = itsOptions_;
    opts.setSize((unsigned int)WOrows->value(),
                 (unsigned int)WOcols->value());
    opts.setSpacing((unsigned int)WOspacing->value());
    unsigned int row = (unsigned int)PProw->value(),
                 col = (unsigned int)PPcol->value();
    
    String filename = PPMSEdit->text().toStdString();
    PMS::Axis x = PMS::axis(PPxAxisChooser->currentText().toStdString()),
              y = PMS::axis(PPyAxisChooser->currentText().toStdString());
    PMS::DataColumn xd = PMS::dataColumn(
                         PPxDataChooser->currentText().toStdString()),
                    yd = PMS::dataColumn(
                         PPyDataChooser->currentText().toStdString());
    bool isSet = !filename.empty();
    PlotMSParameters params(filename, x, y, isSet);
    params.setDataColumns(xd, yd);
    params.setShowAxes(COxAxis->isChecked(), COyAxis->isChecked());
    params.setGrid(COxGridMajor->isChecked(), COxGridMinor->isChecked(),
                   COyGridMajor->isChecked(), COyGridMinor->isChecked());
    params.setShowLegend(COlegendBox->isChecked());
    params.setLegendPosition(PlotCanvas::legendPosition(
                             COlegendChooser->currentText().toStdString()));
    params.setCanvasTitle(COtitleEdit->text().toStdString());
    opts.setParameters(row, col, params);
    
    return opts;
}

QString PlotMSOptionsDock::changedText(QString& t, bool changed) {
    if(changed) t = "<font color=\"red\">"+ t.replace(' ',"&nbsp;")+ "</font>";
    return t;
}


void PlotMSOptionsDock::optionsChanged() {
    if(!itsChangeFlag_) return;
    
    PlotMSOptions opts = currentOptions();
    QString changedStart = "<font color=\"red\">",
            changedClose = "</font>";
    
    WOrowsLabel->setText(changedText(opts.nrows() > 1 ? "rows" : "row",
                         opts.nrows() != itsOptions_.nrows()) + " x");
    WOcolsLabel->setText(changedText(opts.ncols() > 1 ? "cols" : "col",
                         opts.ncols() != itsOptions_.ncols()));
    WOspacingLabel->setText(changedText("Grid spacing:",
                            opts.spacing() != itsOptions_.spacing()));    
    WOspacingFrame->setVisible(opts.nrows() > 1 || opts.ncols() > 1);
    
    unsigned int row = (unsigned int)PProw->value(),
                 col = (unsigned int)PPcol->value();
    const PlotMSParameters& oldParams = itsOptions_.parameters(row, col);
    const PlotMSParameters& newParams = opts.parameters(row, col);

    PPMSLabel->setText(changedText("MS:", newParams.getFilename() !=
                                          oldParams.getFilename()));
    
    PPxAxisLabel->setText(changedText("X Axis:", newParams.getXAxis() !=
                          oldParams.getXAxis() && oldParams.isSet()));    
    PPxDataLabel->setText(changedText("   Data:", newParams.getXDataColumn() !=
                          oldParams.getXDataColumn() && oldParams.isSet()));
    bool show = PMS::axisIsData(newParams.getXAxis());
    PPxDataChooser->setVisible(show); PPxDataLabel->setVisible(show);
    
    PPyAxisLabel->setText(changedText("Y Axis:", newParams.getYAxis() !=
                          oldParams.getYAxis() && oldParams.isSet()));    
    PPyDataLabel->setText(changedText("   Data:", newParams.getYDataColumn() !=
                          oldParams.getYDataColumn() && oldParams.isSet()));
    
    COxAxisLabel->setText(changedText("X Axis  ", newParams.showXAxis() !=
                                                oldParams.showXAxis()));
    COxGridLabel->setText(changedText("   Grid:", newParams.getGridXMajor() !=
                          oldParams.getGridXMajor()|| newParams.getGridXMinor()
                          != oldParams.getGridXMinor()));
    show = PMS::axisIsData(newParams.getYAxis());
    PPyDataChooser->setVisible(show); PPyDataLabel->setVisible(show);
    
    COyAxisLabel->setText(changedText("Y Axis  ", newParams.showYAxis() !=
                                                oldParams.showYAxis()));
    COyGridLabel->setText(changedText("   Grid:", newParams.getGridYMajor() !=
                          oldParams.getGridYMajor()|| newParams.getGridYMinor()
                          != oldParams.getGridYMinor()));

    COlegendLabel->setText(changedText("Legend", newParams.showLegend() !=
        oldParams.showLegend() || (newParams.showLegend() &&
        newParams.getLegendPosition() != oldParams.getLegendPosition())));
    
    COtitleLabel->setText(changedText("Title:", newParams.getCanvasTitle() !=
                                                oldParams.getCanvasTitle()));
}

void PlotMSOptionsDock::rowColChanged() {
    unsigned int row = (unsigned int)PProw->value(),
                 col = (unsigned int)PPcol->value();
    
    bool oldChange = itsChangeFlag_;
    itsChangeFlag_ = false;
    
    const PlotMSParameters& params = itsOptions_.parameters(row, col);
    PPMSEdit->setText(params.isSet() ? params.getFilename().c_str() : "");
    
    QString axis = PMS::axis(params.getXAxis()).c_str();
    for(int i = 0; i < PPxAxisChooser->count(); i++) {
        if(PPxAxisChooser->itemText(i) == axis) {
            PPxAxisChooser->setCurrentIndex(i);
            break;
        }
    }
    QString data = PMS::dataColumn(params.getXDataColumn()).c_str();
    for(int i = 0; i < PPxDataChooser->count(); i++) {
        if(PPxDataChooser->itemText(i) == data) {
            PPxDataChooser->setCurrentIndex(i);
            break;
        }
    }
    
    axis = PMS::axis(params.getYAxis()).c_str();
    for(int i = 0; i < PPyAxisChooser->count(); i++) {
        if(PPyAxisChooser->itemText(i) == axis) {
            PPyAxisChooser->setCurrentIndex(i);
            break;
        }
    }
    data = PMS::dataColumn(params.getYDataColumn()).c_str();
    for(int i = 0; i < PPyDataChooser->count(); i++) {
        if(PPyDataChooser->itemText(i) == data) {
            PPyDataChooser->setCurrentIndex(i);
            break;
        }
    }
    
    COxAxis->setChecked(params.showXAxis());
    COyAxis->setChecked(params.showYAxis());
    
    COxGridMajor->setChecked(params.getGridXMajor());
    COxGridMinor->setChecked(params.getGridXMinor());
    COyGridMajor->setChecked(params.getGridYMajor());
    COyGridMinor->setChecked(params.getGridYMinor());
    
    COlegendBox->setChecked(params.showLegend());
    QString l = PlotCanvas::legendPosition(params.getLegendPosition()).c_str();
    for(int i = 0; i < COlegendChooser->count(); i++) {
        if(COlegendChooser->itemText(i) == l) {
            COlegendChooser->setCurrentIndex(i);
            break;
        }
    }
    
    COtitleEdit->setText(params.getCanvasTitle().c_str());
    itsChangeFlag_ = oldChange;
}

void PlotMSOptionsDock::msBrowse() {
    QString file = QFileDialog::getExistingDirectory (this, "Open MS...");
    if(!file.isEmpty()) PPMSEdit->setText(file);
}

void PlotMSOptionsDock::apply() {
    PlotMSOptions opts = currentOptions();        
    if(opts != itsOptions_) emit optionsSet(opts);
}

}
